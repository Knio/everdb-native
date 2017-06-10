#define CATCH_CONFIG_MAIN
#include "../../lib/catch.hpp"

#include "edb_mock.h"
#include "../../src/page.h"


TEST_CASE("page") {
  init_edb_mock();

  REQUIRE(sizeof(page_header) == 4);

  page_init(db, 0);
  u32* level1 = (u32*) db->data;
  u32* level2 = level1 + LEVEL_ONE;
  u32* nblocks = (u32*) (db->data + BLOCK_SIZE - sizeof(u32));
  u32 page;

  // grow from 0 to 1 page
  REQUIRE(page_resize(db, 0, 1) == 0);
  REQUIRE(*nblocks == 1);
  REQUIRE(level1[0] == 1023);
  REQUIRE(page_read_host_index(db, 0, 0, &page) == 0);
  REQUIRE(page == 1023);
  REQUIRE(page_read_host_index(db, 0, 1, &page) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(level1[1] == 0);
  REQUIRE(level1[2] == 0);
  REQUIRE(freelist.back() == 1022);

  // shrink from 1 to 0 pages
  REQUIRE(page_resize(db, 0, 0) == 0);
  REQUIRE(page_read_host_index(db, 0, 0, &page) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(page_read_host_index(db, 0, 1, &page) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(*nblocks == 0);
  REQUIRE(level1[0] == 0);
  REQUIRE(level1[1] == 0);
  REQUIRE(freelist.back() == 1023);

  // grow from 0 to 512 pages (the limit of single-level page tables)
  REQUIRE(page_resize(db, 0, 512) == 0);
  REQUIRE(*nblocks == 512);
  REQUIRE(page_read_host_index(db, 0, 0, &page) == 0);
  REQUIRE(page == 1023);
  REQUIRE(page_read_host_index(db, 0, 1, &page) == 0);
  REQUIRE(page == 1022);
  REQUIRE(page_read_host_index(db, 0, 510, &page) == 0);
  REQUIRE(page == 513);
  REQUIRE(page_read_host_index(db, 0, 511, &page) == 0);
  REQUIRE(page == 512);
  REQUIRE(page_read_host_index(db, 0, 512, &page) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(page_read_host_index(db, 0, 513, &page) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(level1[0] == 1023);
  REQUIRE(level1[1] == 1022);
  REQUIRE(level1[510] == 513);
  REQUIRE(level1[511] == 512);
  REQUIRE(level2[0] == 0);
  REQUIRE(level2[1] == 0);

  // grow from 512 to 513 pages (the limit of single-level page tables)
  // causes an allocation of a 2nd-level page table
  REQUIRE(page_resize(db, 0, 513) == 0);
  REQUIRE(*nblocks == 513);

  REQUIRE(level2[0] == 511);
  u32* index_page = (u32*) (db->data + BLOCK_SIZE * level2[0]);

  REQUIRE(page_read_host_index(db, 0, 512, &page) == 0);
  REQUIRE(page == 510);
  REQUIRE(page_read_host_index(db, 0, 513, &page) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(index_page[0] == 510);
  REQUIRE(index_page[1] == 0);
  REQUIRE(index_page[2] == 0);

  // shrink from 513 to 512 pages
  // causes the free of the 2nd-level page table
  REQUIRE(page_resize(db, 0, 512) == 0);
  REQUIRE(*nblocks == 512);
  REQUIRE(index_page[0] == 0);
  REQUIRE(index_page[1] == 0);
  REQUIRE(index_page[2] == 0);
  REQUIRE(level2[0] == 0);
  REQUIRE(level2[1] == 0);
  REQUIRE(level2[2] == 0);
  REQUIRE(freelist.back() == 511);

  // shrink from 512 to 511 pages
  REQUIRE(page_resize(db, 0, 511) == 0);
  REQUIRE(*nblocks == 511);
  REQUIRE(level1[510] == 513);
  REQUIRE(level1[511] == 0);
  REQUIRE(freelist.back() == 512);

  // shrink from 512 to 1 page
  REQUIRE(page_resize(db, 0, 1) == 0);
  REQUIRE(*nblocks == 1);
  REQUIRE(level1[0] == 1023);
  REQUIRE(level1[1] == 0);
  REQUIRE(level1[2] == 0);
  REQUIRE(freelist.back() == 1022);

  // shrink from 1 to 0 pages
  REQUIRE(page_resize(db, 0, 0) == 0);
  REQUIRE(level1[0] == 0);
  REQUIRE(freelist.back() == 1023);

  // grom from 0 to 1000 pages
  REQUIRE(page_resize(db, 0, 1000) == 0);
  REQUIRE(*nblocks == 1000);
  REQUIRE(level1[0] == 1023);
  REQUIRE(level1[1] == 1022);
  REQUIRE(level1[510] == 513);
  REQUIRE(level1[511] == 512);
  index_page = (u32*) (db->data + BLOCK_SIZE * level2[0]);
  REQUIRE(index_page[0] == 510);
  REQUIRE(index_page[1] == 509);
  REQUIRE(index_page[10] == 500);
  REQUIRE(index_page[486] == 24);
  REQUIRE(index_page[487] == 23);
  REQUIRE(index_page[488] == 0);
  REQUIRE(index_page[489] == 0);

  // shrink from 1000 to 0 pages
  REQUIRE(page_resize(db, 0, 0) == 0);
  REQUIRE(level1[0] == 0);
  REQUIRE(freelist.back() == 1023);

}
