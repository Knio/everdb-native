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

  long q = 100;
  REQUIRE(array_push(db, ar, &q) == 0);
  q = 200;
  REQUIRE(array_push(db, ar, &q) == 0);
  q = -1;
  REQUIRE(array_pop(db, ar, &q) == 0);
  REQUIRE(q == 200);

  REQUIRE(array_pop(db, ar, &q) == 0);
  REQUIRE(q == 100);

  // array underrun
  REQUIRE(array_pop(db, ar, &q) == ERR_ARRAY_INDEX_OUT_OF_BOUNDS);

  const long N = 1000000;

  for (long i=0; i<N; i++) {
    REQUIRE(array_push(db, ar, &i) == 0);
    REQUIRE(array_length(db, ar) == i + 1);
  }

  for (long i=0; i<N; i++) {
    REQUIRE(array_get(db, ar, i, &q) == 0);
    REQUIRE(i == q);
  }

  for (long i=N-1; i>=0; i--) {
    REQUIRE(array_pop(db, ar, &q) == 0);
    REQUIRE(q == i);
    REQUIRE(array_length(db, ar) == i);
  }

}
