#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#include <sys/types.h> // ftruncate
#include <sys/stat.h>
#include <fcntl.h> // posix_fallocate
#include <sys/mman.h> //mmap
#include <string.h> //memset
#else
#error Unsupported OS
#endif

#include "io.h"
#include "util.h"

int io_open(edb *const db, const char *const fname, int readonly, int overwrite) {
  int err = 0;
  u32 nblocks = 0;
  int is_new = 0;
  memset(db, 0, sizeof(edb));
  #ifdef __linux__
  db->h_file = -1;
  #endif

  if (readonly && overwrite) {
    err = -4;
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
    err = EDB_ERR_IO_FILE_OPEN;
    goto err;
  }

  LARGE_INTEGER f_size;
  if (!GetFileSizeEx(db->h_file, &f_size)) {
    err = EDB_ERR_IO_FILE_SIZE;
    goto err;
  }
  db->filesize = f_size.QuadPart;

  #elif __linux__
  db->h_file = open(fname,
    (readonly ? O_RDONLY : O_RDWR) |
    (overwrite ? O_TRUNC : 0) |
    O_CREAT,
    0644
  );
  if (db->h_file < 0) {
    err = EDB_ERR_IO_FILE_OPEN;
    goto err;
  }
  struct stat fi;
  if (fstat(db->h_file, &fi) < 0) {
    err = EDB_ERR_IO_FILE_SIZE;
    goto err;
  }
  db->filesize = fi.st_size;
  #endif

  if (db->filesize & BLOCK_MASK) {
    err = -5;
    goto err;
  }

  nblocks = (u32) (db->filesize >> BLOCK_BITS);

  if (nblocks == 0) {
    nblocks = 1;
    is_new = 1;
  }

  if ((err = io_resize(db, nblocks))) {
    goto err;
  }

  if (is_new) {
    // new or overwritten file
    // edb_init(db);
  }
  else {
    // existing file
    // edb_check(db);
  }

  return err;

  err:

  io_close(db);

  if (db != NULL) {
    memset(db, 0, sizeof(edb));
  }
  return err;
}


void io_map_close(edb *const db) {
  #ifdef _WIN32
  if (db->data) {
    UnmapViewOfFile(db->data);
    db->data = NULL;
  }
  if (db->h_mapping) {
    CloseHandle(db->h_mapping);
    db->h_mapping = NULL;
  }

  #elif __linux__
  if(db->data) {
    munmap(db->data, db->filesize);
    db->data = NULL;
  }
  #endif
}


int io_close(edb *const db) {
  if (db == NULL) {
    return 0;
  }
  io_map_close(db);

  #ifdef _WIN32
  if (db->h_file) {
    CloseHandle(db->h_file);
  }
  db->h_file = NULL;

  #elif __linux__
  if (db->h_file >= 0) {
    close(db->h_file);
    db->h_file = 0;
  }
  #endif

  db->filesize = 0;
  db->nblocks = 0;

  return 0;
}


int io_resize(edb *const db, u32 nblocks) {
  int err = 0;

  LOG_DEBUG("io_resize nblocks=%d\n", nblocks);

  if (nblocks >= 0xffffff00) {
    return EDB_ERR_IO_SIZE_MAX;
  }
  u64 filesize = nblocks << BLOCK_BITS;

  io_map_close(db);

  #ifdef _WIN32
  long size_hi = 0;
  if (db->filesize > filesize) {
    // truncate file
    size_hi = (long) (filesize >> 32);
    SetFilePointer(
      db->h_file,
      (DWORD) (filesize),
      &size_hi,
      FILE_BEGIN
    );
    if (GetLastError() != 0) {
      err = -34;
      goto err;
    }
    if (SetEndOfFile(db->h_file) == 0) {
      err = -13;
      goto err;
    }
  }

  // win32 grows the file when a mapping is made
  db->h_mapping = CreateFileMapping(
    db->h_file,
    NULL,
    db->readonly ? PAGE_READONLY : PAGE_READWRITE,
    (DWORD) (filesize >> 32), (DWORD) filesize, NULL
  );

  if (db->h_mapping == NULL) {
    err = -2;
    goto err;
  }

  db->data = MapViewOfFile(
    db->h_mapping,
    db->readonly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS,
    0, 0, 0
  );

  if (db->data == NULL) {
    err = -9;
    goto err;
  }

  // ########################
  #elif __linux__
  if (db->filesize < filesize) {
    if (posix_fallocate(db->h_file, (off_t) 0, (off_t) filesize) != 0) {
      perror("posix_fallocate");
      err = -11;
      goto err;
    }
  }
  if (db->filesize > filesize) {
    int x;
    if ((x = ftruncate(db->h_file, (off_t) filesize)) < 0) {
      LOG_DEBUG("ftruncate: %d\n", x);
      perror("ftruncate");
      err = -12;
      goto err;
    }
  }

  db->data = mmap(
    NULL,
    filesize,
    PROT_READ |
      (db->readonly ? 0 : PROT_WRITE),
    MAP_SHARED,
    db->h_file,
    0
  );
  if (db->data == (u8*) MAP_FAILED) {
    db->data = NULL;
    err = -9;
    goto err;
  }
  #endif

  db->filesize = filesize;
  db->nblocks = nblocks;

  return err;

  err:
  io_map_close(db);
  db->filesize = 0;
  db->nblocks = 0;
  return err;
}
