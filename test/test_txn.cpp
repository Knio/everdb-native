#include "../lib/catch.hpp"

#include "../src/edb.h"
#include "../src/txn.h"
#include "../src/page.h"
#include "../src/array.h"

typedef struct txn_state_t {
  u32 nblocks;
  //mem_hash* blocks;
  void* blocks;
  txn_state* next;
} txn_state;

TEST_CASE("txn") {
  edb *db = new edb;

  REQUIRE(edb_open(db, "test_txn.db", 0, 1) == 0);
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

  REQUIRE(txn_commit(db) == 0);
  REQUIRE(db->txn == NULL); // Fails
  REQUIRE(txn_commit(db) != 0); // Segfaults

  edb_close(db);
}

TEST_CASE("txn_trivial_commit") {
  edb *db = new edb;

  REQUIRE(edb_open(db, "test_txn_trivial_commit.db", 0, 1) == 0);
  REQUIRE(db->txn == NULL);

  REQUIRE(txn_begin(db) == 0);
  REQUIRE(edb_resize(db, 2) == 0);
  REQUIRE(db->nblocks == 2);

  REQUIRE(txn_begin(db) == 0);
  REQUIRE(edb_resize(db, 8) == 0);
  REQUIRE(txn_commit(db) == 0);

  REQUIRE(db->nblocks == 8);

  edb_close(db);
}

TEST_CASE("txn_trivial_abort") {
  edb *db = new edb;

  REQUIRE(edb_open(db, "test_txn_trivial_abort.db", 0, 1) == 0);
  REQUIRE(db->txn == NULL);

  REQUIRE(txn_begin(db) == 0);

  REQUIRE(edb_resize(db, 2) == 0);
  REQUIRE(db->nblocks == 2);

  REQUIRE(txn_begin(db) == 0);

  REQUIRE(edb_resize(db, 8) == 0);
  REQUIRE(txn_abort(db) == 0);
  REQUIRE(db->nblocks == 2);

  edb_close(db);
}
