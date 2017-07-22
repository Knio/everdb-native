#include "../../src/edb.h"

#include <vector>
std::vector<u32> freelist;
std::map<u32, int> modified;

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

int edb_free_block(edb *const db, const u32 block) {
  freelist.push_back(block);
  return 0;
}

int edb_modify_block(edb *const db, const u32 block, u32 *const new_block) {
  // TODO invalidate block here to test that old pointer isn't being reused
  *new_block = block;
  modified[block] = 1;
  return 0;
}

int edb_is_writable(const edb *const db, const u32 block) {
  return modified[block] == 0 ? EDB_ERROR_DATA_READONLY : 0;
}

} // extern "C"

edb db_ = {0};
edb *db = &db_;

void clear_edb_mock() {
  freelist.clear();
  modified.clear();
  free(db->data);
  db->data = 0;
}

void init_edb_mock() {
  clear_edb_mock();
  db->data = (u8*) calloc(1024, BLOCK_SIZE);
  for (u32 i=1; i<1024; i++) {
    freelist.push_back(i);
  }
}

