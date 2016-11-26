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
#include "io.h"
#include "math.h"
#include "array.h"
#include "txn.h"

#define FREELIST (1)
#define OBJLIST (2)


int
edb_init(edb *const db);

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


void
edb_close(edb *const db) {
  io_close(db);
}


int edb_init(edb *db) {
  int err = 0;
  CHECK(io_resize(db, 3));

  CHECK(array_init(db, FREELIST, sizeof(u32)));
  CHECK(array_init(db, OBJLIST,  sizeof(u32)));

  db->freelist = FREELIST;
  db->objlist = OBJLIST;

  err:
  return err;
}


int edb_check(edb *db) {

}


int edb_allocate_block(edb *const db, u32 *const new_block) {
  int err = 0;
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
    array_push(db, db->freelist, &i);
  }
  *new_block = nblocks;
  return 0;

  err:
  return err;
}

int edb_free_block(edb *db, u32 page) {
  return array_push(db, db->freelist, &page);
}

