#define CATCH_CONFIG_MAIN
#include "../lib/catch.hpp"

#include "../src/edb.h"
#include "../src/page.h"
#include "../src/array.h"
#include "../src/hash.h"

#ifdef __linux__
#include <string.h> //memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


TEST_CASE("structs") {
  REQUIRE(sizeof(page_header) == 4);
  REQUIRE(sizeof(array_header) == 12);

}

