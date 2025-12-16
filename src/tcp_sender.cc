#include "tcp_sender.hh"
#include "tcp_config.hh"
#include "wrapping_integers.hh"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>

using namespace std;

// 返回当前未确认的字节数，即正在传输的字节数
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return total_outstanding_;
}

// 返回当前发生的连续重传次数
uint64_t TCPSender::consecutive_retransmissions() const
{
  return total_retransmission_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // 只要窗口大小大于当前已发送的字节数，并且数据流没有结束，就继续发送
  while ( ( window_size_ == 0 ? 1 : window_size_ ) > total_outstanding_ ) {
    if ( FIN_sent_ ) {
      break; // 如果已经发送了 FIN，表示数据流结束，停止发送
    }

    // 创建一个空的 TCP 发送消息
    auto msg = make_empty_message();
    
    // 如果尚未发送 SYN 标志，设置 SYN 标志并标记已发送
    if ( not SYN_sent_ ) {
      msg.SYN = true;
      SYN_sent_ = true;
    }

    // 计算剩余窗口大小
    const uint64_t remaining = ( window_size_ == 0 ? 1 : window_size_ ) - total_outstanding_;
    
    // 计算本次要发送的数据长度，确保不超过最大负载大小
    const size_t len = min( TCPConfig::MAX_PAYLOAD_SIZE, remaining - msg.sequence_length() );
    
    // 填充消息的负载部分
    auto& payload { msg.payload };
    while ( reader().bytes_buffered() != 0U and payload.size() < len ) {
      auto view = reader().peek();
      view = view.substr( 0, len - payload.size() );
      payload += view;
      input_.reader().pop( view.size() );
    }

    // 如果尚未发送 FIN 并且数据已完成，设置 FIN 标志
    if ( ( not FIN_sent_ ) and remaining > msg.sequence_length() and reader().is_finished() ) {
      msg.FIN = true;
      FIN_sent_ = true;
    }

    // 如果消息没有有效负载，停止发送
    if ( msg.sequence_length() == 0 ) {
      break;
    }

    // 发送消息
    transmit( msg );

    // 如果定时器没有启动，启动定时器
    if ( not timer_.is_active() ) {
      timer_.start();
    }

    // 更新下一个待发送的序列号
    next_abs_seqno_ += msg.sequence_length();

    // 更新总共发送的字节数
    total_outstanding_ += msg.sequence_length();

    // 将消息添加到未确认消息队列
    outstanding_message_.emplace( move( msg ) );
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // 创建一个空的 TCPSenderMessage，设置其初始序列号
  return { Wrap32::wrap( next_abs_seqno_, isn_ ), false, {}, false, input_.has_error() };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if ( input_.has_error() ) {
    return; // 如果输入流发生错误，直接返回
  }

  // 如果接收到 RST 标志，表示连接重置，设置错误状态
  if ( msg.RST ) {
    input_.set_error();
    return;
  }

  // 更新接收方的窗口大小
  window_size_ = msg.window_size;

  // 如果没有收到有效的确认号，直接返回
  if ( not msg.ackno.has_value() ) {
    return;
  }

  // 解包并获取接收到的绝对序列号
  const uint64_t recv_ack_abs_seqno { msg.ackno->unwrap( isn_, next_abs_seqno_ ) };

  // 如果接收到的确认号大于下一个待发送的序列号，说明确认号无效
  if ( recv_ack_abs_seqno > next_abs_seqno_ ) {
    return;
  }

  bool has_acknowledgment { false };

  // 遍历未确认的消息队列，确认已收到的消息
  while ( not outstanding_message_.empty() ) {
    const auto& message { outstanding_message_.front() };

    // 如果当前消息的序列号范围大于接收到的确认号，停止处理
    if ( ack_abs_seqno_ + message.sequence_length() > recv_ack_abs_seqno ) {
      break; // 确保确认号完全确认了该消息
    }

    has_acknowledgment = true;
    ack_abs_seqno_ += message.sequence_length();  // 更新确认号
    total_outstanding_ -= message.sequence_length(); // 减少未确认的字节数
    outstanding_message_.pop();  // 从队列中移除已确认的消息
  }

  // 如果收到确认消息，重置重传计数器和定时器
  if ( has_acknowledgment ) {
    total_retransmission_ = 0;  // 重传次数重置
    timer_.reload( initial_RTO_ms_ );  // 重载定时器
    outstanding_message_.empty() ? timer_.stop() : timer_.start();  // 如果队列为空，停止定时器，否则继续启动定时器
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // 更新定时器，并检查是否超时
  if ( timer_.tick( ms_since_last_tick ).is_expired() ) {
    if ( outstanding_message_.empty() ) {
      return; // 如果没有未确认的消息，直接返回
    }

    // 重新发送最早的未确认消息
    transmit( outstanding_message_.front() );

    // 如果窗口大小不为零，增加重传次数并执行指数回退
    if ( window_size_ != 0 ) {
      total_retransmission_ += 1;
      timer_.exponential_backoff();
    }

    // 重置定时器
    timer_.reset();
  }
}
