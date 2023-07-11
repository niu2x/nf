#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include "object.h"
#include "api.h"
#include "utils.h"
#include "zio.h"
#include "bytecode.h"

namespace nf {

const Size basic_ci_nr = 8;
const Size basic_stack_nr = 32;

static void Thread_stack_init(Thread* self)
{
    self->stack = NF_ALLOC_ARRAY_P(self, TValue, basic_stack_nr);
    self->stack_nr = basic_stack_nr;
    self->top = self->stack;
    self->base = self->stack;
}

static void StrTab_init(Thread* th, StrTab* self)
{
    self->buckets_nr = 0;
    self->buckets = nullptr;

    self->buckets = NF_REALLOC_ARRAY_P(th, self->buckets, Str*, 31);
    self->buckets_nr = 31;
    for (Index i = 0; i < 31; i++)
        self->buckets[i] = nullptr;
}

void StrTab_insert(Thread* th, StrTab* self, Str* str)
{
    Index bucket_index = str->hash % self->buckets_nr;
    // Str* ptr = self->buckets[bucket_index];
    // while (ptr) {
    //     if (ptr->hash == str->hash && ptr->nr == str->nr) {
    //         if (memcmp(ptr->base, str->base, str->nr) == 0) {
    //             return false;
    //         }
    //     }
    //     ptr = (Str*)(ptr->next);
    // }

    str->next = self->buckets[bucket_index];
    self->buckets[bucket_index] = str;
}

Str* StrTab_search(StrTab* self, const char* str, Size len, Hash hash)
{
    Index bucket_index = hash % self->buckets_nr;
    Str* ptr = self->buckets[bucket_index];
    while (ptr) {
        if (ptr->hash == hash && ptr->nr == len) {
            if (memcmp(ptr->base, str, len) == 0) {
                return ptr;
            }
        }
        ptr = (Str*)(ptr->next);
    }

    return nullptr;
}

static void Thread_init_step_one(Thread* self)
{
    TValue_set_nil(&(self->gt));
    TValue_set_nil(Thread_registry(self));
    auto g = Thread_global(self);
    g->root = self;
    StrTab_init(self, &(g->str_tab));

    self->error_jmp = nullptr;

    self->stack = nullptr;
    self->stack_nr = 0;

    self->pc = nullptr;

    self->base = nullptr;
    self->top = nullptr;
}

static void Thread_init_step_two(Thread* self, void* unused)
{
    Thread_stack_init(self);
}

struct Twin {
    Thread thread;
    GlobalState global;
};

Thread* Thread_open()
{
    auto twin = NF_ALLOC_T(Twin);
    if (!twin)
        return nullptr;

    auto self = &(twin->thread);
    Thread_global(self) = &(twin->global);
    self->type = Type::Thread;

    Thread_init_step_one(self);

    if (E::OK != Thread_run_protected(self, Thread_init_step_two, nullptr)) {
        Thread_close(self);
        self = nullptr;
    }

    return self;
}

void Thread_close(Thread* self)
{
    NF_FREE(self->stack);
    NF_FREE(self);
}

Error Thread_run_protected(Thread* self, ProtectedFunc f, void* ud)
{
    LongJmp recover;
    recover.status = E::OK;
    recover.prev = self->error_jmp;
    self->error_jmp = &recover;

    if (setjmp(recover.b) >= 0) {
        f(self, ud);
    }

    self->error_jmp = recover.prev;
    return recover.status;
}

void Thread_throw(Thread* self, Error err)
{
    if (self->error_jmp) {
        self->error_jmp->status = err;
        longjmp(self->error_jmp->b, -1);
    } else {
        fprintf(stderr, "throw without protect: %d\n", (int)err);
        self->status = err;
        exit(EXIT_FAILURE);
    }
}

struct LoadS {
    const char* s;
    size_t size;
};

static const char* LoadS_read(LoadS* self, size_t* size)
{
    if (self->size == 0)
        return nullptr;
    *size = self->size;
    self->size = 0;
    return self->s;
}

Error Thread_load(Thread* self, Reader reader, void* data, const char* name)
{

    ZIO z;
    Error err = E::LOAD;
    if (!name)
        name = "?";
    ZIO_init(self, &z, reader, data);
    err = protected_parser(self, &z, name);
    return err;
}

Error Thread_load(Thread* self, const char* buff, size_t size, const char* name)
{
    LoadS ls;
    ls.s = buff;
    ls.size = size;
    return Thread_load(self, (Reader)LoadS_read, &ls, name);
}

// Error Thread_pcall(Thread* self, ProtectedFunc f, void* u) { }

// template<class T1, class T2>
// auto op_add(T1 t1, T2 t2) {
//     return t1 + t2;
// }

// template<class T1, class T2>
// auto op_sub(T1 t1, T2 t2) {
//     return t1 - t2;
// }

static void __Thread_run(Thread* self)
{
    while (self->pc) {
        Instruction ins = *(self->pc++);
        switch (INS_OP(ins)) {
            case Opcode::TEST: {
                printf("hello world\n");
                break;
            }
            case Opcode::RET_0: {
                self->top = self->base;

                self->top = Thread_pop_index(self) + self->stack;
                self->base = Thread_pop_index(self) + self->stack;
                self->pc = Thread_pop_pc(self);

                break;
            }

            case Opcode::ADD: {

                TValue* second = self->top - 1;
                TValue* first = self->top - 2;

                TValue result;

                if (first->type == Type::Integer) {
                    if (second->type == Type::Integer) {
                        result = { .type = Type::Integer,
                            .i = first->i + second->i };
                    } else if (second->type == Type::Number) {
                        result = { .type = Type::Number,
                            .n = first->i + second->n };
                    } else {
                        Thread_throw(self, E::OP_NUM);
                    }

                }

                else if (first->type == Type::Number) {
                    if (second->type == Type::Integer) {
                        result = { .type = Type::Number,
                            .n = first->n + second->i };

                    } else if (second->type == Type::Number) {
                        result = { .type = Type::Number,
                            .n = first->n + second->n };
                    } else {
                        Thread_throw(self, E::OP_NUM);
                    }

                }

                else {
                    Thread_throw(self, E::OP_NUM);
                }

                *(self->top - 2) = result;
                self->top--;

                break;
            }

            case Opcode::SUB: {

                TValue* second = self->top - 1;
                TValue* first = self->top - 2;

                TValue result;

                if (first->type == Type::Integer) {
                    if (second->type == Type::Integer) {
                        result = { .type = Type::Integer,
                            .i = first->i - second->i };
                    } else if (second->type == Type::Number) {
                        result = { .type = Type::Number,
                            .n = first->i - second->n };
                    } else {
                        Thread_throw(self, E::OP_NUM);
                    }

                }

                else if (first->type == Type::Number) {
                    if (second->type == Type::Integer) {
                        result = { .type = Type::Number,
                            .n = first->n - second->i };

                    } else if (second->type == Type::Number) {
                        result = { .type = Type::Number,
                            .n = first->n - second->n };
                    } else {
                        Thread_throw(self, E::OP_NUM);
                    }

                }

                else {
                    Thread_throw(self, E::OP_NUM);
                }

                *(self->top - 2) = result;
                self->top--;

                break;
            }

            case Opcode::PRINT: {
                TValue* first = self->top - 1;

                switch (first->type) {
                    case Type::Integer: {
                        printf("%ld", first->i);
                        break;
                    }
                    case Type::Number: {
                        printf("%lf", first->n);
                        break;
                    }
                    default: {
                        printf("print it unsupport for %d", (int)(self->type));
                    }
                }

                self->top--;
                break;
            }

            default: {
                fprintf(stderr, "unsupport bytecode %u\n", INS_OP(ins));
                exit(1);
            }
        }
    }
}

void Thread_call(Thread* self, Index func_i)
{
    auto tv_func = stack_slot(self, func_i);
    Size params_nr = self->top - tv_func - 1;

    Thread_push_pc(self, self->pc);
    Thread_push_index(self, self->base - self->stack);
    Thread_push_index(self, self->top - self->stack);

    auto func = obj2func(tv2obj(tv_func));

    self->base = self->top;
    self->pc = func->proto->ins;

    __Thread_run(self);
}

Error Thread_pcall(Thread* self, ProtectedFunc f, void* ud)
{
    auto old_top = self->top - self->stack;
    auto old_base = self->base - self->stack;
    auto old_pc = self->pc;

    auto err = Thread_run_protected(self, f, ud);
    if (err != E::OK) {
        self->top = self->stack + old_top;
        self->base = self->stack + old_base;
        self->pc = old_pc;
    }
    return err;
}

void Thread_run(Thread* self, const char* code)
{
    auto err = Thread_load(self, code, strlen(code), "no_name");
    if (E::OK == err) {
        Thread_call(self, -1);
    } else {
        Thread_throw(self, err);
    }
}

void Thread_push(Thread* self, TValue* tv)
{
    if (self->stack_nr <= (self->top - self->stack)) {
        self->stack = NF_REALLOC_ARRAY_P(
            self, self->stack, TValue, self->stack_nr * 3 / 2 + 16);
    }
    *(self->top++) = *tv;
}

void Thread_push_func(Thread* self, Func* f)
{
    TValue tv = { .type = Type::Func, .obj = f };
    Thread_push(self, &tv);
}

void Thread_push_index(Thread* self, Index index)
{
    TValue tv = { .type = Type::Index, .index = index };
    Thread_push(self, &tv);
}

Index Thread_pop_index(Thread* self) { return (--self->top)->index; }

void Thread_push_pc(Thread* self, const Instruction* pc)
{
    TValue tv = { .type = Type::PC, .pc = pc };
    Thread_push(self, &tv);
}

const Instruction* Thread_pop_pc(Thread* self) { return (--self->top)->pc; }

} // namespace nf
