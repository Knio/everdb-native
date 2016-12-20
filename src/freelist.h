#ifndef FREELIST_H
#define FREELIST_H

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

int freelist_allocate_block(edb* db, u32 *new_block);
int freelist_free_block(edb* db, u32 block);

#ifdef __cplusplus
}
#endif

#endif /* ifndef FREELIST_H */


