#include "array.h"

void init_array(edb *db, const u32 page, const u32 item_size) {
  init_page(db, page);
  block page_table = BLOCK(db, page);
  array_header *ah = HEADER(page_table, array_header);
}

int array_resize(edb *db, const u32 page, const u32 capacity);

u32 array_length(edb *db, const u32 page);
u32 array_capacity(edb *db, const u32 page);

void* array_get(edb *db, const u32 page, const u32 index);
void array_set(edb *db, const u32 page, const u32 index, const void *data);

void* array_push(edb *db, const u32 page, void *data);
void* array_pop(edb *db, const u32 page);
