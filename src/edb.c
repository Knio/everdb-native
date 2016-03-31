#include "edb.h"
#include "page.h"

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h> //mmap
#include <string.h> //memset
#else
#error Unsupported OS
#endif


int
edb_allocate(edb *db, u64 size);
void
edb_map_close(edb *db);
int
edb_init(edb *db);
int
edb_check(const edb *db);


int edb_open(edb *db, const char* fname, int readonly, int overwrite) {
  int ret = 0;
  u64 size = 0;
  memset(db, 0, sizeof(edb));


  if (readonly && overwrite) {
    ret = -4;
    goto err;
  }
  db->readonly = readonly;

#ifdef _WIN32

  db->h_file = CreateFileA(
    fname,
    readonly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
    0, NULL,
    overwrite ? CREATE_ALWAYS : OPEN_ALWAYS,
    0, NULL
  );
  if (db->h_file == INVALID_HANDLE_VALUE) {
    ret = -1;
    goto err;
  }

  LARGE_INTEGER f_size;
  if (!GetFileSizeEx(db->h_file, &f_size)) {
    ret = -3;
    goto err;
  }
  db->size = f_size.QuadPart;
#elif __linux__
  db->h_file = open(fname,
    (readonly ? O_RDONLY : O_RDWR) |
    (overwrite ? O_TRUNC : 0) |
    O_CREAT,
    0644
  );
  if (db->h_file < 0) {
    ret = -1;
    goto err;
  }
  struct stat fi;
  if (fstat(db->h_file, &fi) < 0) {
    ret = -3;
    goto err;
  }
  db->size = fi.st_size;
#endif
  if (db->size & BLOCK_MASK) {
    ret = -5;
    goto err;
  }

  size = db->size;

  if (size == 0) {
    size = BLOCK_SIZE;
  }

  ret = edb_allocate(db, size);
  if (ret < 0) {
    ret -= 100;
    goto err;
  }

  if (db->size == 0) {
    // new or overwritten file
    // edb_init(db);
  }
  else {
    // existing file
    // edb_check(db);
  }

  return ret;

  err:

  edb_close(db);

  if (db != NULL) {
    memset(db, 0, sizeof(edb));
  }
  return ret;
}

void edb_close(edb *db) {
  if (db == NULL) return;

  edb_map_close(db);
#ifdef _WIN32
  if (db->h_file != INVALID_HANDLE_VALUE) {
    CloseHandle(db->h_file);
    db->h_file = INVALID_HANDLE_VALUE;
  }
#elif __linux__
  if (db->h_file >= 0) {
    close(db->h_file);
    db->h_file = -1;
  }
#endif
}

void edb_map_close(edb *db) {
  if (db == NULL) return;
#ifdef _WIN32
  if (db->data != NULL) {
    UnmapViewOfFile(db->data);
    db->data = NULL;
  }

  if (db->h_mapping != NULL) {
    CloseHandle(db->h_mapping);
    db->h_mapping = NULL;
  }
#elif __linux__
  if(db->data != NULL) {
    munmap(db->data, db->size);
    db->data = NULL;
  }
#endif
}

int edb_allocate(edb *db, u64 size) {
  if (db == NULL) return -1;
  int ret = 0;
  long size_hi = 0;

  edb_map_close(db);

  #ifdef _WIN32
  if (db->size > size) {
    // truncate file
    size_hi = (long) (size >> 32);
    SetFilePointer(
      db->h_file,
      (DWORD) (size),
      &size_hi,
      FILE_BEGIN
    );
    if (GetLastError() != 0) {
      ret = -34;
      goto err;
    }
    if (SetEndOfFile(db->h_file) == 0) {
      ret = -13;
      goto err;
    }
  }

  db->h_mapping = CreateFileMapping(
    db->h_file,
    NULL,
    db->readonly ? PAGE_READONLY : PAGE_READWRITE,
    (DWORD) (size >> 32), (DWORD) size, NULL
  );

  if (db->h_mapping == NULL) {
    ret = -2;
    goto err;
  }

  db->data = (block) MapViewOfFile(
    db->h_mapping,
    db->readonly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS,
    0, 0, 0
  );

  if (db->data == NULL) {
    ret = -9;
    goto err;
  }

  #elif __linux__
  if (db->size < size) {
    if (posix_fallocate(db->h_file, 0, size) != 0) {
      ret = -11;
      goto err;
    }
  }
  if (db->size > size) {
    if (ftruncate(db->h_file, size) < 0) {
      ret = -12;
      goto err;
    }
  }
  db->data = (block) mmap(
    NULL,
    size,
    PROT_READ |
      (db->readonly ? 0 : PROT_WRITE),
    MAP_SHARED,
    db->h_file,
    0
  );
  if(db->data == (char*) MAP_FAILED) {
    db->data = NULL;
    ret = -9;
    goto err;
  }
#endif
  db->size = size;

  return ret;

  err:
  edb_map_close(db);
  db->size = 0;
  return ret;
}

/*
int edb_check(const edb *db) {
  return 0;
}

int edb_init(edb *db) {
  return 0;
}

char* edb_get(const edb *db,
    const char* key, uint32_t nkey) {

  return NULL;
}

int edb_put(edb *db,
    const char* key, uint32_t nkey,
    const char* value, uint32_t nvalue) {

  return 0;
}
*/

