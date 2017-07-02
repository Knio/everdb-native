#ifndef PAGE_H
#define PAGE_H

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INDEX_BITS (10)
#define INDEX_SIZE (1 << INDEX_BITS)
#define INDEX_MASK (INDEX_SIZE - 1)
#define LEVEL_ONE (512)

#define EDB_MAGIC_PAGE_SMALL (3101)
#define EDB_MAGIC_PAGE_FULL (3102)

#define EDB_ERR_PAGE_FULL (3152)

#define EDB_PAGE_DATA_BLOCKS_PER_PAGE (510)
#define EDB_PAGE_INDEX_BLOCKS_PER_PAGE (510)
#define EDB_PAGE_DATA_BLOCKS_PER_INDEX (1024)

#define EDB_PAGE_SMALL_MAX_SIZE (BLOCK_SIZE - 16)
#define EDB_PAGE_MAX_BLOCKS \
  (EDB_PAGE_DATA_BLOCKS_PER_PAGE + (EDB_PAGE_INDEX_BLOCKS_PER_PAGE * EDB_PAGE_DATA_BLOCKS_PER_INDEX))
#define EDB_PAGE_FULL_MAX_SIZE \
  (BLOCK_SIZE * EDB_PAGE_MAX_BLOCKS)


#define EDB_ERR_PAGE_OUT_OF_BOUNDS (3101)
#define EDB_ERR_PAGE_TOO_LARGE (3102)

typedef struct page_table_small_t {
  u32 magic;
  u32 nblocks;
  u64 size;
  u8 data[EDB_PAGE_SMALL_MAX_SIZE];
} page_table_small;

typedef struct page_table_full_t {
  u32 magic;
  u32 nblocks;
  u64 size;
  u32 data_blocks[EDB_PAGE_DATA_BLOCKS_PER_PAGE];
  u32 index_blocks[EDB_PAGE_INDEX_BLOCKS_PER_PAGE];
} page_table_full;

typedef struct page_table_index_t {
  u32 data_blocks[EDB_PAGE_DATA_BLOCKS_PER_INDEX];
} page_table_index;


int page_init(edb *const db, const u32 root);

int page_get_block(const edb *const db, const u32 root, const u32 index, u32 *const block);
int page_get_writable_block(edb *const db, const u32 root, const u32 index, u32 *const block);

int page_convert_to_small(edb *const db, const u32 root);
int page_convert_to_full(edb *const db, const u32 root);

int page_reallocate(edb *const db, const u32 root, const u32 nblocks);
int page_resize(edb *const db, const u32 root, const u64 size);

int page_read(const edb *const db, const u32 root, u64 offset, u32 length, u8 *data);
int page_write(edb *const db, const u32 root, u64 offset, u32 length, const u8 *data);

#ifdef __cplusplus
}
#endif
#endif
