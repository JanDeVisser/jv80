/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define TESTNAME PushPopFlags
#include "controllertest.h"

const byte assembly[] = {
    /* 0000 */ MOV_A_CONST, 0x42, /* mov a,#42  4 */
    /* 0002 */ MOV_B_CONST, 0x42, /* mov b,#42  4 */
    /* 0004 */ CLR_C,             /* clr c      5 */
    /* 0005 */ CMP_A_B,           /* cmp a,b    4 */
    /* 0006 */ JNZ, 0x0e, 0x80,   /* jnz #hlt   7 */
    /* 0009 */ PUSH_FLAGS,          /* pushfl     4 */
  /* 000a */ MOV_C_CONST, 0x37,   /* mov c,#37  4 */
  /* 000c */ CMP_A_C,             /* cmp a,c    4 */
  /* 000d */ POP_FLAGS,           /* popfl      4 */
  /* 000e */ HLT,                 /* hlt        3 */
                                  /* Total     42 */
};

TEST_F(TESTNAME, pushflPopfl) {
  mem -> initialize(ROM_START, 16, assembly);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_A_CONST);

  sp -> setValue(RAM_START);
  ASSERT_EQ(sp -> getValue(), RAM_START);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  nmiAt = 0x8011;
  auto cycles = system -> run();
  ASSERT_EQ(system -> error(), NoError);
  ASSERT_EQ(cycles, 42);
  ASSERT_EQ(system -> bus().halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
  ASSERT_EQ(gp_b -> getValue(), 0x42);
  ASSERT_EQ(gp_c -> getValue(), 0x37);
  ASSERT_TRUE(system->bus().isSet(SystemBus::Z));
}
