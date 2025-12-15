#pragma once

#include "byte_stream.hh"

#include <cstdint>
#include <map>
#include <optional>

class Reassembler
{
public:
  // 构造 Reassembler 以便写入给定的 ByteStream。
  // 参数 output 是一个可变的 ByteStream 对象，用于存储重组后的字节流。
  explicit Reassembler( ByteStream&& output ) : output_( std::move( output ) ) {}

  /*
   * 插入一个新的子串以重新组装到 ByteStream 中。
   *   `first_index`: 子串第一个字节的索引（在原始字节流中的位置）
   *   `data`: 子串数据
   *   `is_last_substring`: 如果此子串是流的最后一部分，设为 true
   *
   * Reassembler 的任务是将乱序且可能重叠的带索引的子串重新组装回原始字节流。
   * 一旦 Reassembler 知道流中下一个字节，它就应该立即将其写入到 ByteStream 输出流。
   *
   * 如果 Reassembler 知道流中下一个字节，但由于更早的字节尚未到达，无法立即写入，
   * 它将这些字节存储在内部缓冲区中，直到这些“间隙”被填补。
   *
   * Reassembler 应该丢弃任何超出流容量的字节（即使填补了较早的间隙，无法写入的字节也会被丢弃）。
   *
   * 一旦 Reassembler 写入了最后一个字节（即 `is_last_substring` 为真），它应关闭流。
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring );

  // 返回 Reassembler 内部缓冲区中存储的字节数。
  // 这个函数仅用于测试；不要为此功能添加额外的状态。
  uint64_t count_bytes_pending() const;

  // 返回输出流的 Reader 引用，用于读取重组后的字节流。
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // 返回输出流的 Writer 引用，但只读（不能外部写入）。
  const Writer& writer() const { return output_.writer(); }

private:
  ByteStream output_;                          // 用于存储重组后的字节流的 ByteStream
  std::map<uint64_t, std::string> buf_ {};         // 缓存已接收的子串，按索引排序
  uint64_t total_pending_ {};                  // 当前内部存储的字节总数
  std::optional<uint64_t> end_index_ {};

  using mIterator = decltype( buf_ )::iterator;
  mIterator split( uint64_t pos );
};
