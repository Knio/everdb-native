#define CATCH_CONFIG_MAIN
#include "../../lib/catch.hpp"

#include "edb_mock.h"
#include "../../src/page.h"

TEST_CASE("static") {
  REQUIRE(sizeof(page_table_small) == BLOCK_SIZE);
  REQUIRE(sizeof(page_table_full) == BLOCK_SIZE);
  REQUIRE(sizeof(page_table_index) == BLOCK_SIZE);
  REQUIRE(INDEX_SIZE == EDB_PAGE_DATA_BLOCKS_PER_INDEX);
  REQUIRE(INDEX_SIZE == 1024);
}

TEST_CASE("page init") {
  init_edb_mock();
  modified[0] = 1;

  page_table_small EMPTY_PAGE = {
    .magic = 3, .user1 = 0, .user2 = 0, .user3 = 0,
  };
  REQUIRE(page_init(db, 0) == 0);
  REQUIRE(memcmp(db->data, (u8*)(&EMPTY_PAGE), BLOCK_SIZE) == 0);
}

TEST_CASE("resize small") {
  init_edb_mock();
  modified[0] = 1;
  page_table_small& pt = *(page_table_small*)db->data;
  pt = {
    .magic = 3, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 0,
    .size = 0,
  };
  page_table_small EXPECTED = {
    .magic = 3, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 0,
    .size = 13,
  };
  REQUIRE(page_resize(db, 0, 13) == 0);
  REQUIRE(memcmp(&pt, &EXPECTED, BLOCK_SIZE) == 0);
  REQUIRE(page_resize(db, 0, EDB_PAGE_SMALL_MAX_SIZE + 1) == EDB_ERR_PAGE_TOO_LARGE);
  REQUIRE(page_resize(db, 0, EDB_PAGE_SMALL_MAX_SIZE) == 0);
  EXPECTED.size = EDB_PAGE_SMALL_MAX_SIZE;
  REQUIRE(memcmp(&pt, &EXPECTED, BLOCK_SIZE) == 0);
}

TEST_CASE("resize full") {
  init_edb_mock();
  modified[0] = 1;
  page_table_full& pt = *(page_table_full*)db->data;
  pt = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 1,
    .size = 0,
    .data_blocks = {1023},
  };
  page_table_full EXPECTED = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 1,
    .size = 13,
    .data_blocks = {1023},
  };
  REQUIRE(page_resize(db, 0, 13) == 0);
  CAPTURE(pt.magic);
  CAPTURE(pt.nblocks);
  CAPTURE(pt.size);
  CAPTURE(pt.data_blocks[0]);
  REQUIRE(memcmp(&pt, &EXPECTED, BLOCK_SIZE) == 0);
  // REQUIRE(page_resize(db, 0, EDB_PAGE_FULL_MAX_SIZE + 1) == EDB_ERR_PAGE_FULL);
  // REQUIRE(page_resize(db, 0, BLOCK_SIZE) == 0);
  // EXPECTED.size = BLOCK_SIZE;
  // REQUIRE(memcmp(&pt, &EXPECTED, BLOCK_SIZE) == 0);
}


