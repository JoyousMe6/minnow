#pragma once

#include "address.hh"
#include "ethernet_frame.hh"
#include "ipv4_datagram.hh"

#include <memory>
#include <queue>

// 一个“网络接口”，它连接 IP（互联网层，或网络层）与以太网（网络接入层，或链路层）。

// 该模块是 TCP/IP 堆栈的最底层部分
// （连接 IP 与更低层的网络协议，例如以太网）。但这个模块也可以反复使用
// 作为路由器的一部分：一个路由器通常有多个网络接口，路由器的工作是路由互联网数据报在不同接口之间。

// 网络接口将数据报（来自“客户”，例如 TCP/IP 堆栈或路由器）转换成以太网帧。
// 为了填充以太网目标地址，它查找下一个 IP 跳跃的以太网地址，并使用地址解析协议（ARP）进行请求。
// 反向方向，网络接口接收以太网帧，检查它们是否是发送给它的，如果是，则根据其类型处理有效载荷。
// 如果是 IPv4 数据报，网络接口会将其推送到堆栈中。如果是 ARP 请求或回复，网络接口会处理该帧并根据需要学习或回复。
class NetworkInterface
{
public:
  // 网络接口的物理输出端口抽象层
  class OutputPort
  {
  public:
    // 传输以太网帧
    virtual void transmit( const NetworkInterface& sender, const EthernetFrame& frame ) = 0;
    virtual ~OutputPort() = default;
  };

  // 构造一个具有给定以太网（网络接入层）和 IP（互联网层）地址的网络接口
  NetworkInterface( std::string_view name,
                    std::shared_ptr<OutputPort> port,
                    const EthernetAddress& ethernet_address,
                    const Address& ip_address );

  // 发送一个互联网数据报，封装在以太网帧中（如果知道以太网目标地址）。  
  // 如果目标以太网地址未知，则通过 ARP 查找目标地址。
  void send_datagram( InternetDatagram dgram, const Address& next_hop );

  // 接收一个以太网帧并适当地响应。
  // 如果类型是 IPv4，推送数据报到 datagrams_received 队列。
  // 如果类型是 ARP 请求，则学习发送方的映射，并发送 ARP 回复。
  // 如果类型是 ARP 回复，则学习发送方的映射。
  void recv_frame( EthernetFrame frame );

  // 随着时间推移周期性地调用，用于过期已失效的 IP 到以太网的映射
  void tick( size_t ms_since_last_tick );

  // 访问器
  const std::string& name() const { return name_; }
  const OutputPort& output() const { return *port_; }
  OutputPort& output() { return *port_; }
  std::queue<InternetDatagram>& datagrams_received() { return datagrams_received_; }

private:
  // 接口的可读名称
  std::string name_;

  // 输出端口的共享指针 (+ 一个帮助函数 `transmit`，用于通过该端口发送以太网帧)
  std::shared_ptr<OutputPort> port_;
  void transmit( const EthernetFrame& frame ) const { port_->transmit( *this, frame ); }

  // 接口的以太网（硬件，网络接入层，或链路层）地址
  EthernetAddress ethernet_address_;

  // 接口的 IP（互联网层或网络层）地址
  Address ip_address_;

  // 接收到的数据报队列
  std::queue<InternetDatagram> datagrams_received_ {};
};
