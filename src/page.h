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

#define INDEX0(index) ((index - LEVEL_ONE) >> INDEX_BITS)
#define INDEX1(index) ((index - LEVEL_ONE) & INDEX_MASK)

#define BLOCK(db, index) (db->data + (index * BLOCK_SIZE))


typedef struct page_t {
  u8 data[BLOCK_SIZE];
} page;


typedef struct page_header_t {
  // u8 header_length;
  u32 nblocks;
  // u32 checksum;
} page_header;


#pragma warning(disable:4201)
typedef struct page_table_t {
  union {
    struct page_t;
    struct {
      u32 data_blocks[LEVEL_ONE];
      u32 index_blocks[LEVEL_ONE];
    };
    struct {
      u8 __padding[BLOCK_SIZE - sizeof(page_header)];
      struct page_header_t;
    };
  };
} page_table;


void
page_init(edb *db, const u32 root);

u32
page_get_host_index(const edb *db, const u32 root, const u32 index);

int
page_resize(edb *db, const u32 root, const u32 nblocks);


#ifdef __cplusplus
}
#endif
#endif
