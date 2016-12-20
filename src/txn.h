#ifndef TXN_H
#define TXN_H

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EDB_ERR_TXN_NO_TRANSACTION (3001)
#define EDB_ERR_TXN_NOT_IMPLEMENTED (3099)

int txn_begin(edb *db);
int txn_begin_master(edb *db);
int txn_allocate_block(edb *db, u32 *new_block);
int txn_modify_block(edb *db, u32 block, u32 *new_block);
int txn_free_block(edb *db, u32 block);
int txn_abort(edb *db);
int txn_commit(edb *db);
int txn_commit_master(edb *db);

#ifdef __cplusplus
}
#endif

#endif /* ifndef TXN_H */
