#ifndef NF_BYTECODE_H
#define NF_BYTECODE_H

#include "basic_types.h"

namespace nf {

enum class Opcode : uint16_t {
    // printf niu2x say hello to ff
    TEST = 100,
    RET_0,
    ADD,
    SUB,
    MUL,
    DIV,
    PRINT,
    CONST,
    SET_NIL,
    LOAD_NIL,
    PUSH,
    SET,
    NEW_TABLE,
    TABLE_SET,
    TABLE_GET,
};

// NO_ARGS(48)_OP(16)

#define INS_OP(op)              (((Opcode)((op)&0xFFFF)))
#define INS_ABCDEF(op)          ((op) >> 16)
#define INS_ABCD(op)            ((op) >> 32)

#define INS_FROM_OP_NO_ARGS(op) ((Instruction)(op))
#define INS_FROM_OP_ABCDEF(op, ABCDEF)                                         \
    INS_FROM_OP_NO_ARGS(op) | (((Instruction)((ABCDEF))) << 16)
#define INS_FROM_OP_ABCD(op, ABCD)                                             \
    INS_FROM_OP_NO_ARGS(op) | (((Instruction)((ABCD))) << 32)

} // namespace nf

#endif