#include "byte_stream.hh"
#include "debug.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

// 将数据推入流中，但只能推入可用容量允许的量。
void Writer::push( string data )
{
  // 您的代码在这里（以及下面每个方法中）
  debug( "Writer::push({}) not yet implemented", data );
}

// 表明流已到达末尾。不会再有任何数据被写入。
void Writer::close()
{
  debug( "Writer::close() not yet implemented" );
}

// 流是否已被关闭？
bool Writer::is_closed() const
{
  debug( "Writer::is_closed() not yet implemented" );
  return {}; // 您的代码在这里。
}

// 当前可以推入流中的字节数是多少？
uint64_t Writer::available_capacity() const
{
  debug( "Writer::available_capacity() not yet implemented" );
  return {}; // 您的代码在这里。
}

// 累计推入流中的总字节数
uint64_t Writer::bytes_pushed() const
{
  debug( "Writer::bytes_pushed() not yet implemented" );
  return {}; // 您的代码在这里。
}

// 查看（窥视）缓冲区中的下一组字节 — 理想情况下是尽可能多的字节。
// 并非必须返回整个缓冲区的 string_view，但如果窥视的 string_view 一次只有一个字节，
// 可能会迫使调用者进行大量的额外工作。
string_view Reader::peek() const
{
  debug( "Reader::peek() not yet implemented" );
  return {}; // 您的代码在这里。
}

// 从缓冲区中移除 `len` 个字节。
void Reader::pop( uint64_t len )
{
  debug( "Reader::pop({}) not yet implemented", len );
}

// 流是否已完成（已关闭且已完全弹出）？
bool Reader::is_finished() const
{
  debug( "Reader::is_finished() not yet implemented" );
  return {}; // 您的代码在这里。
}

// 当前缓冲（已推入但尚未弹出）的字节数
uint64_t Reader::bytes_buffered() const
{
  debug( "Reader::bytes_buffered() not yet implemented" );
  return {}; // 您的代码在这里。
}

// 累计从流中弹出的总字节数
uint64_t Reader::bytes_popped() const
{
  debug( "Reader::bytes_popped() not yet implemented" );
  return {}; // 您的代码在这里。
}