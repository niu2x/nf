#ifndef NF_BASIC_TYPES_H
#define NF_BASIC_TYPES_H

namespace nf {

enum class Type : uint8_t {
    NIL,
    Integer,
    Number,
    Thread,
    Table,
    String,
};

using Flags = uint8_t;
using Hash = uint32_t;

using Integer = int64_t;
using Number = double;

using Size = size_t;
using Instruction = uint32_t;

} // namespace nf

#endif