#include "../lib/catch.hpp"

#include "../src/edb.h"

#ifdef __linux__
#include <string.h> //memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


u64 file_size(edb *db) {
  #ifdef _WIN32

  LARGE_INTEGER f_size;
  if (!GetFileSizeEx(db->h_file, &f_size)) {
    return -1;
  }
  return f_size.QuadPart;

  #else

  struct stat fi;
  if (fstat(db->h_file, &fi) < 0) {
    return -1;
  }
  return fi.st_size;

  #endif
}

TEST_CASE("open") {
  edb *db = new edb;

  // open new db + overwrite
  REQUIRE(edb_open(db, "test.db", 0, 1) == 0);
  REQUIRE(db->filesize == 4096);

  SECTION("data") {

    // hacks
    memcpy(db->data, "test 123", 8);

    SECTION("save has same data") {
      // save it
      edb_close(db);

      // open it again and see if data still there
      REQUIRE(edb_open(db, "test.db", 0, 0) == 0);
      REQUIRE(db->filesize == 4096);
      REQUIRE(memcmp(db->data, "test 123", 8) == 0);
    }

  }

  SECTION("resize") {
      REQUIRE(edb_resize(db, 1) == 0);
      REQUIRE(db->nblocks == 1);
      REQUIRE(file_size(db) == BLOCK_SIZE);

      REQUIRE(edb_resize(db, 4) == 0);
      REQUIRE(db->nblocks == 4);
      REQUIRE(file_size(db) == BLOCK_SIZE * 4);

      REQUIRE(edb_resize(db, 1) == 0);
      REQUIRE(db->nblocks == 1);
      REQUIRE(file_size(db) == BLOCK_SIZE);
  }

  edb_close(db);

}
