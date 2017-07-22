#ifndef TXN_H
#define TXN_H

#include "edb.h"

#ifdef __cplusplus
extern "C" {
#endif

int txn_begin(edb *const db);
int txn_begin_master(edb *const db);
int txn_allocate_block(edb *const db, u32 *const new_block);
int txn_modify_block(edb *const db, const u32 block, u32 *const new_block);
int txn_block_is_writable(const edb *const db, const u32 block);
int txn_free_block(edb *const db, const u32 block);
int txn_abort(edb *const db);
int txn_commit(edb *const db);
int txn_commit_master(edb *const db);

#ifdef __cplusplus
}
#endif

#endif // TXN_H
