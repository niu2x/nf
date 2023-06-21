#ifndef VM_BYTECODE_H
#define VM_BYTECODE_H

#include <sstream>
#include <stdint>
#include "utils.h"

namespace nf {

enum class InstructionType : uint8_t {
    PRINT = 1,
};

class Instruction {
public:
    NF_INLINE void encode(std::stringstream* ss) { _encode(ss); }

    NF_INLINE void decode(std::stringstream* ss) { _decode(ss); }

protected:
    virtual void _encode(std::stringstream* ss) = 0;
    virtual void _decode(std::stringstream* ss) = 0;
};

Instruction*

} // namespace nf

#endif