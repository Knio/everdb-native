#ifdef _WIN32
#error need headers?
#elif __linux__
#include <stdlib.h> // malloc
#include <string.h> // memset
#else
#error Unsupported OS
#endif

#include "btree.h"
#include "util.h"


int btree_init(edb *const db, const u32 root) {
  btree *bt = BTREE(db, root);
  memset(bt, 0, BLOCK_SIZE);
  bt->magic = EDB_MAGIC_BTREE_LEAF;
  bt->self_size = 0;
  return 0;
}


static inline int list_find(const btree *const bt, const u32 key) {
    LOG_HERE;
    int b = 0;
    int e = bt->self_size;
    while (b < e) {
        int m = (b + e) >> 1;
        // LOG_DEBUG("b=%d e=%d m=%d k[m]=%d key=%d\n", b, e, m, bt->key[m], key)
        if (key > bt->key[m]) {
            b = m + 1;
        } else {
            e = m;
        }
    }
    return b;
}


static inline void list_insert(btree *const bt, int j, u32 key, u32 data) {
    LOG_HERE;
    for (int i = bt->self_size; i > j; i--) {
        bt->key[i] = bt->key[i - 1];
        bt->data[i] = bt->data[i - 1];
    }
    bt->key[j] = key;
    bt->data[j] = data;
    bt->self_size++;
}


static inline void list_remove(btree *const bt, int j) {
    LOG_HERE;
    for (; j < bt->self_size - 1; j++) {
        bt->key[j] = bt->key[j + 1];
        bt->data[j] = bt->data[j + 1];
    }
    bt->self_size--;
    bt->key[bt->self_size] = 0;
    bt->data[bt->self_size] = 0;
}


int btree_first(const edb *const db, const u32 root, u32 *const key, u32 *const value) {
    LOG_HERE;
    int err = 0;
    btree* bt = BTREE(db, root);
    CHECK_CODE(bt->magic == EDB_MAGIC_BTREE_LEAF || bt->magic == EDB_MAGIC_BTREE, EDB_ERROR_DATA_CORRUPT);
    CHECK_CODE(bt->self_size, EDB_ERROR_BTREE_INVALID_KEY);

    if (bt->magic == EDB_MAGIC_BTREE_LEAF) {
        if (key) { *key = bt->key[0]; }
        if (value) { *value = bt->data[0]; }
        return 0;
    }
    if (bt->magic == EDB_MAGIC_BTREE) {
        CHECK(btree_first(db, bt->data[0], key, value));
        return 0;
    }
    err:
    if (key) *key = 0;
    if (value) *value = 0;
    return err;
}


int btree_last(const edb *const db, const u32 root, u32 *const key, u32 *const value) {
    LOG_HERE;
    int err = 0;
    btree* bt = BTREE(db, root);
    CHECK_CODE(bt->magic == EDB_MAGIC_BTREE_LEAF || bt->magic == EDB_MAGIC_BTREE, EDB_ERROR_DATA_CORRUPT);
    CHECK_CODE(bt->self_size, EDB_ERROR_BTREE_INVALID_KEY);
    if (bt->magic == EDB_MAGIC_BTREE_LEAF) {
        if (key) { *key = bt->key[bt->self_size - 1]; }
        if (value) { *value = bt->data[bt->self_size - 1]; }
        return 0;
    }
    if (bt->magic == EDB_MAGIC_BTREE) {
        CHECK(btree_last(db, bt->data[bt->self_size - 1], key, value));
        return 0;
    }
    err:
    if (key) { *key = 0; }
    if (value) { *value = 0; }
    return err;
}


int btree_get(const edb *const db, const u32 root, const u32 key, u32 *const value) {
    LOG_HERE;
    int err = 0;
    btree* bt = BTREE(db, root);
    CHECK_CODE(bt->magic == EDB_MAGIC_BTREE_LEAF || bt->magic == EDB_MAGIC_BTREE, EDB_ERROR_DATA_CORRUPT);

    int b = list_find(bt, key);
    CHECK_CODE(b < bt->self_size, EDB_ERROR_BTREE_INVALID_KEY);
    if (bt->magic == EDB_MAGIC_BTREE_LEAF) {
        CHECK_CODE(bt->key[b] == key, EDB_ERROR_BTREE_INVALID_KEY);
        *value = bt->data[b];
        return 0;
    }
    else if (bt->magic == EDB_MAGIC_BTREE) {
        return btree_get(db, bt->data[b], key, value);
    }
    err:
    if (value) { *value = 0; }
    return err;
}


