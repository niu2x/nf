#ifndef NF_API_H
#define NF_API_H

#include <cstdint>

#include <nf/api.h>
#include "basic_types.h"
#include "utils.h"

namespace nf {

using ProtectedFunc = void (*)(Thread* self, void* ud);
using CFunc = int (*)(Thread* self);
using Reader = const char* (*)(Thread* self, void* ud, size_t* sz);

enum class Error : uint8_t {
    OK,
    OUT_OF_MEMORY,
    PARSE,
};

using E = Error;
struct TValue;

Error Thread_run_protected(Thread* self, ProtectedFunc f, void* ud);
void Thread_throw(Thread* self, Error err);

Error Thread_load(Thread* self, const char* buff, Size size, const char* name);
void Thread_run(Thread*, const char* code);
void Thread_push(Thread* self, TValue* tv);
void Thread_push_index(Thread* self, Index index);
void Thread_push_pc(Thread* self, const Instruction* ins);

struct ZIO;
Error protected_parser(Thread* th, ZIO* z, const char* name);

} // namespace nf

#define NF_ALLOC_P(th, size)                                                   \
    (NF_ALLOC(size) ?: (Thread_throw(th, E::OUT_OF_MEMORY), nullptr))

#define NF_REALLOC_P(ptr, size)                                                \
    (NF_REALLOC((ptr), (size)) ?: (Thread_throw(th, E::OUT_OF_MEMORY), nullptr))

#define NF_ALLOC_ARRAY_P(th, T, nr) (T*)NF_ALLOC_P(th, (sizeof(T) * (nr)))

#define normalize_stack_index(th, i) ((i) >= 0 ?: ((th)->top - (th)->base + i))
#define stack_slot(th, index)                                                  \
    ((th)->base + normalize_stack_index((th), (index)))

#endif