#ifndef NF_BYTECODE_H
#define NF_BYTECODE_H

#include <boost/preprocessor.hpp>
#include "basic_types.h"

namespace nf::imp {

enum class InsType : uint16_t {
    NO_ARGS,
    AB,
    AB_CD,
    ABCD,
    AB_CD_EF,
    AB_CDEF,
};

#define ALL_OPCODE_DESC                                                        \
    ((RET_0)(NO_ARGS))((RET_TOP)(NO_ARGS))((ADD)(AB_CD_EF))((SUB)(AB_CD_EF))(  \
        (MUL)(AB_CD_EF))((DIV)(AB_CD_EF))((EQ)(AB_CD_EF))((LE)(AB_CD_EF))(     \
        (GE)(AB_CD_EF))((NE)(AB_CD_EF))((LESS)(AB_CD_EF))((GREATE)(AB_CD_EF))( \
        (CONST)(AB_CD))((LOAD_NIL)(AB))((PUSH)(AB_CD))((SET)(AB_CD))(          \
        (NEW_TABLE)(AB))((TABLE_GET)(AB_CD_EF))((TABLE_SET)(AB_CD_EF))(        \
        (LEN)(AB_CD))((NEG)(AB_CD))((NEW_NF_FUNC)(AB))((GET_UP_VALUE)(AB_CD))( \
        (SET_UP_VALUE)(AB_CD))((CLOSE_UV_TO)(AB))((JUMP)(ABCD))(               \
        (JUMP_IF_FALSE)(AB_CDEF))((POP_TO)(NO_ARGS))((CALL)(AB_CD_EF))

#define Opcode_enum_define(r, data, desc) BOOST_PP_SEQ_ELEM(0, desc),

enum class Opcode : uint16_t {
    BOOST_PP_SEQ_FOR_EACH(Opcode_enum_define, ~, ALL_OPCODE_DESC)
};

#define Opcode_type_define(r, data, desc)                                      \
    BOOST_PP_CAT(BOOST_PP_SEQ_ELEM(0, desc), _TYPE)                            \
        = (uint16_t)(InsType::BOOST_PP_SEQ_ELEM(1, desc)),

enum class OpcodeType : uint16_t {
    BOOST_PP_SEQ_FOR_EACH(Opcode_type_define, ~, ALL_OPCODE_DESC)
};

extern const char* opcode_names[];
extern const InsType opcode_types[];

#define INS_OP(op)              (((Opcode)((op)&0xFFFF)))
#define INS_ABCDEF(op)          ((op) >> 16)
#define INS_ABCD(op)            ((op) >> 32)
#define INS_CDEF(op)            (((op) >> 16) & 0xFFFFFFFF)
#define INS_AB(op)              (((op) >> 48) & 0xFFFF)
#define INS_CD(op)              (((op) >> 32) & 0xFFFF)
#define INS_EF(op)              (((op) >> 16) & 0xFFFF)

#define __INS_FROM_OP_NO_ARGS(op) ((Instruction)(op))

#define __INS_FROM_OP_ABCDEF(op, ABCDEF)                                       \
    (__INS_FROM_OP_NO_ARGS(op) | (((Instruction)((ABCDEF))) << 16))

#define __INS_FROM_OP_AB_CDEF(op, AB, CDEF)                                    \
    (__INS_FROM_OP_NO_ARGS(op) | (((Instruction)((CDEF))) << 16))              \
        | (((Instruction)((AB))) << 48)

#define __INS_FROM_OP_ABCD(op, ABCD)                                           \
    (__INS_FROM_OP_NO_ARGS(op) | (((Instruction)((ABCD))) << 32))

#define __INS_FROM_OP_AB(op, AB)                                               \
    (__INS_FROM_OP_NO_ARGS(op) | (((Instruction)((AB))) << 48))

#define __INS_FROM_OP_AB_CD(op, AB, CD)                                        \
    (__INS_FROM_OP_NO_ARGS(op) | (((Instruction)((AB))) << 48)                 \
     | (((Instruction)((CD))) << 32))

#define __INS_FROM_OP_AB_CD_EF(op, AB, CD, EF)                                 \
    (__INS_FROM_OP_NO_ARGS(op) | (((Instruction)((AB))) << 48)                 \
     | (((Instruction)((CD))) << 32) | (((Instruction)((EF))) << 16))

template <uint16_t OPCODE, int ARG_NR>
class InsBuilder {
public:
};

template <uint16_t OPCODE>
class InsBuilder<OPCODE, (uint16_t)(InsType::NO_ARGS)> {
public:
    static Instruction build() { return __INS_FROM_OP_NO_ARGS(OPCODE); }
};

template <uint16_t OPCODE>
class InsBuilder<OPCODE, (uint16_t)(InsType::AB)> {
public:
    static Instruction build(uint16_t ab)
    {
        return __INS_FROM_OP_AB(OPCODE, ab);
    }
};

template <uint16_t OPCODE>
class InsBuilder<OPCODE, (uint16_t)(InsType::AB_CD)> {
public:
    static Instruction build(uint16_t ab, uint16_t cd)
    {
        return __INS_FROM_OP_AB_CD(OPCODE, ab, cd);
    }
};

template <uint16_t OPCODE>
class InsBuilder<OPCODE, (uint16_t)(InsType::ABCD)> {
public:
    static Instruction build(uint32_t abcd)
    {
        return __INS_FROM_OP_ABCD(OPCODE, abcd);
    }
};

template <uint16_t OPCODE>
class InsBuilder<OPCODE, (uint16_t)(InsType::AB_CD_EF)> {
public:
    static Instruction build(uint16_t ab, uint16_t cd, uint16_t ef)
    {
        return __INS_FROM_OP_AB_CD_EF(OPCODE, ab, cd, ef);
    }
};

template <uint16_t OPCODE>
class InsBuilder<OPCODE, (uint16_t)(InsType::AB_CDEF)> {
public:
    static Instruction build(uint16_t ab, uint32_t cdef)
    {
        return __INS_FROM_OP_AB_CDEF(OPCODE, ab, cdef);
    }
};

#define INS_BUILD(op, ...)                                                     \
    InsBuilder<(uint16_t)(Opcode::op),                                         \
               (uint16_t)(BOOST_PP_CAT(OpcodeType::op,                         \
                                       _TYPE))>::build(__VA_ARGS__)

#define INS_TYPE(ins) ((InsType)(opcode_types[(int)INS_OP(ins)]))

} // namespace nf::imp

#endif