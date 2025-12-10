#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  debug( "unimplemented insert({}, {}, {}) called", first_index, data, is_last_substring );
}

// Reassembler 内部存储了多少字节？
// 此函数仅用于测试；不要添加额外的状态来支持它。
uint64_t Reassembler::count_bytes_pending() const
{
  debug( "unimplemented count_bytes_pending() called" );
  return {};
}
