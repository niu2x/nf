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
    LOAD,
    OP_NUM,
    BUG,
};

using E = Error;
struct TValue;
struct Func;

Error Thread_run_protected(Thread* self, ProtectedFunc f, void* ud);
void Thread_throw(Thread* self, Error err, const char* = nullptr);

Error Thread_load(Thread* self, const char* buff, Size size, const char* name);
void Thread_run(Thread*, const char* code);
void Thread_push(Thread* self, TValue* tv);
void Thread_push_index(Thread* self, StackIndex index);
void Thread_push_pc(Thread* self, const Instruction* ins);
Error Thread_pcall(Thread* self, ProtectedFunc f, void* ud);
void Thread_push_func(Thread* self, Func* f);

const Instruction* Thread_pop_pc(Thread* self);
StackIndex Thread_pop_index(Thread* self);

struct ZIO;
Error protected_parser(Thread* th, ZIO* z, const char* name);

struct Str;
Str* Str_new(Thread* th, const char* ptr, Size nr);
Str* Str_concat(Thread* th, Str*, Str*);

struct StrTab;
void StrTab_insert(Thread* th, StrTab* self, Str* str);
Str* StrTab_search(StrTab* self, const char* str, Size len, Hash hash);

} // namespace nf

#define NF_ALLOC_P(th, size)                                                   \
    (NF_ALLOC(size) ?: (Thread_throw((th), E::OUT_OF_MEMORY), nullptr))

#define NF_REALLOC_P(th, ptr, size)                                            \
    (NF_REALLOC((ptr), (size))                                                 \
            ?: (Thread_throw((th), E::OUT_OF_MEMORY), nullptr))

#define NF_ALLOC_ARRAY_P(th, T, nr) (T*)NF_ALLOC_P(th, (sizeof(T) * (nr)))
#define NF_REALLOC_ARRAY_P(th, old_ptr, T, nr)                                 \
    (T*)NF_REALLOC_P((th), (old_ptr), (sizeof(T) * (nr)))

#define normalize_stack_index(th, i)                                           \
    ((i) >= 0 ? i : ((th)->top - (th)->base + i))
#define stack_slot(th, index)                                                  \
    ((th)->base + normalize_stack_index((th), (index)))

#endif