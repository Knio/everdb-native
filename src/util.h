#ifndef UTIL_H
#define UTIL_H

#define BLOCK(db, index) ((db)->data + ((index) * BLOCK_SIZE))
#define CHECK(exp) if ((err = (exp))) { goto err; }
#define CHECK_CODE(exp, code) if (!(exp)) { err = (code); goto err; }

#endif