TEST_CASE("read small") {
  init_edb_mock();
  page_table_small& pt = *(page_table_small*)db->data;
  pt = {
    .magic = 3, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 0,
    .size = 13,
  };
  memcpy(pt.data, "\0Hello World!", 13);
  char data[512];
  REQUIRE(page_read(db, 0, 1, 12, (u8*)data) == 0);
  REQUIRE(page_read(db, 0, 1, 13, (u8*)data) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(memcmp(data, "Hello World!", 12) == 0);
}


TEST_CASE("write small") {
  init_edb_mock();
  modified[0] = 1;
  page_table_small& pt = *(page_table_small*)db->data;
  pt = {
    .magic = 3, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 0,
    .size = 0,
  };
  page_table_small EXPECTED = {
    .magic = 3, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 0,
    .size = 13,
  };
  memcpy(EXPECTED.data, "\0Hello World!", 13);

  REQUIRE(page_resize(db, 0, 13) == 0);
  REQUIRE(page_write(db, 0, 1, 12, (const u8*)"Hello World.") == 0);
  REQUIRE(page_write(db, 0, 1, 13, (const u8*)"Hello World!!") == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(page_write(db, 0, 12, 1, (const u8*)"!") == 0);
  CAPTURE(pt.magic);
  CAPTURE(pt.nblocks);
  CAPTURE(pt.size);
  CAPTURE((const char*)&pt.data[1]);
  REQUIRE(memcmp(&pt, &EXPECTED, BLOCK_SIZE) == 0);
}


TEST_CASE("read full") {
  init_edb_mock();
  page_table_full& pt = *(page_table_full*)db->data;
  pt = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 1,
    .size = 13,
    .data_blocks = {1023},
  };
  memcpy(db->data + (1023 * BLOCK_SIZE), "\0Hello World!", 13);
  char data[512];
  REQUIRE(page_read(db, 0, 1, 12, (u8*)data) == 0);
  REQUIRE(page_read(db, 0, 1, 13, (u8*)data) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(memcmp(data, "Hello World!", 12) == 0);
}


TEST_CASE("write full") {
  init_edb_mock();
  modified[0] = 1;
  page_table_full& pt = *(page_table_full*)db->data;
  pt = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 1,
    .size = 0,
    .data_blocks = {1023},
  };
  page_table_full EXPECTED = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 1,
    .size = 13,
    .data_blocks = {1023},
  };

  REQUIRE(page_resize(db, 0, 13) == 0);
  REQUIRE(page_write(db, 0, 1, 12, (const u8*)"Hello World.") == 0);
  REQUIRE(page_write(db, 0, 1, 13, (const u8*)"Hello World!!") == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(page_write(db, 0, 12, 1, (const u8*)"!") == 0);
  CAPTURE(pt.magic);
  CAPTURE(pt.nblocks);
  CAPTURE(pt.size);
  REQUIRE(memcmp(&pt, &EXPECTED, BLOCK_SIZE) == 0);
  REQUIRE(memcmp(db->data + (1023 * BLOCK_SIZE), "\0Hello World!", 13) == 0);
}


TEST_CASE("convert empty full to small") {
  init_edb_mock();
  modified[0] = 1;
  page_table_full& pt = *(page_table_full*)db->data;
  pt = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 0,
    .size = 0,
  };
  page_table_full expected = {
    .magic = 3, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 0,
    .size = 0,
  };
  REQUIRE(page_convert_to_small(db, 0) == 0);
  REQUIRE(memcmp(&pt, &expected, BLOCK_SIZE) == 0);
}


TEST_CASE("convert empty small to full") {
  init_edb_mock();
  modified[0] = 1;
  page_table_full& pt = *(page_table_full*)db->data;
  pt = {
    .magic = 3, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 0,
    .size = 0,
  };
  page_table_full expected = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 0,
    .size = 0,
    .data_blocks = {0}
  };
  REQUIRE(page_convert_to_full(db, 0) == 0);
  CAPTURE(pt.magic);
  CAPTURE(pt.nblocks);
  CAPTURE(pt.size);
  REQUIRE(memcmp(&pt, &expected, BLOCK_SIZE) == 0);
}


TEST_CASE("grow from 0 to 1") {
  init_edb_mock();
  modified[0] = 1;

  page_table_full& pt = *(page_table_full*)db->data;

  pt = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 0,
    .size = 0,
  };

  page_table_full expected = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 1,
    .size = 0,
    .data_blocks = {0}
  };
  expected.data_blocks[0] = freelist.back();
  REQUIRE(page_reallocate(db, 0, 1) == 0);
  CAPTURE(pt.magic);
  CAPTURE(pt.nblocks);
  CAPTURE(pt.data_blocks[0]);
  REQUIRE(memcmp(&pt, &expected, BLOCK_SIZE) == 0);
}


TEST_CASE("shrink from 1 to 0") {
  init_edb_mock();
  modified[0] = 1;

  page_table_full& pt = *(page_table_full*)db->data;

  pt = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 1,
    .size = 0,
    .data_blocks = {0}
  };
  u32 block;
  edb_allocate_block(db, &block);
  pt.data_blocks[0] = block;

  page_table_full expected = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 0,
    .size = 0,
    .data_blocks = {0}
  };
  REQUIRE(page_reallocate(db, 0, 0) == 0);
  CAPTURE(pt.magic);
  CAPTURE(pt.nblocks);
  CAPTURE(pt.data_blocks[0]);
  REQUIRE(memcmp(&pt, &expected, BLOCK_SIZE) == 0);
  REQUIRE(freelist.back() == block);
}


