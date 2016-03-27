#include "edb.h"
#include "page.h"

// get the host index of a relative index in a page table
u32 page_get_host_index(edb *db, const u32 page, const u32 index) {
  block page_table = BLOCK(db->data, page);
  const page_header *ph = HEADER(page_table, page_header);
  if (ph->nblocks < index) {
    return -1;
  }

  if (ph->nblocks == 0) {
    // small block
    return page;
  }

  u32 *block_ptrs = ((u32*)page_table);
  if (index <= LEVEL_ONE) {
    // level 1 large block
    return block_ptrs[index];
  }
  // level 2 large block
  block l1 = BLOCK(db->data, INDEX0(index));
  block_ptrs = ((u32*)l1);
  return block_ptrs[INDEX1(index)];
}

inline block page_get_host_block(edb *db, const u32 page, const u32 index) {
  return BLOCK(db->data, page_get_host_index(db, page, index));
}

void page_init(edb *db, const u32 page) {
  block page_table = BLOCK(db->data, page);
  const page_header *ph = HEADER(page_table, page_header);

  memset(page_table, 0, BLOCK_SIZE);

  // TODO
  exit(1);
}

void page_resize(edb *db, const u32 page, const u32 nblocks) {
  block page_table = BLOCK(db->data, page);
  const page_header *ph = HEADER(page_table, page_header);

  while (ph->nblocks < nblocks) {
    // grow
  }

  while (ph->nblocks > nblocks) {
    // shrink
  }

}
