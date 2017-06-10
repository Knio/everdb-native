#define CATCH_CONFIG_MAIN
#include "../../lib/catch.hpp"

#include "edb_mock.h"
#include "../../src/btree.h"


TEST_CASE("btree") {
  init_edb_mock();

  REQUIRE(sizeof(btree) == BLOCK_SIZE);

  REQUIRE(btree_init(db, 0) == 0);
  btree* bt = (btree*)db->data;

  // get on empty
  u32 key = 1;
  u32 value = 1;

  value = 10;
  REQUIRE(btree_get(db, 0, 0, &value) == EDB_ERROR_BTREE_INVALID_KEY);
  REQUIRE(value == 0);

  value = 10;
  REQUIRE(btree_get(db, 0, 4294967295u, &value) == EDB_ERROR_BTREE_INVALID_KEY);
  REQUIRE(value == 0);

  REQUIRE(btree_first(db, 0, &key, &value) == EDB_ERROR_BTREE_INVALID_KEY);
  REQUIRE(btree_last(db, 0, &key, &value) == EDB_ERROR_BTREE_INVALID_KEY);

  // insert
  value = 77;
  REQUIRE(btree_set(db, 0, 10, value) == 0);
  REQUIRE(bt->self_size == 1);
  REQUIRE(bt->magic == EDB_MAGIC_BTREE_LEAF);
  REQUIRE(bt->key[0] == 10);
  REQUIRE(bt->data[0] == 77);

  // check
  value = 10;
  REQUIRE(btree_get(db, 0, 10, &value) == 0);
  REQUIRE(value == 77);

  value = 10;
  REQUIRE(btree_get(db, 0, 9, &value) == EDB_ERROR_BTREE_INVALID_KEY);
  REQUIRE(value == 0);

  value = 10;
  REQUIRE(btree_get(db, 0, 11, &value) == EDB_ERROR_BTREE_INVALID_KEY);
  REQUIRE(value == 0);

  key = 1; value = 1;
  REQUIRE(btree_first(db, 0, &key, &value) == 0);
  REQUIRE(key == 10);
  REQUIRE(value == 77);

  key = 1; value = 1;
  REQUIRE(btree_last(db, 0, &key, &value) == 0);
  REQUIRE(key == 10);
  REQUIRE(value == 77);

  // insert after
  value = 99;
  REQUIRE(btree_set(db, 0, 20, value) == 0);

  REQUIRE(bt->self_size == 2);
  REQUIRE(bt->key[0] == 10);
  REQUIRE(bt->data[0] == 77);
  REQUIRE(bt->key[1] == 20);
  REQUIRE(bt->data[1] == 99);

  REQUIRE(btree_get(db, 0, 20, &value) == 0);
  REQUIRE(value == 99);

  REQUIRE(btree_first(db, 0, &key, &value) == 0);
  REQUIRE(key == 10);
  REQUIRE(value == 77);

  REQUIRE(btree_last(db, 0, &key, &value) == 0);
  REQUIRE(key == 20);
  REQUIRE(value == 99);

  // insert before
  value = 44;
  REQUIRE(btree_set(db, 0, 5, value) == 0);

  REQUIRE(bt->self_size == 3);
  REQUIRE(bt->key[0] == 5);
  REQUIRE(bt->key[1] == 10);
  REQUIRE(bt->key[2] == 20);
  REQUIRE(bt->data[0] == 44);
  REQUIRE(bt->data[1] == 77);
  REQUIRE(bt->data[2] == 99);

  REQUIRE(btree_get(db, 0, 5, &value) == 0);
  REQUIRE(value == 44);

  REQUIRE(btree_first(db, 0, &key, &value) == 0);
  REQUIRE(key == 5);
  REQUIRE(value == 44);

  // overwrite
  value = 3;
  REQUIRE(btree_set(db, 0, 10, value) == 0);
  REQUIRE(bt->self_size == 3);
  REQUIRE(bt->key[0] == 5);
  REQUIRE(bt->key[1] == 10);
  REQUIRE(bt->key[2] == 20);
  REQUIRE(bt->data[0] == 44);
  REQUIRE(bt->data[1] == 3);
  REQUIRE(bt->data[2] == 99);
  REQUIRE(btree_get(db, 0, 10, &value) == 0);
  REQUIRE(value == 3);

  // pop
  REQUIRE(btree_pop(db, 0, 9, &value) == EDB_ERROR_BTREE_INVALID_KEY);
  REQUIRE(value == 0);
  REQUIRE(btree_pop(db, 0, 10, &value) == EDB_ERROR_BTREE_EMPTY);
  REQUIRE(value == 3);
  REQUIRE(bt->self_size == 2);
  REQUIRE(bt->key[0] == 5);
  REQUIRE(bt->key[1] == 20);
  REQUIRE(bt->data[0] == 44);
  REQUIRE(bt->data[1] == 99);
  REQUIRE(btree_get(db, 0, 10, &value) == EDB_ERROR_BTREE_INVALID_KEY);

  clear_edb_mock();

}

