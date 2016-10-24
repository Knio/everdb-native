#include "array.h"

#ifdef __linux
#include <stdlib.h> // malloc
#include <string.h> // memset, memcpy
#endif

#define PAGE_TABLE_ARRAY(db, page) ((page_table_array*) (db->data + BLOCK_SIZE * page))

int array_init(edb* const db, const u32 root, const u8 item_size) {
  page_init(db, root);
  page_table_array* pt = PAGE_TABLE_ARRAY(db, root);
  pt->item_size = item_size;
  return 0;
}

int array_resize(edb* const db, const u32 root, const u32 capacity) {
  int err = 0;
  u8* data = NULL;
  page_table_array* pt = PAGE_TABLE_ARRAY(db, root);

  const u32 current_capacity = array_capacity(db, root);

  if (capacity == current_capacity) {
    goto err;
  }

  const u32 small_capacity = (BLOCK_SIZE
     - sizeof(array_header)
     - sizeof(page_header)
  ) / pt->item_size;

  if (capacity <= small_capacity) {
    if (pt->nblocks != 0) {
      // make small block
      data = malloc(BLOCK_SIZE);
      u8* first_block = BLOCK(db, page_get_host_index(db, root, 0));
      memcpy(data, first_block, small_capacity * pt->item_size);
      if ((err = page_resize(db, root, 0))) {
          goto err;
      }
      pt = PAGE_TABLE_ARRAY(db, root);
      memcpy(pt->data, data, small_capacity * pt->item_size);
      free(data);
      data = NULL;
    }
    else {
      // already a smll block
    }

    // TODO zero unused elements?

    if (pt->length > small_capacity) {
      pt->length = small_capacity;
    }
    return 0;
  }

  const u32 items_per_block = BLOCK_SIZE / pt->item_size;
  const u32 nblocks = (capacity + items_per_block - 1) / items_per_block;
  const u32 new_capacity = nblocks * items_per_block;

  if (pt->nblocks == nblocks) {

    // if (pt->length > length) {
    //   shrink, cleanup last block
    //   u8* last_block = BLOCK(db, page_get_host_index(db, root, nblocks - 1));
    //   memset(
    //     last_block
    //       + (length % items_per_block) * pt->item_size,
    //     0,
    //     (pt->length - length) * pt->item_size
    //   );
    // }
  }

  else { // nblocks is different
    if (pt->nblocks == 0) {
      // small_capacity data is stored where the page table will go,
      // need to save it
      data = malloc(BLOCK_SIZE);
      memcpy(data, pt->data, small_capacity * pt->item_size);
      memset(pt->data, 0, small_capacity * pt->item_size);
    }

    // if this is the freelist, push/pop can happen here
    if ((err = page_resize(db, root, nblocks))) {
      goto err;
    }
    pt = PAGE_TABLE_ARRAY(db, root);
    if (data) {
      u8* first_block = BLOCK(db, page_get_host_index(db, root, 0));
      memcpy(first_block, data, small_capacity * pt->item_size);
      free(data);
      data = NULL;
    }
  }

  if (pt->length > new_capacity) {
    pt->length = new_capacity;
  }

  err:
  if (data) {
    free(data);
    data = NULL;
  }

  return err;
}


u32 array_length(const edb* const db, const u32 root) {
  const page_table_array const* pt = PAGE_TABLE_ARRAY(db, root);
  return pt->length;
}


u32 array_capacity(const edb* const db, const u32 root) {
  const page_table_array const* pt = PAGE_TABLE_ARRAY(db, root);
  if (pt->nblocks == 0) {
    // small
    return (BLOCK_SIZE
       - sizeof(array_header)
       - sizeof(page_header)
    ) / pt->item_size;
  }
  const u32 items_per_block = BLOCK_SIZE / pt->item_size;
  return pt->nblocks * items_per_block;
}


static inline void* array_data(const edb* const db, const u32 root, const u32 index) {
  const page_table_array const* pt = PAGE_TABLE_ARRAY(db, root);

  if (index >= pt->length) {
    return NULL;
  }

  const u8* block = pt->data;
  u32 local_index = index;

  if (pt->nblocks == 0) {
    // small
  }
  else {
    const u32 items_per_block = BLOCK_SIZE / pt->item_size;
    block = BLOCK(db, page_get_host_index(db, root, index / items_per_block));
    local_index = index % items_per_block;
  }

  return (void*) (block + (local_index * pt->item_size));
}


int array_get(const edb* const db, const u32 root, const u32 index, void* data) {
  const page_table_array const* pt = PAGE_TABLE_ARRAY(db, root);

  const void const* element = array_data(db, root, index);
  if (element == NULL) {
    return ERR_ARRAY_INDEX_OUT_OF_BOUNDS;
  }

  memcpy(
    data,
    element,
    pt->item_size);

  return 0;
}


int array_set(edb* const db, const u32 root, const u32 index, const void* data) {
  const page_table_array* pt = PAGE_TABLE_ARRAY(db, root);

  void* const element = array_data(db, root, index);
  if (element == NULL) {
    return ERR_ARRAY_INDEX_OUT_OF_BOUNDS;
  }

  memcpy(
    element,
    data,
    pt->item_size);

  return 0;
}


int array_push(edb* const db, const u32 root, const void *data) {
  int err = 0;

  page_table_array* pt = PAGE_TABLE_ARRAY(db, root);
  const u32 capacity = array_capacity(db, root);

  if (pt->length + 1 > capacity) {
    if ((err = array_resize(db, root, pt->length + 1))) {
      goto err;
    }
    pt = PAGE_TABLE_ARRAY(db, root);
  }

  pt->length++;
  if ((err = array_set(db, root, pt->length - 1, data))) {
    goto err;
  }
  err:
  return err;
}


int array_pop(edb* const db, const u32 root, void* data) {
  int err = 0;
  page_table_array* const pt = PAGE_TABLE_ARRAY(db, root);

  if ((err = array_get(db, root, pt->length - 1, data))) {
    goto err;
  }
  pt->length--;

  // TODO shrink
  // if ((err = array_resize(db, root, index - 1))) {
  //   goto err;
  // }
  err:
  return err;
}
