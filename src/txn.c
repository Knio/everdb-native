
#ifdef _WIN32
#error unimplimented
#elif __linux__
#include <stdlib.h> // malloc
#include <string.h> // memcpy
#else
#error Unsupported OS
#endif

#include "txn.h"
#include "util.h"
#include "io.h"
#include "mem_hash.h"

#define ALLOCATED (0xffffff01)
#define FREED (0xffffff02)


typedef struct txn_state_t {
  u32 nblocks;
  // mapping of new_block -> state
  // where state is either ALLOCATED, FREED,
  // or old_block if it was copy-on-write
  mem_hash* blocks;
  txn_state* next;
} txn_state;


txn_state*
txn_state_new() {
    txn_state* ts = (txn_state*) calloc(1, sizeof(txn_state));
    ts->blocks = mem_hash_new();
    return ts;
}


void
txn_state_free(txn_state* ts) {
    mem_hash_free(ts->blocks);
    free(ts);
}


int
txn_allocate_block(edb* db, u32 *new_block) {
    int err = 0;
    CHECK_CODE(db->txn != NULL, EDB_ERR_TXN_NO_TRANSACTION);

    CHECK(edb_allocate_block(db, new_block));
    mem_hash_set(db->txn->blocks, *new_block, ALLOCATED);

    err:
    return err;
}


int
txn_free_block(edb *db, u32 block) {
    int err = 0;
    CHECK_CODE(db->txn != NULL, EDB_ERR_TXN_NO_TRANSACTION);

    u32 block_state = mem_hash_get(db->txn->blocks, block);
    if (block_state == ALLOCATED) {
        // newly allocated in this transaction, can just free it
        mem_hash_set(db->txn->blocks, block, 0);
        edb_free_block(db, block);
        return 0;
    }
    if (block_state == FREED) {
        err = -1;
        goto err;
    }
    if (block_state != 0) {
        // copy-on-write allocated in this transaction. pop and free it,
        // and mark the original as freed
        mem_hash_set(db->txn->blocks, block, 0);
        mem_hash_set(db->txn->blocks, block_state, FREED);
        edb_free_block(db, block);
        return 0;
    }
    // old block, mark as freed to act on commit/abort
    mem_hash_set(db->txn->blocks, block, FREED);

    err:
    return err;
}


int
txn_modify_block(edb *db, u32 block, u32* new_block) {
    int err = 0;
    CHECK_CODE(db->txn != NULL, EDB_ERR_TXN_NO_TRANSACTION);

    u32 block_state = mem_hash_get(db->txn->blocks, block);
    if (block_state == ALLOCATED) {
        // newly allocated this transaction,
        // don't need to do anything
        *new_block = block;
        return 0;
    }
    if (block_state == FREED) {
        // modify after free, logic error
        err = -1;
        goto err;
    }
    if (block_state != 0) {
        // already modified this transaction,
        // don't need to do anything
        *new_block = block;
        return 0;
    }
    // copy on write allocation
    CHECK(edb_allocate_block(db, new_block))
    memcpy(BLOCK(db, *new_block), BLOCK(db, block), BLOCK_SIZE);
    mem_hash_set(db->txn->blocks, *new_block, block);

    err:
    return err;
}


int
txn_begin_master(edb *db) {
    // first transcation
    // bootstrap the freelist cow block
    int err = 0;

    CHECK_CODE(db->freelist == 1, 3111);
    CHECK_CODE(db->objlist == 3,  3112);

    memcpy(BLOCK(db, 2), BLOCK(db, 1), BLOCK_SIZE);
    mem_hash_set(db->txn->blocks, 2, 1);
    db->freelist = 2;

    err:
    return err;
}


int
txn_begin(edb *db) {
    int err = 0;

    txn_state* ts = txn_state_new();
    ts->next = db->txn;
    db->txn_id++;
    ts->nblocks = db->nblocks;

    if (ts->next == NULL) {
        db->txn = ts;
        CHECK(txn_begin_master(db));
    }
    else {
        // TODO need to cow freelist
        CHECK_CODE(0, 3105);
        db->txn = ts;
    }

    err:
    return err;
}


