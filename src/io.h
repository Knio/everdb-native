#ifndef IO_H
#define IO_H

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EDB_ERR_IO_FILE_OPEN (2001)
#define EDB_ERR_IO_FILE_SIZE (2002)
#define EDB_ERR_IO_SIZE_MAX (2003)


int io_open(edb *db, const char* f_name, int readonly, int overwrite);
int io_close(edb *db);
int io_resize(edb *db, u32 nblocks);


#ifdef __cplusplus
}
#endif
#endif
