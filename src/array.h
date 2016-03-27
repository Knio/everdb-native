#ifndef ARRAY_H
#define ARRAY_H
#include "edb.h"
#include "page.h"

typedef struct {
    u8 item_size;
    u32 length;
    u32 capacity;
    struct page_header_t;
} array_header;

void array_init(edb *db, const u32 page, const u32 item_size);
int array_resize(edb *db, const u32 page, const u32 length);

u32 array_length(const edb *db, const u32 page);
u32 array_capacity(const edb *db, const u32 page);

void* array_data(edb *db, const u32 page, const u32 index);
void array_get(const edb *db, const u32 page, const u32 index, void* data);
void array_set(edb *db, const u32 page, const u32 index, const void *data);

void array_push(edb *db, const u32 page, const void *data);
void array_pop(edb *db, const u32 page, void* data);

#endif
