#ifdef _WIN32
#error need headers?
#elif __linux__
#include <stdlib.h> // malloc
#include <string.h> // memset, memcpy
#else
#error Unsupported OS
#endif

#include "page.h"
#include "util.h"

#define PAGE_TABLE_SMALL(db, page) ((page_table_small*) (db->data + BLOCK_SIZE * page))
#define PAGE_TABLE_FULL(db, page)  ((page_table_full* ) (db->data + BLOCK_SIZE * page))
#define INDEX_PAGE(db, page)       ((page_table_index*) (db->data + BLOCK_SIZE * page))
#define DATA_PAGE(db, page)        ((u8*) (db->data + BLOCK_SIZE * page))

#define INDEX0(index) ((index - EDB_PAGE_DATA_BLOCKS_PER_PAGE) >> INDEX_BITS)
#define INDEX1(index) ((index - EDB_PAGE_DATA_BLOCKS_PER_PAGE)  & INDEX_MASK)

#define CHECK_IS_PAGE(pt) CHECK_CODE(pt->magic == EDB_MAGIC_PAGE_SMALL || pt->magic == EDB_MAGIC_PAGE_FULL, EDB_ERROR_DATA_CORRUPT)


int page_get_block(const edb *const db, const u32 root, const u32 index, u32 *const block) {
  int err = 0;
  page_table_full* pt = PAGE_TABLE_FULL(db, root);
  CHECK_CODE(pt->magic == EDB_MAGIC_PAGE_FULL, EDB_ERROR_DATA_CORRUPT);
  CHECK_CODE(index < pt->nblocks, EDB_ERR_PAGE_OUT_OF_BOUNDS);

  if (index < EDB_PAGE_DATA_BLOCKS_PER_PAGE) {
    *block = pt->data_blocks[index];
    return 0;
  }

  u32 index_block = pt->index_blocks[INDEX0(index)];
  page_table_index* ip = INDEX_PAGE(db, index_block);
  *block = ip->data_blocks[INDEX1(index)];
  return 0;

  err:
  *block = (0xffffff00);
  return err;
}


int page_get_writable_block(edb *const db, u32 root, const u32 index, u32 *const block) {
  int err = 0;
  page_table_full* pt = PAGE_TABLE_FULL(db, root);
  CHECK_CODE(pt->magic == EDB_MAGIC_PAGE_FULL, EDB_ERROR_DATA_CORRUPT);
  CHECK_CODE(index < pt->nblocks, EDB_ERR_PAGE_OUT_OF_BOUNDS);
  CHECK(edb_is_writable(db, root));

  if (index < EDB_PAGE_DATA_BLOCKS_PER_PAGE) {
    *block = pt->data_blocks[index];
    CHECK(edb_modify_block(db, *block, block));
    pt = PAGE_TABLE_FULL(db, root);
    pt->data_blocks[index] = *block;
    return 0;
  }

  u32 index_block = pt->index_blocks[INDEX0(index)];
  CHECK(edb_modify_block(db, index_block, &index_block));
  pt = PAGE_TABLE_FULL(db, root);
  pt->index_blocks[INDEX0(index)] = index_block;

  page_table_index* ip = INDEX_PAGE(db, index_block);
  *block = ip->data_blocks[INDEX1(index)];
  CHECK(edb_modify_block(db, *block, block));
  ip = INDEX_PAGE(db, index_block);
  ip->data_blocks[INDEX1(index)] = *block;
  return 0;

  err:
  *block = -1;
  return err;
}


int page_init(edb* db, const u32 root) {
  int err = 0;
  CHECK(edb_is_writable(db, root));
  page_table_small* pt = PAGE_TABLE_SMALL(db, root);
  memset(pt, 0, BLOCK_SIZE);
  pt->magic = EDB_MAGIC_PAGE_SMALL;
  err:
  return err;
}

