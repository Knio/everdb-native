# Nested Copy-On-Write Transactions For Block Devices

### freelist
the freelist is the set of blocks that are not currently in use by *any* ongoing transaction in the database.

### edb_open()
open a database for read(/write) access. use the last good committed transaction, while warning/erroring about a missed transaction
 - two root blocks hardcoded: block0 and block block1
     + each contains copy of master block
     + master block contains
         * `file_len`
         * `txn_id`
         * `txn_committed`
     + possible states:
         * both have `txn_committed`=1 and `file_len` matches:
             - normal workflow, read from the block with the higher `txn_id` and use the other as a spare for the next txn
         * only one has `txn_committed`=1:
             - this will be the one with the lower `txn_id`
             - the block with the higher `txn_id` is an uncommitted (failed) transaction
             - roll back the transaction by loading the old master and truncating the file to `file_len`
             - warn/error about incomplete transaction

### txn_begin()
begin a new transaction. transactions can be nested indefinitely, and are arranged as a stack. thus for data to be written permanently, all transactions that were begun must be committed, and the very last one will commit to the disk.
 - push new `txn_state` to `txn_stack`. `txn_state` is a struct containing:
     + `file_len` the current size of the file, in blocks
     + `freed{}` a set of blocks that this transaction will free
     + `allocated{}` a set of blocks that this transaction will allocate
     + `modified{}` a mapping of a `newblock` -> `oldblock` where the new block will be copy-on-write modified from the old block


### txn_begin_master()
begin the very first transaction. this must manage the master blocks.
 - copy the current master block to the spare master
     + set `txn_committed` to false
     + increment `txn_id`
     + set the current master pointer to the new master block


### txn_allocate_block() -> `newblock`
allocate a new block as part of normal data structure operation. blocks are first allocated from the freelist, and if the freelist is empty, by growing the database file and using the new end. to abort an allocation, we need to be able to give it back to the freelist or shrink the file.
 - alocate `newblock` from popping from the freelist or growing the file
 - add `newblock`) to `allocated{}`
 - return `newblock`


### txn_free_block(`curblock`)
free a block as part of normal data structure operation. to be able to abort a free operation, we can't actually free the block until the txn is committed and must keep it in limbo until txn end.
 - if `curblock` is in `allocated{}`, add it back to the freelist (it was allocated in this transaction)
 - if `curblock` is a key in `modified{}`, pop it from `modified{}` and add it back to the freelist (it was copy-on-write allocated in this transaction)
 - add `curblock` to `freed{}`

<!--
### txn_register_block_pointer(`parentblock`, `offset`)
register a `parentblock` as a parent of `childblock` block pointed to by `curblock[offset]`. when `childblock` is modified, `parentblock` will also be modified to point to the new value of `childblock`.
 - set `parents[childblock]` = `parentblock`
 -->

### txn_modify_block(`curblock`) -> `newblock`
modify a block as part of the current txn. to be able to abort, we need to keep the old block contents in limbo until the end of the txn. if the block has already been modified in this txn, theres is no need to do anything.
 - if `curblock` is in `allocated{}`, do nothing and return `curblock` (it is new this transaction)
 - if `curblock` is a key in `modified{}`, do nothing and return `curblock` (block has already been copy-on-write allocated in this transaction)
 - allocate `newblock` from popping from the freelist or growing the file
 - copy the data on `curblock` to `newblock`
 - set `modified[newblock]` to `curblock`
 - return `newblock`


### txn_abort()
abort the current transaction.
 - pop `txn_state` from `txn_stack`
 - truncate file to `file_len`
 - for `newblock` in `allocated{}`:
     + add `newblock` back to the freelist (it was allocated in this transaction)
 - for `oldblock` in `freed{}`: do nothing
 - for `newblock` ->  `oldblock` in `modified{}`:
     + add `newblock` back to the freelist (copy-on-write allocation in this transaction)


### txn_commit()
commit the current transaction
 - pop `txn_state` from `txn_stack`
 - if the `txn_stack` is now empty, call `txn_commit_master(txn_state)`
 - else: merge `txn_state` to the new top of the stack `parent_txn_state`:
     + for `newblock` in `allocated{}`: add to `parent.allocated{}`
     + for `oldblock` in `freed{}`:
         * if `oldblock` is a key in `parent.modified{}`: pop it from `parent.modified{}` and add it to the freelist
         * else: add it to `parent.freed{}`
     + for `newblock` ->  `oldblock` in `modified{}`:
         * if `oldblock` is a key in `parent.modified{}`:
             - add `oldblock` to the freelist
             - set `parent.modified[newblock]` = `parent.modified[oldblock]`
             - pop `parent.modified[oldblock]`
         * else: add entry to `parent.modified{}`
     + if a block was modified in the parent transaction, add `curblock` to the freelist


### txn_commit_master(`txn_state`)
 - for (`curblock`, `newblock`) in `modified{}`:
     + if `curblock` is `NULL`, do nothing (leave an allocated block allocated)
     + add `curblock` to the freelist (actually free a freed block, free a block that was modified)
 - flush all modified blocks to disk
 - flush the current master block
 - set `txn_committed` to true on the current master
 - flush the current master block again

