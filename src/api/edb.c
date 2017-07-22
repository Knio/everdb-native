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

#include "../../include/edb.h"
#include "../util.h"
#include "../io.h"
#include "../txn.h"
#include "../array.h"


static int edb_check(const edb *const db) {
  return 0;
}


static int edb_init(edb *db) {
  LOG_HERE;
  int err = 0;
  CHECK(io_resize(db, EDB_USER_DATA));
  CHECK(array_init(db, db->freelist, sizeof(u32)));
  CHECK(array_init(db, db->objlist,  sizeof(u32)));
  err:
  return err;
}


int edb_open(edb **const dbp, const char *const fname, int readonly, int overwrite) {
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
  int is_new = (overwrite || (readonly = 0 && db->nblocks == 1));

  db->readonly = readonly;
  db->freelist = EDB_FREELIST_PRIMARY;
  db->objlist = EDB_OBJLIST;

  if (is_new) {
    CHECK(edb_init(db));
  } else {
    CHECK(edb_check(db));
  }
  return 0;

  err:
  io_close(db);
  free(db);
  *dbp = NULL;
  return err;
}


int edb_close(edb *const db) {
  int err = 0;
  LOG_DEBUG("edb_close: %p\n", db);

  CHECK_CODE(db->txn == NULL, EDB_ERROR_OPEN_TXN);
  CHECK(io_close(db));

  free(db);

  err:
  return err;
}


int edb_txn_begin(edb *db) {
  LOG_HERE;
  int err = 0;
  CHECK_CODE(db->readonly == 0, EDB_ERROR_READ_ONLY);
  CHECK(txn_begin(db));
  err:
  return err;
}


int edb_txn_commit(edb *db) {
  LOG_HERE;
  int err = 0;
  CHECK_CODE(db->readonly == 0, EDB_ERROR_READ_ONLY);
  CHECK(txn_commit(db));
  err:
  return err;
}


int edb_txn_abort(edb *db) {
  LOG_HERE;
  int err = 0;
  CHECK_CODE(db->readonly == 0, EDB_ERROR_READ_ONLY);
  CHECK(txn_abort(db));
  err:
  return err;
}

