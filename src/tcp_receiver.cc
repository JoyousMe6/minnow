#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // 你的代码在这里实现。
  debug( "unimplemented receive() called" );
  (void)message;
}

TCPReceiverMessage TCPReceiver::send() const
{
  // 你的代码在这里实现。
  debug( "unimplemented send() called" );
  return {};
}
