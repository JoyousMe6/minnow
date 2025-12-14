#pragma once

#include "wrapping_integers.hh"

#include <optional>

/*
 * TCPReceiverMessage 结构体包含 TCP 接收方发给发送方的信息。
 *
 * 它包含三个字段：
 *
 * 1) 确认号（ackno）：TCP 接收方需要的 *下一个* 序列号。
 *    这是一个可选字段，如果 TCPReceiver 尚未接收到初始序列号，则该字段为空。
 *
 * 2) 窗口大小。该值表示 TCP 接收方希望接收的序列号数量，从 ackno 开始（如果存在）。
 *    最大值为 65,535（来自 <cstdint> 头文件中的 UINT16_MAX）。
 *
 * 3) RST（复位）标志位。若设置，则字节流发生了错误，连接应当被中止。
 */

struct TCPReceiverMessage
{
  std::optional<Wrap32> ackno {};
  uint16_t window_size {};
  bool RST {};
};
