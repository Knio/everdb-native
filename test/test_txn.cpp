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
  edb *db;
  edb_open(&db, "test.db", 0, 1);

  REQUIRE(db->txn == NULL);

  REQUIRE(txn_begin(db) == 0);
  REQUIRE(db->txn != NULL);
  REQUIRE(db->txn->next == NULL);

  // TODO nested transactions unimplimented
  // REQUIRE(txn_begin(db) == 0);
  // REQUIRE(db->txn != NULL);
  // REQUIRE(db->txn->next != NULL);

  // REQUIRE(txn_commit(db) == 0);
  // REQUIRE(db->txn != NULL);
  // REQUIRE(db->txn->next == NULL);

  // SECTION("abort") {

  //   REQUIRE(txn_abort(db) == 0);
  //   REQUIRE(db->txn == NULL);
  //   REQUIRE(txn_commit(db) != 0);
  //   REQUIRE(txn_abort(db) != 0);
  //   REQUIRE(edb_close(db) == 0);
  //   free(db);

  // }

  // SECTION("commit") {
    REQUIRE(txn_commit(db) == 0);
    REQUIRE(db->txn == NULL);
    REQUIRE(txn_commit(db) != 0);
    // REQUIRE(txn_abort(db) != 0);
    REQUIRE(edb_close(db) == 0);

    // delete db;
  // }


}

// TEST_CASE("txn_trivial_commit") {
//   edb *db = new edb;

//   REQUIRE(edb_open(db, "test.db", 0, 1) == 0);
//   REQUIRE(db->txn == NULL);

//   REQUIRE(txn_begin(db) == 0);
//   REQUIRE(io_resize(db, 10) == 0);
//   REQUIRE(db->nblocks == 10);

//   REQUIRE(txn_begin(db) == 0);
//   REQUIRE(io_resize(db, 8) == 0);
//   REQUIRE(txn_commit(db) == 0);

//   REQUIRE(db->nblocks == 8);

//   io_close(db);
// }

TEST_CASE("txn_trivial_abort") {
  edb *db;

  REQUIRE(edb_open(&db, "test.db", 0, 1) == 0);
  REQUIRE(db->txn == NULL);
  REQUIRE(db->nblocks == 4);

  REQUIRE(txn_begin(db) == 0);

  u32 new_block = 0;
  REQUIRE(edb_allocate_block(db, &new_block) == 0);
  REQUIRE(db->nblocks == 5);

  // TODO nested transactions

  REQUIRE(txn_abort(db) == 0);
  REQUIRE(db->nblocks == 4);

  REQUIRE(edb_close(db) == 0);

  // delete db;
}
