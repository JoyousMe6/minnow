#pragma once

#include <cstdint>

/*
 * Wrap32 类型表示一个 32 位无符号整数，它：
 *    - 从一个任意的“零点”开始（初始值），并且
 *    - 当它达到 2^32 - 1 时会回绕到零。
 */

class Wrap32
{
public:
  explicit Wrap32( uint32_t raw_value ) : raw_value_( raw_value ) {}

  /* 给定一个绝对序列号 n 和零点，构造一个 Wrap32 实例。 */
  static Wrap32 wrap( uint64_t n, Wrap32 zero_point );

  /*
   * unwrap 方法返回一个绝对序列号，该序列号会回绕到此 Wrap32，给定零点
   * 和一个“检查点”：另一个接近目标答案的绝对序列号。
   *
   * 有许多可能的绝对序列号，它们都回绕到相同的 Wrap32。
   * unwrap 方法应该返回与检查点最接近的那个绝对序列号。
   */
  uint64_t unwrap( Wrap32 zero_point, uint64_t checkpoint ) const;

  Wrap32 operator+( uint32_t n ) const { return Wrap32 { raw_value_ + n }; }
  bool operator==( const Wrap32& other ) const { return raw_value_ == other.raw_value_; }

protected:
  uint32_t raw_value_ {};
  static constexpr uint64_t MASK_LOW_32 { 0x0000'0000'FFFF'FFFF };
  static constexpr uint64_t MASK_HIGH_32 { 0xFFFF'FFFF'0000'0000 };
  static constexpr uint64_t BASE { MASK_LOW_32 + 1 };
};
