#ifndef MEM_HASH_H
#define MEM_HASH_H

#ifdef _WIN32
#error unimplimented
#elif __linux__
#include <stdlib.h> // malloc
#else
#error Unsupported OS
#endif

#include "core.h"


// fast u32->u32 in-memory hash table

static const u32 HASH_SIZES[] = {
  3, 7,
  13, 23, 41, 71,
  127, 223, 373, 631,
  1061, 1777, 1801, 3011, 5021, 8369,
  13963, 23279, 38803, 64679,
  107827, 179717, 299539, 499253, 832103,
  1386839,  2311409, 3852361, 6420683
};



typedef struct mem_hash_item_t {
  u32 k;
  u32 v;
} mem_hash_item;


typedef struct mem_hash_t {
  u32 count;
  u32 capacity;
  mem_hash_item* entries;
} mem_hash;


mem_hash* mem_hash_new();
static inline u32 mem_hash_hash(u32 k);
static inline u32 mem_hash_probe(u32 h, u32 i);
void mem_hash_resize(mem_hash *t);
void mem_hash_set(mem_hash *t, u32 k, u32 v);
static inline u32 mem_hash_get(const mem_hash *t, u32 k);


static inline u32 mem_hash_hash(u32 k) {
  k = ((k >> 16) ^ k) * 0x45d9f3b;
  k = ((k >> 16) ^ k) * 0x45d9f3b;
  k = ((k >> 16) ^ k);
  return k;
}

static inline u32 mem_hash_probe(u32 h, u32 i) {
  return h + (i * i + i) / 2;
}

void mem_hash_resize(mem_hash *t) {
  u32 capacity = t->capacity;

  t->capacity = t->count * 2 + 1;
  for (u32 i=0; i<sizeof(HASH_SIZES); i++) {
    if (HASH_SIZES[i] >= t->capacity) {
      t->capacity = HASH_SIZES[i];
      break;
    }
  }
  if (capacity >= t->capacity) {
    return;
  }

  mem_hash_item* entries = t->entries;
  t->entries = (mem_hash_item*)calloc(t->capacity, sizeof(mem_hash_item));
  t->count = 0;

  for (u32 i=0; i<capacity; i++) {
    mem_hash_item* e = &entries[i];
    if (e->k != 0) {
      mem_hash_set(t, e->k, e->v);
    }
  }
  free(entries);
}


void mem_hash_set(mem_hash *t, u32 k, u32 v) {
  if (t->count * 2 + 1 > t->capacity) {
    mem_hash_resize(t);
  }
  u32 h = mem_hash_hash(k);
  u32 i = 0;
  // TODO swap found entry with first entry for load balancing
  mem_hash_item* entry;
  do {
    entry = &(t->entries[mem_hash_probe(h, i++) % t->capacity]);
  } while (entry->k != 0 && entry->k != k);
  if (entry->k == 0) {
    t->count++;
  }
  entry->k = k;
  entry->v = v;
}

static inline u32 mem_hash_get(const mem_hash *t, u32 k) {
  u32 h = mem_hash_hash(k);
  u32 i = 0;
  mem_hash_item* entry;
  do {
    entry = &t->entries[mem_hash_probe(h, i++) % t->capacity];
  } while (entry->k != 0 && entry->k != k);
  return entry->v;
}

mem_hash* mem_hash_new() {
  mem_hash*m = (mem_hash*)calloc(1, sizeof(mem_hash));
  return m;
}


void mem_hash_free(mem_hash* t) {
  free(t->entries);
  free(t);
}

#endif /* ifndef MEM_HASH_H */
