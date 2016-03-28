#ifndef ARRAY_H
#define ARRAY_H
#include "edb.h"
#include "page.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u8 item_size;
    u32 length;
    u32 capacity;
    page_header;
} array_header;

int array_init(edb *db, const u32 page, const u8 item_size);
int array_resize(edb *db, const u32 page, const u32 length);

u32 array_length(const edb *db, const u32 page);
u32 array_capacity(const edb *db, const u32 page);

void* array_data(const edb *db, const u32 page, const u32 index);
int array_get(const edb *db, const u32 page, const u32 index, void* data);
int array_set(edb *db, const u32 page, const u32 index, const void *data);

int array_push(edb *db, const u32 page, const void *data);
int array_pop(edb *db, const u32 page, void* data);

#ifdef __cplusplus
}
#endif

#endif
