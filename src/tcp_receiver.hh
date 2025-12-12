#pragma once

#include "reassembler.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

class TCPReceiver
{
public:
  // 使用给定的 Reassembler 构造
  explicit TCPReceiver( Reassembler&& reassembler ) : reassembler_( std::move( reassembler ) ) {}

  /*
   * TCPReceiver 接收 TCPSenderMessage，将其有效载荷插入到 Reassembler
   * 中的正确流索引。
   */
  void receive( TCPSenderMessage message );

  // TCPReceiver 向对等方的 TCPSender 发送 TCPReceiverMessage。
  TCPReceiverMessage send() const;

  // 访问输出
  const Reassembler& reassembler() const { return reassembler_; }
  Reader& reader() { return reassembler_.reader(); }
  const Reader& reader() const { return reassembler_.reader(); }
  const Writer& writer() const { return reassembler_.writer(); }

private:
  Reassembler reassembler_;
};
