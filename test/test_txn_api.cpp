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
  REQUIRE(db->txn_id == 0);

  REQUIRE(edb_txn_commit(db) == EDB_ERR_TXN_NO_TRANSACTION);
  REQUIRE(edb_txn_abort(db) == EDB_ERR_TXN_NO_TRANSACTION);

  REQUIRE(edb_txn_begin(db) == 0);
  REQUIRE(db->txn != NULL);
  REQUIRE(db->txn->next == NULL);
  REQUIRE(db->txn_id == 1);

  // TODO nested transactions unimplimented

  SECTION("abort") {
    REQUIRE(edb_txn_abort(db) == 0);
    REQUIRE(db->txn == NULL);
    REQUIRE(edb_txn_commit(db) == EDB_ERR_TXN_NO_TRANSACTION);
    REQUIRE(edb_txn_abort(db) == EDB_ERR_TXN_NO_TRANSACTION);
    REQUIRE(edb_close(db) == 0);
  }

  SECTION("commit") {
    REQUIRE(edb_txn_commit(db) == 0);
    REQUIRE(db->txn == NULL);
    REQUIRE(edb_txn_commit(db) == EDB_ERR_TXN_NO_TRANSACTION);
    REQUIRE(edb_txn_abort(db) == EDB_ERR_TXN_NO_TRANSACTION);
    REQUIRE(edb_close(db) == 0);
  }

}

// TEST_CASE("txn_trivial_commit") {
//   edb *db = new edb;

//   REQUIRE(edb_open(db, "test.db", 0, 1) == 0);
//   REQUIRE(db->txn == NULL);

//   REQUIRE(edb_txn_begin(db) == 0);
//   REQUIRE(io_resize(db, 10) == 0);
//   REQUIRE(db->nblocks == 10);

//   REQUIRE(edb_txn_begin(db) == 0);
//   REQUIRE(io_resize(db, 8) == 0);
//   REQUIRE(edb_txn_commit(db) == 0);

//   REQUIRE(db->nblocks == 8);

//   io_close(db);
// }

TEST_CASE("txn_trivial_abort") {
  edb *db;

  REQUIRE(edb_open(&db, "test.db", 0, 1) == 0);
  REQUIRE(db->txn == NULL);
  REQUIRE(db->nblocks == 4);

  REQUIRE(edb_txn_begin(db) == 0);

  u32 new_block = 0;
  REQUIRE(edb_allocate_block(db, &new_block) == 0);
  REQUIRE(db->nblocks == 5);

  // TODO nested transactions



  REQUIRE(edb_txn_abort(db) == 0);
  REQUIRE(db->nblocks == 4);

  REQUIRE(edb_close(db) == 0);

}
