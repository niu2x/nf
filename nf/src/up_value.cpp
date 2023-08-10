#include "object.h"

namespace nf::imp {

UpValue* UpValue_new(Thread* th, uint32_t uv_pos_u32)
{
    UpValuePos uv_pos;
    uv_pos.u32 = uv_pos_u32;

    TValue* base = th->base;
    while (--uv_pos.deep > 0) {
        base = (base - 2)->index + th->stack;
    }
    StackIndex abs_stack_index = base + uv_pos.slot - th->stack;

    if (auto uv = Thread_search_opened_uv(th, abs_stack_index)) {
        return uv;
    }

    auto uv = NF_ALLOC_ARRAY_P(th, UpValue, 1);
    uv->type = Type::UpValue;
    uv->closed = false;
    uv->abs_stack_index = abs_stack_index;

    Thread_insert_opened_uv(th, uv);

    return uv;
}

UpValue* Thread_search_opened_uv(Thread* th, StackIndex uv)
{
    for (uint64_t i = th->closed_uv_nr; i < th->up_values_nr; i++) {
        if (th->up_values[i]->abs_stack_index == uv)
            return th->up_values[i];
    }
    return nullptr;
}

void Thread_insert_opened_uv(Thread* th, UpValue* uv)
{

    if (th->up_values_nr >= th->up_values_alloc) {
        th->up_values = NF_REALLOC_ARRAY_P(
            th, th->up_values, UpValue*, th->up_values_alloc * 3 / 2 + 16);
        th->up_values_alloc = th->up_values_alloc * 3 / 2 + 16;
    }

    th->up_values[th->up_values_nr++] = uv;
}

} // namespace nf::imp