int page_convert_to_small(edb *db, const u32 root) {
  int err = 0;
  page_table_full* pt = PAGE_TABLE_FULL(db, root);
  CHECK_CODE(pt->magic == EDB_MAGIC_PAGE_FULL, EDB_ERROR_DATA_CORRUPT);
  CHECK_CODE(pt->size <= EDB_PAGE_SMALL_MAX_SIZE, EDB_ERR_PAGE_TOO_LARGE);
  CHECK(edb_is_writable(db, root));
  u8* data = (u8*)malloc(pt->size);
  CHECK(page_read(db, root, 0, pt->size, data));
  CHECK(page_reallocate(db, root, 0));
  page_table_small* pts = PAGE_TABLE_SMALL(db, root);
  pts->magic = EDB_MAGIC_PAGE_SMALL;
  CHECK(page_write(db, root, 0, pts->size, data));
  free(data);
  memset(pts->data, 0, EDB_PAGE_SMALL_MAX_SIZE - pts->size);
  err:
  return 0;
}


int page_convert_to_full(edb *db, const u32 root) {
  int err = 0;
  page_table_small* pts = PAGE_TABLE_SMALL(db, root);
  CHECK_CODE(pts->magic == EDB_MAGIC_PAGE_SMALL, EDB_ERROR_DATA_CORRUPT);
  CHECK(edb_is_writable(db, root));
  u8* data = (u8*)malloc(pts->size);
  CHECK(page_read(db, root, 0, pts->size, data));
  page_table_full* pt = PAGE_TABLE_FULL(db, root);
  pt->magic =EDB_MAGIC_PAGE_FULL;
  pt->nblocks = 0;
  memset(pt->data_blocks, 0, sizeof(u32) * EDB_PAGE_DATA_BLOCKS_PER_PAGE);
  memset(pt->index_blocks, 0, sizeof(u32) * EDB_PAGE_INDEX_BLOCKS_PER_PAGE);
  CHECK(page_reallocate(db, root, (pt->size + BLOCK_MASK) >> BLOCK_BITS));
  CHECK(page_write(db, root, 0, pt->size, data));
  free(data);
  err:
  return err;
}


int page_read(const edb *const db, const u32 root, u64 offset, u32 length, u8* data) {
  int err = 0;
  page_table_full* pt = PAGE_TABLE_FULL(db, root);
  CHECK_IS_PAGE(pt);
  CHECK_CODE(pt->size >= offset + length, EDB_ERR_PAGE_OUT_OF_BOUNDS);


  if (pt->magic == EDB_MAGIC_PAGE_SMALL) {
    page_table_small* pts = PAGE_TABLE_SMALL(db, root);
    memcpy(data, pts->data + offset, length);
    return 0;
  }

  CHECK_CODE(pt->magic == EDB_MAGIC_PAGE_FULL, EDB_ERROR_DATA_CORRUPT);
  while (length) {
    u32 block, off, len;
    CHECK(page_get_block(db, root, offset >> BLOCK_BITS, &block));
    off = (u32) offset & BLOCK_MASK;
    len = ((off + length) & ~BLOCK_MASK) ? (BLOCK_SIZE - off) : (length);
    memcpy(data, DATA_PAGE(db, block) + off, len);
    data += len;
    offset += len;
    length -= len;
  }
  err:
  return err;
}


int page_write(edb *const db, const u32 root, u64 offset, u32 length, const u8 *data) {
  int err = 0;
  page_table_full* pt = PAGE_TABLE_FULL(db, root);
  CHECK_IS_PAGE(pt);
  CHECK_CODE(pt->size >= offset + length, EDB_ERR_PAGE_OUT_OF_BOUNDS);
  CHECK(edb_is_writable(db, root));

  if (pt->magic == EDB_MAGIC_PAGE_SMALL) {
    page_table_small* pts = PAGE_TABLE_SMALL(db, root);
    memcpy(pts->data + offset, data, length);
    return 0;
  }

  CHECK_CODE(pt->magic == EDB_MAGIC_PAGE_FULL, EDB_ERROR_DATA_CORRUPT);
  while (length) {
    u32 block, off, len;
    CHECK(page_get_writable_block(db, root, offset >> BLOCK_BITS, &block));
    off = (u32) offset & BLOCK_MASK;
    len = ((off + length) & ~BLOCK_MASK) ? (BLOCK_SIZE - off) : (length);
    memcpy(DATA_PAGE(db, block) + off, data, len);
    data += len;
    offset += len;
    length -= len;
  }
  err:
  return err;
}


