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

  u32 l;
  u64 x, y;
  obj_handle* h;

  // fail - no transaction
  REQUIRE(edb_array_create(db, &h, sizeof(x)) == 3001);
  REQUIRE(h == NULL);

  REQUIRE(edb_txn_begin(db) == 0);

  REQUIRE(edb_array_create(db, &h, sizeof(x)) == 0);
  REQUIRE(h != NULL);
  // REQUIRE(h->txn_id == 1);
  // REQUIRE(h->obj_id == 0);
  // REQUIRE(h->root == 4);

  INFO("empty array");
  REQUIRE(edb_array_length(h, &l) == 0);
  REQUIRE(l == 0);
  REQUIRE(edb_array_capacity(h, &l) == 0);
  REQUIRE(l == 510);

  // push 42
  x = 42;
  REQUIRE(edb_array_push(h, &x) == 0);
  REQUIRE(edb_array_length(h, &l) == 0);
  REQUIRE(l == 1);
  REQUIRE(edb_array_capacity(h, &l) == 0);
  REQUIRE(l == 510);
  REQUIRE(edb_array_get(h, 0, &y) == 0);
  REQUIRE(y == x);
  REQUIRE(edb_array_get(h, 1, &y) == 4101);

  // modify it to 43
  x = 43;
  REQUIRE(edb_array_set(h, 0, &x) == 0);
  REQUIRE(edb_array_get(h, 0, &y) == 0);
  REQUIRE(y == x);
  REQUIRE(edb_array_get(h, 1, &y) == 4101);

  // commit
  REQUIRE(edb_txn_commit(db) == 0);

  // no transaction - read only
  REQUIRE(edb_array_length(h, &l) == 0);
  REQUIRE(l == 1);
  REQUIRE(edb_array_get(h, 0, &y) == 0);
  REQUIRE(y == x);
  REQUIRE(edb_array_get(h, 1, &y) == 4101);

  // new transaction - pop and abort
  REQUIRE(edb_txn_begin(db) == 0);
  REQUIRE(edb_array_length(h, &l) == 0);
  REQUIRE(l == 1);
  REQUIRE(edb_array_get(h, 0, &y) == 0);
  REQUIRE(y == x);
  REQUIRE(edb_array_get(h, 1, &y) == 4101);


  REQUIRE(edb_array_length(h, &l) == 0);
  REQUIRE(l == 1);

  // pop
  REQUIRE(edb_array_pop(h, &y) == 0);
  REQUIRE(y == x);
  REQUIRE(edb_array_length(h, &l) == 0);
  REQUIRE(l == 0);

  // abort
  REQUIRE(edb_txn_abort(db) == 0);

  // 43 is back
  REQUIRE(edb_array_length(h, &l) == 0);
  REQUIRE(l == 1);
  REQUIRE(edb_array_get(h, 0, &y) == 0);
  REQUIRE(y == x);

  REQUIRE(edb_close(db) == 0);


}
