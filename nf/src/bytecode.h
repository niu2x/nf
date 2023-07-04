#ifndef NF_BYTECODE_H
#define NF_BYTECODE_H

#include "basic_types.h"

namespace nf {

enum class Opcode : uint16_t {
    // printf niu2x say hello to ff
    TEST = 100,
    RET_0,
};

// NO_ARGS(48)_OP(16)

#define INS_OP(op)              (((Opcode)((op)&0xFFFF)))
#define INS_FROM_OP_NO_ARGS(op) ((Instruction)(op))

} // namespace nf

#endif