#include "../lib/catch.hpp"

#include "../src/edb.h"
#include "../src/txn.h"

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
}
