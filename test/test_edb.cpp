#define CATCH_CONFIG_MAIN
#include "../lib/catch.hpp"

#include "../src/edb.h"
extern "C" {
#include "../src/edb.c"
}

#ifdef __linux__
#include "string.h" //memcpy
#endif

TEST_CASE() {

  edb db;

  // open new db + overwrite
  REQUIRE(edb_open(&db, "test.db", 0, 1) == 0);
  REQUIRE(db.size == 4096);

  SECTION("Data") {

    // hacks
    memcpy(db.data, "test 123", 8);

    SECTION("Save has same data") {
      // save it
      edb_close(&db);

      // open it again and see if data still there
      REQUIRE(edb_open(&db, "test.db", 0, 0) == 0);
      REQUIRE(db.size == 4096);
      REQUIRE(memcmp(db.data, "test 123", 8) == 0);
    }

  }

  SECTION("Resize") {
    REQUIRE(edb_map(&db, 8192) == 0);
    REQUIRE(db.size == 8192);
  }

  edb_close(&db);

}
