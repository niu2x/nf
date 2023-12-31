#include <cstdio>
#include <utility>
#include <gtest/gtest.h>

#include "basic_types.h"
#include "bytecode.h"

namespace nf::imp {

TEST(Instruction, build)
{
    EXPECT_EQ(INS_FROM_OP_NO_ARGS(Opcode::TEST), 0x64);
    EXPECT_EQ(INS_FROM_OP_AB(Opcode::TEST, 0xF1FB), 0xF1FB000000000064);
    EXPECT_EQ(INS_FROM_OP_AB_CD(0xF1F2, 0xF1FB, 0xB1B2), 0xF1FBB1B20000F1F2);
}

} // namespace nf::imp
