#ifndef EDB_H
#define EDB_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EDB_ERROR_FILE_OPEN (1001)
#define EDB_ERROR_FILE_SIZE (1002)
#define EDB_ERROR_OPEN_TXN (1003)
#define EDB_ERROR_READ_ONLY (1004)
#define EDB_ERROR_ALREADY_OPEN (1005)

// TODO
// enum edb_open_read_modes {
//     edb_open_read_only = 0,
//     edb_open_read_write = 1;
// }
// #define EDB_OPEN_READ_ONLY (0)
// #define EDB_OPEN_READ_WRITE (1)

// public interface to everdb

typedef struct edb_t edb;
typedef struct obj_handle_t obj_handle;

// edb
int edb_open(edb **db, const char *f_name, int readonly, int overwrite);
int edb_close(edb *db);

// txn
int edb_txn_begin(edb *db);
int edb_txn_commit(edb *db);
int edb_txn_abort(edb *db);

// block
int edb_allocate_block(edb *db, u32 *const new_block);
int edb_modify_block(edb *db, u32 block, u32* new_block);
int edb_free_block(edb *db, u32 block);

// array
int edb_array_create(edb *db, obj_handle **hp, u8 obj_size);
int edb_array_open(edb *db, obj_handle **hp, u32 obj_id);
int edb_array_get(obj_handle *h, u32 index, void *data);
int edb_array_set(obj_handle *h, u32 index, void *data);
int edb_array_push(obj_handle *h, void *data);
int edb_array_pop(obj_handle *h, void *data);
int edb_array_length(obj_handle *h, u32 *length);
int edb_array_capacity(obj_handle *h, u32 *capacity);


#ifdef __cplusplus
}
#endif
#endif
