#define CATCH_CONFIG_MAIN
#include "../lib/catch.hpp"

#include "../src/edb.h"
#include "../src/page.h"
#include "../src/array.h"

#ifdef __linux__
#include <string.h> //memcpy
#endif

TEST_CASE("open") {

  edb db;

  // open new db + overwrite
  REQUIRE(edb_open(&db, "test.db", 0, 1) == 0);
  REQUIRE(db.size == 4096);

  SECTION("data") {

    // hacks
    memcpy(db.data, "test 123", 8);

    SECTION("save has same data") {
      // save it
      edb_close(&db);

      // open it again and see if data still there
      REQUIRE(edb_open(&db, "test.db", 0, 0) == 0);
      REQUIRE(db.size == 4096);
      REQUIRE(memcmp(db.data, "test 123", 8) == 0);
    }

  }

  SECTION("resize") {
      REQUIRE(edb_allocate(&db, BLOCK_SIZE) == 0);
      REQUIRE(db.size == BLOCK_SIZE);
      REQUIRE(edb_allocate(&db, BLOCK_SIZE * 4) == 0);
      REQUIRE(db.size == BLOCK_SIZE * 4);
      REQUIRE(edb_allocate(&db, BLOCK_SIZE) == 0);
      REQUIRE(db.size == BLOCK_SIZE);
  }

  edb_close(&db);

}


TEST_CASE("array") {

  edb *db = new edb;

  // open new db + overwrite
  REQUIRE(edb_open(db, "test.db", 0, 1) == 0);
  REQUIRE(db->size == BLOCK_SIZE);

  REQUIRE(edb_allocate(db, BLOCK_SIZE * 2) == 0);
  REQUIRE(db->size == BLOCK_SIZE * 2);

  u64 x = 42, y = 43;
  array_init(db, 0, sizeof(u64));

  REQUIRE(sizeof(page_header) == 12);

  block page_table = BLOCK(db->data, 0);
  const array_header *ah = (array_header*)
      (page_table + BLOCK_SIZE - sizeof(page_header) - sizeof(array_header));
  REQUIRE(ah->item_size == 8);

  // array is length 0
  REQUIRE(array_length(db, 0) == 0);
  REQUIRE(array_capacity(db, 0) == 0);
  // get & set fail
  REQUIRE(array_get(db, 0, 0, &x) == 1);
  REQUIRE(x == 42);
  REQUIRE(array_set(db, 0, 0, &x) == 1);
  REQUIRE(x == 42);

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
  REQUIRE(db->size == BLOCK_SIZE * 2);

  REQUIRE(y == 42);
  page_table = BLOCK(db->data, 0);
  ah = (array_header*)
      (page_table + BLOCK_SIZE - sizeof(page_header) - sizeof(array_header));
  REQUIRE(ah->item_size == 8);
  REQUIRE(array_length(db, 0) == 1);
  REQUIRE(array_get(db, 0, 0, &y) == 0);

  edb_close(db);

}
