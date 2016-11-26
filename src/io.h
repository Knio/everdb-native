#ifndef IO_H
#define IO_H

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IO_ERROR_FILE_OPEN (1001)
#define IO_ERROR_FILE_SIZE (1002)

int
io_open(edb *db, const char* f_name, int readonly, int overwrite);

void
io_close(edb *db);

int
io_resize(edb *db, u32 nblocks);


#ifdef __cplusplus
}
#endif
#endif
