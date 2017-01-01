#include "../lib/catch.hpp"

#include "../src/edb.h"

#ifdef __linux__
#include <stdlib.h> //memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


TEST_CASE("array_api") {

  edb *db;

  REQUIRE(edb_open(&db, "test.db", 0, 1) == 0);

  u64 x, y;
  obj_handle* h;
  REQUIRE(edb_array_create(db, &h, sizeof(x)) == 3001);

  REQUIRE(edb_txn_begin(db) == 0);

  REQUIRE(edb_array_create(db, &h, sizeof(x)) == 0);

  REQUIRE(edb_array_length(h) == 0);
  REQUIRE(edb_array_capacity(h) == 510);

  x = 42;
  REQUIRE(edb_array_push(h, &x) == 0);
  REQUIRE(edb_array_length(h) == 1);
  REQUIRE(edb_array_capacity(h) == 510);
  REQUIRE(edb_array_get(h, 0, &y) == 0);
  REQUIRE(y == x);
  REQUIRE(edb_array_get(h, 1, &y) == 4101);

  x = 43;
  REQUIRE(edb_array_set(h, 0, &x) == 0);
  REQUIRE(edb_array_get(h, 0, &y) == 0);
  REQUIRE(y == x);

  REQUIRE(edb_array_pop(h, &y) == 0);
  REQUIRE(y == x);

  REQUIRE(edb_txn_commit(db) == 0);
  REQUIRE(edb_close(db) == 0);
}
