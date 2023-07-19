#ifndef NF_BYTECODE_H
#define NF_BYTECODE_H

#include "basic_types.h"

namespace nf::imp {

enum class Opcode : uint16_t {
    TEST = 100,
    RET_0,
    ADD,
    SUB,
    MUL,
    DIV,
    CONST,
    LOAD_NIL,
    PUSH,
    SET,
    NEW_TABLE,
    TABLE_SET,
    TABLE_GET,
    POP,
    LEN,
    NEG,
    CALL,
};

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