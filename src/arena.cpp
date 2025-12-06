#include "roc/ds/arena.h"

#include "roc/common.h"
#include "roc/os/os.h"
#include "roc/os/memory.h"

auto Alloc__(const char* file, int line) -> Arena * {
  auto sys_info = Os::GetSysInfo();
  u64 reserve_size = AlignPow2(DEFAULT_RESERVE, sys_info.page_size);
  u64 commit_size = AlignPow2(DEFAULT_COMMIT, sys_info.page_size);

  auto *base = Os::ReserveMemory(reserve_size);
  Os::CommitMemory(base, commit_size);

  Arena *ret = static_cast<Arena *>(base);
  ret->curr = ret;
  ret->prev = nullptr;
  ret->reserve_size = reserve_size;
  ret->commit_size = commit_size;
  ret->commit_pos = commit_size;
  ret->pos = ARENA_HEADER_SIZE;
  ret->alloc_file_location = file;
  ret->alloc_file_line = line;

  return ret;
}

auto Arena::Release() -> void {
  Arena *curr = this->curr;
  while (curr != nullptr) {
    Arena *tmp = this->prev;
    Os::ReleaseMemory(curr, curr->reserve_size);
    curr = tmp;
  }
}

auto Arena::Push(u64 size) -> void *{
  auto *curr = this->curr;
  auto pos = AlignPow2(curr->pos, 1);
  auto pos_final = pos + size;

  // I imagine it's extremely unlikely to do a single push more than the reserve size
  Assert(size < DEFAULT_RESERVE);

  if (curr->reserve_size < pos_final) {
    Arena *next = Alloc__(curr->alloc_file_location, curr->alloc_file_line);

    this->curr = next;
    this->prev = curr;
    curr = next;
  }

  while (curr->commit_size < pos_final) {
    u64 commit_pos_final = curr->commit_pos + DEFAULT_COMMIT;
    u64 commit_size = Min(commit_pos_final, curr->reserve_size) - curr->commit_pos;
    Os::CommitMemory((u8*)curr + curr->commit_pos, commit_size);
    curr->commit_pos += commit_size;
  }

  void* result = nullptr;
  Assert(curr->commit_pos >= pos_final);
  // return the pointer to the old pos
  result = (u8*)curr + pos;
  curr->pos = pos_final;

  return result;
}

auto Arena::Pop(u64 size) -> void {
  auto *curr = this->curr;
  u64 final_pos = Max(ARENA_HEADER_SIZE, curr->pos - size);
  u64 pop_size = curr->pos - final_pos;
  curr->pos = final_pos;
  if (pop_size < size) {
    this->curr = this->prev;
    Os::ReleaseMemory(curr, DEFAULT_RESERVE);
    this->Pop(size - pop_size);
  }
}

auto Arena::Clear() -> void {
  auto *curr = this->curr;
  while (this->prev != nullptr) {
    auto *prev = this->prev;
    Os::ReleaseMemory(curr, DEFAULT_RESERVE);
    this->curr = prev;
    curr = prev;
  }
  this->prev = nullptr;
}
