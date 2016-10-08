#ifndef TXN_HEADER_INCLUDE
#define TXN_HEADER_INCLUDE

int txn_begin(edb *db);
int txn_begin_master(edb *db);
int txn_allocate_block(edb *db);
int txn_free_block(edb *db, u32 block);
int txn_abort(edb *db);
int txn_commit(edb *db);
int txn_commit_master(edb *db);

#endif /* ifndef TXN_HEADER_INCLUDE */
