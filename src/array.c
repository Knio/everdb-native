#ifdef _WIN32
#error need headers?
#elif __linux__
#include <stdlib.h> // malloc
#include <string.h> // memset, memcpy
#else
#error Unsupported OS
#endif

#include "array.h"
#include "util.h"

#define PAGE_TABLE_SMALL(db, page) ((page_table_small*) (db->data + BLOCK_SIZE * page))
#define PAGE_TABLE_FULL(db, page)  ((page_table_full* ) (db->data + BLOCK_SIZE * page))
#define CHECK_IS_PAGE(pt) CHECK_CODE(pt->magic == EDB_MAGIC_PAGE_SMALL || pt->magic == EDB_MAGIC_PAGE_FULL, EDB_ERROR_DATA_CORRUPT)


int array_init(edb* db, const u32 root, const u8 item_size) {
  int err = 0;
  CHECK(page_init(db, root));
  page_table_small* pt = PAGE_TABLE_SMALL(db, root);
  pt->user1 = item_size;
  err: return err;
}


int array_resize(edb* db, const u32 root, const u64 length) {
  int err = 0;
  page_table_small* pt = PAGE_TABLE_SMALL(db, root);
  CHECK_IS_PAGE(pt);
  CHECK(page_resize(db, root, pt->user1 * length));
  err: return err;
}


int array_length(const edb *const db, const u32 root, u64 *const size) {
  int err = 0;
  page_table_small* pt = PAGE_TABLE_SMALL(db, root);
  CHECK_IS_PAGE(pt);
  *size = pt->size / pt->user1;
  err: return err;
}


int array_capacity(const edb* db, const u32 root, u64 *const size) {
  int err = 0;
  page_table_small* pt = PAGE_TABLE_SMALL(db, root);
  CHECK_IS_PAGE(pt);
  if (pt->magic == EDB_MAGIC_PAGE_FULL) {
    *size = pt->size / pt->user1;
    return 0;
  }
  if (pt->magic == EDB_MAGIC_PAGE_SMALL) {
    *size = EDB_PAGE_SMALL_MAX_SIZE / pt->user1;
    return 0;
  }
  err: return err;
}


int array_get(const edb* db, const u32 root, const u64 index, void *const data) {
  int err = 0;
  page_table_small* pt = PAGE_TABLE_SMALL(db, root);
  CHECK_IS_PAGE(pt);
  CHECK(page_read(db, root, index * pt->user1, pt->user1, data));
  err: return err;
}


int array_set(edb* db, const u32 root, const u64 index, const void *const data) {
  int err = 0;
  page_table_small* pt = PAGE_TABLE_SMALL(db, root);
  CHECK_IS_PAGE(pt);
  CHECK(page_write(db, root, index * pt->user1, pt->user1, data));
  err: return err;
}


int array_push(edb* db, const u32 root, const void *const data) {
  int err = 0;
  page_table_small* pt = PAGE_TABLE_SMALL(db, root);
  CHECK_IS_PAGE(pt);
  CHECK(page_resize(db, root, pt->size + pt->user1));
  CHECK(page_write(db, root, pt->size - pt->user1, pt->user1, data));
  err: return err;
}


int array_pop(edb* db, const u32 root, void *const data) {
  int err = 0;
  page_table_small* pt = PAGE_TABLE_SMALL(db, root);
  CHECK_IS_PAGE(pt);
  CHECK(page_read(db, root, pt->size - pt->user1, pt->user1, data));
  CHECK(page_resize(db, root, pt->size - pt->user1));
  err: return err;
}
