#pragma once

#include <cstdint>
#include <queue>
#include <string>
#include <string_view>

class Reader;
class Writer;

class ByteStream
{
public:
  explicit ByteStream( uint64_t capacity );

  // 返回 ByteStream 的 Reader 接口，用于读取数据
  Reader& reader();
  const Reader& reader() const;

  // 返回 ByteStream 的 Writer 接口，用于写入数据
  Writer& writer();
  const Writer& writer() const;

  // 设置流发生错误
  void set_error() { error_ = true; };

  // 检查流是否发生错误
  bool has_error() const { return error_; };

protected:
  std::queue<std::string> stream_ {};
  std::queue<std::string_view> stream_view_ {};

  uint64_t capacity_;         // 字节流的最大容量
  uint64_t total_popped_ {};  // 累计从流中读取的字节数
  uint64_t total_pushed_ {};  // 累计推入流中的字节数
  uint64_t total_buffered_ {};

  bool closed_ {};            // 流是否已关闭
  bool error_ {};             // 流是否发生错误
};

class Writer : public ByteStream
{
public:
  // 将数据推入字节流中，但只能推入可用容量允许的字节数
  void push( std::string data );

  // 关闭流，表示不会再有任何数据被写入
  void close();

  // 检查流是否已关闭
  bool is_closed() const;

  // 获取当前可以推入流中的字节数
  uint64_t available_capacity() const;

  // 获取已推入流中的总字节数
  uint64_t bytes_pushed() const;
};

class Reader : public ByteStream
{
public:
  // 查看（窥视）缓冲区中的下一组字节，理想情况下是尽可能多的字节，返回字符串视图
  std::string_view peek() const;

  // 从缓冲区中移除 `len` 个字节
  void pop( uint64_t len );

  // 检查流是否已完成，流已关闭且所有字节已被读取
  bool is_finished() const;

  // 获取当前缓冲区中尚未被弹出的字节数
  uint64_t bytes_buffered() const;

  // 获取已从流中弹出的总字节数
  uint64_t bytes_popped() const;
};

/*
 * read: 从 ByteStream 的 Reader 中查看并弹出最多 `max_len` 个字节到（输出）字符串中
 */
void read( Reader& reader, uint64_t max_len, std::string& out );