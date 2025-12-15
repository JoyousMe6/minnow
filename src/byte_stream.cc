#include "byte_stream.hh"

#include <cstdint>
#include <string_view>
#include <utility>

using namespace std;

// 构造函数，初始化 ByteStream，设置流的最大容量
ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

// 将数据推入字节流中，但只能推入可用容量允许的字节数
void Writer::push( string data )
{
  if ( Writer::is_closed() or Writer::available_capacity() == 0 or data.empty() ) {
    return;
  }

  if ( data.size() > Writer::available_capacity() ) {
    data.resize( Writer::available_capacity() );
  }
  total_pushed_ += data.size();
  total_buffered_ += data.size();
  
  stream_.emplace( move( data ) );
}

// 标记流已经到达末尾，表示流不再接受任何数据的写入
void Writer::close()
{
  closed_ = true;
}

// 检查流是否已关闭
bool Writer::is_closed() const
{
  return closed_;
}

// 返回当前可用的容量，即流中还可以推入的字节数
uint64_t Writer::available_capacity() const
{
  return capacity_ - total_buffered_;
}

// 返回已推入流中的字节数
uint64_t Writer::bytes_pushed() const
{
  return total_pushed_;
}

// 查看缓冲区中的下一组字节，尽量返回尽可能多的字节
// 返回整个缓冲区的 string_view，但如果只返回一个字节可能会导致调用者做很多额外工作
string_view Reader::peek() const
{
  return stream_.empty() ? string_view {} // std::string_view dependents on the initializer through its lifetime.
                         : string_view { stream_.front() }.substr( removed_prefix_ );
}

// 从缓冲区中移除指定数量的字节
void Reader::pop( uint64_t len )
{
  total_buffered_ -= len;
  total_popped_ += len;
  while ( len != 0U ) {
    const auto size = stream_.front().size() - removed_prefix_;
    if ( len < size ) {
      removed_prefix_ += len;
      break; // with len = 0;
    }
    stream_.pop();
    removed_prefix_ = 0;
    len -= size;
  }
}

// 检查流是否已完成，即流已关闭并且缓冲区已完全弹出
bool Reader::is_finished() const
{
  return closed_ and total_buffered_ == 0;
}

// 返回当前缓冲区中尚未被弹出的字节数
uint64_t Reader::bytes_buffered() const
{
  return total_buffered_;
}

// 返回已从流中弹出的字节数
uint64_t Reader::bytes_popped() const
{
  return total_popped_;
}
