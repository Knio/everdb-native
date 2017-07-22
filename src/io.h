#ifndef IO_H
#define IO_H

#include "edb.h"

#ifdef __cplusplus
extern "C" {
#endif

int io_open(edb *db, const char* f_name, int readonly, int overwrite);
int io_close(edb *db);
int io_resize(edb *db, u32 nblocks);


#ifdef __cplusplus
}
#endif
#endif
