#include "edb.h"
#include "page.h"

#ifdef __linux__
#include <string.h> //memset
#endif

#define PAGE_TABLE(db, page) ((page_table*) (db->data + BLOCK_SIZE * page))
#define INDEX_PAGE(db, page) ((index_page*) (db->data + BLOCK_SIZE * page))
#define DATA_PAGE( db, page)       ((page*) (db->data + BLOCK_SIZE * page))


typedef struct index_page_t {
  u32 data_blocks[INDEX_SIZE];
} index_page;


// get the host index of a relative index in a page table
// returns 0 on error
u32 page_get_host_index(const edb* db, const u32 root, const u32 index) {
  page_table* pt = PAGE_TABLE(db, root);
  if (index > pt->nblocks) {
    return 0;
  }

  if (pt->nblocks == 0) {
    // small block
    return root;
  }

  if (index < LEVEL_ONE) {
    // level 1 large block
    return pt->data_blocks[index];
  }

  // level 2 large block
  u32 index_block = pt->index_blocks[INDEX0(index)];
  index_page* ip = INDEX_PAGE(db, index_block);
  return ip->data_blocks[INDEX1(index)];
}


// page page_get_host_page(const edb* db, const u32 root, const u32 index) {
//   return DATA_PAGE(db, page_get_host_index(db, root, index));
// }e


void page_init(edb* db, const u32 root) {
  page_table* pt = PAGE_TABLE(db, root);
  memset(pt, 0, BLOCK_SIZE);
}


int page_resize(edb* db, const u32 root, const u32 nblocks) {
  int err = 0;
  page_table* pt = PAGE_TABLE(db, root);
  index_page* ip;

  while (pt->nblocks < nblocks && pt->nblocks < LEVEL_ONE) {
    // grow data_blocks
    u32 new_block = edb_allocate_block(db);
    if (new_block == 0) {
      err = -1;
      goto err;
    }
    pt = PAGE_TABLE(db, root);
    pt->data_blocks[pt->nblocks++] = new_block;
  }

  while (pt->nblocks < nblocks) {
    // grow index_blocks
    u32 index_index = INDEX0(pt->nblocks);
    u32 data_index  = INDEX1(pt->nblocks);
    u32 index_block = pt->index_blocks[index_index];

    if (index_block == 0) {
      // allocate new index_block
      index_block = edb_allocate_block(db);
      if (index_block == 0) {
        err = -1;
        goto err;
      }
      pt = PAGE_TABLE(db, root);
      pt->index_blocks[index_index] = index_block;
    }

    u32 new_block = edb_allocate_block(db);
    if (new_block == 0) {
      err = -1;
      goto err;
    }
    pt = PAGE_TABLE(db, root);
    ip = INDEX_PAGE(db, index_block);
    ip->data_blocks[data_index] = new_block;
    pt->nblocks++;
  }

  while (pt->nblocks > nblocks && pt->nblocks > LEVEL_ONE) {
    // shrink
    u32 index_index = INDEX0(pt->nblocks - 1);
    u32 data_index  = INDEX1(pt->nblocks - 1);
    u32 index_block = pt->index_blocks[index_index];
    ip = INDEX_PAGE(db, index_block);
    u32 data_block = ip->data_blocks[data_index];

    if ((err = edb_free_block(db, data_block))) {
      goto err;
    }
    pt = PAGE_TABLE(db, root);
    ip = INDEX_PAGE(db, index_block);

    ip->data_blocks[data_index] = 0;
    if (INDEX1(pt->nblocks - 1) == 0) {
      // free index page
      if ((err = edb_free_block(db, index_block))) {
        goto err;
      }
      pt = PAGE_TABLE(db, root);
      ip = INDEX_PAGE(db, index_block);
      pt->index_blocks[index_index] = 0;
    }
    pt->nblocks--;
  }

  while (pt->nblocks > nblocks) {
    // shrink
    u32 data_block = pt->data_blocks[pt->nblocks - 1];
    if ((err = edb_free_block(db, data_block))) {
      goto err;
    }
    pt = PAGE_TABLE(db, root);
    pt->data_blocks[--pt->nblocks] = 0;
  }

  err:
  return err;
}
