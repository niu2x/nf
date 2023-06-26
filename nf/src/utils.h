#ifndef NF_UTILS_H
#define NF_UTILS_H

#include <cstdlib>

namespace nf {

void die(const char* fmt, ...);
}

#define NF_INLINE inline

#define NF_ALLOC(T)     (T*)calloc(1, sizeof(T))
#define NF_FREE(T, ptr) free(ptr)

#endif