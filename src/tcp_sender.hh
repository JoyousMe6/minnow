#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <functional>

// 重传定时器类，用于控制 TCP 发送方的重传超时
class RetransmissionTimer
{
public:
  // 构造函数，初始化重传超时时间（RTO）
  explicit RetransmissionTimer( uint64_t initial_RTO_ms ) : RTO_ms_( initial_RTO_ms ) {}

  // 返回定时器是否激活
  [[nodiscard]] constexpr auto is_active() const noexcept -> bool { return is_active_; }

  // 判断定时器是否已过期
  // 定时器激活且经过的时间超过了 RTO 时，定时器过期
  [[nodiscard]] constexpr auto is_expired() const noexcept -> bool { return is_active_ and timer_ >= RTO_ms_; }

  // 重置定时器，将已计时的时间清零
  constexpr auto reset() noexcept -> void { timer_ = 0; }

  // 指数回退，重传超时时间加倍
  constexpr auto exponential_backoff() noexcept -> void { RTO_ms_ *= 2; }

  // 重新加载定时器，使用新的 RTO 值并重置计时器
  constexpr auto reload( uint64_t initial_RTO_ms ) noexcept -> void { RTO_ms_ = initial_RTO_ms, reset(); };

  // 启动定时器并重置计时器
  constexpr auto start() noexcept -> void { is_active_ = true, reset(); }

  // 停止定时器并重置计时器
  constexpr auto stop() noexcept -> void { is_active_ = false, reset(); }

  // 根据过去的毫秒数更新定时器
  constexpr auto tick( uint64_t ms_since_last_tick ) noexcept -> RetransmissionTimer&
  {
    timer_ += is_active_ ? ms_since_last_tick : 0;
    return *this;
  }

private:
  bool is_active_ {};  // 定时器是否激活
  uint64_t RTO_ms_;   // 重传超时时间（毫秒）
  uint64_t timer_ {}; // 计时器当前的已过时间
};

// TCP 发送方类，负责将字节流数据转换为 TCP 段并处理重传
class TCPSender
{
public:
  /* 使用给定的默认重传超时时间和可能的初始序列号（ISN）构造 TCP 发送方 */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), initial_RTO_ms_( initial_RTO_ms ), timer_( initial_RTO_ms )
  {}

  /* 生成一个空的 TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  // 用于接收对端发送的确认信息，更新接收方窗口和状态
  void receive( const TCPReceiverMessage& msg );

  /* `transmit` 函数的类型，push 和 tick 方法可以用它来发送消息 */
  // 用于发送 TCPSenderMessage 的函数类型
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* 从出站字节流中推送字节 */
  // 从字节流中读取数据并创建 TCP 段发送给接收方
  void push( const TransmitFunction& transmit );

  /* 自上次调用 tick() 方法以来，已经过去的毫秒数 */
  // 根据时间的流逝更新定时器并执行重传操作
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // 访问器
  uint64_t sequence_numbers_in_flight() const;  // 返回当前在传输中的序列号数量
  uint64_t consecutive_retransmissions() const; // 返回当前连续的重传次数
  const Writer& writer() const { return input_.writer(); }  // 获取字节流的写入器
  const Reader& reader() const { return input_.reader(); }  // 获取字节流的读取器
  Writer& writer() { return input_.writer(); }  // 获取字节流的写入器（可修改）

private:
  Reader& reader() { return input_.reader(); }  // 获取字节流的读取器（可修改）

  ByteStream input_;   // 输入的字节流，包含待发送的数据
  Wrap32 isn_;         // 初始序列号（ISN）
  uint64_t initial_RTO_ms_; // 初始重传超时时间（毫秒）

  RetransmissionTimer timer_;  // 重传定时器，用于管理重传超时

  bool SYN_sent_ {};  // 是否已发送 SYN 标志
  bool FIN_sent_ {};  // 是否已发送 FIN 标志

  uint64_t next_abs_seqno_ {};  // 下一个待发送的绝对序列号
  uint64_t ack_abs_seqno_ {};   // 接收方确认的绝对序列号
  uint16_t window_size_ { 1 };  // 窗口大小，默认1
  std::queue<TCPSenderMessage> outstanding_message_ {};  // 存储未确认的 TCP 发送消息

  uint64_t total_outstanding_ {};  // 当前未确认的字节数
  uint64_t total_retransmission_ {};  // 重传的总次数
}; 
