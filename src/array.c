#include "array.h"

void array_init(edb *db, const u32 page, const u32 item_size) {
  page_init(db, page);
  const block page_table = BLOCK(db->data, page);
  array_header *ah = HEADER(page_table, array_header);
  ah->item_size = item_size;
}


int array_resize(edb *db, const u32 page, const u32 length) {
  const block page_table = BLOCK(db->data, page);
  array_header *ah = HEADER(page_table, array_header);
  u32 small_capacity = (BLOCK_SIZE - sizeof(array_header)) / ah->item_size;
  if (length <= small_capacity) {
    // make small block
    page_resize(db, page, 0);
    ah->capacity = small_capacity;
    ah->length = length;
    // zero everyfrom from length..capacity-1
    memset(
      page_table + length * ah->item_size,
      0,
      (small_capacity - length) * ah->item_size);
    return 0;
  }
  const u32 items_per_block = BLOCK_SIZE / ah->item_size;
  const u32 pages = (length + items_per_block - 1) / items_per_block;
  page_resize(db, page, pages);
  ah->capacity = items_per_block * pages;
  ah->length = length;

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

void* array_data(edb *db, const u32 page, const u32 index) {
  const block page_table = BLOCK(db->data, page);
  const array_header *ah = HEADER(page_table, array_header);

  block item_block = page_table;
  u32 local_index = index;
  if (ah->nblocks != 0) {
    const u32 items_per_block = BLOCK_SIZE / ah->item_size;
    item_block = page_get_host_block(db, page, index / items_per_block);
    u32 local_index = index % items_per_block;
  }
  return (void*) (item_block + local_index * ah->item_size);
}

void array_get(const edb *db, const u32 page, const u32 index, void* data) {
  const block page_table = BLOCK(db->data, page);
  const array_header *ah = HEADER(page_table, array_header);

  memcpy(
    array_data(db, page, index),
    data,
    ah->item_size);
}

void array_set(edb *db, const u32 page, const u32 index, void* data) {
  const block page_table = BLOCK(db->data, page);
  const array_header *ah = HEADER(page_table, array_header);

  memcpy(
    data,
    array_data(db, page, index),
    ah->item_size);
}

void array_push(edb *db, const u32 page, void *data) {
  u32 index = array_length(db, page);
  array_resize(db, page, index + 1);
  array_set(db, page, index, data);
}

void array_pop(edb *db, const u32 page, void* data) {
  u32 index = array_length(db, page) - 1;
  array_get(db, page, index, data);
  array_resize(db, page, index);
}