int
txn_commit_master(edb *db) {
    int err = 0;
    txn_state* ts = db->txn;

    CHECK_CODE(mem_hash_get(ts->blocks, 2) == 1, 3101);

    // clear transacction state
    mem_hash_item* e;
    for (size_t i=0; i<ts->blocks->capacity; i++) {
        e = &ts->blocks->entries[i];

        if (e->k == 0) {
            continue;
        }

        if (e->k == 2) {
            // freelist special case
            // we'll handle this below
            continue;
        }

        if (e->k == db->objlist) {
            // objlist
            memcpy(BLOCK(db, 3), BLOCK(db, e->k), BLOCK_SIZE);
            CHECK(edb_free_block(db, e->k));
            db->objlist = 3;
        }

        if (e->v == ALLOCATED) {
            continue;
        }

        if (e->v == FREED) {
            // freed blocks actually freed
            CHECK(edb_free_block(db, e->k));
            continue;
        }

        else {
            // free parent of cow'd blocks
            CHECK(edb_free_block(db, e->v));
            continue;
        }
    }

    // handle freelist
    memcpy(BLOCK(db, 1), BLOCK(db, 2), BLOCK_SIZE);
    db->freelist = 1;


    txn_state_free(ts);
    db->txn = NULL;

    err:
    return err;
}


int
txn_commit(edb *db) {
    int err = 0;

    CHECK_CODE(db->txn != NULL, EDB_ERR_TXN_NO_TRANSACTION);
    txn_state* ts = db->txn;

    if (db->txn->next == NULL) {
        err = txn_commit_master(db);
        goto err;
    }

    txn_state* parent = ts->next;

    // merge everything we did into the parent txn
    mem_hash_item* e;
    for (size_t i=0; i<ts->blocks->capacity; i++) {
        e = &ts->blocks->entries[i];
        if (e->k == 0) {
            continue;
        }
        if (e->v == ALLOCATED) {
            mem_hash_set(parent->blocks, e->k, ALLOCATED);
            continue;
        }

        if (e->v == FREED) {
            u32 old_block_state = mem_hash_get(parent->blocks, e->k);
            if (old_block_state == ALLOCATED) {
                // block was allocated in the parent
                // collapse the alloc+free into nothing
                mem_hash_set(parent->blocks, e->k, 0);
                edb_free_block(db, e->k);
            }
            else if (old_block_state == FREED) {
                err = -902;
                goto err;
            }
            else if (old_block_state != 0) {
                // block was modified in the parent
                // forget the modification and free the original
                mem_hash_set(parent->blocks, e->k, 0);
                mem_hash_set(parent->blocks, old_block_state, FREED);
            }
            else if (old_block_state == 0) {
                // block was pristine in the parent
                // just propagate the free upwards
                mem_hash_set(parent->blocks, e->k, FREED);
            }
            continue;
        }
        else {
            // e->v was cow from e->v
            u32 old_block_state = mem_hash_get(parent->blocks, e->v);
            if (old_block_state == ALLOCATED) {
                // allocated in the parent
                // collapse the alloc+modify into just an alloc
                edb_free_block(db, e->v);
            }
            else if (old_block_state == FREED) {
                err = -903;
                goto err;
            }
            else if (old_block_state != 0) {
                // block was modified in the parent
                // collapse the cow+cow into a single modification
                edb_free_block(db, e->v);
                mem_hash_set(parent->blocks, e->k, old_block_state);
            }
            else if (old_block_state == 0) {
                // block was pristine in the parent
                // just propagate the modification upwards
                mem_hash_set(parent->blocks, e->k, e->v);
            }
            continue;
        }
    }

    db->txn = parent;
    txn_state_free(ts);

    err:
    return err;
}


int
txn_abort(edb *db) {
    int err = 0;

    CHECK_CODE(db->txn != NULL, EDB_ERR_TXN_NO_TRANSACTION);
    txn_state* ts = db->txn;

    // TODO implement abort for master
    CHECK_CODE(ts->next != NULL, 9482);


    db->txn = ts->next;

    CHECK(io_resize(db, ts->nblocks));

    // undo everything we did
    mem_hash_item* e;
    for (size_t i=0; i<ts->blocks->capacity; i++) {
        e = &ts->blocks->entries[i];
        if (e->k == 0) {
            continue;
        }
        if (e->v == FREED) {
            continue;
        }
        CHECK(edb_free_block(db, e->k));
    }

    err:
    mem_hash_free(ts->blocks);
    free(ts);
    return err;
}

