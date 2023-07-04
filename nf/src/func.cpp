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
    proto->ins = NF_ALLOC_ARRAY_P(th, Instruction, 2);
    proto->ins[0] = INS_FROM_OP_NO_ARGS(Opcode::TEST);
    proto->ins[1] = INS_FROM_OP_NO_ARGS(Opcode::RET_0);
    proto->ins_nr = 2;
    return proto;
}

} // namespace nf