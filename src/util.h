#ifndef UTIL_H
#define UTIL_H

#define BLOCK(db, index) ((db)->data + ((index) * BLOCK_SIZE))

#include <stdio.h>
#ifdef DEBUG
#define LOG_DEBUG(fmt, ...) \
    do { \
        fprintf(stderr, \
            "DEBUG:%s:%d:%s: " fmt, \
            __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
    } while (0);

#define LOG_HERE \
    do { \
        fprintf(stderr, \
            "DEBUG:%s:%d:%s\n", \
            __FILE__, __LINE__, __FUNCTION__); \
    } while (0);
#else

#define LOG_DEBUG(...)
#define LOG_HERE

#endif

#define CHECK(exp) \
    if ((err = (exp))) { goto err; }

#define CHECK_CODE(exp, code) \
    if (!(exp)) { \
        LOG_DEBUG("check failed: (%s), error: %s\n", #exp, #code); \
        err = (code); \
        goto err; \
    }

#endif
