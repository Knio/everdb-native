#ifndef CORE_H
#define CORE_H

#include "../include/edb.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define BLOCK_BITS (12) // 4096
#define BLOCK_SIZE (1 << BLOCK_BITS)
#define BLOCK_MASK (BLOCK_SIZE - 1)

#define EDB_FREELIST_PRIMARY (1)
#define EDB_FREELIST_SECONDARY (2)
#define EDB_OBJLIST (3)
#define EDB_USER_DATA (4)

#define EDB_MAGIC_ROOT_PRIMARY (1)
#define EDB_MAGIC_ROOT_SECONDARY (2)
#define EDB_MAGIC_PAGE_SMALL (3)
#define EDB_MAGIC_PAGE_FULL (4)
#define EDB_MAGIC_BTREE (5)
#define EDB_MAGIC_BTREE_LEAF (6)

typedef struct txn_state_t txn_state;

typedef struct edb_t {
  int readonly;
#ifdef _WIN32
  HANDLE h_file;
  HANDLE h_mapping;
#else
  int h_file;
  void* h_mapping;
#endif
  u8* data;

  u64 filesize;
  u32 nblocks;

  u32 freelist;
  u32 objlist;

  u32 txn_id;
  txn_state* txn;
} edb;


int edb_allocate_block(edb *const db, u32 *const new_block);
int edb_modify_block(edb *const db, const u32 block, u32 *const new_block);
int edb_free_block(edb *const db, const u32 block);
int edb_is_writable(const edb *const db, const u32 block);

#ifdef __cplusplus
}
#endif

#endif // CORE_H
