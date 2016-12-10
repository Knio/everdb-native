#ifndef ARRAY_H
#define ARRAY_H

#include "core.h"
#include "page.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_ARRAY_INDEX_OUT_OF_BOUNDS (4101)

typedef struct array_header_t {
    u8 item_size;
    u32 length;
} array_header;

#ifdef _MSC_VER
#pragma warning(disable:4201)
#endif
typedef struct page_table_array_t {
  union {
    struct page_table_t;
    struct {
      u8 __padding_array[BLOCK_SIZE
          - sizeof(struct array_header_t)
          - sizeof(struct page_header_t)
      ];
      struct array_header_t;
      // page_header_t
    };
  };
} page_table_array;


int array_init(edb* db, const u32 root, const u8 item_size);
int array_resize(edb* db, const u32 root, const u32 length);

u32 array_length(const edb* db, const u32 root);
u32 array_capacity(const edb* db, const u32 root);

int array_get(const edb* db, const u32 root, const u32 index, void* data);
int array_set(edb* db, const u32 root, const u32 index, const void *data);

int array_push(edb* db, const u32 root, const void *data);
int array_pop(edb* db, const u32 root, void* data);


#ifdef __cplusplus
}
#endif
#endif
