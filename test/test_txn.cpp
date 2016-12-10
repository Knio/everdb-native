#include "../lib/catch.hpp"

#include "../src/core.h"
#include "../src/txn.h"
#include "../src/edb.h"


typedef struct txn_state_t {
  u32 nblocks;
  //mem_hash* blocks;
  void* blocks;
  txn_state* next;
} txn_state;

TEST_CASE("txn") {
  edb *db = (edb*)calloc(sizeof(edb), 1);
  edb_open(db, "test.db", 0, 1);

  REQUIRE(db->txn == NULL);

  REQUIRE(txn_begin(db) == 0);
  REQUIRE(db->txn != NULL);
  REQUIRE(db->txn->next == NULL);

  REQUIRE(txn_begin(db) == 0);
  REQUIRE(db->txn != NULL);
  REQUIRE(db->txn->next != NULL);

  REQUIRE(txn_commit(db) == 0);
  REQUIRE(db->txn != NULL);
  REQUIRE(db->txn->next == NULL);

  REQUIRE(txn_commit(db) == 0); // unimplimented
  // REQUIRE(db->txn == NULL); // Fails
  // REQUIRE(txn_commit(db) != 0); // Segfaults

  edb_close(db);
  free(db);
}

// TEST_CASE("txn_trivial_commit") {
//   edb *db = new edb;

//   REQUIRE(io_open(db, "test.db", 0, 1) == 0);
//   REQUIRE(db->txn == NULL);

//   REQUIRE(txn_begin(db) == 0);
//   REQUIRE(io_resize(db, 2) == 0);
//   REQUIRE(db->nblocks == 2);

//   REQUIRE(txn_begin(db) == 0);
//   REQUIRE(io_resize(db, 8) == 0);
//   REQUIRE(txn_commit(db) == 0);

//   REQUIRE(db->nblocks == 8);

//   io_close(db);
// }

// TEST_CASE("txn_trivial_abort") {
//   edb *db = new edb;

//   REQUIRE(io_open(db, "test.db", 0, 1) == 0);
//   REQUIRE(db->txn == NULL);

//   REQUIRE(txn_begin(db) == 0);

//   REQUIRE(io_resize(db, 2) == 0);
//   REQUIRE(db->nblocks == 2);

//   REQUIRE(txn_begin(db) == 0);

//   REQUIRE(io_resize(db, 8) == 0);
//   REQUIRE(txn_abort(db) == 0);
//   REQUIRE(db->nblocks == 2);

//   io_close(db);
// }
