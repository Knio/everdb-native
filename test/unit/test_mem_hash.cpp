#define CATCH_CONFIG_MAIN
#include "../../lib/catch.hpp"

#include "../../src/mem_hash.h"


TEST_CASE("mem_hash") {

    mem_hash* m = mem_hash_new();

    REQUIRE(mem_hash_get(m, 1) == 0);

    mem_hash_set(m, 1, 2);
    REQUIRE(mem_hash_get(m, 1) == 2);
    REQUIRE(mem_hash_get(m, 2) == 0);

    size_t N = 100000;
    for (uint32_t i=1; i<N; i++) {
        if (i & 1) {
            mem_hash_set(m, i, ~i);
        } else {
            mem_hash_set(m, ~i, i);
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

    mem_hash_free(m);

}



