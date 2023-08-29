#include "bytecode.h"

namespace nf::imp {

#define Opcode_name_define(r, data, desc)                                      \
    BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(0, desc)),

const char* opcode_names[] = {
    BOOST_PP_SEQ_FOR_EACH(Opcode_name_define, ~, ALL_OPCODE_DESC)
};

#define Opcode_type_list(r, data, desc) InsType::BOOST_PP_SEQ_ELEM(1, desc),
const InsType opcode_types[] = {
    BOOST_PP_SEQ_FOR_EACH(Opcode_type_list, ~, ALL_OPCODE_DESC)
};

} // namespace nf::imp