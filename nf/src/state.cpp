#include <setjmp.h>

#include "object.h"
#include "api.h"
#include "utils.h"

namespace nf {

const Size basic_ci_alloc = 8;

static void State_stack_init(State* self)
{
    /* initialize CallInfo array */
    // L1->base_ci = luaM_newvector(L, BASIC_CI_SIZE, CallInfo);
    // L1->ci = L1->base_ci;
    // L1->size_ci = BASIC_CI_SIZE;
    // L1->end_ci = L1->base_ci + L1->size_ci - 1;
    // /* initialize stack array */
    // L1->stack = luaM_newvector(L, BASIC_STACK_SIZE + EXTRA_STACK, TValue);
    // L1->stacksize = BASIC_STACK_SIZE + EXTRA_STACK;
    // L1->top = L1->stack;
    // L1->stack_last = L1->stack + (L1->stacksize - EXTRA_STACK) - 1;
    // /* initialize first ci */
    // L1->ci->func = L1->top;
    // setnilvalue(L1->top++); /* `function' entry for this `ci' */
    // L1->base = L1->ci->base = L1->top;
    // L1->ci->top = L1->top + LUA_MINSTACK;
}

State* State_open()
{
    struct Twin {
        State state;
        GlobalState global;
    };

    auto twin = NF_ALLOC(Twin);
    if (!twin)
        return nullptr;

    auto self = &(twin->state);
    State_global(self) = &(twin->global);

    self->type = Type::Thread;
    TValue_set_nil(&(self->gt));

    TValue_set_nil(State_registry(self));
    State_global(self)->root = self;

    self->error_jmp = nullptr;

    NF_PLACEMENT_NEW(&(self->call_infos));
    NF_PLACEMENT_NEW(&(self->stack));

    self->saved_pc = 0;

    self->base = nullptr;
    self->top = nullptr;

    return self;
}

void State_close(State* self)
{
    NF_PLACEMENT_DELETE(&(self->call_infos));
    NF_PLACEMENT_DELETE(&(self->stack));
    NF_FREE(State, self);
}

Error State_run_protected(State* self, Pfunc f, void* ud)
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

void State_throw(State* self, Error err)
{
    if (self->error_jmp) {
        self->error_jmp->status = err;
        longjmp(self->error_jmp->b, -1);
    } else {
        self->status = err;
        exit(EXIT_FAILURE);
    }
}

} // namespace nf
