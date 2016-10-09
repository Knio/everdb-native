#include "../lib/catch.hpp"

#include "../src/edb.h"
#include "../src/array.h"

#ifdef __linux__
#include <stdlib.h> //memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


TEST_CASE("array") {
  REQUIRE(sizeof(array_header) == 12);

  edb *db = new edb;

  // open new db + overwrite
  REQUIRE(edb_open(db, "test.db", 0, 1) == 0);
  REQUIRE(db->nblocks == 1);

  REQUIRE(edb_resize(db, 2) == 0);
  REQUIRE(db->nblocks == 2);

  u64 x = 42, y = 43;
  array_init(db, 0, sizeof(u64));

  char* pt = BLOCK(db, 0);
  const array_header *ah = (array_header*)
      (pt + BLOCK_SIZE - sizeof(page_header) - sizeof(array_header));
  REQUIRE(ah->item_size == 8);

  // array is length 0
  REQUIRE(array_length(db, 0) == 0);
  REQUIRE(array_capacity(db, 0) == 0);
  // get & set fail
  REQUIRE(array_get(db, 0, 0, &x) == 1);
  REQUIRE(x == 42);
  REQUIRE(array_set(db, 0, 0, &x) == 1);
  REQUIRE(x == 42);

  SECTION("resize") {

    // resize to 1
    REQUIRE(array_resize(db, 0, 1) == 0);

    REQUIRE(array_length(db, 0) == 1);
    REQUIRE(array_capacity(db, 0) == (
      (BLOCK_SIZE - sizeof(page_header) - sizeof(array_header)) / sizeof(u64)
    ));

    // get and set element 0 work
    REQUIRE(array_get(db, 0, 0, &x) == 0);
    REQUIRE(x == 0);

    x = 42;
    REQUIRE(array_set(db, 0, 0, &x) == 0);
    REQUIRE(array_get(db, 0, 0, &y) == 0);
    REQUIRE(y == 42);

    // get and set element 1 fail
    REQUIRE(array_set(db, 0, 1, &x) == 1);
    REQUIRE(x == 42);
    REQUIRE(array_get(db, 0, 1, &x) == 1);
    REQUIRE(x == 42);

    // close and open db
    edb_close(db);
    REQUIRE(edb_open(db, "test.db", 0, 0) == 0);
    REQUIRE(db->nblocks == 2);

    pt = BLOCK(db, 0);
    ah = (array_header*)
      (pt + BLOCK_SIZE - sizeof(page_header) - sizeof(array_header));

    REQUIRE(ah->item_size == 8);
    REQUIRE(array_length(db, 0) == 1);
    REQUIRE(array_get(db, 0, 0, &y) == 0);
    REQUIRE(y == 42);
  }

  SECTION("push/pop") {
    x = 12;
    y = 19;
    REQUIRE(array_length(db, 0) == 0);
    REQUIRE(array_push(db, 0, &x) == 0);
    REQUIRE(array_length(db, 0) == 1);
    REQUIRE(array_pop(db, 0, &y) == 0);
    REQUIRE(array_length(db, 0) == 0);
    REQUIRE(y == 12);
  }

  edb_close(db);

}
