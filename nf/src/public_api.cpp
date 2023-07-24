#include <stdarg.h>

#include <nf/api.h>
#include "object.h"
#include "api.h"
#include "lib.h"

namespace nf {

using namespace imp;

void new_table(ThreadPtr self)
{
    auto th = (Thread*)self;
    TValue v = {
        .type = Type::Table,
        .obj = Table_new(th),
    };
    Thread_push(th, &v);
}

void new_str(ThreadPtr self, const char* sz, int len)
{
    auto th = (Thread*)self;
    TValue v = {
        .type = Type::String,
        .obj = Str_new(th, sz, len ?: strlen(sz)),
    };
    Thread_push(th, &v);
}

ThreadPtr open()
{
    auto th = Thread_open();
    lib_core_init(th);
    lib_nf_init(th);
    return th;
}

void close(ThreadPtr self) { Thread_close((Thread*)self); }

void run(ThreadPtr self, const char* code) { Thread_run((Thread*)self, code); }

void run(ThreadPtr self, FILE* fp) { Thread_run((Thread*)self, fp); }

void set_global(ThreadPtr self, StackIndex key)
{
    set_table(self, PSEUDO_INDEX_GLOBAL, key);
}

void set_table(ThreadPtr self, StackIndex table_slot, StackIndex key_slot)
{
    auto th = (Thread*)self;
    auto table = tv2table(stack_slot(th, table_slot));
    auto key = stack_slot(th, key_slot);
    auto value = Table_set(th, table, key);
    *value = *(--(th->top));
}

void pop(ThreadPtr self, StackIndex n)
{
    auto th = (Thread*)self;
    th->top -= n;
}

void set_top(ThreadPtr self, StackIndex top)
{
    auto th = (Thread*)self;
    NF_CHECK(th, th->top - th->base >= top, "you only can shrink stack.");
    th->top = th->base + top;
}

StackIndex top(ThreadPtr self)
{
    auto th = (Thread*)self;
    return th->top - th->base;
}

void new_str_fmt(ThreadPtr self, const char* fmt, ...)
{
    auto th = (Thread*)self;
    va_list ap;
    va_start(ap, fmt);
    TValue v = {
        .type = Type::String,
        .obj = Str_new_fmt(th, fmt, ap),
    };
    Thread_push(th, &v);
    va_end(ap);
}

void push_func(ThreadPtr self, CFunc c_func)
{
    auto th = (Thread*)self;
    Thread_push_func(th, Func_new(th, c_func));
}

} // namespace nf