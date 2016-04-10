#include "../lib/catch.hpp"

#include "../src/edb.h"
#include "../src/array.h"

#ifdef __linux__
#include <string.h> //memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif




TEST_CASE("integration") {
  edb *db = new edb;

  // open new db + overwrite
  REQUIRE(edb_open(db, "test.db", 0, 1) == 0);
  REQUIRE(db->nblocks == 1);

  db->freelist = 0;
  REQUIRE(array_init(db, 0, sizeof(u32)) == 0);

  u32 ar = edb_allocate_block(db);
  REQUIRE(ar != 0);
  REQUIRE(array_init(db, ar, sizeof(long)) == 0);

  for (long i=0; i<999999; i++) {
    REQUIRE(array_push(db, ar, &i) == 0);
  }

}


