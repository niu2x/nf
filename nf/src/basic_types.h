#ifndef NF_BASIC_TYPES_H
#define NF_BASIC_TYPES_H

#include <cstdint>
#include <cstddef>

namespace nf {

enum class Type : uint8_t {
    NIL,
    Integer,
    Number,
    PC,
    Index,
    Proto,
    Thread,
    Table,
    String,
    Func,

};

using Flags = uint8_t;
using Hash = uint32_t;

using Integer = int64_t;
using Number = double;

using Index = int32_t;

using Size = size_t;
using Instruction = uint64_t;

} // namespace nf

#endif