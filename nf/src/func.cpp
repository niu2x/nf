#include "api.h"
#include "object.h"
#include "bytecode.h"

namespace nf::imp {

static void Func_init_up_values(Thread* th, Func* self)
{
    if (self->func_type == FuncType::NF) {
        auto proto = self->proto;
        for (StackIndex i = 0; i < proto->up_values_nr; i++) {
            UpValue* uv = UpValue_new(th, proto->up_values[i]);
            self->up_values[self->up_values_nr++] = uv;
        }
    }
}

Func* Func_new(Thread* th, Proto* proto)
{
    auto func = NF_ALLOC_ARRAY_P(th, Func, 1);
    func->type = Type::Func;
    func->func_type = FuncType::NF;
    func->proto = proto;
    func->prev = nullptr;
    func->up_values_nr = 0;
    Func_init_up_values(th, func);
    return func;
}

Func* Func_new(Thread* th, CFunc c_func)
{
    auto func = NF_ALLOC_ARRAY_P(th, Func, 1);
    func->type = Type::Func;
    func->func_type = FuncType::C;
    func->c_func = c_func;
    func->prev = nullptr;
    func->up_values_nr = 0;
    Func_init_up_values(th, func);
    return func;
}

Proto* Proto_new(Thread* th)
{
    auto proto = NF_ALLOC_ARRAY_P(th, Proto, 1);
    proto->type = Type::Proto;
    proto->ins = nullptr;
    proto->ins_nr = 0;
    proto->ins_alloc = 0;

    proto->const_arr = nullptr;
    proto->const_nr = 0;
    proto->const_alloc = 0;

    proto->used_slots = 0;

    proto->name = nullptr;
    proto->args_nr = 0;

    proto->scope = nullptr;
    proto->parent = nullptr;

    // proto->up_values = nullptr;
    proto->up_values_nr = 0;

    return proto;
}

InsIndex Proto_append_ins(Thread* th, Proto* self, Instruction ins)
{
    if (self->ins_nr == self->ins_alloc) {
        Size new_alloc = (self->ins_nr * 3 / 2 + 16);
        self->ins = NF_REALLOC_ARRAY_P(th, self->ins, Instruction, new_alloc);
        self->ins_alloc = new_alloc;
    }
    self->ins[self->ins_nr++] = ins;
    return self->ins_nr - 1;
}

void Proto_update_ins(Proto* self, InsIndex i, Instruction ins)
{
    self->ins[i] = ins;
}

StackIndex Proto_insert_uv(Thread* th, Proto* self, UpValuePos uv)
{
    for (StackIndex i = 0; i < self->up_values_nr; i++) {
        if (self->up_values[i] == uv.u32)
            return i;
    }

    NF_CHECK(th, self->up_values_nr + 1 <= MAX_CONST_NR, "too may uv");
    self->up_values[self->up_values_nr++] = uv.u32;
    return self->up_values_nr - 1;
}

ConstIndex Proto_insert_const(Thread* th, Proto* self, TValue* v)
{
    for (ConstIndex i = 0; i < self->const_nr; i++) {
        auto* v_arr = &(self->const_arr[i]);
        if (v->type == v_arr->type) {
            if (v->i == v_arr->i) {
                return i;
            }
        }
    }

    NF_CHECK(th, self->const_nr + 1 <= MAX_CONST_NR, "too may constants");

    if (self->const_nr == self->const_alloc) {
        Size new_alloc = (self->const_nr * 3 / 2 + 16);
        self->const_arr
            = NF_REALLOC_ARRAY_P(th, self->const_arr, TValue, new_alloc);
        self->const_alloc = new_alloc;
    }
    self->const_arr[self->const_nr++] = *v;
    return self->const_nr - 1;
}

} // namespace nf::imp