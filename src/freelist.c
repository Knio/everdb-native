#include "freelist.h"
#include "edb.h"
#include "io.h"
#include "array.h"
#include "util.h"
#include "math.h"

int txn_modify_block(edb *db, u32 block, u32 *new_block);

int
freelist_allocate_block(edb* db, u32* new_block) {
  int err = 0;
  CHECK(edb_modify_block(db, db->freelist, &db->freelist));

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
freelist_free_block(edb *db, u32 block) {
  int err = 0;

  CHECK(txn_modify_block(db, db->freelist, &db->freelist));
  CHECK(array_push(db, db->freelist, &block));

  err:
  return err;

}
