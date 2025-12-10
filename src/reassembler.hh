#pragma once

#include "byte_stream.hh"

class Reassembler
{
public:
  // 构造 Reassembler 以写入给定的 ByteStream。
  explicit Reassembler( ByteStream&& output ) : output_( std::move( output ) ) {}

  /*
   * 插入一个新的子串以重新组装到 ByteStream 中。
   *   `first_index`: 子串的第一个字节的索引
   *   `data`: 子串本身
   *   `is_last_substring`: 此子串代表流的结束
   *   `output`: 一个可变的 Writer 引用
   *
   * Reassembler 的工作是将带索引的子串（可能乱序且可能重叠）重新组装回原始的 ByteStream。
   * 一旦 Reassembler 知道流中的下一个字节，就应该立即将其写入输出。
   *
   * 如果 Reassembler 了解适合流可用容量但尚不能写入的字节（因为更早的字节仍然未知），
   * 它应该将这些字节存储在内部，直到间隙被填补。
   *
   * Reassembler 应丢弃任何超出流可用容量的字节（即即使填补了较早的间隙也无法写入的字节）。
   *
   * Reassembler 在写入最后一个字节后应关闭流。
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring );

  // Reassembler 内部存储了多少字节？
  // 此函数仅用于测试；不要添加额外的状态来支持它。
  uint64_t count_bytes_pending() const;

  // 访问输出流 reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // 访问输出流 writer，但只读（不能从外部写入）
  const Writer& writer() const { return output_.writer(); }

private:
  ByteStream output_;
};