int btree_grow(edb *const db, u32 root, u32* new_root) {
    LOG_HERE;
    int err = 0;
    btree* b0 = BTREE(db, *new_root);
    CHECK_CODE(b0->magic == EDB_MAGIC_BTREE_LEAF || b0->magic == EDB_MAGIC_BTREE, EDB_ERROR_DATA_CORRUPT);

    u32 child1;
    CHECK(edb_modify_block(db, root, &root));
    CHECK(edb_allocate_block(db, new_root));
    CHECK(edb_allocate_block(db, &child1));
    CHECK(btree_init(db, *new_root));
    CHECK(btree_init(db, child1));
    b0 = BTREE(db, root);
    btree* b1 = BTREE(db, child1);
    b1->magic = b0->magic;

    int n = b0->self_size >> 1;
    for (int i = n; i < b0->self_size; i++) {
        b1->key[i - n] = b0->key[i];
        b1->data[i - n] = b0->data[i];
    }
    b1->self_size = b0->self_size - n;
    b0->self_size = n;

    btree* bt = BTREE(db, *new_root);
    bt->magic = EDB_MAGIC_BTREE;
    bt->key[0] = b0->key[b0->self_size - 1];
    bt->key[1] = b1->key[b1->self_size - 1];
    bt->data[0] = root;
    bt->data[1] = child1;
    bt->self_size = 2;
    err:
    return err;
}


int btree_set(edb *const db, const u32 root, u32 const key, u32 const value) {
    LOG_HERE;
    int err = 0;
    btree* bt = BTREE(db, root);
    CHECK_CODE(bt->magic == EDB_MAGIC_BTREE_LEAF || bt->magic == EDB_MAGIC_BTREE, EDB_ERROR_DATA_CORRUPT);

    int b = list_find(bt, key);
    LOG_DEBUG("b=%d size=%d key=%d\n", b, bt->self_size, key);
    if (bt->magic == EDB_MAGIC_BTREE_LEAF) {
        // leaf node
        if (b < bt->self_size && bt->key[b] == key) {
            // overwrite
            bt->data[b] = value;
            return 0;
        }
        // insert
        CHECK_CODE(bt->self_size < EDB_BTREE_SIZE, EDB_ERROR_BTREE_FULL);
        list_insert(bt, b, key, value);
        return 0;
    }

    if (bt->magic == EDB_MAGIC_BTREE) {
        // internal node
        if (b == bt->self_size) {
            b--;
        }
        u32 child0 = bt->data[b];
        CHECK(edb_modify_block(db, child0, &child0));
        btree* bt = BTREE(db, root);
        btree* b0 = BTREE(db, child0);
        bt->data[b] = child0;
        int r = btree_set(db, child0, key, value);
        if (r == 0) {
            bt->key[b] = b0->key[b0->self_size - 1];
            return 0;
        }
        else if (r == EDB_ERROR_BTREE_FULL) {
            // split child node b into b and b + 1
            CHECK_CODE(bt->self_size < EDB_BTREE_SIZE, EDB_ERROR_BTREE_FULL);
            list_insert(bt, b + 1, 0, 0);
            u32 child1;
            CHECK(edb_allocate_block(db, &child1));
            btree_init(db, child1);
            bt = BTREE(db, root);
            b0 = BTREE(db, child0);
            btree* b1 = BTREE(db, child1);
            b1->magic = b0->magic;
            bt->data[b + 1] = child1;
            for (int i = EDB_BTREE_SPLIT; i < b0->self_size; i++) {
                b1->key[i - EDB_BTREE_SPLIT] = b0->key[i];
                b1->data[i - EDB_BTREE_SPLIT] = b0->data[i];
            }
            b1->self_size = b0->self_size - EDB_BTREE_SPLIT;
            b0->self_size = EDB_BTREE_SPLIT;
            // retry recursive insert - will have free space now
            if (key <= bt->key[b]) {
                CHECK(btree_set(db, bt->data[b], key, value));
            } else {
                CHECK(btree_set(db, bt->data[b + 1], key, value));
            }
            bt->key[b] = b0->key[b0->self_size - 1];
            bt->key[b + 1] = b1->key[b1->self_size - 1];
            return 0;
        }
        CHECK(r);
    }
    err:
    return err;
}


