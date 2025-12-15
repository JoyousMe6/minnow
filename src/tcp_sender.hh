#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <functional>

class RetransmissionTimer
{
public:
  explicit RetransmissionTimer( uint64_t initial_RTO_ms ) : RTO_ms_( initial_RTO_ms ) {}

  [[nodiscard]] constexpr auto is_active() const noexcept -> bool { return is_active_; }
  [[nodiscard]] constexpr auto is_expired() const noexcept -> bool { return is_active_ and timer_ >= RTO_ms_; }
  constexpr auto reset() noexcept -> void { timer_ = 0; }
  constexpr auto exponential_backoff() noexcept -> void { RTO_ms_ *= 2; }
  constexpr auto reload( uint64_t initial_RTO_ms ) noexcept -> void { RTO_ms_ = initial_RTO_ms, reset(); };
  constexpr auto start() noexcept -> void { is_active_ = true, reset(); }
  constexpr auto stop() noexcept -> void { is_active_ = false, reset(); }
  constexpr auto tick( uint64_t ms_since_last_tick ) noexcept -> RetransmissionTimer&
  {
    timer_ += is_active_ ? ms_since_last_tick : 0;
    return *this;
  }

private:
  bool is_active_ {};
  uint64_t RTO_ms_;
  uint64_t timer_ {};
};

class TCPSender
{
public:
  /* 使用给定的默认重传超时时间和可能的初始序列号（ISN）构造TCP发送方 */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), initial_RTO_ms_( initial_RTO_ms ), timer_( initial_RTO_ms )
  {}

  /* 生成一个空的 TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* 接收并处理来自对端接收方的 TCPReceiverMessage */
  void receive( const TCPReceiverMessage& msg );

  /* `transmit` 函数的类型，push 和 tick 方法可以用它来发送消息 */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* 从出站字节流中推送字节 */
  void push( const TransmitFunction& transmit );

  /* 自上次调用 tick() 方法以来，已经过去的毫秒数 */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // 访问器
  uint64_t sequence_numbers_in_flight() const;  // 当前有多少个序列号在飞行中？
  uint64_t consecutive_retransmissions() const; // 当前有多少次连续重传？
  const Writer& writer() const { return input_.writer(); }
  const Reader& reader() const { return input_.reader(); }
  Writer& writer() { return input_.writer(); }

private:
  Reader& reader() { return input_.reader(); }

  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  RetransmissionTimer timer_;

  bool SYN_sent_ {};
  bool FIN_sent_ {};

  uint64_t next_abs_seqno_ {};
  uint64_t ack_abs_seqno_ {};
  uint16_t window_size_ { 1 };
  std::queue<TCPSenderMessage> outstanding_message_ {};

  uint64_t total_outstanding_ {};
  uint64_t total_retransmission_ {};
}; 
