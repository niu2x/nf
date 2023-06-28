#include <setjmp.h>
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

static Instruction halt_ins = INS_OP(Opcode::HALT);

static void Thread_init_step_one(Thread* self)
{

    TValue_set_nil(&(self->gt));

    TValue_set_nil(Thread_registry(self));
    Thread_global(self)->root = self;

    self->error_jmp = nullptr;

    self->stack = nullptr;
    self->stack_nr = 0;

    self->pc = &halt_ins;

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
    Error err;
    if (!name)
        name = "?";
    ZIO_init(self, &z, reader, data);
    // status = luaD_protectedparser(L, &z, chunkname);
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
}

void Thread_run(Thread* self, const char* code)
{
    if (E::OK == Thread_load(self, code, strlen(code), "no_name")) {
        Thread_call(self, -1);
    } else {
        Thread_throw(self, E::PARSE);
    }
}

void Thread_push(Thread* self, TValue* tv) { }

void Thread_push_index(Thread* self, Index index)
{
    TValue tv = { .type = Type::Index, .index = index };
    Thread_push(self, &tv);
}

void Thread_push_pc(Thread* self, const Instruction* pc)
{
    TValue tv = { .type = Type::PC, .pc = pc };
    Thread_push(self, &tv);
}

} // namespace nf
