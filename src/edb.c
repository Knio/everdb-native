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

#include "edb.h"

#include "util.h"
#include "io.h"
#include "txn.h"

#include "array.h"

int
edb_init(edb *const db);

int
edb_check(const edb *const db);


int
edb_open(edb **const dbp, const char *const fname, int readonly, int overwrite) {
  /*
  open everdb database file `fname`

  bool readonly   open db as read-only; all write ops will fail
                  db file must exist

  bool overwrite  create a new, empty database, even if fname already exists
                  existing db will be destroyed
                  readonly must be false
  */

  int err = 0;
  LOG_DEBUG("edb_open: %p from:%s readonly:%d overwrite:%d\n",
      dbp, fname, readonly, overwrite);

  edb *db = calloc(1, sizeof(edb));
  *dbp = db;

  CHECK(io_open(db, fname, readonly, overwrite));

  int is_new = 0;
  if (overwrite || (readonly = 0 && db->nblocks == 1)) {
    is_new = 1;
  }

  db->readonly = readonly;
  db->freelist = EDB_FREELIST_PRIMARY;
  db->objlist = EDB_OBJLIST;

  if (is_new) {
    CHECK(edb_init(db));
  }

  else {
    CHECK(edb_check(db));
  }

  return err;

  err:
  io_close(db);
  free(db);
  *dbp = NULL;

  return err;
}


int
edb_close(edb *const db) {
  int err = 0;
  LOG_DEBUG("edb_close: %p\n", db);

  CHECK_CODE(db->txn == NULL, EDB_ERROR_OPEN_TXN);
  CHECK(io_close(db));

  free(db);

  err:
  return err;
}


int
edb_check(const edb *const db) {
  return 0;
}


int
edb_init(edb *db) {
  LOG_HERE;
  int err = 0;
  CHECK(io_resize(db, EDB_USER_DATA));

  CHECK(array_init(db, db->freelist, sizeof(u32)));
  CHECK(array_init(db, db->objlist,  sizeof(u32)));

  err:
  return err;
}


int
edb_txn_begin(edb *db) {
  LOG_HERE;
  int err = 0;

  CHECK_CODE(db->readonly == 0, EDB_ERROR_READ_ONLY);
  CHECK(txn_begin(db));

  err:
  return err;
}


int
edb_txn_commit(edb *db) {
  LOG_HERE;
  int err = 0;

  CHECK_CODE(db->readonly == 0, EDB_ERROR_READ_ONLY);
  CHECK(txn_commit(db));

  err:
  return err;
}


int
edb_txn_abort(edb *db) {
  LOG_HERE;
  int err = 0;

  CHECK_CODE(db->readonly == 0, EDB_ERROR_READ_ONLY);
  CHECK(txn_abort(db));

  err:
  return err;
}


// block device api
int
edb_allocate_block(edb *const db, u32 *const new_block) {
  LOG_HERE;
  return txn_allocate_block(db, new_block);
}


int
edb_modify_block(edb *const db, u32 block, u32 *const new_block) {
  LOG_DEBUG("block:%d\n", block);
  int err = txn_modify_block(db, block, new_block);
  LOG_DEBUG("block:%d new_block:%d\n", block, *new_block);
  return err;
}


int
edb_free_block(edb *db, u32 block) {
  LOG_DEBUG("block:%d\n", block);
  return txn_free_block(db, block);
}




typedef struct obj_handle_t
{
  edb *db;
  u32 obj_id;
  u32 txn_id;
  u32 root;
} obj_handle;


static inline
int
obj_read(obj_handle* h, u32 *root) {
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
obj_create(edb *db, obj_handle **hp) {
  LOG_HERE;
  int err = 0;
  obj_handle* h = NULL;
  CHECK(edb_modify_block(db, db->objlist, &db->objlist));
  h = calloc(1, sizeof(obj_handle));
  *hp = h;
  h->db = db;
  h->txn_id = db->txn_id;
  h->obj_id = array_length(db, db->objlist);
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
edb_array_create(edb *db, obj_handle **hp, const u8 item_size) {
  LOG_HERE;
  int err = 0;
  CHECK(obj_create(db, hp));
  CHECK(array_init(db, (*hp)->root, item_size));

  err:
  return err;
}

int
edb_array_open(edb *db, obj_handle **hp, const u32 obj_id) {
  LOG_HERE;
  obj_handle *h = calloc(1, sizeof(obj_handle));
  *hp = h;

  h->db = db;
  h->obj_id = obj_id;
  // set txn_id to invalid so that the root will be populated
  h->txn_id = -1;
  return 0;
}


int edb_array_get(obj_handle *h, u32 index, void* data) {
  LOG_HERE;
  int err = 0;
  u32 root;
  CHECK(obj_read(h, &root));
  CHECK(array_get(h->db, root, index, data));
  err: return err;
}

int edb_array_set(obj_handle *h, u32 index, void* data) {
  LOG_HERE;
  int err = 0;
  u32 root;
  CHECK(obj_write(h, &root));
  CHECK(array_get(h->db, root, index, data));
  err: return err;
}

int edb_array_push(obj_handle *h, void* data) {
  LOG_HERE;
  int err = 0;
  u32 root;
  CHECK(obj_write(h, &root));
  CHECK(array_push(h->db, root, data));
  err: return err;
}

int edb_array_pop(obj_handle *h, void* data) {
  LOG_HERE;
  int err = 0;
  u32 root;
  CHECK(obj_write(h, &root));
  CHECK(array_pop(h->db, root, data));
  err: return err;
}

int edb_array_length(obj_handle *h, u32 *length) {
  LOG_HERE;
  int err = 0;
  u32 root;
  CHECK(obj_read(h, &root));
  *length = array_length(h->db, root);
  err: return err;
}

int edb_array_capacity(obj_handle *h, u32 *capacity) {
  LOG_HERE;
  int err = 0;
  u32 root;
  CHECK(obj_read(h, &root));
  *capacity = array_capacity(h->db, root);
  err: return err;
}


