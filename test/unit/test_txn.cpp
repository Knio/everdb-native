#define CATCH_CONFIG_MAIN
#include "../../lib/catch.hpp"

#include "../../src/txn.h"


TEST_CASE("txn") {

    mem_hash* m = mem_hash_new();

    mem_hash_put(m, 1, 2);
    REQUIRE(mem_hash_get(m, 1) == 2);
    REQUIRE(mem_hash_get(m, 2) == 0);

    size_t N = 1000000;
    for (uint32_t i=1; i<N; i++) {
        if (i & 1) {
            mem_hash_put(m, i, ~i);
        } else {
            mem_hash_put(m, ~i, i);
        }
    }

    for (uint32_t i=1; i<N; i++) {
        if (i & 1) {
            REQUIRE(mem_hash_get(m,  i) == ~i);
            REQUIRE(mem_hash_get(m, ~i) == 0);
        } else {
            REQUIRE(mem_hash_get(m,  i) == 0);
            REQUIRE(mem_hash_get(m, ~i) == i);
        }
    }

}



