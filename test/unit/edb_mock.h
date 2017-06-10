#include "../../src/core.h"

#include <vector>
std::vector<u32> freelist;

// block API mocks
extern "C" {

int edb_allocate_block(edb *const db, u32 *const new_block) {
  if (freelist.size()) {
    u32 back = freelist.back();
    freelist.pop_back();
    *new_block = back;
    return 0;
  }
  return -1;
}

int edb_free_block(edb* db, const u32 block) {
  freelist.push_back(block);
  return 0;
}

int edb_modify_block(edb* db, const u32 block, u32 *const new_block) {
  // TODO invalidate block here to test that old pointer isn't being reused
  *new_block = block;
  return 0;
}

} // extern "C"

edb db_;
edb *db = &db_;

void init_edb_mock() {
  db->data = (u8*) malloc(BLOCK_SIZE * 1024);
  for (u32 i=1; i<1024; i++) {
    freelist.push_back(i);
  }
}

void clear_edb_mock() {
  freelist.clear();
  free(db->data);
}

