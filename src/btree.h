#ifndef EDB_BTREE_H
#define EDB_BTREE_H

#include "edb.h"

#ifdef __cplusplus
extern "C" {
#endif


#define EDB_BTREE_SIZE (510)
#define EDB_BTREE_SPLIT (EDB_BTREE_SIZE >> 2)

#define EDB_ERROR_BTREE_INVALID_KEY (3351)
#define EDB_ERROR_BTREE_FULL (3352)
#define EDB_ERROR_BTREE_EMPTY (3353)
#define EDB_ERROR_BTREE_REALLY_EMPTY (3354)

typedef struct btree_t {
    u32 magic;
    u32 self_size;
    u32 total_size;
    u32 checksum;
    u32 key[EDB_BTREE_SIZE];
    u32 data[EDB_BTREE_SIZE];
} btree;


#define BTREE(db, root) ((btree*) (db->data + BLOCK_SIZE * root))

int btree_init(edb *db, const u32 root);
int btree_get(const edb *const db, const u32 root, const u32 key, u32 *const value);
int btree_set(edb *const db, const u32 root, u32 const key, u32 const value);
int btree_pop(edb *const db, const u32 root, const u32 key, u32 *const value);
int btree_grow(edb *const db, u32 root, u32 *const new_root);
int btree_first(const edb *const db, const u32 root, u32 *const key, u32 *const value);
int btree_last(const edb *const db, const u32 root, u32 *const key, u32 *const value);

#ifdef __cplusplus
}
#endif
#endif
