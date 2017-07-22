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
#include "freelist.h"
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


txn_state* txn_state_new() {
    txn_state* ts = (txn_state*) calloc(1, sizeof(txn_state));
    ts->blocks = mem_hash_new();
    return ts;
}


void txn_state_free(txn_state* ts) {
    mem_hash_free(ts->blocks);
    free(ts);
}


int txn_block_is_writable(const edb *const db, const u32 block) {
    int err = 0;
    CHECK_CODE(db->txn != NULL, EDB_ERR_TXN_NO_TRANSACTION);
    u32 v = mem_hash_get(db->txn->blocks, block);
    return v == 0 ? EDB_ERR_TXN_INVALID_BLOCK : 0;
    err:
    return err;
}

int txn_allocate_block(edb *const db, u32 *const new_block) {
    int err = 0;
    CHECK_CODE(db->txn != NULL, EDB_ERR_TXN_NO_TRANSACTION);

    CHECK(freelist_allocate_block(db, new_block));
    mem_hash_set(db->txn->blocks, *new_block, ALLOCATED);

    err:
    return err;
}


int txn_free_block(edb *const db, const u32 block) {
    int err = 0;
    CHECK_CODE(db->txn != NULL, EDB_ERR_TXN_NO_TRANSACTION);

    u32 block_state = mem_hash_get(db->txn->blocks, block);

    if (block_state == ALLOCATED) {
        // newly allocated in this transaction, can just free it
        mem_hash_set(db->txn->blocks, block, 0);
        freelist_free_block(db, block);
        return 0;
    }
    else if (block_state == FREED) {
        // double free this transaction
        err = EDB_ERR_TXN_INVALID_BLOCK;
        goto err;
    }
    else if (block_state != 0) {
        // copy-on-write allocated in this transaction. pop and free it,
        // and mark the original as freed
        mem_hash_set(db->txn->blocks, block, 0);
        mem_hash_set(db->txn->blocks, block_state, FREED);
        freelist_free_block(db, block);
        return 0;
    }
    else {
        // old block, mark as freed to act on commit/abort
        mem_hash_set(db->txn->blocks, block, FREED);
    }

    err:
    return err;
}


int txn_modify_block(edb *const db, const u32 block, u32 *const new_block) {
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
    CHECK(freelist_allocate_block(db, new_block));
    memcpy(BLOCK(db, *new_block), BLOCK(db, block), BLOCK_SIZE);
    mem_hash_set(db->txn->blocks, *new_block, block);

    err:
    return err;
}


int txn_begin_master(edb *const db) {
    LOG_HERE;
    int err = 0;

    CHECK_CODE(db->freelist == EDB_FREELIST_PRIMARY, 3111);
    CHECK_CODE(db->objlist == EDB_OBJLIST,  3112);

    // bootstrap the freelist cow block
    memcpy(
        BLOCK(db, EDB_FREELIST_SECONDARY),
        BLOCK(db, EDB_FREELIST_PRIMARY),
        BLOCK_SIZE
    );

    // mark secondary as cow to primary
    mem_hash_set(
        db->txn->blocks, EDB_FREELIST_SECONDARY, EDB_FREELIST_PRIMARY);

    db->freelist = EDB_FREELIST_SECONDARY;

    err:
    return err;
}


int txn_begin(edb *const db) {
    LOG_HERE;
    int err = 0;

    txn_state* ts = txn_state_new();
    ts->next = db->txn;
    ts->nblocks = db->nblocks;

    if (ts->next == NULL) {
        db->txn = ts;
        CHECK(txn_begin_master(db));
    }
    else {
        // TODO need to cow freelist
        u32 freelist = db->freelist;
        txn_modify_block(db, freelist, &db->freelist);

        // not implimented
        CHECK_CODE(0, EDB_ERR_TXN_NOT_IMPLEMENTED);
        db->txn = ts;
    }

    db->txn_id++;

    err:
    return err;
}


int txn_commit_master(edb *const db) {
    LOG_HERE;
    int err = 0;

    txn_state* ts = db->txn;
    CHECK_CODE(ts != NULL, EDB_ERR_TXN_NO_TRANSACTION);
    CHECK_CODE(ts->next == NULL, -3);
    CHECK_CODE(mem_hash_get(ts->blocks, EDB_FREELIST_SECONDARY) ==
            EDB_FREELIST_PRIMARY, -4);

    // clear transacction state
    mem_hash_item* e;
    for (size_t i=0; i<ts->blocks->capacity; i++) {
        e = &ts->blocks->entries[i];

        if (e->k == 0) {
            continue;
        }

        if (e->k == EDB_FREELIST_SECONDARY) {
            // freelist special case
            // we'll handle this below
            continue;
        }

        if (e->k == db->objlist) {
            // objlist
            LOG_DEBUG("copying objlist root back to primary\n", NULL);
            memcpy(BLOCK(db, EDB_OBJLIST), BLOCK(db, db->objlist), BLOCK_SIZE);
            CHECK(freelist_free_block(db, e->k));
            db->objlist = EDB_OBJLIST;
            continue;
        }

        if (e->v == ALLOCATED) {
            // stays allocated
            continue;
        }

        else if (e->v == FREED) {
            // freed blocks actually freed
            CHECK(freelist_free_block(db, e->k));
            continue;
        }

        else {
            // free parent of cow'd blocks
            LOG_HERE;
            CHECK(freelist_free_block(db, e->v));
            continue;
        }
    }

    // handle freelist
    LOG_DEBUG("copying freelist root back to primary\n", NULL);
    memcpy(BLOCK(db, EDB_FREELIST_PRIMARY), BLOCK(db, db->freelist), BLOCK_SIZE);
    db->freelist = EDB_FREELIST_PRIMARY;

    txn_state_free(ts);
    db->txn = NULL;
    db->txn_id++;

    err:
    return err;
}


int txn_commit(edb *const db) {
    LOG_HERE;
    int err = 0;

    txn_state* ts = db->txn;
    CHECK_CODE(ts != NULL, EDB_ERR_TXN_NO_TRANSACTION);

    txn_state* parent = ts->next;
    if (parent == NULL) {
        CHECK(txn_commit_master(db));
        return 0;
    }

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
                freelist_free_block(db, e->k);
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
                freelist_free_block(db, e->v);
            }
            else if (old_block_state == FREED) {
                err = -903;
                goto err;
            }
            else if (old_block_state != 0) {
                // block was modified in the parent
                // collapse the cow+cow into a single modification
                freelist_free_block(db, e->v);
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

    txn_state_free(ts);
    db->txn = parent;
    db->txn_id++;

    err:
    return err;
}


int txn_abort_master(edb *const db) {
    LOG_HERE;
    int err = 0;

    txn_state* ts = db->txn;

    db->freelist = EDB_FREELIST_PRIMARY;
    db->objlist = EDB_OBJLIST;

    CHECK(io_resize(db, ts->nblocks));

    txn_state_free(ts);
    db->txn = NULL;
    db->txn_id++;


    err:
    return err;
}

int txn_abort(edb *const db) {
    int err = 0;
    LOG_DEBUG("%s\n", "txn_abort");

    txn_state* ts = db->txn;
    CHECK_CODE(ts != NULL, EDB_ERR_TXN_NO_TRANSACTION);

    if (ts->next == NULL) {
        err = txn_abort_master(db);
        goto err;
    }

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
        CHECK(freelist_free_block(db, e->k));
    }

    txn_state_free(ts);
    db->txn_id++;

    err:
    return err;
}
