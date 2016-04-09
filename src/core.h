#ifndef CORE_H
#define CORE_H

#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_BITS (12) // 4096
#define BLOCK_SIZE (1 << BLOCK_BITS)
#define BLOCK_MASK (BLOCK_SIZE - 1)


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


typedef struct {
  int readonly;
#ifdef _WIN32
  HANDLE h_file;
  HANDLE h_mapping;
#else
  int h_file;
  void* h_map;
#endif
  char* data;
  u64 filesize;
  u64 nblocks;
  u32 freelist;
} edb;


u32 edb_allocate_block(edb *db);
int edb_free_block(edb *db, u32 block);

#ifdef __cplusplus
}
#endif
#endif