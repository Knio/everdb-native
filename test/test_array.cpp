#include "../lib/catch.hpp"

#include "../src/io.h"
#include "../src/edb.h"
#include "../src/array.h"

#ifdef __linux__
#include <stdlib.h> //memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


TEST_CASE("array") {
  REQUIRE(sizeof(array_header) == 8);

  edb *db = new edb;

  // open new db + overwrite
  REQUIRE(edb_open(db, "test.db", 0, 1) == 0);
  REQUIRE(db->nblocks == 4);


  // REQUIRE(io_resize(db, 2) == 0);
  // REQUIRE(db->nblocks == 2);
  REQUIRE(edb_txn_begin(db) == 0);

  u64 x = 42, y = 43;

  u32 small_capacity = (
    (BLOCK_SIZE - sizeof(page_header) - sizeof(array_header)) / sizeof(u64)
  );

  array_init(db, 0, sizeof(u64));

  u8* pt = db->data;
  array_header *ah = (array_header*)
      (pt + BLOCK_SIZE - sizeof(page_header) - sizeof(array_header));
  REQUIRE(ah->item_size == 8);

  // array is length 0
  REQUIRE(array_length(db, 0) == 0);
  REQUIRE(array_capacity(db, 0) == small_capacity);
  // get & set fail
  REQUIRE(array_get(db, 0, 0, &x) == ERR_ARRAY_INDEX_OUT_OF_BOUNDS);
  REQUIRE(x == 42);
  REQUIRE(array_set(db, 0, 0, &x) == ERR_ARRAY_INDEX_OUT_OF_BOUNDS);
  REQUIRE(x == 42);

  // pop fails
  REQUIRE(array_pop(db, 0, &x) == ERR_ARRAY_INDEX_OUT_OF_BOUNDS);

  SECTION("resize") {

    // resize to 1
    REQUIRE(array_resize(db, 0, 1) == 0);

    REQUIRE(array_length(db, 0) == 0);
    REQUIRE(array_capacity(db, 0) == small_capacity);

    x = 42;
    // get & set [0] fail
    REQUIRE(array_get(db, 0, 0, &x) == ERR_ARRAY_INDEX_OUT_OF_BOUNDS);
    REQUIRE(x == 42);
    REQUIRE(array_set(db, 0, 0, &x) == ERR_ARRAY_INDEX_OUT_OF_BOUNDS);
    REQUIRE(x == 42);

    // close and open db
    edb_close(db);
    REQUIRE(edb_open(db, "test.db", 0, 0) == 0);
    REQUIRE(db->nblocks == 4);
    // REQUIRE(edb_txn_begin(db) == 0);

    pt = db->data;
    ah = (array_header*)
      (pt + BLOCK_SIZE - sizeof(page_header) - sizeof(array_header));

    REQUIRE(ah->item_size == 8);
    REQUIRE(array_length(db, 0) == 0);
    REQUIRE(array_capacity(db, 0) == small_capacity);
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

  SECTION("resize 2") {
    REQUIRE(array_resize(db, 0, 0) == 0);
    REQUIRE(array_length(db, 0) == 0);
    REQUIRE(array_capacity(db, 0) == small_capacity);

    REQUIRE(array_resize(db, 0, small_capacity) == 0);
    REQUIRE(array_length(db, 0) == 0);
    REQUIRE(array_capacity(db, 0) == small_capacity);

    REQUIRE(array_resize(db, 0, small_capacity + 1) == 0);
    REQUIRE(array_length(db, 0) == 0);
    REQUIRE(array_capacity(db, 0) == BLOCK_SIZE / sizeof(u64));
    // ah->length = (BLOCK_SIZE / sizeof(u64));

    // REQUIRE(array_resize(db, 0, small_capacity) == 0);
    // REQUIRE(array_length(db, 0) == small_capacity);
    // REQUIRE(array_capacity(db, 0) == small_capacity);
  }


  REQUIRE(edb_close(db) == 0);
  delete db;
}
