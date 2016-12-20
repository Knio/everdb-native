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

  // CHECK_CODE(db->h_file     == NULL,  EDB_ERROR_ALREADY_OPEN);
  // CHECK_CODE(db->h_mapping  == NULL,  EDB_ERROR_ALREADY_OPEN);
  // CHECK_CODE(db->data       == NULL,  EDB_ERROR_ALREADY_OPEN);
  // CHECK_CODE(db->filesize   == 0,     EDB_ERROR_ALREADY_OPEN);
  // CHECK_CODE(db->nblocks    == 0,     EDB_ERROR_ALREADY_OPEN);
  // CHECK_CODE(db->freelist   == 0,     EDB_ERROR_ALREADY_OPEN);
  // CHECK_CODE(db->objlist    == 0,     EDB_ERROR_ALREADY_OPEN);
  // CHECK_CODE(db->txn_id     == 0,     EDB_ERROR_ALREADY_OPEN);

  CHECK(io_open(db, fname, readonly, overwrite));

  int is_new = 0;
  if (overwrite || (readonly = 0 && db->nblocks == 1)) {
    is_new = 1;
  }

  db->readonly = readonly;
  db->freelist = 1;
  db->objlist = 3;

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
  int err = 0;
  CHECK(io_resize(db, 4));

  CHECK(array_init(db, db->freelist, sizeof(u32)));
  CHECK(array_init(db, db->objlist,  sizeof(u32)));

  err:
  return err;
}


int
edb_txn_begin(edb *db) {
  int err = 0;

  CHECK_CODE(db->readonly == 0, EDB_ERROR_READ_ONLY);
  CHECK(txn_begin(db));

  err:
  return err;
}


int
edb_txn_commit(edb *db) {
  int err = 0;

  CHECK(txn_commit(db));

  err:
  return err;
}


// blocl device api
int
edb_allocate_block(edb *const db, u32 *const new_block) {
  return txn_allocate_block(db, new_block);
}


int
edb_modify_block(edb *const db, u32 block, u32 *const new_block) {
  return txn_modify_block(db, block, new_block);
}


int
edb_free_block(edb *db, u32 block) {
  return txn_free_block(db, block);
}



static inline
u32
edb_obj_root(obj_handle* h) {
  int err = 0;

  if (h->db->txn_id != h->txn_id) {
    CHECK(array_get(h->db, h->db->objlist, h->obj_id, &h->root));
    h->txn_id = h->db->txn_id;
  }

  return h->root;

  err:
  return 0xfffffffa;
}



int edb_array_create(edb *db, obj_handle **hp, const u8 item_size) {
  int err = 0;
  obj_handle* h = calloc(1, sizeof(obj_handle));
  *hp = h;

  h->db = db;
  h->txn_id = db->txn_id;
  h->obj_id = array_length(db, db->objlist);
  CHECK(edb_allocate_block(db, &h->root));
  CHECK(array_init(db, h->root, item_size));

  err:
  return err;
}


int edb_array_get(obj_handle *h, u32 index, void* data) {
  u32 block = edb_obj_root(h);
  return array_get(h->db, block, index, data);
}

int edb_array_set(obj_handle *h, u32 index, void* data) {
  u32 block = edb_obj_root(h);
  return array_get(h->db, block, index, data);
}



















