#ifndef EDB_H
#define EDB_H

#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#endif


#define BLOCK_BITS (12) // 4096
#define BLOCK_SIZE (1 << BLOCK_BITS)
#define BLOCK_MASK (BLOCK_SIZE - 1)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Hashdb object
 */
typedef struct {
  int readonly;
#ifdef _WIN32
  HANDLE h_file;
  HANDLE h_mapping;
#else
  int h_file;
  void* h_map;
#endif
  char* data;
  uint64_t size;
} edb;


/**
 * Open or create a edbdb file
 * The database or null is returned in the edbdb param
 * @return 0 on success, -1 on open error, other values on other errors
 */
int edb_open(edb *db, const char* f_name,
    int readonly,
    int overwrite);

void edb_close(edb *db);

/**
 * lookup a key
 * @return value of key
 */
char* edb_get(const edb *db,
    const char* key, uint32_t nkey);

/**
 * Insert or overwrite a key
 */
int edb_put(edb *db,
    const char* key, uint32_t nkey,
    const char* value, uint32_t nvalue);


#ifdef __cplusplus
}
#endif

#endif
