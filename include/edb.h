#ifndef EDB_H
#define EDB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "errors.h"

// TODO
// enum edb_open_read_modes {
//     edb_open_read_only = 0,
//     edb_open_read_write = 1;
// }
// #define EDB_OPEN_READ_ONLY (0)
// #define EDB_OPEN_READ_WRITE (1)

// public interface to everdb

typedef struct edb_t edb;
#ifndef EDB_H_IMPL
typedef struct edb_array_t edb_array;
// typedef struct edb_btree_t edb_btree;
// typedef struct edb_blob_t edb_blob;
#endif

// edb
int edb_open(edb **const db, const char *f_name, int readonly, int overwrite);
int edb_close(edb *const db);

// txn
int edb_txn_begin(edb *const db);
int edb_txn_commit(edb *const db);
int edb_txn_abort(edb *const db);

// block
int edb_allocate_block(edb *const db, uint32_t *const new_block);
int edb_modify_block(edb *const db, const uint32_t block, uint32_t *const new_block);
int edb_free_block(edb *const db, const uint32_t block);

// array
int edb_array_create(edb *const db, edb_array **const hp, uint8_t obj_size);
int edb_array_open(edb *const db, edb_array **const hp, uint32_t obj_id);
int edb_array_get(edb_array *const h, const uint32_t index, void *const data);
int edb_array_set(edb_array *const h, const uint32_t index, const void *const data);
int edb_array_push(edb_array *const h, const void *const data);
int edb_array_pop(edb_array *const h, void *const data);
int edb_array_length(edb_array *const h, uint64_t *const length);
int edb_array_capacity(edb_array *const h, uint64_t *const capacity);


#ifdef __cplusplus
}
#endif
#endif
