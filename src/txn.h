#ifndef TXN_H
#define TXN_H

int txn_begin(edb *db);
int txn_begin_master(edb *db);
int txn_allocate_block(edb *db);
int txn_free_block(edb *db, u32 block);
int txn_abort(edb *db);
int txn_commit(edb *db);
int txn_commit_master(edb *db);


#include <stdlib.h>

#include "core.h"


static const u32 BASE = 95629l;
static const u32 HASH_SIZES[] = {
  13, 23, 41, 71,
  127, 223, 373, 631,
  1061, 1777, 1801, 3011, 5021, 8369,
  13963, 23279, 38803, 64679,
  107827, 179717, 299539, 499253, 832103,
  1386839,  2311409, 3852361, 6420683
};


// fast u32->u32 in-memory hash table

typedef struct mem_hash_item_t {
  u32 k;
  u32 v;
} mem_hash_item;


typedef struct mem_hash_t {
  u32 count;
  u32 capacity;
  mem_hash_item* entries;
} mem_hash;


u32 mem_hash_hash(u32 k);
u32 mem_hash_probe(u32 h, u32 i);
void mem_hash_resize(mem_hash *t);
void mem_hash_put(mem_hash *t, u32 k, u32 v);
u32 mem_hash_get(mem_hash *t, u32 k);


u32 mem_hash_hash(u32 k) {
  u32 b = BASE;
  u32 h = 1;
  while (k) {
    if (k&1) h = h * b;
    b = b * b;
    k <<= 1;
  }
  return h;
}

u32 mem_hash_probe(u32 h, u32 i) {
  return h + (i * i + i) / 2;
}

void mem_hash_resize(mem_hash *t) {
  // old table
  u32 capacity = t->capacity;
  mem_hash_item* entries = t->entries;

  t->capacity = (t->count + 1) * 3 / 2;
  for (u32 i=0; i<sizeof(HASH_SIZES); i++) {
    if (HASH_SIZES[i] >= capacity) {
      t->capacity = HASH_SIZES[i];
      break;
    }
  }
  if (t->capacity == capacity) return;

  t->entries = calloc(capacity, sizeof(mem_hash_item));

  for (u32 i=0; i<capacity; i++) {
    mem_hash_item* e = &entries[i];
    if (e->k == 0) continue;
    mem_hash_put(t, e->k, e->v);
  }
  free(entries);
}


void mem_hash_put(mem_hash *t, u32 k, u32 v) {
  mem_hash_resize(t);
  u32 h = mem_hash_hash(k);
  u32 i = 0;
  mem_hash_item* first_entry;
  mem_hash_item* entry;
  do {
    entry = &t->entries[mem_hash_probe(h, i++)];
  } while (entry->k != 0 && entry->k != k);
  entry->v = v;
}

u32 mem_hash_get(mem_hash *t, u32 k) {
  u32 h = mem_hash_hash(k);
  u32 i = 0;
  mem_hash_item* entry;
  do {
    entry = &t->entries[mem_hash_probe(h, i++)];
  } while (entry->k != 0 && entry->k != k);
  return entry->v;
}


typedef struct txn_state_t {
  u32 nblocks;
  mem_hash freed;
  mem_hash modified;
  mem_hash allocated;
} txn_state;


int txn_begin(edb *db) {

}


#endif /* ifndef TXN_HEADER_INCLUDE */
