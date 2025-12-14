#pragma once

#include "wrapping_integers.hh"

#include <string>

/*
 * TCPSenderMessage 结构体包含 TCP 发送方发给接收方的信息。
 *
 * 它包含五个字段：
 *
 * 1) 段起始位置的序列号（seqno）。如果设置了 SYN 标志位，那么这里是 SYN 标志位的序列号。
 *    否则，这里是负载（payload）起始位置的序列号。
 *
 * 2) SYN 标志位。若设置，则该段是字节流的开始，seqno 字段包含初始序列号（ISN）——零点。
 *
 * 3) 负载（payload）：字节流的一个子串（可能为空）。
 *
 * 4) FIN 标志位。若设置，则该负载表示字节流的结束。
 *
 * 5) RST（复位）标志位。若设置，则字节流发生了错误，连接应当被中止。
 */

struct TCPSenderMessage
{
  Wrap32 seqno { 0 };

  bool SYN {};
  std::string payload {};
  bool FIN {};

  bool RST {};

  // 该段使用了多少个序列号？
  size_t sequence_length() const { return SYN + payload.size() + FIN; }
};
