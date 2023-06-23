#include "bytecode.h"
#include <string.h>
#include "value.h"

namespace nf {

static void operator<<(std::stringstream& ss, const InstructionType& type)
{
    ss.write((const char*)&type, sizeof(type));
}

static void operator>>(std::stringstream& ss, InstructionType& type)
{
    ss.read((char*)&type, sizeof(type));
}

Instruction::Instruction()
: type_(InstructionType::NONE)
{
}
Instruction::~Instruction() { }

void Instruction::_encode(std::stringstream* ss)
{
    ss->write((const char*)&type_, sizeof(type_));
    switch (type_) {
        case InstructionType::PRINT: {
            break;
        }
        case InstructionType::LOAD_CONSTANT_INTEGER: {
            ss->write((const char*)&constant_index_, sizeof(constant_index_));
            break;
        }
        default: {
            die("unsupport Instruction.");
        }
    }
}

size_t Instruction::_decode(const uint8_t* bytecodes)
{
    size_t ins_len = 0;
    memcpy(&type_, bytecodes + ins_len, sizeof(type_));
    ins_len += sizeof(type_);

    switch (type_) {
        case InstructionType::PRINT: {
            break;
        }
        case InstructionType::LOAD_CONSTANT_INTEGER: {
            memcpy(
                &constant_index_, bytecodes + ins_len, sizeof(constant_index_));
            ins_len += sizeof(constant_index_);
            break;
        }
        default: {
            die("unsupport Instruction.");
        }
    }
    return ins_len;
}

// void Instruction::_execute(VM* vm)
// {
//     switch (type_) {
//         case InstructionType::PRINT: {
//             int64_t i;
//             vm->pop_value(&i);
//             printf("%ld\n", i);
//             break;
//         }
//         case InstructionType::LOAD_CONSTANT_INTEGER: {
//             int64_t i;
//             vm->lookup_constant(constant_index_, &i);
//             vm->push_value(i);
//             break;
//         }
//         default: {
//             die("unsupport Instruction.");
//         }
//     }
// }

Instruction Instruction::load_constant_integer(int constant_index)
{
    Instruction ins;
    ins.type_ = InstructionType::LOAD_CONSTANT_INTEGER;
    ins.constant_index_ = constant_index;
    return ins;
}

Instruction Instruction::print()
{
    Instruction ins;
    ins.type_ = InstructionType::PRINT;
    return ins;
}

} // namespace nf