TEST_CASE("grow from 510 to 511") {
  init_edb_mock();
  modified[0] = 1;

  page_table_full& pt = *(page_table_full*)db->data;
  pt = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 510,
    .size = 0,
  };

  page_table_full expected = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 511,
    .size = 0,
    .data_blocks = {0},
    .index_blocks = {0}
  };
  expected.index_blocks[0] = freelist.back();
  REQUIRE(page_reallocate(db, 0, 511) == 0);
  CAPTURE(pt.magic);
  CAPTURE(pt.nblocks);
  CAPTURE(pt.index_blocks[0]);
  REQUIRE(memcmp(&pt, &expected, BLOCK_SIZE) == 0);
  page_table_index& ip = *(page_table_index*)(db->data + BLOCK_SIZE * expected.index_blocks[0]);
  REQUIRE(ip.data_blocks[0] == 1022);
}


TEST_CASE("shrink from 511 to 510") {
  init_edb_mock();
  modified[0] = 1;

  page_table_full& pt = *(page_table_full*)db->data;
  pt = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 511,
    .size = 0,
  };
  u32 block;
  edb_allocate_block(db, &block);
  page_table_index& ip = *(page_table_index*)(db->data + BLOCK_SIZE * block);
  pt.index_blocks[0] = block;
  ip = {0};
  ip.data_blocks[0] = 9999;

  page_table_full expected = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = 510,
    .size = 0,
    .data_blocks = {0},
    .index_blocks = {0}
  };
  REQUIRE(page_reallocate(db, 0, 510) == 0);
  CAPTURE(pt.magic);
  CAPTURE(pt.nblocks);
  CAPTURE(pt.index_blocks[0]);
  REQUIRE(memcmp(&pt, &expected, BLOCK_SIZE) == 0);
  REQUIRE(ip.data_blocks[0] == 0);
  REQUIRE(freelist.back() == block);
  freelist.pop_back();
  REQUIRE(freelist.back() == 9999);
}


TEST_CASE("grow from max") {
  init_edb_mock();
  modified[0] = 1;

  page_table_full& pt = *(page_table_full*)db->data;
  pt = {
    .magic = 4, .user1 = 0, .user2 = 0, .user3 = 0,
    .nblocks = EDB_PAGE_MAX_BLOCKS,
  };

  REQUIRE(page_reallocate(db, 0, EDB_PAGE_MAX_BLOCKS + 1) == EDB_ERR_PAGE_FULL);
}


