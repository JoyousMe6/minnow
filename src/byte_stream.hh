#pragma once

#include <cstdint>
#include <string>
#include <string_view>

class Reader;
class Writer;

class ByteStream
{
public:
  explicit ByteStream( uint64_t capacity );

  // 辅助函数（已提供），用于访问 ByteStream 的 Reader 和 Writer 接口
  Reader& reader();
  const Reader& reader() const;
  Writer& writer();
  const Writer& writer() const;

  void set_error() { error_ = true; };       // 表明流发生了错误。
  bool has_error() const { return error_; }; // 流是否发生过错误？

protected:
  // 请将所有额外的状态添加到 ByteStream 这里，而不是添加到 Writer 和 Reader 接口中。
  std::string buf_ {};
  uint64_t total_popped_ {};
  uint64_t total_pushed_ {};
  bool closed_ {};
  uint64_t capacity_;
  bool error_ {};
};

class Writer : public ByteStream
{
public:
  // 将数据推入流中，但只能推入可用容量允许的量。
  void push( std::string data );
  void close(); // 表明流已到达末尾。不会再有任何数据被写入。

  bool is_closed() const;          // 流是否已被关闭？
  uint64_t available_capacity() const; // 当前可以推入流中的字节数是多少？
  uint64_t bytes_pushed() const;       // 累计推入流中的总字节数
};

class Reader : public ByteStream
{
public:
  // 查看（窥视）缓冲区中的下一组字节 — 理想情况下是尽可能多的字节。
  std::string_view peek() const;
  void pop( uint64_t len ); // 从缓冲区中移除 `len` 个字节。

  bool is_finished() const;      // 流是否已完成（已关闭且已完全弹出）？
  uint64_t bytes_buffered() const; // 当前缓冲（已推入但尚未弹出）的字节数
  uint64_t bytes_popped() const;   // 累计从流中弹出的总字节数
};

/*
 * read: 一个（已提供的）辅助函数，它从 ByteStream Reader 中查看并弹出（peek and pop）
 * 最多 `max_len` 个字节到（输出）字符串中。
 */
void read( Reader& reader, uint64_t max_len, std::string& out );