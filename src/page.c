#ifdef __linux__
#include <string.h> //memset
#endif

#include "edb.h"
#include "util.h"
#include "page.h"

#define PAGE_TABLE(db, page) ((page_table*) (db->data + BLOCK_SIZE * page))
#define INDEX_PAGE(db, page) ((index_page*) (db->data + BLOCK_SIZE * page))
#define DATA_PAGE( db, page)       ((page*) (db->data + BLOCK_SIZE * page))


typedef struct index_page_t {
  u32 data_blocks[INDEX_SIZE];
} index_page;


int
page_read_host_index(const edb *const db, const u32 root, const u32 index, u32 *const block) {
  // get the host index of a relative index in a page table
  page_table* pt = PAGE_TABLE(db, root);
  if (index >= pt->nblocks) {
    *block = -1;
    return EDB_ERR_PAGE_OUT_OF_BOUNDS;
  }

  if (pt->nblocks == 0) {
    // small block
    *block = root;
    return 0;
  }

  if (index < LEVEL_ONE) {
    // level 1 large block
    *block = pt->data_blocks[index];
    return 0;
  }

  // level 2 large block
  u32 index_block = pt->index_blocks[INDEX0(index)];
  index_page* ip = INDEX_PAGE(db, index_block);
  *block = ip->data_blocks[INDEX1(index)];
  return 0;
}


int
page_write_host_index(edb *const db, u32 root, const u32 index, u32 *const block) {
  // get the host index of a relative index in a page table
  int err = 0;

  page_table* pt = PAGE_TABLE(db, root);
  if (index >= pt->nblocks) {
    *block = -1;
    return EDB_ERR_PAGE_OUT_OF_BOUNDS;
  }

  if (pt->nblocks == 0) {
    // small block
    CHECK(edb_modify_block(db, root, &root));
    *block = root;
    return 0;
  }

  if (index < LEVEL_ONE) {
    // level 1 large block
    CHECK(edb_modify_block(db, root, &root));
    pt = PAGE_TABLE(db, root);
    *block = pt->data_blocks[index];
    CHECK(edb_modify_block(db, *block, block));
    return 0;
  }

  // level 2 large block
  CHECK(edb_modify_block(db, root, &root));
  pt = PAGE_TABLE(db, root);
  u32 index_block = pt->index_blocks[INDEX0(index)];
  CHECK(edb_modify_block(db, index_block, &index_block));
  pt = PAGE_TABLE(db, root);
  index_page* ip = INDEX_PAGE(db, index_block);
  *block = ip->data_blocks[INDEX1(index)];
  CHECK(edb_modify_block(db, *block, block));
  return 0;

  err:
  *block = -1;
  return err;
}


void page_init(edb* db, const u32 root) {
  page_table* pt = PAGE_TABLE(db, root);
  memset(pt, 0, BLOCK_SIZE);
}


int page_resize(edb* db, u32 root, const u32 nblocks) {
  int err = 0;
  CHECK(edb_modify_block(db, root, &root));
  page_table* pt = PAGE_TABLE(db, root);
  index_page* ip;

  while (pt->nblocks < nblocks && pt->nblocks < LEVEL_ONE) {
    // grow data_blocks
    u32 new_block;
    CHECK(edb_allocate_block(db, &new_block))
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
      CHECK(edb_allocate_block(db, &index_block))
    }
    else {
      // TODO optimiaztion: can only call this once per index block per resize
      CHECK(edb_modify_block(db, index_block, &index_block));
    }
    pt = PAGE_TABLE(db, root);
    pt->index_blocks[index_index] = index_block;

    u32 new_block;
    CHECK(edb_allocate_block(db, &new_block))
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

    CHECK(edb_free_block(db, data_block))
    pt = PAGE_TABLE(db, root);
    ip = INDEX_PAGE(db, index_block);

    ip->data_blocks[data_index] = 0;
    if (INDEX1(pt->nblocks - 1) == 0) {
      // free index page
      CHECK(edb_free_block(db, index_block));
      index_block = 0;
    }
    else {
      CHECK(edb_modify_block(db, index_block, &index_block));
    }
    pt = PAGE_TABLE(db, root);
    pt->index_blocks[index_index] = index_block;
    pt->nblocks--;
  }

  while (pt->nblocks > nblocks) {
    // shrink
    u32 data_block = pt->data_blocks[pt->nblocks - 1];
    CHECK(edb_free_block(db, data_block))
    pt = PAGE_TABLE(db, root);
    pt->data_blocks[--pt->nblocks] = 0;
  }

  err:
  return err;
}
