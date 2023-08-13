#ifndef NF_BASIC_TYPES_H
#define NF_BASIC_TYPES_H

#include <cstdint>
#include <cstddef>
#include <nf/api.h>

#include "utils.h"

namespace nf::imp {

enum class Type : uint8_t {
    NIL,
    Integer,
    Number,
    Bool,

    Table,
    String,
    Func,

    Thread,
    UpValue,
    PC,
    Index,
    Proto,
};

using Flags = uint8_t;
using Hash = uint64_t;

using Integer = int64_t;
using Number = double;
// static_assert sizeof(Integer) == sizeof(Number)

// using Index = int32_t;
using VarIndex = int16_t;
using ConstIndex = int16_t;

using InsIndex = uint32_t;

// 主要受限于16位的VarIndex、ConstIndex、StackIndex
#define MAX_VAR_NR   512
#define MAX_CONST_NR 30000
#define MAX_STACK_NR 30000
#define MAX_UV_NR    64
#define MAX_INS_NR   (0xFFFFFFFF - 16)

// 最大的函数参数个数
#define MAX_ARGS 256

using StackIndex = nf::StackIndex;
using PseudoIndex = nf::PseudoIndex;

using Size = size_t;
using Instruction = uint64_t;

NF_INLINE Size next_power_of_2(Size s)
{
    s |= s >> 32;
    s |= s >> 16;
    s |= s >> 8;
    s |= s >> 4;
    s |= s >> 2;
    s |= s >> 1;
    return s + 1;
}

NF_INLINE uint32_t i2u32(int32_t i)
{
    union {
        int32_t i;
        uint32_t u;
    } u;
    u.i = i;
    return u.u;
};

NF_INLINE int32_t u2i32(uint32_t u)
{
    union {
        int32_t i;
        uint32_t u;
    } un;
    un.u = u;
    return un.i;
};

} // namespace nf::imp

#endif