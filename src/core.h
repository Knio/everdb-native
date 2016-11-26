#ifndef CORE_H
#define CORE_H

#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_EDB_DB_SIZE_MAX (1003)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define BLOCK_BITS (12) // 4096
#define BLOCK_SIZE (1 << BLOCK_BITS)
#define BLOCK_MASK (BLOCK_SIZE - 1)

#define BLOCK(db, index) ((db)->data + ((index) * BLOCK_SIZE))

// struct txn_state_t;
// typedef txn_state_t txn_state;
typedef struct txn_state_t txn_state;

typedef struct edb_t {
  int readonly;
#ifdef _WIN32
  HANDLE h_file;
  HANDLE h_mapping;
#else
  int h_file;
  void* h_map;
#endif
  u8* data;
  u64 filesize;
  u32 nblocks;
  u32 freelist;
  txn_state* txn;
} edb;


int edb_allocate_block(edb* db, u32 *new_block);
int edb_free_block(edb* db, const u32 block);


#ifdef __cplusplus
}
#endif

#endif
