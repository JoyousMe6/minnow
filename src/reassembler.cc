#include "reassembler.hh"
#include "debug.hh"

#include <iterator>

using namespace std;

void Reassembler::insert(uint64_t first_index, string data, bool is_last_substring)
{
  const auto try_close = [&]() -> void {
    if (end_check_ && count_bytes_pending() == 0) {
      output_.writer().close();
    }
  };

  if (data.empty()) {
    // 如果是空字符串且是最后一个子串，确保流关闭并标记为完成
    end_check_ |= is_last_substring;
    if (is_last_substring) {
      try_close();
    }
    return;  // 不做其他处理
  }

  const auto L = writer().bytes_pushed(); // Reassembler's internal storage: [L, R)
  const auto R = L + writer().available_capacity();

  if (L >= R) {
    return; // available_capacity == 0
  }

  // 如果子串超出范围，裁剪数据
  if (first_index + data.size() <= L || first_index >= R) {
    return; // Out of range
  }

  if (first_index + data.size() > R) { // Remove unacceptable bytes
    data = data.substr(0, R - first_index);
    is_last_substring = false;
  }

  if (first_index < L) { // Remove popped/buffered bytes
    data = data.substr(L - first_index);
    first_index = L;
  }

  end_check_ |= is_last_substring;

  const auto split = [&](const uint64_t& pos) {
    auto it = buf_.lower_bound(pos);
    if (it != buf_.end() && it->first == pos)
      return it;
    if (it == buf_.begin())
      return it;
    const auto pit = prev(it);
    if (pit->first + pit->second.size() <= pos)
      return it;
    const auto str = pit->second;
    pit->second = str.substr(0, pos - pit->first);
    return buf_.emplace(pos, str.substr(pos - pit->first)).first;
  };

  if (!buf_.empty()) {
    auto it = split(first_index);
    const auto& pr = split(first_index + data.size());
    while (it != pr) {
      total_pending_ -= it->second.size();
      it = buf_.erase(it);
    }
  }

  // Only insert once
  auto [iter, inserted] = buf_.emplace(first_index, data);
  if (inserted) {
    total_pending_ += data.size();
  }

  while (!buf_.empty() && buf_.begin()->first == writer().bytes_pushed()) {
    output_.writer().push(buf_.begin()->second);
    total_pending_ -= buf_.begin()->second.size();
    buf_.erase(buf_.begin());
  }

  return try_close();
}

// Reassembler 内部存储了多少字节？
// 此函数仅用于测试；不要添加额外的状态来支持它。
uint64_t Reassembler::count_bytes_pending() const
{
  return total_pending_;
}