TEST_CASE("page") {
  init_edb_mock();
  modified[0] = 1;

  REQUIRE(page_init(db, 0) == 0);
  REQUIRE(page_convert_to_full(db, 0) == 0);
  page_table_full& pt = *(page_table_full*)db->data;
  u32 page;

  // grow from 0 to 1 page
  REQUIRE(page_reallocate(db, 0, 1) == 0);
  REQUIRE(pt.nblocks == 1);
  REQUIRE(pt.data_blocks[0] == 1023);
  REQUIRE(page_get_block(db, 0, 0, &page) == 0);
  REQUIRE(page == 1023);
  REQUIRE(page_get_block(db, 0, 1, &page) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(pt.data_blocks[1] == 0);
  REQUIRE(pt.data_blocks[2] == 0);
  REQUIRE(freelist.back() == 1022);

  // shrink from 1 to 0 pages
  REQUIRE(page_reallocate(db, 0, 0) == 0);
  REQUIRE(page_get_block(db, 0, 0, &page) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(page_get_block(db, 0, 1, &page) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(pt.nblocks == 0);
  REQUIRE(pt.data_blocks[0] == 0);
  REQUIRE(pt.data_blocks[1] == 0);
  REQUIRE(freelist.back() == 1023);

  // grow from 0 to 512 pages (the limit of single-level page tables)
  REQUIRE(page_reallocate(db, 0, 510) == 0);
  REQUIRE(pt.nblocks == 510);
  REQUIRE(page_get_block(db, 0, 0, &page) == 0);
  REQUIRE(page == 1023);
  REQUIRE(page_get_block(db, 0, 1, &page) == 0);
  REQUIRE(page == 1022);
  REQUIRE(page_get_block(db, 0, 512, &page) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(page == (0xffffff00));
  REQUIRE(page_get_block(db, 0, 513, &page) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(pt.data_blocks[0] == 1023);
  REQUIRE(pt.data_blocks[1] == 1022);
  REQUIRE(pt.data_blocks[508] == 515);
  REQUIRE(pt.data_blocks[509] == 514);
  REQUIRE(pt.index_blocks[0] == 0);
  REQUIRE(pt.index_blocks[1] == 0);

  // grow from 512 to 513 pages (the limit of single-level page tables)
  // causes an allocation of a 2nd-level page table
  REQUIRE(page_reallocate(db, 0, 511) == 0);
  REQUIRE(pt.nblocks == 511);

  REQUIRE(pt.index_blocks[0] == 513);
  page_table_index& index_page = *(page_table_index*) (db->data + BLOCK_SIZE * pt.index_blocks[0]);

  REQUIRE(page_get_block(db, 0, 510, &page) == 0);
  REQUIRE(page == 512);
  REQUIRE(page_get_block(db, 0, 511, &page) == EDB_ERR_PAGE_OUT_OF_BOUNDS);
  REQUIRE(index_page.data_blocks[0] == 512);
  REQUIRE(index_page.data_blocks[1] == 0);
  REQUIRE(index_page.data_blocks[2] == 0);

  // shrink from 513 to 512 pages
  // causes the free of the 2nd-level page table
  REQUIRE(page_reallocate(db, 0, 510) == 0);
  REQUIRE(pt.nblocks == 510);
  REQUIRE(index_page.data_blocks[0] == 0);
  REQUIRE(index_page.data_blocks[1] == 0);
  REQUIRE(index_page.data_blocks[2] == 0);
  REQUIRE(pt.index_blocks[0] == 0);
  REQUIRE(pt.index_blocks[1] == 0);
  REQUIRE(pt.index_blocks[2] == 0);
  REQUIRE(freelist.back() == 513);

  // shrink from 512 to 511 pages
  REQUIRE(page_reallocate(db, 0, 509) == 0);
  REQUIRE(pt.nblocks == 509);
  REQUIRE(pt.data_blocks[508] == 515);
  REQUIRE(pt.data_blocks[509] == 0);
  REQUIRE(freelist.back() == 514);

  // shrink from 512 to 1 page
  REQUIRE(page_reallocate(db, 0, 1) == 0);
  REQUIRE(pt.nblocks == 1);
  REQUIRE(pt.data_blocks[0] == 1023);
  REQUIRE(pt.data_blocks[1] == 0);
  REQUIRE(pt.data_blocks[2] == 0);
  REQUIRE(freelist.back() == 1022);

  // shrink from 1 to 0 pages
  REQUIRE(page_reallocate(db, 0, 0) == 0);
  REQUIRE(pt.data_blocks[0] == 0);
  REQUIRE(freelist.back() == 1023);

  // grom from 0 to 1000 pages
  REQUIRE(page_reallocate(db, 0, 1000) == 0);
  REQUIRE(pt.nblocks == 1000);
  REQUIRE(pt.data_blocks[0] == 1023);
  REQUIRE(pt.data_blocks[1] == 1022);
  REQUIRE(pt.data_blocks[508] == 515);
  REQUIRE(pt.data_blocks[509] == 514);

  index_page = *(page_table_index*) (db->data + BLOCK_SIZE * pt.index_blocks[0]);
  REQUIRE(index_page.data_blocks[0] == 512);
  REQUIRE(index_page.data_blocks[1] == 511);
  REQUIRE(index_page.data_blocks[10] == 502);
  REQUIRE(index_page.data_blocks[486] == 26);
  REQUIRE(index_page.data_blocks[487] == 25);
  REQUIRE(index_page.data_blocks[488] == 24);
  REQUIRE(index_page.data_blocks[489] == 23);
  REQUIRE(index_page.data_blocks[490] == 0);
  REQUIRE(index_page.data_blocks[491] == 0);

  // shrink from 1000 to 0 pages
  REQUIRE(page_reallocate(db, 0, 0) == 0);
  REQUIRE(pt.data_blocks[0] == 0);
  REQUIRE(freelist.back() == 1023);
}
