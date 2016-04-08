#ifndef EDB_H
#define EDB_H

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

typedef struct page_t {
  u8 data[BLOCK_SIZE];
} page;


/**
 * Hashdb object
 */
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
  u64 size;
  u32 freelist;
} edb;


typedef struct edb_root_t {
  u32 txn_id;
  u32 committed;
  u32 freelist;
} edb_root;


/**
 * Open or create a edbdb file
 * The database or null is returned in the edbdb param
 * @return 0 on success, -1 on open error, other values on other errors
 */
int edb_open(edb *db, const char* f_name,
    int readonly,
    int overwrite);

void edb_close(edb *db);

int edb_resize(edb *db, u64 size);

u32 edb_allocate_block(edb *db);
int edb_free_block(edb *db, u32 block);



#ifdef __cplusplus
}
#endif
#endif
