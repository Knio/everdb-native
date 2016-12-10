#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h> //mmap
#include <string.h> //memset
#else
#error Unsupported OS
#endif

#include "edb.h"

#include "util.h"
#include "math.h"
#include "io.h"
#include "txn.h"

#include "array.h"

int
edb_init(edb *const db);

int
edb_check(const edb *const db);


int
edb_open(edb *const db, const char *const fname, int readonly, int overwrite) {
  /*
  open everdb database file `fname`

  bool readonly   open db as read-only; all write ops will fail
                  db file must exist

  bool overwrite  create a new, empty database, even if fname already exists
                  existing db will be destroyed
                  readonly must be false
  */

  int err = 0;
  CHECK(io_open(db, fname, readonly, overwrite));

  int is_new = 0;
  if (overwrite || (readonly = 0 && db->nblocks == 1)) {
    is_new = 1;
  }

  if (is_new) {
    CHECK(edb_init(db));
  }

  else {
    CHECK(edb_check(db));
  }

  return 0;

  err:
  io_close(db);
  return err;
}


int
edb_close(edb *const db) {
  return io_close(db);
}


int
edb_check(const edb *const db) {
  return 0;
}


int
edb_init(edb *db) {
  int err = 0;
  CHECK(io_resize(db, 4));

  db->freelist = 1;
  db->objlist = 3;

  CHECK(array_init(db, db->freelist, sizeof(u32)));
  CHECK(array_init(db, db->objlist,  sizeof(u32)));

  // CHECK(txn_begin(db));

  err:
  return err;
}


int
edb_txn_begin(edb *db) {
  int err = 0;

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


int
edb_allocate_block(edb *const db, u32 *const new_block) {
  int err = 0;
  CHECK(txn_modify_block(db, db->freelist, &db->freelist));

  u32 length = array_length(db, db->freelist);

  if (length > 0) {
    CHECK(array_pop(db, db->freelist, new_block));
    return 0;
  }

  u32 nblocks = db->nblocks;
  u32 step = next_power_of_two(nblocks >> 3);

  // NOTE: after resizing, db->data has changed and all local pointers
  // need to be updated!

  CHECK(io_resize(db, (nblocks + step) & ~(step-1)));

  for (u32 i = nblocks + 1; i < db->nblocks; i++) {
    CHECK(array_push(db, db->freelist, &i));
  }
  *new_block = nblocks;
  return 0;

  err:
  return err;
}


int
edb_free_block(edb *db, u32 page) {
  int err = 0;

  CHECK(txn_modify_block(db, db->freelist, &db->freelist));
  CHECK(array_push(db, db->freelist, &page));

  err:
  return err;
}


u32
edb_obj_root(obj_handle *h) {
  int err = 0;

  if (h->db->txn_id != h->txn_id) {
    CHECK(array_get(h->db, h->db->objlist, h->obj_id, &h->root));
    h->txn_id = h->db->txn_id;
  }

  return h->root;

  err:
  return 0;
}



int edb_array_get(obj_handle *h, u32 index, void* data) {
  u32 block = edb_obj_root(h);
  return array_get(h->db, block, index, data);
}