int btree_pop(edb *const db, const u32 root, const u32 key, u32 *const value) {
    int err = 0;
    btree *bt = BTREE(db, root);
    CHECK_CODE(bt->magic == EDB_MAGIC_BTREE_LEAF || bt->magic == EDB_MAGIC_BTREE, EDB_ERROR_DATA_CORRUPT);
    LOG_DEBUG("root=%d size=%d key=%d\n", root, bt->self_size, key);

    int b = list_find(bt, key);
    if (bt->magic == EDB_MAGIC_BTREE_LEAF) {
        // leaf node
        CHECK_CODE(bt->key[b] == key, EDB_ERROR_BTREE_INVALID_KEY);
        *value = bt->data[b];
        list_remove(bt, b);
        if (bt->self_size < EDB_BTREE_SPLIT) {
            // this is still success
            return EDB_ERROR_BTREE_EMPTY;
        }
        return 0;
    }
    if (bt->magic == EDB_MAGIC_BTREE) {
        // internal node
        CHECK_CODE(bt->key[b] >= key, EDB_ERROR_BTREE_INVALID_KEY);
        u32 child0 = bt->data[b];
        CHECK(edb_modify_block(db, child0, &child0));
        btree* bt = BTREE(db, root);
        btree* b0 = BTREE(db, child0);
        bt->data[b] = child0;
        int r = btree_pop(db, child0, key, value);
        if (r == 0) {
            bt->key[b] = b0->key[b0->self_size - 1];
            return 0;
        }
        if (r == EDB_ERROR_BTREE_EMPTY) {
            // CHECK_CODE(b + 1 < bt->self_size, 9999);
            // if this is the rightmost child, then merge from the left
            CHECK_CODE(bt->self_size > 1, EDB_ERROR_BTREE_REALLY_EMPTY);
            int merge_from_left = (b + 1 == bt->self_size);
            u32 other, first;
            if (merge_from_left) {
                other = b - 1;
                first = other;
            } else {
                other = b + 1;
                first = b;
            }
            u32 child1 = bt->data[other];
            btree* b1;
            CHECK(edb_modify_block(db, child0, &child0));
            CHECK(edb_modify_block(db, child1, &child1));
            bt = BTREE(db, root);
            b0 = BTREE(db, child0);
            b1 = BTREE(db, child1);
            bt->data[b] = child0;
            bt->data[other] = child1;
            if (b1->self_size > EDB_BTREE_SPLIT) {
                LOG_DEBUG("stealing merge_from_left=%d\n", merge_from_left);
                // can steal
                if (merge_from_left) {
                    // b - 1 to b
                    list_insert(b0, 0,
                        b1->key[b1->self_size - 1],
                        b1->data[b1->self_size - 1]);
                    list_remove(b1, b1->self_size - 1);
                } else {
                    // b + 1 to b
                    list_insert(b0, b0->self_size, b1->key[0], b1->data[0]);
                    list_remove(b1, 0);
                }
                bt->key[b] = b0->key[b0->self_size - 1];
                bt->key[other] = b1->key[b1->self_size - 1];
                return 0;
            }
            // merge nodes
            if (merge_from_left) {
                child0 = bt->data[other];
                child1 = bt->data[b];
            } else {
                child0 = bt->data[b];
                child1 = bt->data[other];
            }
            b0 = BTREE(db, child0);
            b1 = BTREE(db, child1);

            LOG_DEBUG("merging nodes merge_from_left=%d b=%d other=%d child0=%d (size=%d) child1=%d (size=%d)\n",
                merge_from_left, b, other, child0, b0->self_size, child1, b1->self_size);
            // merge b1 into b0
            for (int i = 0; i < b1->self_size; i++) {
                b0->key[b0->self_size + i] = b1->key[i];
                b0->data[b0->self_size + i] = b1->data[i];
            }
            b0->self_size += b1->self_size;

            // delete b1 from parent
            CHECK(edb_free_block(db, child1));
            list_remove(bt, first + 1);
            bt->key[first] = b0->key[b0->self_size - 1];
            if (bt->self_size < EDB_BTREE_SPLIT) {
                // this is still success
                return EDB_ERROR_BTREE_EMPTY;
            }
        }
        CHECK(r);
    }
    err:
    if (value) {
        *value = 0;
    }
    return err;
}
