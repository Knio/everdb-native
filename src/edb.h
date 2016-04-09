#ifndef EDB_H
#define EDB_H

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EDB_ERROR_FILE_OPEN (1001)
#define EDB_ERROR_FILE_SIZE (1002)

typedef struct edb_root_t {
  u32 txn_id;
  u32 committed;
  u32 freelist;
} edb_root;


int
edb_open(edb *db, const char* f_name,
    int readonly,
    int overwrite);

void
edb_close(edb *db);

int
edb_resize(edb *db, u32 nblocks);



#ifdef __cplusplus
}
#endif
#endif
