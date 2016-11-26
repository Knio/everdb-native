#include "edb.h"
#include "io.h"
#include "math.h"
#include "array.h"
#include "txn.h"

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


int
edb_init(edb *const db);

int
edb_open(edb *const db, const char *const fname, int readonly, int overwrite) {
  int err = 0;
  if ((err = io_open(db, fname, readonly, overwrite))) {
    goto err;
  }


  err:
  return err;
}


void
edb_close(edb *const db) {
  io_close(db);
}


int edb_allocate_block(edb *const db, u32 *const new_block) {
  int err = 0;
  u32 length = array_length(db, db->freelist);

  if (length > 0) {
    if ((err = array_pop(db, db->freelist, new_block))) {
      goto err;
    }
    return 0;
  }

  u32 nblocks = db->nblocks;
  u32 step = next_power_of_two(nblocks >> 3);

  // NOTE: after resizing, db->data has changed and all local pointers
  // need to be updated!
  if ((err = io_resize(db, (nblocks + step) & ~(step-1)))) {
    goto err;
  }

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

