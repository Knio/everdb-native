#include "hash.h"

#ifdef __linux
#include <string.h> //memcpy
#endif

#define PAGE_TABLE_HASH(db, page) ((page_table_hash*) (db->data + BLOCK_SIZE * page))

u32 hash_hash(void* data, u32 len) {
  return 0;
}


void hash_init(edb* db, const u32 root) {
  page_init(db, root);
}

u32 hash_nitems(const edb* db, const u32 root) {
  page_table_hash* pt = PAGE_TABLE_HASH(db, root);
  return pt->nitems;
};

int hash_get(const edb* db, const u32 root,
    const void* key, u32 nkey, void* value, u32* nvalue) {

  page_table_hash* pt = PAGE_TABLE_HASH(db, root);



  return -1;
}

int hash_set(edb* db, const u32 root,
    const void* key, u32 nkey, const void* value, const u32 nvalue) {

  return -1;
}
