#include "array.h"

#ifdef __linux
#include <string.h> //memcpy
#endif

int array_init(edb *db, const u32 page, const u8 item_size) {
  page_init(db, page);
  const block page_table = BLOCK(db->data, page);
  array_header *ah = HEADER(page_table, array_header);
  ah->item_size = item_size;
  return 0;
}


int array_resize(edb *db, const u32 page, const u32 length) {
  const block page_table = BLOCK(db->data, page);
  array_header *ah = HEADER(page_table, array_header);

  if (ah->length == length) { return 0; }

  u32 small_capacity = (BLOCK_SIZE - sizeof(array_header)) / ah->item_size;
  if (length <= small_capacity) {
    if (ah->capacity != small_capacity) {
      // make small block
      page_resize(db, page, 0);
      ah->capacity = small_capacity;
    }

    if (ah->length < length) {
      // grow, init new items
      // memset(
      //   page_table + ah->length * ah->item_size,
      //   0,
      //   (length - ah->length) * ah->item_size
      // );
    }

    else if (ah->length < length) {
      // shrink, cleanup truncated items
      memset(
        page_table + length * ah->item_size,
        0,
        (ah->length - length) * ah->item_size
      );
    }

    ah->length = length;
    return 0;
  }

  const u32 items_per_block = BLOCK_SIZE / ah->item_size;
  const u32 blocks = (length + items_per_block - 1) / items_per_block;
  const u32 capacity = items_per_block * blocks;

  { volatile int* p = 0; *p = 1; }

  page_resize(db, page, blocks);
  ah->capacity = capacity;
  ah->length = length;
  return 0;
}


u32 array_length(const edb *db, const u32 page) {
  const block page_table = BLOCK(db->data, page);
  const array_header *ah = HEADER(page_table, array_header);
  return ah->length;
}


u32 array_capacity(const edb *db, const u32 page) {
  const block page_table = BLOCK(db->data, page);
  const array_header *ah = HEADER(page_table, array_header);
  return ah->capacity;
}


void* array_data(const edb *db, const u32 page, const u32 index) {
  const block page_table = BLOCK(db->data, page);
  const array_header *ah = HEADER(page_table, array_header);

  if (index >= ah->length) {
    return NULL;
  }

  block item_block = page_table;
  u32 local_index = index;


  if (ah->nblocks == 0) {
    // small page
  }
  else {
    const u32 items_per_block = BLOCK_SIZE / ah->item_size;
    item_block = page_get_host_block(db, page, index / items_per_block);
    local_index = index % items_per_block;
  }

  return (void*) (item_block + local_index * ah->item_size);
}


int array_get(const edb *db, const u32 page, const u32 index, void* data) {
  const block page_table = BLOCK(db->data, page);
  const array_header *ah = HEADER(page_table, array_header);

  void* element = array_data(db, page, index);
  if (element == NULL) {
    return 1;
  }

  memcpy(
    data,
    element,
    ah->item_size);

  return 0;
}


int array_set(edb *db, const u32 page, const u32 index, const void* data) {
  const block page_table = BLOCK(db->data, page);
  const array_header *ah = HEADER(page_table, array_header);

  void* element = array_data(db, page, index);
  if (element == NULL) {
    return 1;
  }

  memcpy(
    element,
    data,
    ah->item_size);

  return 0;
}


int array_push(edb *db, const u32 page, const void *data) {
  int err = 0;
  u32 index = array_length(db, page);
  if ((err = array_resize(db, page, index + 1))) {
    goto err;
  }
  if ((err = array_set(db, page, index, data))) {
    goto err;
  }
  err:
  return err;
}


int array_pop(edb *db, const u32 page, void* data) {
  int err = 0;
  u32 index = array_length(db, page);
  if ((err = array_get(db, page, index - 1, data))) {
    goto err;
  }
  if ((err = array_resize(db, page, index - 1))) {
    goto err;
  }
  err:
  return err;
}
