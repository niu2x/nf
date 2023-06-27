#ifndef NF_API_H
#define NF_API_H

#include <cstdint>

#include <nf/api.h>
#include "utils.h"

namespace nf {

using Pfunc = void (*)(Thread* self, void* ud);
using Reader = const char* (*)(Thread* self, void* ud, size_t* sz);

enum class Error : uint8_t {
    OK,
    OUT_OF_MEMORY,
};

using E = Error;

Error Thread_run_protected(Thread* self, Pfunc f, void* ud);
void Thread_throw(Thread* self, Error err);

Error Thread_load(Thread* self, const char* buff, Size size, const char* name);

struct ZIO;
Error protected_parser(Thread* th, ZIO* z, const char* name);

} // namespace nf

#define NF_ALLOC_P(state, size)                                                \
    (NF_ALLOC(size) ?: (Thread_throw(state, E::OUT_OF_MEMORY), nullptr))

#define NF_REALLOC_P(ptr, size)                                                \
    (NF_REALLOC((ptr), (size))                                                 \
            ?: (Thread_throw(state, E::OUT_OF_MEMORY), nullptr))

#define NF_ALLOC_ARRAY_P(state, T, nr) (T*)NF_ALLOC_P(state, (sizeof(T) * (nr)))

#endif