TEST_CASE("big") {
  init_edb_mock();

  u32 root = 0;
  REQUIRE(btree_init(db, root) == 0);
  btree* bt = (btree*)db->data;

  u32 i;
  u32 x, y;
  for (i = 1000; i < 1000 + EDB_BTREE_SIZE; i++) {
    REQUIRE(btree_set(db, root, i, i) == 0);
    REQUIRE(bt->self_size == i - 1000 + 1);
    REQUIRE(btree_get(db, root, i, &x) == 0);
    REQUIRE(x == i);
  }
  REQUIRE(bt->self_size == EDB_BTREE_SIZE);
  REQUIRE(btree_set(db, root, i, i) == EDB_ERROR_BTREE_FULL);
  REQUIRE(btree_grow(db, root, &root) == 0);
  bt = (btree*)(db->data + BLOCK_SIZE * root);
  REQUIRE(bt->magic == EDB_MAGIC_BTREE);
  REQUIRE(bt->self_size == 2);
  REQUIRE(bt->key[0] == 1254);
  REQUIRE(bt->key[1] == 1509);
  REQUIRE(bt->key[2] == 0);

  // grow from right
  for (; i < 1000 + EDB_BTREE_SIZE * 3 / 2; i++) {
    CAPTURE(i);
    REQUIRE(btree_set(db, root, i, i) == 0);
    REQUIRE(bt->key[0] == 1254);
    REQUIRE(bt->key[1] == i);
    REQUIRE(bt->key[2] == 0);
    REQUIRE(btree_get(db, root, i, &x) == 0);
    REQUIRE(x == i);
    REQUIRE(bt->self_size == 2);
  }

  REQUIRE(btree_set(db, root, i, i) == 0);
  REQUIRE(btree_get(db, root, i, &x) == 0);
  REQUIRE(x == i);
  REQUIRE(bt->self_size == 3);

  // grow from left
  for (int j = 999; j >= 0; j--) {
    CAPTURE(j);
    REQUIRE(btree_set(db, root, j, j) == 0);
    REQUIRE(btree_get(db, root, j, &x) == 0);
    REQUIRE(x == j);
  }
  REQUIRE(bt->self_size == 5);

  REQUIRE(btree_first(db, root, &y, &x) == 0);
  REQUIRE(y == 0);
  REQUIRE(x == 0);
  REQUIRE(btree_last(db, root, &y, &x) == 0);
  REQUIRE(y == 1000 + EDB_BTREE_SIZE * 3 / 2);
  REQUIRE(x == 1000 + EDB_BTREE_SIZE * 3 / 2);

  // pop and merge
  REQUIRE(btree_pop(db, root, i, &x) == 0);
  REQUIRE(x == i);
  i--;

  // from the right
  for (; i >= 1000 + EDB_BTREE_SIZE - 1; i--) {
    CAPTURE(i);
    REQUIRE(btree_pop(db, root, i, &x) == 0);
    REQUIRE(x == i);
  }
  REQUIRE(btree_pop(db, root, i, &x) == EDB_ERROR_BTREE_EMPTY);
  REQUIRE(x == i);
  i--;
  REQUIRE(bt->self_size == 4);
  for (; i > 1125; i--) {
    CAPTURE(i);
    REQUIRE(btree_pop(db, root, i, &x) == 0);
    REQUIRE(x == i);
  }
  REQUIRE(btree_pop(db, root, i, &x) == EDB_ERROR_BTREE_EMPTY);
  REQUIRE(x == i);
  i--;
  REQUIRE(bt->self_size == 3);

  // pop from the left
  int j;
  for (j = 0; j < 618; j++) {
    CAPTURE(j);
    REQUIRE(btree_pop(db, root, j, &x) == 0);
    REQUIRE(x == j);
  }
  REQUIRE(btree_pop(db, root, j, &x) == EDB_ERROR_BTREE_EMPTY);
  REQUIRE(x == j);
  j++;
  REQUIRE(bt->self_size == 2);

  for (; j < 871; j++) {
    CAPTURE(j);
    REQUIRE(btree_pop(db, root, j, &x) == 0);
    REQUIRE(x == j);
  }
  REQUIRE(btree_pop(db, root, j, &x) == EDB_ERROR_BTREE_EMPTY);
  REQUIRE(x == j);
  j++;
  REQUIRE(bt->self_size == 1);

  for (; j < 998; j++) {
    CAPTURE(j);
    REQUIRE(btree_pop(db, root, j, &x) == 0);
    REQUIRE(x == j);
  }

  REQUIRE(btree_pop(db, root, j, &x) == EDB_ERROR_BTREE_REALLY_EMPTY);

  clear_edb_mock();
}
