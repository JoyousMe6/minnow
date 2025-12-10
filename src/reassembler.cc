#include "reassembler.hh"
#include "debug.hh"

#include <iterator>

using namespace std;

void Reassembler::insert(uint64_t first_index, string data, bool is_last_substring)
{
  // 尝试关闭流的内部函数，当所有字节已被写入且结束标志为 true 时关闭流
  const auto try_close = [&]() -> void {
    if (end_check_ && count_bytes_pending() == 0&& current_index>=end_index) {
      output_.writer().close();
    }
  };

  if (is_last_substring){
    end_index = first_index + data.size();
  }
  
  if (data.empty()) {
    // 如果是空字符串且是最后一个子串，确保流关闭并标记为完成
    end_check_ |= is_last_substring;
    if (is_last_substring) {
      try_close();
    }
    return;  // 不做其他处理，直接返回
  }

  // 获取当前字节流写入的字节范围 [L, R)，其中 L 是已推送字节数，R 是可推送容量
  const auto L = writer().bytes_pushed(); // 当前已推送字节的数量
  const auto R = L + writer().available_capacity(); // 当前可推送字节的容量

  if (L >= R) {
    return; // 如果没有可用容量，直接返回
  }

  // 如果子串完全在可推送范围之外，直接返回
  if (first_index + data.size() <= L || first_index >= R) {
    return; // 子串在可接受的范围之外，丢弃该子串
  }

  // 如果子串超出了右边界，裁剪数据使其符合范围
  if (first_index + data.size() > R) { // 剪裁不可接受的字节
    data = data.substr(0, R - first_index);
    is_last_substring = false; // 如果超出范围，标记为非最后子串
  }

  // 如果子串起始索引小于已推送字节的起始位置，移除已经弹出的字节
  if (first_index < L) { // 移除已经推送的字节
    data = data.substr(L - first_index);
    first_index = L;
  }

  end_check_ |= is_last_substring; // 更新流的结束标记

  // 分割并处理缓存中的子串，确保子串不重叠
  const auto split = [&](const uint64_t& pos) {
    auto it = buf_.lower_bound(pos);
    if (it != buf_.end() && it->first == pos)
      return it; // 如果位置已存在，直接返回
    if (it == buf_.begin())
      return it; // 如果是第一个元素，直接返回
    const auto pit = prev(it);
    if (pit->first + pit->second.size() <= pos)
      return it; // 如果前一个元素已不重叠，返回当前位置
    const auto str = pit->second;
    pit->second = str.substr(0, pos - pit->first); // 剪裁重叠部分
    return buf_.emplace(pos, str.substr(pos - pit->first)).first; // 插入新的子串
  };

  // 清理已经存在并重叠的子串
  if (!buf_.empty()) {
    auto it = split(first_index); // 处理当前子串
    const auto& pr = split(first_index + data.size()); // 处理当前子串的结束位置
    while (it != pr) {
      total_pending_ -= it->second.size(); // 从总待处理字节数中减去
      it = buf_.erase(it); // 删除重叠的子串
    }
  }

  // 只插入一次
  auto [iter, inserted] = buf_.emplace(first_index, data);
  if (inserted) {
    total_pending_ += data.size(); // 如果插入成功，增加待处理字节数
  }

  // 从缓冲区中弹出已重组的数据并推送到输出流
  while (!buf_.empty() && buf_.begin()->first == writer().bytes_pushed()) {
    output_.writer().push(buf_.begin()->second); // 将已重组的字节推送到输出流
    total_pending_ -= buf_.begin()->second.size(); // 减少待处理字节数
    current_index=buf_.begin()->first + buf_.begin()->second.size();
    buf_.erase(buf_.begin()); // 从缓冲区中删除已处理的字节
  }

  return try_close(); // 尝试关闭流
}

// 返回 Reassembler 内部缓冲区存储的字节数。
// 此函数仅用于测试；不要为此功能添加额外的状态。
uint64_t Reassembler::count_bytes_pending() const
{
  return total_pending_;
}
