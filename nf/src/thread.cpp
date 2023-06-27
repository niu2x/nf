#include <setjmp.h>

#include "object.h"
#include "api.h"
#include "utils.h"
#include "zio.h"

namespace nf {

const Size basic_ci_nr = 8;
const Size basic_stack_nr = 32;

static void Thread_stack_init(Thread* self)
{
    self->ci_base = NF_ALLOC_ARRAY_P(self, CallInfo, basic_ci_nr);
    self->ci_nr = basic_ci_nr;
    self->ci = self->ci_base;

    self->stack = NF_ALLOC_ARRAY_P(self, TValue, basic_stack_nr);
    self->stack_nr = basic_stack_nr;
    self->top = self->stack;
    self->base = self->stack;

    self->ci->func = self->top;
    TValue_set_nil(self->top++);

    self->ci->base = self->base = self->top;
    self->ci->top = self->top + 20;
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
    TValue_set_nil(&(self->gt));

    TValue_set_nil(Thread_registry(self));
    Thread_global(self)->root = self;

    self->error_jmp = nullptr;

    self->stack = nullptr;
    self->stack_nr = 0;

    self->ci = self->ci_base = nullptr;
    self->ci_nr = 0;

    self->saved_pc = 0;

    self->base = nullptr;
    self->top = nullptr;

    Thread_stack_init(self);

    return self;
}

void Thread_close(Thread* self)
{
    NF_FREE(self->stack);
    NF_FREE(self->ci_base);
    NF_FREE(self);
}

Error Thread_run_protected(Thread* self, Pfunc f, void* ud)
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

} // namespace nf
