#ifndef NF_UTILS_H
#define NF_UTILS_H

#include <stdlib.h>
#include <cstdlib>
#include <new>

namespace nf {

void die(const char* fmt, ...);
}

#define NF_INLINE inline

#define NF_ALLOC(size)        malloc(size)
#define NF_FREE(ptr)          free(ptr)
#define NF_REALLOC(ptr, size) realloc(ptr, size)

#define NF_ALLOC_T(T)         (T*)NF_ALLOC(sizeof(T))
#define NF_ALLOC_ARRAY(T, nr) (T*)NF_ALLOC(sizeof(T) * (nr))

// #define NF_PLACEMENT_DELETE(T, addr) (addr)->~

// template <class T>
// void NF_PLACEMENT_NEW(T* ptr)
// {
//     new (ptr) T;
// }

// template <class T>
// void NF_PLACEMENT_DELETE(T* ptr)
// {
//     ptr->~T();
// }

#endif