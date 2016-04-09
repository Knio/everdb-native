#define CATCH_CONFIG_MAIN
#include "../../lib/catch.hpp"

#include "../../src/page.h"


#include <vector>
std::vector<u32> freelist;
extern "C" {

u32 edb_allocate_block(edb* db) {
  if (freelist.size()) {
    u32 block = freelist.back();
    freelist.pop_back();
    return block;
  }
  return 0;
}

int edb_free_block(edb* db, const u32 block) {
  freelist.push_back(block);
  return 0;
}
}


TEST_CASE("page") {
  REQUIRE(sizeof(page_header) == 4);

  edb *db = new edb;
  db->data = (char*) malloc(BLOCK_SIZE * 1024);
  for (u32 i=1; i<1024; i++) {
    freelist.push_back(i);
  }

  page_init(db, 0);
  u32* level1 = (u32*) db->data;
  u32* level2 = level1 + LEVEL_ONE;
  u32* nblocks = (u32*) (db->data + BLOCK_SIZE - sizeof(u32));

  REQUIRE(page_resize(db, 0, 1) == 0);
  REQUIRE(*nblocks == 1);
  REQUIRE(level1[0] == 1023);
  REQUIRE(freelist.back() == 1022);

  REQUIRE(page_resize(db, 0, 0) == 0);
  REQUIRE(*nblocks == 0);
  REQUIRE(level1[0] == 0);
  REQUIRE(freelist.back() == 1023);

  REQUIRE(page_resize(db, 0, 512) == 0);
  REQUIRE(*nblocks == 512);
  REQUIRE(level1[0] == 1023);
  REQUIRE(level1[1] == 1022);
  REQUIRE(level1[510] == 513);
  REQUIRE(level1[511] == 512);
  REQUIRE(level1[512] == 0);
  REQUIRE(level2[0] == 0);

  REQUIRE(page_resize(db, 0, 513) == 0);

  REQUIRE(level2[0] == 511);
  REQUIRE(*nblocks == 513);

  u32* index_page = (u32*) (db->data + BLOCK_SIZE * 511);

  REQUIRE(index_page[0] == 510);

}
