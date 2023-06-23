#ifndef VM_BYTECODE_H
#define VM_BYTECODE_H

#include <sstream>
#include <cstdint>
#include "utils.h"

namespace nf {

// class VM;

enum class InstructionType : uint8_t {
    NONE = 0,
    PRINT,
    LOAD_CONSTANT_INTEGER,
};

class Instruction {
public:
    static Instruction load_constant_integer(int constant_index);
    static Instruction print();

    Instruction();
    ~Instruction();

    NF_INLINE void encode(std::stringstream* ss) { _encode(ss); }
    NF_INLINE size_t decode(const uint8_t* bytecodes)
    {
        return _decode(bytecodes);
    }
    // NF_INLINE void execute(VM* vm) { _execute(vm); }

private:
    void _encode(std::stringstream* ss);
    size_t _decode(const uint8_t* bytecodes);
    // void _execute(VM* vm);

    InstructionType type_;
    int constant_index_;
};

} // namespace nf

#endif