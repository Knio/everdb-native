#include "edb.h"
#include "page.h"

#ifdef __linux__
#include <string.h> //memset
#endif

typedef struct page_table_t {
  u32 data_blocks[LEVEL_ONE];
  u32 index_blocks[LEVEL_ONE];
} page_table;

typedef struct index_page_t {
  u32 data_blocks[INDEX_SIZE];
} index_page;

// get the host index of a relative index in a page table
// returns 0 on error
u32 page_get_host_index(const edb* db, const u32 page, const u32 index) {
  block page_table = BLOCK(db->data, page);
  const page_header* ph = HEADER(page_table, page_header);
  if (ph->nblocks < index) {
    return 0;
  }

  if (ph->nblocks == 0) {
    // small block
    return page;
  }

  u32* block_ptrs = ((u32*)page_table);
  if (index <= LEVEL_ONE) {
    // level 1 large block
    return block_ptrs[index];
  }
  // level 2 large block
  block l1 = BLOCK(db->data, INDEX0(index));
  block_ptrs = ((u32*)l1);
  return block_ptrs[INDEX1(index)];
}

block page_get_host_block(const edb* db, const u32 page, const u32 index) {
  return BLOCK(db->data, page_get_host_index(db, page, index));
}

void page_init(edb* db, const u32 page) {
  block page_table = BLOCK(db->data, page);
  const page_header* ph = HEADER(page_table, page_header);

  memset(page_table, 0, BLOCK_SIZE);
}

int page_resize(edb* db, const u32 page, const u32 nblocks) {
  int err = 0;
  page_table* pt = (page_table*) BLOCK(db->data, page);
  page_header* ph = HEADER(pt, page_header);

  while (ph->nblocks < nblocks && ph->nblocks < LEVEL_ONE) {
    // grow
    u32 new_block = edb_allocate_block(db);
    if (new_block == 0) {
      err = -1;
      goto err;
    }
    pt->data_blocks[ph->nblocks++] = new_block;
  }

  while (ph->nblocks < nblocks) {
    // grow
    u32 index_block = pt->index_blocks[INDEX0(ph->nblocks)];
    if (index_block == 0) {
      // allocate new index page
      index_block = edb_allocate_block(db);
      if (index_block == 0) {
        err = -1;
        goto err;
      }
      pt->index_blocks[INDEX0(ph->nblocks)] = index_block;
    }
    index_page* ip = (index_page*) BLOCK(db->data, index_block);

    u32 new_block = edb_allocate_block(db);
    if (new_block == 0) {
      err = -1;
      goto err;
    }
    ip->data_blocks[ph->nblocks++] = new_block;
  }


  while (ph->nblocks > nblocks && ph->nblocks > LEVEL_ONE) {
    // shrink
    u32 index_index = INDEX0(ph->nblocks - 1);
    u32 data_index  = INDEX1(ph->nblocks - 1);
    u32 index_block = pt->index_blocks[index_index];
    index_page* ip = (index_page*) BLOCK(db->data, index_block);
    u32 data_block = ip->data_blocks[data_index];
    edb_free_block(db, data_block);
    ip->data_blocks[data_index] = 0;
    if (INDEX1(ph->nblocks - 1) == 0) {
      // free index page
      if (err = edb_free_block(db, index_block)) {
        goto err;
      }
      pt->index_blocks[index_index] = 0;
    }
    ph->nblocks--;
  }

  while (ph->nblocks > nblocks && ph->nblocks) {
    // shrink
    u32 data_block = pt->data_blocks[ph->nblocks - 1];
    if (err = edb_free_block(db, data_block)) {
      goto err;
    }
    pt->data_blocks[ph->nblocks - 1] = 0;
    ph->nblocks--;
  }

  err:
  return err;
}
