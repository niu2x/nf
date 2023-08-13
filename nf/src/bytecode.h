#ifndef NF_BYTECODE_H
#define NF_BYTECODE_H

#include <boost/preprocessor.hpp>
#include "basic_types.h"

namespace nf::imp {

enum class InsType {
    NO_ARGS,
    AB,
    AB_CD,
};

#define ALL_OPCODE_DESC                                                        \
    ((RET_0))((RET_TOP))((ADD))((SUB))((MUL))((DIV))((CONST))((LOAD_NIL))(     \
        (PUSH))((SET))((NEW_TABLE))((TABLE_GET))((TABLE_SET))((POP_TO))(       \
        (LEN))((NEG))((CALL))((NEW_NF_FUNC))((GET_UP_VALUE))((SET_UP_VALUE))(  \
        (CLOSE_UV_TO))((JUMP_IF_FALSE))((LESS))((GREATE))((EQ))((LE))((GE))

#define VISIT_ALL_INS(visitor)                                                 \
    visitor(visitor, RET_0, "RET_0", NO_ARGS)                                  \
        visitor(visitor, RET_TOP, "RET_TOP", NO_ARGS)

#define Opcode_enum_define(r, data, desc) BOOST_PP_SEQ_ELEM(0, desc),
enum class Opcode : uint16_t {
    BOOST_PP_SEQ_FOR_EACH(Opcode_enum_define, ~, ALL_OPCODE_DESC)
};

extern const char* opcode_names[];

// NO_ARGS(48)_OP(16)

#define INS_OP(op)              (((Opcode)((op)&0xFFFF)))
#define INS_ABCDEF(op)          ((op) >> 16)
#define INS_ABCD(op)            ((op) >> 32)
#define INS_AB(op)              (((op) >> 48) & 0xFFFF)
#define INS_CD(op)              (((op) >> 32) & 0xFFFF)

#define INS_FROM_OP_NO_ARGS(op) ((Instruction)(op))

#define INS_FROM_OP_ABCDEF(op, ABCDEF)                                         \
    (INS_FROM_OP_NO_ARGS(op) | (((Instruction)((ABCDEF))) << 16))

#define INS_FROM_OP_ABCD(op, ABCD)                                             \
    (INS_FROM_OP_NO_ARGS(op) | (((Instruction)((ABCD))) << 32))

#define INS_FROM_OP_AB(op, AB)                                                 \
    (INS_FROM_OP_NO_ARGS(op) | (((Instruction)((AB))) << 48))

#define INS_FROM_OP_AB_CD(op, AB, CD)                                          \
    (INS_FROM_OP_NO_ARGS(op) | (((Instruction)((AB))) << 48)                   \
        | (((Instruction)((CD))) << 32))

} // namespace nf::imp

#endif