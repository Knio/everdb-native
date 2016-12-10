#ifndef EDB_H
#define EDB_H

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EDB_ERROR_FILE_OPEN (1001)
#define EDB_ERROR_FILE_SIZE (1002)

// public interface to everdb

int edb_open(edb* db, const char* f_name, int only, int write);
int edb_close(edb* db);

int edb_txn_begin(edb* db);
int edb_txn_commit(edb* db);
int edb_txn_abort(edb* db);

int edb_allocate_block(edb* db, u32 *const new_block);
int edb_free_block(edb* db, u32 block);


#ifdef __cplusplus
}
#endif
#endif
