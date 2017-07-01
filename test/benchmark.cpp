#define CATCH_CONFIG_MAIN
#include "../lib/catch.hpp"

#include <cstdio>
#include <time.h>
#include <stdint.h>

#include "../src/edb.h"
#include "../src/array.h"
#include "../src/btree.h"


uint64_t now() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * 1000000000 + t.tv_nsec;
}

uint64_t duration(uint64_t start) {
    return now() - start;
}

edb* db;
obj_handle* h;
const int N = 2000000;

uint64_t benchmark_write() {
    uint64_t start = now();

    int r;
    for (int i = 0; i < N; i++) {
        r = edb_array_push(h, &i);
        if (r) {
            abort();
        }
        if ((i % 100000) == 0) {
            edb_txn_commit(db);
            edb_txn_begin(db);
        }
    }

    return duration(start);
}


uint64_t benchmark_read() {
    uint64_t start = now();

    int x;
    int r;
    for (int i = 0; i < N; i++) {
        r = edb_array_get(h, i, &x);
        if ((r) || (x != i)) {
            abort();
        }
    }

    return duration(start);
}


TEST_CASE("benchmark") {

    REQUIRE(edb_open(&db, "benchmark.db", 0, 1) == 0);
    REQUIRE(edb_txn_begin(db) == 0);
    REQUIRE(edb_array_create(db, &h, sizeof(int)) == 0);

    printf("write %d in %.3fs\n", N, benchmark_write() / 1.0e9);
    printf("read %d in %.3fs\n", N, benchmark_read() / 1.0e9);

    // REQUIRE(edb_txn_abort(db) == 0);
    REQUIRE(edb_txn_commit(db) == 0);
    REQUIRE(edb_close(db) == 0);

}

