#ifndef PAGE_H
#define PAGE_H
#include "edb.h"

#define INDEX_BITS (10)
#define INDEX_SIZE (1 << INDEX_BITS)
#define INDEX_MASK (INDEX_SIZE - 1)

#define LEVEL_ONE (512)

#define INDEX0(index) (LEVEL_ONE + ((index - LEVEL_ONE) >> INDEX_BITS))
#define INDEX1(index) ((index - LEVEL_ONE) & INDEX_MASK)

#define BLOCK(root, index) (root + (index * BLOCK_SIZE))

#define HEADER(root, type) ((type*) (root + BLOCK_SIZE - sizeof(type)))

typedef struct page_header_t {
  u32 header_length;
  u32 nblocks;
  u32 checksum;
} page_header;

#endif
