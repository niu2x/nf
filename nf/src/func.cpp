#include "api.h"
#include "object.h"
#include "bytecode.h"

namespace nf::imp {

Func* Func_new(Thread* th, Proto* proto)
{
    auto func = NF_ALLOC_ARRAY_P(th, Func, 1);
    func->type = Type::Func;
    func->func_type = FuncType::NF;
    func->proto = proto;
    func->prev = nullptr;
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

    return proto;
}

void Proto_append_ins(Thread* th, Proto* self, Instruction ins)
{
    if (self->ins_nr == self->ins_alloc) {
        Size new_alloc = (self->ins_nr * 3 / 2 + 16);
        self->ins = NF_REALLOC_ARRAY_P(th, self->ins, Instruction, new_alloc);
        self->ins_alloc = new_alloc;
    }
    self->ins[self->ins_nr++] = ins;
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