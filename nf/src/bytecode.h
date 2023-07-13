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
};

// NO_ARGS(48)_OP(16)

#define INS_OP(op)              (((Opcode)((op)&0xFFFF)))
#define INS_ABCDEF(op)          ((op) >> 16)

#define INS_FROM_OP_NO_ARGS(op) ((Instruction)(op))
#define INS_FROM_OP_ABCDEF(op, ABCDEF)                                         \
    INS_FROM_OP_NO_ARGS(op) | ((Instruction)((ABCDEF) << 16))

} // namespace nf

#endif