#include "reassembler.hh"

#include <cstdint>          // 引入 uint64_t 类型
#include <iterator>         // 用于迭代器操作
#include <utility>          // 用于 std::move

using namespace std;

// 辅助函数：根据给定的位置拆分缓存，返回插入位置的迭代器
auto Reassembler::split( uint64_t pos ) -> mIterator
{
  auto it = buf_.lower_bound( pos );  // 查找第一个大于等于 pos 的索引
  if ( it != buf_.end() and it->first == pos ) {
    return it;  // 如果找到匹配的索引，直接返回该位置
  }
  if ( it == buf_.begin() ) { // 如果缓冲区为空，返回当前位置
    return it;
  }
  const auto pit = prev( it );  // 获取当前位置前一个元素的迭代器
  if ( pit->first + pit->second.size() <= pos ) {
    return it;  // 如果前一个元素的末尾小于 pos，直接返回当前位置
  }
  auto str = pit->second;  // 复制前一个元素的子串
  pit->second.resize( pos - pit->first );  // 调整前一个元素的子串大小
  str.erase( 0, pos - pit->first );  // 截取从 pos 开始的部分
  return buf_.emplace( pos, move( str ) ).first;  // 将剩余部分插入缓存并返回插入位置
}

// 插入新的子串数据，并根据索引重组字节流
void Reassembler::insert(uint64_t first_index, string data, bool is_last_substring)
{
  if ( data.empty() ) {  // 如果传入的子串为空
    if ( is_last_substring and ( not end_index_.has_value() ) ) {
      end_index_.emplace( first_index );  // 如果是最后一个子串，设置结束索引
    }
    if ( end_index_.has_value() and end_index_.value() == writer().bytes_pushed() ) {
      output_.writer().close();  // 如果重组完成，关闭输出流
    }
    return;
  }

  // 计算当前可用的字节范围
  const auto unassembled_index = writer().bytes_pushed();  // 当前已推送的字节数
  const auto unacceptable_index = unassembled_index + writer().available_capacity();  // 可接收字节的上限
  if ( unassembled_index >= unacceptable_index ) {
    return; // 如果没有剩余空间，直接返回
  }
  
  // 如果子串索引超出可接受范围，丢弃该子串
  if ( first_index + data.size() <= unassembled_index or first_index >= unacceptable_index ) {
    return; // 超出范围
  }

  if ( first_index + data.size() > unacceptable_index ) { // 如果子串超出可接受范围，截断
    data.resize( unacceptable_index - first_index );
    is_last_substring = false;  // 非最后一个子串
  }

  if ( first_index < unassembled_index ) { // 如果子串在已重组的部分之前，修正起始位置
    data.erase( 0, unassembled_index - first_index );
    first_index = unassembled_index;
  }

  if ( is_last_substring and ( not end_index_.has_value() ) ) {
    end_index_.emplace( first_index + data.size() );  // 设置结束索引
  }

  // 使用 split 拆分子串并移除已处理的部分
  const auto upper { split( first_index + data.size() ) };
  for ( auto it { split( first_index ) }; it != upper; it = buf_.erase( it ) ) {
    total_pending_ -= it->second.size();  // 更新待处理字节数
  }
  total_pending_ += data.size();  // 更新总字节数
  buf_.emplace( first_index, move( data ) );  // 插入新的子串

  // 处理已到达并可写入的字节
  while ( ( not buf_.empty() ) and buf_.begin()->first == writer().bytes_pushed() ) {
    total_pending_ -= buf_.begin()->second.size();
    output_.writer().push( move( buf_.begin()->second ) );  // 写入字节到输出流
    buf_.erase( buf_.begin() );  // 移除已写入的部分
  }
  
  // 如果已处理完最后一个字节，关闭输出流
  if ( end_index_.has_value() and end_index_.value() == writer().bytes_pushed() ) {
    output_.writer().close();
  }
}

// 返回当前内部缓存中存储的字节数，仅用于测试
uint64_t Reassembler::count_bytes_pending() const
{
  return total_pending_;
}
