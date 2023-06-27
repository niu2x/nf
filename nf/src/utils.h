#ifndef NF_UTILS_H
#define NF_UTILS_H

#include <cstdlib>
#include <new>

namespace nf {

void die(const char* fmt, ...);
}

#define NF_INLINE inline

#define NF_ALLOC(T)     (T*)calloc(1, sizeof(T))
#define NF_FREE(T, ptr) free(ptr)

// #define NF_PLACEMENT_DELETE(T, addr) (addr)->~

template <class T>
void NF_PLACEMENT_NEW(T* ptr)
{
    new (ptr) T;
}

template <class T>
void NF_PLACEMENT_DELETE(T* ptr)
{
    ptr->~T();
}

#endif