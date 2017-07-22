#ifndef ARRAY_H
#define ARRAY_H

#include "edb.h"
#include "page.h"

#ifdef __cplusplus
extern "C" {
#endif

int array_init(edb* db, const u32 root, const u8 item_size);
int array_resize(edb* db, const u32 root, const u64 length);

int array_length(const edb* db, const u32 root, u64 *const size);
int array_capacity(const edb* db, const u32 root, u64 *const size);

int array_get(const edb* db, const u32 root, const u64 index, void *const data);
int array_set(edb* db, const u32 root, const u64 index, const void *const data);

int array_push(edb* db, const u32 root, const void *const data);
int array_pop(edb* db, const u32 root, void *const data);


#ifdef __cplusplus
}
#endif
#endif
