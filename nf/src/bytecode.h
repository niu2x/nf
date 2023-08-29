#ifndef NF_BYTECODE_H
#define NF_BYTECODE_H

#include <boost/preprocessor.hpp>
#include "basic_types.h"

namespace nf::imp {

enum class InsType : uint16_t {
    NO_ARGS,
    AB,
    AB_CD,
    AB_CD_EF,
};

#define ALL_OPCODE_DESC                                                        \
    ((RET_0)(NO_ARGS))((RET_TOP)(NO_ARGS))((ADD)(AB_CD_EF))((SUB)(AB_CD_EF))(  \
        (MUL)(AB_CD_EF))((DIV)(AB_CD_EF))((EQ)(AB_CD_EF))((LE)(AB_CD_EF))(     \
        (GE)(AB_CD_EF))((NE)(AB_CD_EF))((LESS)(AB_CD_EF))((GREATE)(AB_CD_EF))( \
        (CONST)(AB_CD))((LOAD_NIL)(AB))((PUSH)(AB_CD))((SET)(AB_CD))(          \
        (NEW_TABLE)(AB))((TABLE_GET)(AB_CD_EF))((TABLE_SET)(AB_CD))(           \
        (LEN)(AB_CD))((NEG)(AB_CD))((NEW_NF_FUNC)(AB))((GET_UP_VALUE)(AB_CD))( \
        (SET_UP_VALUE)(AB_CD))((CLOSE_UV_TO)(AB))((JUMP)(NO_ARGS))(            \
        (JUMP_IF_FALSE)(NO_ARGS))((POP_TO)(NO_ARGS))((CALL)(NO_ARGS))

#define VISIT_ALL_INS(visitor)                                                 \
    visitor(visitor, RET_0, "RET_0", NO_ARGS)                                  \
        visitor(visitor, RET_TOP, "RET_TOP", NO_ARGS)

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

struct InsDesc {
    InsType type;
};

extern const char* opcode_names[];

#define INS_OP(op)              (((Opcode)((op)&0xFFFF)))
#define INS_ABCDEF(op)          ((op) >> 16)
#define INS_ABCD(op)            ((op) >> 32)
#define INS_AB(op)              (((op) >> 48) & 0xFFFF)
#define INS_CD(op)              (((op) >> 32) & 0xFFFF)
#define INS_EF(op)              (((op) >> 16) & 0xFFFF)

#define INS_FROM_OP_NO_ARGS(op) ((Instruction)(op))

#define INS_FROM_OP_ABCDEF(op, ABCDEF)                                         \
    (INS_FROM_OP_NO_ARGS(op) | (((Instruction)((ABCDEF))) << 16))

#define INS_FROM_OP_ABCD(op, ABCD)                                             \
    (INS_FROM_OP_NO_ARGS(op) | (((Instruction)((ABCD))) << 32))

#define INS_FROM_OP_AB(op, AB)                                                 \
    (INS_FROM_OP_NO_ARGS(op) | (((Instruction)((AB))) << 48))

#define INS_FROM_OP_AB_CD(op, AB, CD)                                          \
    (INS_FROM_OP_NO_ARGS(op) | (((Instruction)((AB))) << 48)                   \
        | (((Instruction)((CD))) << 32))

#define INS_FROM_OP_AB_CD_EF(op, AB, CD, EF)                                   \
    (INS_FROM_OP_NO_ARGS(op) | (((Instruction)((AB))) << 48)                   \
     | (((Instruction)((CD))) << 32) | (((Instruction)((EF))) << 16))

template <uint16_t OPCODE, int ARG_NR>
class InsBuilder {
public:
};

template <uint16_t OPCODE>
class InsBuilder<OPCODE, (uint16_t)(InsType::NO_ARGS)> {
public:
    static Instruction build() { return INS_FROM_OP_NO_ARGS(OPCODE); }
};

template <uint16_t OPCODE>
class InsBuilder<OPCODE, (uint16_t)(InsType::AB)> {
public:
    static Instruction build(uint16_t ab) { return INS_FROM_OP_AB(OPCODE, ab); }
};

template <uint16_t OPCODE>
class InsBuilder<OPCODE, (uint16_t)(InsType::AB_CD)> {
public:
    static Instruction build(uint16_t ab, uint16_t cd)
    {
        return INS_FROM_OP_AB_CD(OPCODE, ab, cd);
    }
};

template <uint16_t OPCODE>
class InsBuilder<OPCODE, (uint16_t)(InsType::AB_CD_EF)> {
public:
    static Instruction build(uint16_t ab, uint16_t cd, uint16_t ef)
    {
        return INS_FROM_OP_AB_CD_EF(OPCODE, ab, cd, ef);
    }
};

#define INS_BUILD(op, ...)                                                     \
    InsBuilder<(uint16_t)(Opcode::op),                                         \
               (uint16_t)(BOOST_PP_CAT(OpcodeType::op,                         \
                                       _TYPE))>::build(__VA_ARGS__)

} // namespace nf::imp

#endif