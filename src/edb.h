#ifndef EDB_H
#define EDB_H

#include "core.h"
#include "txn.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EDB_ERROR_FILE_OPEN (1001)
#define EDB_ERROR_FILE_SIZE (1002)
#define ERR_EDB_DB_SIZE_MAX (1003)

int
edb_open(edb *db, const char* f_name, int readonly, int overwrite);

void
edb_close(edb *db);

#ifdef __cplusplus
}
#endif
#endif
