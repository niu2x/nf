#ifndef NF_BYTECODE_H
#define NF_BYTECODE_H

#include "basic_types.h"

namespace nf {

enum class Opcode : uint16_t {
    HALT,
};

#define INS_OP(op) (((Instruction)(op)) << 6)

} // namespace nf

#endif