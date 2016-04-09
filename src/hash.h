#ifndef HASH_H
#define HASH_H

#include "core.h"
#include "page.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BUCKET_SIZE (256)


typedef struct hash_header_t {
  u32 level;
  u32 split;
  u32 nitems;
} hash_header;


typedef struct hash_bucket_entry_t {
  u16 offset;
  u16 length;
} hash_bucket_entry;


typedef struct hash_bucket_t {
  hash_bucket_entry entries[BUCKET_SIZE];
  char* data[BLOCK_SIZE - sizeof(hash_bucket_entry) * BUCKET_SIZE];
} hash_bucket;


typedef struct page_table_hash_t {
  union {
    struct page_table_t;
    struct {
      u8 __padding_array[BLOCK_SIZE
          - sizeof(struct hash_header_t)
          - sizeof(struct page_header_t)
      ];
      struct hash_header_t;
      // page_header_t
    };
  };
} page_table_hash;


void hash_init(edb* db, const u32 root);

u32 hash_nitems(const edb* db, const u32 root);

int hash_get(const edb* db, const u32 root,
  const void* key, u32 nkey, void* value, u32* nvalue);

int hash_set(edb* db, const u32 root,
  const void* key, u32 nkey, const void* value, const u32 nvalue);


#ifdef __cplusplus
}
#endif
#endif