int page_resize(edb *const db, const u32 root, const u64 size) {
  int err = 0;
  page_table_small* pt = PAGE_TABLE_SMALL(db, root);
  CHECK_IS_PAGE(pt);
  CHECK(edb_is_writable(db, root));
  if (pt->magic == EDB_MAGIC_PAGE_SMALL) {
    CHECK_CODE(size <= EDB_PAGE_SMALL_MAX_SIZE, EDB_ERR_PAGE_TOO_LARGE);
    if (size < pt->size) {
      // shrink, zero data
      memset(pt->data + pt->size, 0, pt->size - size);
    }
    pt->size = size;
    return 0;
  }

  CHECK_CODE(pt->magic == EDB_MAGIC_PAGE_FULL, EDB_ERROR_DATA_CORRUPT);
  u32 nblocks = (u32)((size + BLOCK_MASK) >> BLOCK_BITS);
  if (nblocks != pt->nblocks) {
    CHECK(page_reallocate(db, root, nblocks));
  }
  if (size < pt->size) {
    // shrink, zero data
    u32 block, off, len;
    CHECK(page_get_writable_block(db, root, (u32)(size >> BLOCK_BITS), &block));
    off = (u32)(size & BLOCK_MASK);
    len = BLOCK_SIZE - off;
    memset(DATA_PAGE(db, block) + off, 0, len);
  }
  pt->size = size;
  err:
  return err;
}


int page_reallocate(edb *const db, const u32 root, const u32 nblocks) {
  int err = 0;
  page_table_full* pt = PAGE_TABLE_FULL(db, root);
  CHECK_CODE(pt->magic == EDB_MAGIC_PAGE_FULL, EDB_ERROR_DATA_CORRUPT);
  CHECK_CODE(nblocks <= EDB_PAGE_MAX_BLOCKS, EDB_ERR_PAGE_FULL);
  CHECK(edb_is_writable(db, root));
  page_table_index* ip;

  while (pt->nblocks < nblocks && pt->nblocks < EDB_PAGE_DATA_BLOCKS_PER_PAGE) {
    // grow data_blocks
    u32 new_block;
    CHECK(edb_allocate_block(db, &new_block))
    pt = PAGE_TABLE_FULL(db, root);
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
    pt = PAGE_TABLE_FULL(db, root);
    pt->index_blocks[index_index] = index_block;

    u32 new_block;
    CHECK(edb_allocate_block(db, &new_block))
    pt = PAGE_TABLE_FULL(db, root);
    ip = INDEX_PAGE(db, index_block);
    ip->data_blocks[data_index] = new_block;
    pt->nblocks++;
  }

  while (pt->nblocks > nblocks && pt->nblocks > EDB_PAGE_DATA_BLOCKS_PER_PAGE) {
    // shrink
    u32 index_index = INDEX0(pt->nblocks - 1);
    u32 data_index  = INDEX1(pt->nblocks - 1);
    u32 index_block = pt->index_blocks[index_index];
    ip = INDEX_PAGE(db, index_block);
    u32 data_block = ip->data_blocks[data_index];

    CHECK(edb_free_block(db, data_block))
    pt = PAGE_TABLE_FULL(db, root);
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
    pt = PAGE_TABLE_FULL(db, root);
    pt->index_blocks[index_index] = index_block;
    pt->nblocks--;
  }

  while (pt->nblocks > nblocks) {
    // shrink
    u32 data_block = pt->data_blocks[pt->nblocks - 1];
    CHECK(edb_free_block(db, data_block))
    pt = PAGE_TABLE_FULL(db, root);
    pt->data_blocks[--pt->nblocks] = 0;
  }

  err:
  return err;
}
