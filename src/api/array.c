#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h> // calloc, free
#include <sys/mman.h> //mmap
#include <string.h> //memset
#else
#error Unsupported OS
#endif

typedef struct obj_handle_t obj_handle, edb_array;

#define EDB_H_IMPL
#include "../../include/edb.h"
#include "../util.h"
#include "../io.h"
#include "../txn.h"
#include "../array.h"


typedef struct obj_handle_t
{
  edb *db;
  u32 obj_id;
  u32 txn_id;
  u32 root;
} obj_handle, edb_array;


static inline int obj_read(obj_handle* h, u32 *root) {
  LOG_HERE;
  int err = 0;

  if (h->db->txn_id != h->txn_id) {
    LOG_DEBUG("upgrading old obj id:%d txn_old:%d txn_new:%d\n",
      h->obj_id, h->txn_id, h->db->txn_id);
    CHECK(array_get(h->db, h->db->objlist, h->obj_id, &h->root));
    h->txn_id = h->db->txn_id;
  }

  *root = h->root;
  return 0;

  err:
  *root = -1;
  return err;
}


static inline
int
obj_write(obj_handle* h, u32 *root) {
  LOG_HERE;
  int err = 0;

  if (h->db->txn_id != h->txn_id) {
    LOG_DEBUG("upgrading old obj id:%d txn_old:%d txn_new:%d\n",
      h->obj_id, h->txn_id, h->db->txn_id);
    CHECK(array_get(h->db, h->db->objlist, h->obj_id, &h->root));
    h->txn_id = h->db->txn_id;
  }

  CHECK(edb_modify_block(h->db, h->root, &h->root));
  *root = h->root;
  return 0;

  err:
  *root = -1;
  return err;
}


int
obj_create(edb *const db, obj_handle **const hp) {
  LOG_HERE;
  int err = 0;
  obj_handle* h = NULL;
  CHECK(edb_modify_block(db, db->objlist, &db->objlist));
  h = calloc(1, sizeof(obj_handle));
  *hp = h;
  h->db = db;
  h->txn_id = db->txn_id;
  u64 length = 0;
  CHECK(array_length(db, db->objlist, &length));
  h->obj_id = (u32)length;
  CHECK(edb_allocate_block(db, &h->root));
  LOG_DEBUG("created obj id:%d txn:%d objlist:%d root:%d\n",
      h->obj_id, h->txn_id, db->objlist, h->root);
  CHECK(array_push(db, db->objlist, &h->root));
  return 0;

  err:
  *hp = NULL;
  if (h != NULL) {
    if (h->root != 0) {
    int r = edb_free_block(db, h->root);
    if (r) { abort(); }
    }
    free(h);
  }
  return err;
}


int
edb_array_create(edb *const db, edb_array **const hp, const u8 item_size) {
  LOG_HERE;
  int err = 0;
  CHECK(obj_create(db, hp));
  CHECK(array_init(db, (*hp)->root, item_size));

  err:
  return err;
}

int
edb_array_open(edb *const db, edb_array **const hp, const u32 obj_id) {
  LOG_HERE;
  obj_handle *h = calloc(1, sizeof(obj_handle));
  *hp = h;

  h->db = db;
  h->obj_id = obj_id;
  // set txn_id to invalid so that the root will be populated
  h->txn_id = -1;
  return 0;
}


int edb_array_get(edb_array *const h, const u32 index, void *const data) {
  LOG_HERE;
  int err = 0;
  u32 root;
  CHECK(obj_read(h, &root));
  CHECK(array_get(h->db, root, index, data));
  err: return err;
}


int edb_array_set(edb_array *const h, const u32 index, const void *const data) {
  LOG_HERE;
  int err = 0;
  u32 root;
  CHECK(obj_write(h, &root));
  CHECK(array_set(h->db, root, index, data));
  err: return err;
}


int edb_array_push(edb_array *const h, const void *const data) {
  LOG_HERE;
  int err = 0;
  u32 root;
  CHECK(obj_write(h, &root));
  CHECK(array_push(h->db, root, data));
  err: return err;
}


int edb_array_pop(edb_array *const h, void *const data) {
  LOG_HERE;
  int err = 0;
  u32 root;
  CHECK(obj_write(h, &root));
  CHECK(array_pop(h->db, root, data));
  err: return err;
}


int edb_array_length(edb_array *const h, u64 *const length) {
  LOG_HERE;
  int err = 0;
  u32 root;
  CHECK(obj_read(h, &root));
  CHECK(array_length(h->db, root, length));
  err: return err;
}


int edb_array_capacity(edb_array *const h, u64 *const capacity) {
  LOG_HERE;
  int err = 0;
  u32 root;
  CHECK(obj_read(h, &root));
  CHECK(array_capacity(h->db, root, capacity));
  err: return err;
}
