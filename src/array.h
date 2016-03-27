#ifndef ARRAY_H
#define ARRAY_H
#include "edb.h"
#include "page.h"

typedef struct array_header_t {
    u8 item_size;
    u32 size;
    u32 capacity;
    // page_header_t;
} array_header;

void array_init(edb *db, const u32 page, const u32 item_size);
int array_resize(edb *db, const u32 page, const u32 capacity);

u32 array_length(edb *db, const u32 page);
u32 array_capacity(edb *db, const u32 page);

void* array_get(edb *db, const u32 page, const u32 index);
void array_set(edb *db, const u32 page, const u32 index, const void *data);


void* array_push(edb *db, const u32 page, void *data);
void* array_pop(edb *db, const u32 page);

#endif
