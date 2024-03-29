/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define TESTNAME Stack
#include "controllertest.h"

const byte push_a[] = {
    /* 8000 */ MOV_SP_CONST, 0x00, 0x20,
    /* 8003 */ MOV_A_CONST, 0x42,
    /* 8005 */ PUSH_A,
    /* 8006 */ HLT
};

TEST_F(TESTNAME, pushA) {
  mem -> initialize(ROM_START, 7, push_a);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_SP_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov sp, #xxxx  6 cycles
  // mov a, #xx     4 cycles
  // push a         4 cycles
  // hlt            3 cycles
  // total          17
  ASSERT_EQ(system -> run(), 17);
  ASSERT_EQ(system -> bus().halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
  ASSERT_EQ(sp -> getValue(), 0x2001);
  ASSERT_EQ((*mem)[0x2000], 0x42);
}

const byte push_pop_a[] = {
  /* 8000 */ MOV_SP_CONST, 0x00, 0x20,
  /* 8003 */ MOV_A_CONST, 0x42,
  /* 8005 */ PUSH_A,
  /* 8006 */ MOV_A_CONST, 0x37,
  /* 8008 */ POP_A,
  /* 8009 */ HLT
};

TEST_F(TESTNAME, pushPopA) {
  mem -> initialize(ROM_START, 10, push_pop_a);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_SP_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov sp, #xxxx  6 cycles
  // mov a, #xx     4 cycles
  // push a         4 cycles
  // mov a, #xx     4 cycles
  // pop a          4 cycles
  // hlt            3 cycles
  // total          25
  ASSERT_EQ(system -> run(), 25);
  ASSERT_EQ(system -> bus().halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
  ASSERT_EQ(sp -> getValue(), 0x2000);
  ASSERT_EQ((*mem)[0x2000], 0x42);
}

const byte push_pop_gp_regs[] = {
  /* 8000 */ MOV_SP_CONST, 0x00, 0x20,
  /* 8003 */ MOV_A_CONST, 0x42,
  /* 8005 */ MOV_B_CONST, 0x43,
  /* 8007 */ MOV_C_CONST, 0x44,
  /* 8009 */ MOV_D_CONST, 0x45,
  /* 800B */ PUSH_A,
  /* 800C */ PUSH_B,
  /* 800D */ PUSH_C,
  /* 800E */ PUSH_D,
  /* 800F */ MOV_A_CONST, 0x37,
  /* 8011 */ MOV_B_CONST, 0x36,
  /* 8013 */ MOV_C_CONST, 0x35,
  /* 8015 */ MOV_D_CONST, 0x34,
  /* 8017 */ POP_D,
  /* 8018 */ POP_C,
  /* 8019 */ POP_B,
  /* 801A */ POP_A,
  /* 801B */ HLT
};

TEST_F(TESTNAME, pushPopGPRegs) {
  mem -> initialize(ROM_START, 29, push_pop_gp_regs);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_SP_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov sp, #xxxx  6 cycles      6
  // mov x, #xx     4 cycles 4x  16
  // push a         4 cycles 4x  16
  // mov a, #xx     4 cycles 4x  16
  // pop a          4 cycles 4x  16
  // hlt            3 cycles      3
  // total                       73
  ASSERT_EQ(system -> run(), 73);
  ASSERT_EQ(system -> bus().halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
  ASSERT_EQ(gp_b -> getValue(), 0x43);
  ASSERT_EQ(gp_c -> getValue(), 0x44);
  ASSERT_EQ(gp_d -> getValue(), 0x45);
  ASSERT_EQ(sp -> getValue(), 0x2000);
  ASSERT_EQ((*mem)[0x2000], 0x42);
  ASSERT_EQ((*mem)[0x2001], 0x43);
  ASSERT_EQ((*mem)[0x2002], 0x44);
  ASSERT_EQ((*mem)[0x2003], 0x45);
}

const byte push_pop_addr_regs[] = {
  /* 8000 */ MOV_SP_CONST, 0x00, 0x20,
  /* 8003 */ MOV_SI_CONST, 0x34, 0x12,
  /* 8006 */ MOV_DI_CONST, 0x78, 0x56,
  /* 8009 */ PUSH_SI,
  /* 800A */ PUSH_DI,
  /* 800B */ MOV_SI_CONST, 0x55, 0x44,
  /* 800E */ MOV_DI_CONST, 0x77, 0x66,
  /* 8012 */ POP_DI,
  /* 8013 */ POP_SI,
  /* 8015 */ HLT
};

TEST_F(TESTNAME, pushPopAddrRegs) {
  mem -> initialize(ROM_START, 22, push_pop_addr_regs);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_SP_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov xx, #xxxx  6 cycles 3x  18
  // push xx        6 cycles 2x  12
  // mov xx, #xxxx  6 cycles 2x  12
  // pop xx         6 cycles 2x  12
  // hlt            3 cycles      3
  // total                       57
  auto cycles = system -> run();
  ASSERT_EQ(system -> error(), NoError);
  ASSERT_EQ(cycles, 57);
  ASSERT_EQ(system -> bus().halt(), false);
  ASSERT_EQ(si -> getValue(), 0x1234);
  ASSERT_EQ(di -> getValue(), 0x5678);
  ASSERT_EQ(sp -> getValue(), 0x2000);
  ASSERT_EQ((*mem)[0x2000], 0x34);
  ASSERT_EQ((*mem)[0x2001], 0x12);
  ASSERT_EQ((*mem)[0x2002], 0x78);
  ASSERT_EQ((*mem)[0x2003], 0x56);
}
