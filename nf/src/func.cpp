#include "api.h"
#include "object.h"
#include "bytecode.h"

namespace nf {

Func* Func_new(Thread* th, Proto* proto)
{
    auto func = NF_ALLOC_ARRAY_P(th, Func, 1);
    func->type = Type::Func;
    func->func_type = FuncType::NF;
    func->proto = proto;
    return func;
}

Proto* Proto_new(Thread* th)
{
    auto proto = NF_ALLOC_ARRAY_P(th, Proto, 1);
    proto->type = Type::Proto;
    proto->ins = nullptr;
    proto->ins_nr = 0;
    proto->var_nr = 0;
    return proto;
}

} // namespace nf