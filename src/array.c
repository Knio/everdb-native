#include "array.h"

#ifdef __linux
#include <string.h> //memcpy
#endif

#define PAGE_TABLE_ARRAY(db, page) ((page_table_array*) (db->data + BLOCK_SIZE * page))

int array_init(edb* db, const u32 root, const u8 item_size) {
  page_init(db, root);
  page_table_array* pt = PAGE_TABLE_ARRAY(db, root);
  pt->item_size = item_size;
  return 0;
}


int array_resize(edb* db, const u32 root, const u32 length) {
  page_table_array* pt = PAGE_TABLE_ARRAY(db, root);

  if (pt->length == length) { return 0; }

  const u32 small_capacity = (BLOCK_SIZE
     - sizeof(array_header)
     - sizeof(page_header)
  ) / pt->item_size;

  if (length <= small_capacity) {
    if (pt->capacity != small_capacity) {
      // make small block
      page_resize(db, root, 0);
      pt = PAGE_TABLE_ARRAY(db, root);
      pt->capacity = small_capacity;
    }

    if (pt->length < length) {
      // grow, init new items
      // memset(
      //   pt + ah->length * ah->item_size,
      //   0,
      //   (length - ah->length) * ah->item_size
      // );
    }

    else if (pt->length < length) {
      // shrink, cleanup truncated items
      memset(
        pt
            + (length * pt->item_size),
        0,
        (pt->length - length) * pt->item_size
      );
    }

    pt->length = length;
    return 0;
  }

  const u32 items_per_block = BLOCK_SIZE / pt->item_size;
  const u32 blocks = (length + items_per_block - 1) / items_per_block;
  const u32 capacity = items_per_block * blocks;

  if (pt->capacity == capacity && pt->length < length) {
    // shrink, cleanup last block
    char* last_block = BLOCK(db, page_get_host_index(db, root, blocks - 1));
    memset(
      last_block
        + (length % items_per_block) * pt->item_size,
      0,
      (pt->length - length) * pt->item_size
    );
  }
  else {
    page_resize(db, root, blocks);
    pt = PAGE_TABLE_ARRAY(db, root);
  }

  pt->capacity = capacity;
  pt->length = length;
  return 0;
}


u32 array_length(const edb* db, const u32 root) {
  const page_table_array* pt = PAGE_TABLE_ARRAY(db, root);
  return pt->length;
}


u32 array_capacity(const edb* db, const u32 root) {
  const page_table_array* pt = PAGE_TABLE_ARRAY(db, root);
  return pt->capacity;
}


void* array_data(const edb* db, const u32 root, const u32 index) {
  const page_table_array* pt = PAGE_TABLE_ARRAY(db, root);

  if (index >= pt->length) {
    return NULL;
  }

  char* item_block = pt;
  u32 local_index = index;

  if (pt->nblocks == 0) {
    // small
  }
  else {
    const u32 items_per_block = BLOCK_SIZE / pt->item_size;
    item_block = BLOCK(db, page_get_host_index(db, root, index / items_per_block));
    local_index = index % items_per_block;
  }

  return (void*) (item_block + (local_index * pt->item_size));
}


int array_get(const edb* db, const u32 root, const u32 index, void* data) {
  const page_table_array* pt = PAGE_TABLE_ARRAY(db, root);

  void* element = array_data(db, root, index);
  if (element == NULL) {
    return 1;
  }

  memcpy(
    data,
    element,
    pt->item_size);

  return 0;
}


int array_set(edb* db, const u32 root, const u32 index, const void* data) {
  const page_table_array* pt = PAGE_TABLE_ARRAY(db, root);

  void* element = array_data(db, root, index);
  if (element == NULL) {
    return 1;
  }

  memcpy(
    element,
    data,
    pt->item_size);

  return 0;
}


int array_push(edb* db, const u32 root, const void *data) {
  int err = 0;
  u32 index = array_length(db, root);
  if ((err = array_resize(db, root, index + 1))) {
    goto err;
  }
  if ((err = array_set(db, root, index, data))) {
    goto err;
  }
  err:
  return err;
}


int array_pop(edb* db, const u32 root, void* data) {
  int err = 0;
  u32 index = array_length(db, root);
  if ((err = array_get(db, root, index - 1, data))) {
    goto err;
  }
  if ((err = array_resize(db, root, index - 1))) {
    goto err;
  }
  err:
  return err;
}
