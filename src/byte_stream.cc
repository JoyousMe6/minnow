#include "byte_stream.hh"
#include "debug.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

// 将数据推入流中，但只能推入可用容量允许的量。
void Writer::push( string data )
{
  if ( data.size() > available_capacity() ) {
    data = data.substr( 0, available_capacity() );
  }
  buf_ += data;
  total_pushed_ += data.size();
}

// 表明流已到达末尾。不会再有任何数据被写入。
void Writer::close()
{
  closed_ = true;
}

// 流是否已被关闭？
bool Writer::is_closed() const
{
  return closed_;
}

// 当前可以推入流中的字节数是多少？
uint64_t Writer::available_capacity() const
{
  return capacity_ - buf_.size();
}

// 累计推入流中的总字节数
uint64_t Writer::bytes_pushed() const
{
  return total_pushed_;
}

// 查看（窥视）缓冲区中的下一组字节 — 理想情况下是尽可能多的字节。
// 并非必须返回整个缓冲区的 string_view，但如果窥视的 string_view 一次只有一个字节，
// 可能会迫使调用者进行大量的额外工作。
string_view Reader::peek() const
{
  return buf_;
}

// 从缓冲区中移除 `len` 个字节。
void Reader::pop( uint64_t len )
{
  if ( len > bytes_buffered() ) {
    set_error();
    return;
  }

  buf_ = buf_.substr( len );
  total_popped_ += len;
}

// 流是否已完成（已关闭且已完全弹出）？
bool Reader::is_finished() const
{
  return closed_ and buf_.empty();
}

// 当前缓冲（已推入但尚未弹出）的字节数
uint64_t Reader::bytes_buffered() const
{
  return buf_.size();
}

// 累计从流中弹出的总字节数
uint64_t Reader::bytes_popped() const
{
  return total_popped_;
}