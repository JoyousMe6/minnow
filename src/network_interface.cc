#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address 接口的以太网（ARP称之为“硬件”）地址
//! \param[in] ip_address 接口的IP（ARP称之为“协议”）地址
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram 要发送的IPv4数据报
//! \param[in] next_hop 要发送数据报的接口的IP地址（通常是路由器或默认网关，但如果与目标直接连接，也可以是另一台主机）
//! 注意：Address 类型可以通过使用 Address::ipv4_numeric() 方法转换为 uint32_t（原始的32位IP地址）。
void NetworkInterface::send_datagram( InternetDatagram dgram, const Address& next_hop )
{
  debug( "unimplemented send_datagram called" );
  (void)dgram;
  (void)next_hop;
}

//! \param[in] frame 传入的以太网帧
void NetworkInterface::recv_frame( EthernetFrame frame )
{
  debug( "unimplemented recv_frame called" );
  (void)frame;
}

//! \param[in] ms_since_last_tick 自上次调用该方法以来的毫秒数
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  debug( "unimplemented tick({}) called", ms_since_last_tick );
}
