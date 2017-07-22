#ifdef _WIN32

#elif __linux__

#else
#error Unsupported OS
#endif

#include "util.h"
#include "edb.h"
#include "txn.h"

// block device api
int edb_allocate_block(edb *const db, u32 *const new_block) {
  LOG_HERE;
  return txn_allocate_block(db, new_block);
}


int edb_modify_block(edb *const db, u32 block, u32 *const new_block) {
  LOG_DEBUG("block:%d\n", block);
  int err = txn_modify_block(db, block, new_block);
  LOG_DEBUG("block:%d new_block:%d\n", block, *new_block);
  return err;
}


int edb_free_block(edb *db, u32 block) {
  LOG_DEBUG("block:%d\n", block);
  return txn_free_block(db, block);
}

int edb_is_writable(const edb *const db, const u32 block) {
  LOG_DEBUG("block:%d\n", block);
  return txn_block_is_writable(db, block);
}
