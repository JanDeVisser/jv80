#include <chrono>
#include <iostream>
#include <gtest/gtest.h>
#include "alu.h"
#include "memory.h"
#include "controller.h"
#include "harness.h"

#include "src/cpu/microcode.inc"

constexpr word RAM_START = 0x2000;
constexpr word RAM_SIZE = 0x2000;
constexpr word ROM_START = 0x8000;
constexpr word ROM_SIZE = 0x2000;
constexpr word START_VECTOR = ROM_START;
constexpr word RAM_VECTOR = RAM_START;


class ControllerTest : public ::testing::Test {
protected:
  Harness *system = nullptr;
  Memory *mem = new Memory(RAM_START, RAM_SIZE, ROM_START, ROM_SIZE, nullptr);
  Controller *c = new Controller(mc);
  Register *gp_a = new Register(0x0);
  Register *gp_b = new Register(0x1);
  Register *gp_c = new Register(0x2);
  Register *gp_d = new Register(0x3);
  AddressRegister *pc = new AddressRegister(PC, "PC");
  AddressRegister *tx = new AddressRegister(TX, "TX");
  AddressRegister *sp = new AddressRegister(SP, "SP");
  AddressRegister *si = new AddressRegister(Si, "Si");
  AddressRegister *di = new AddressRegister(Di, "Di");
  ALU *alu = new ALU(RHS, new Register(LHS));

  void SetUp() override {
    system = new Harness();
    system -> insert(mem);
    system -> insert(c);
    system -> insert(gp_a);
    system -> insert(gp_b);
    system -> insert(gp_c);
    system -> insert(gp_d);
    system -> insert(pc);
    system -> insert(tx);
    system -> insert(sp);
    system -> insert(si);
    system -> insert(di);
    system -> insert(alu);
    system -> insert(alu -> lhs());
//    system -> printStatus = true;
  }

  void TearDown() override {
    delete system;
  }

};

byte mov_a_direct[] = {
  /* 8000 */ MOV_A_CONST, 0x42,
  /* 8002 */ HLT
};

TEST_F(ControllerTest, testMovADirect) {
  mem -> initialize(ROM_START, 3, mov_a_direct);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_A_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov a, #42 takes 4 cycles. hlt takes 3.
  ASSERT_EQ(system -> run(), 7);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
}

TEST_F(ControllerTest, testMovADirectUsingRun) {
  mem -> initialize(ROM_START, 3, mov_a_direct);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_A_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  ASSERT_EQ(system -> run(), 7);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
}

byte mov_a_absolute[] = {
  /* 8000 */ MOV_A_ADDR, 0x04, 0x80,
  /* 8003 */ HLT,
  /* 8004 */ 0x42
};

TEST_F(ControllerTest, testMovAAbsolute) {
  mem -> initialize(ROM_START, 5, mov_a_absolute);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_A_ADDR);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov a, (8004) takes 8 cycles. hlt takes 3.
  system -> cycles(11);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
}

byte mov_x_a[] = {
  MOV_A_CONST, 0x42,
  MOV_B_A,
  MOV_C_A,
  MOV_D_A,
  HLT,
};

TEST_F(ControllerTest, testMovAToOtherGPRs) {
  mem -> initialize(ROM_START, 6, mov_x_a);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_A_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov a, #42  4 cycles
  // mov x, a    3 cycles x3
  // hlt         3 cycles
  // Total       16 cycles
  ASSERT_EQ(system -> run(), 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
  ASSERT_EQ(gp_b -> getValue(), 0x42);
  ASSERT_EQ(gp_c -> getValue(), 0x42);
  ASSERT_EQ(gp_d -> getValue(), 0x42);
}

byte mov_x_b[] = {
  MOV_B_CONST, 0x42,
  MOV_A_B,
  MOV_C_B,
  MOV_D_B,
  HLT,
};

TEST_F(ControllerTest, testMovBToOtherGPRs) {
  mem -> initialize(ROM_START, 6, mov_x_b);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_B_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  ASSERT_EQ(system -> run(), 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_b -> getValue(), 0x42);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
  ASSERT_EQ(gp_c -> getValue(), 0x42);
  ASSERT_EQ(gp_d -> getValue(), 0x42);
}

byte mov_x_c[] = {
  MOV_C_CONST, 0x42,
  MOV_A_C,
  MOV_B_C,
  MOV_D_C,
  HLT,
};

TEST_F(ControllerTest, testMovCToOtherGPRs) {
  mem -> initialize(ROM_START, 6, mov_x_c);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_C_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  ASSERT_EQ(system -> run(), 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_c -> getValue(), 0x42);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
  ASSERT_EQ(gp_b -> getValue(), 0x42);
  ASSERT_EQ(gp_d -> getValue(), 0x42);
}

byte mov_x_d[] = {
  MOV_D_CONST, 0x42,
  MOV_A_D,
  MOV_B_D,
  MOV_C_D,
  HLT,
};

TEST_F(ControllerTest, testMovDToOtherGPRs) {
  mem -> initialize(ROM_START, 6, mov_x_d);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_D_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  ASSERT_EQ(system -> run(), 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_d -> getValue(), 0x42);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
  ASSERT_EQ(gp_b -> getValue(), 0x42);
  ASSERT_EQ(gp_c -> getValue(), 0x42);
}

byte mov_x_absolute[] = {
  /* 8000 */ MOV_A_ADDR, 0x0D, 0x80,
  /* 8003 */ MOV_B_ADDR, 0x0D, 0x80,
  /* 8006 */ MOV_C_ADDR, 0x0D, 0x80,
  /* 8009 */ MOV_D_ADDR, 0x0D, 0x80,
  /* 800C */ HLT,
  /* 800D */ 0x42
};

TEST_F(ControllerTest, testMovXAbsolute) {
  mem -> initialize(ROM_START, 14, mov_x_absolute);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_A_ADDR);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov x, (800D) takes 8 cycles x4
  // hlt takes 3.
  ASSERT_EQ(system -> run(),35);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
  ASSERT_EQ(gp_b -> getValue(), 0x42);
  ASSERT_EQ(gp_c -> getValue(), 0x42);
  ASSERT_EQ(gp_d -> getValue(), 0x42);
}

byte mov_addr_regs_direct[] = {
  /* 8000 */ MOV_SI_CONST, 0x42, 0x37,
  /* 8003 */ MOV_DI_CONST, 0x42, 0x37,
  /* 8006 */ MOV_SP_CONST, 0x42, 0x37,
  /* 8009 */ HLT,
};

TEST_F(ControllerTest, testMovAddrRegsDirect) {
  mem -> initialize(ROM_START, 10, mov_addr_regs_direct);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_SI_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov si, #3742 takes 6 cycles x3.
  // hlt takes 3.
  ASSERT_EQ(system -> run(), 21);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(si -> getValue(), 0x3742);
  ASSERT_EQ(di -> getValue(), 0x3742);
  ASSERT_EQ(sp -> getValue(), 0x3742);
}

byte mov_addr_regs_absolute[] = {
  /* 8000 */ MOV_SI_ADDR, 0x0A, 0x80,
  /* 8003 */ MOV_DI_ADDR, 0x0A, 0x80,
  /* 8006 */ MOV_SP_ADDR, 0x0A, 0x80,
  /* 8009 */ HLT,
  /* 800A */ 0x42, 0x37
};

TEST_F(ControllerTest, testMovAddrRegsAbsolute) {
  mem -> initialize(ROM_START, 12, mov_addr_regs_absolute);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_SI_ADDR);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov si, (8004) takes 10 cycles x3
  // hlt takes 3.
  ASSERT_EQ(system -> run(), 33);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(si -> getValue(), 0x3742);
  ASSERT_EQ(di -> getValue(), 0x3742);
  ASSERT_EQ(sp -> getValue(), 0x3742);
}

byte mov_addr_regs_from_other_regs[] = {
  /* 8000 */ MOV_C_CONST, 0x42,
  /* 8002 */ MOV_D_CONST, 0x37,
  /* 8004 */ MOV_SI_CD,
  /* 8005 */ MOV_DI_CD,
  /* 8006 */ MOV_SP_SI,
  /* 8007 */ HLT,
};

TEST_F(ControllerTest, testMovAddrRegsFromOtherRegs) {
  mem -> initialize(ROM_START, 8, mov_addr_regs_from_other_regs);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_C_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov c, #xx     4 cycles x2
  // mov xx, cd     4 cycles x2
  // mov sp, si     3 cycles
  // hlt            3 cycles
  // total          22
  ASSERT_EQ(system -> run(), 22);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(si -> getValue(), 0x3742);
  ASSERT_EQ(di -> getValue(), 0x3742);
  ASSERT_EQ(sp -> getValue(), 0x3742);
}

byte mov_gp_regs_from_si[] = {
  /* 8000 */ MOV_SI_CONST, 0x08, 0x80,
  /* 8003 */ MOV_A_SI,
  /* 8004 */ MOV_B_SI,
  /* 8005 */ MOV_C_SI,
  /* 8006 */ MOV_D_SI,
  /* 8007 */ HLT,
  /* 8008 */ 0x42,
  /* 8009 */ 0x43,
  /* 800A */ 0x44,
  /* 800B */ 0x45
};

TEST_F(ControllerTest, testMovGPRegsFromSI) {
  mem -> initialize(ROM_START, 12, mov_gp_regs_from_si);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_SI_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov si, #xxxx  6 cycles
  // mov xx, (si)   4 cycles x4
  // hlt            3 cycles
  // total          17
  ASSERT_EQ(system -> run(), 25);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(si -> getValue(), 0x800C);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
  ASSERT_EQ(gp_b -> getValue(), 0x43);
  ASSERT_EQ(gp_c -> getValue(), 0x44);
  ASSERT_EQ(gp_d -> getValue(), 0x45);
}

byte mov_gp_regs_from_di[] = {
  /* 8000 */ MOV_DI_CONST, 0x08, 0x80,
  /* 8003 */ MOV_A_DI,
  /* 8004 */ MOV_B_DI,
  /* 8005 */ MOV_C_DI,
  /* 8006 */ MOV_D_DI,
  /* 8007 */ HLT,
  /* 8008 */ 0x42,
  /* 8009 */ 0x43,
  /* 800A */ 0x44,
  /* 800B */ 0x45
};

TEST_F(ControllerTest, testMovGPRegsFromDI) {
  mem -> initialize(ROM_START, 12, mov_gp_regs_from_di);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_DI_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov di, #xxxx  6 cycles
  // mov xx, (si)   4 cycles x4
  // hlt            3 cycles
  // total          17
  ASSERT_EQ(system -> run(), 25);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(di -> getValue(), 0x800C);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
  ASSERT_EQ(gp_b -> getValue(), 0x43);
  ASSERT_EQ(gp_c -> getValue(), 0x44);
  ASSERT_EQ(gp_d -> getValue(), 0x45);
}

byte mov_di_from_si[] = {
  /* 8000 */ MOV_SI_CONST, 0x0B, 0x80,
  /* 8003 */ MOV_DI_CONST, 0x00, 0x20,
  /* 8006 */ MOV_DI_SI,
  /* 8007 */ MOV_DI_SI,
  /* 8008 */ MOV_DI_SI,
  /* 8009 */ MOV_DI_SI,
  /* 800A */ HLT,
  /* 800B */ 0x42,
  /* 800C */ 0x43,
  /* 800D */ 0x44,
  /* 800E */ 0x45
};

TEST_F(ControllerTest, testMovDIFromSI) {
  mem -> initialize(ROM_START, 16, mov_di_from_si);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_SI_CONST);
  ASSERT_EQ((*mem)[0x800B], 0x42);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov si, #xxxx  6 cycles x2
  // mov (di), (si) 6 cycles x4
  // hlt            3 cycles
  // total          39
  ASSERT_EQ(system -> run(), 39);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(si -> getValue(), 0x800F);
  ASSERT_EQ(di -> getValue(), 0x2004);
  ASSERT_EQ((*mem)[0x2000], 0x42);
  ASSERT_EQ((*mem)[0x2001], 0x43);
  ASSERT_EQ((*mem)[0x2002], 0x44);
  ASSERT_EQ((*mem)[0x2003], 0x45);
}

const byte push_a[] = {
  /* 8000 */ MOV_SP_CONST, 0x00, 0x20,
  /* 8003 */ MOV_A_CONST, 0x42,
  /* 8005 */ PUSH_A,
  /* 8006 */ HLT
};

TEST_F(ControllerTest, testPushA) {
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
  ASSERT_EQ(system -> bus.halt(), false);
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

TEST_F(ControllerTest, testPushPopA) {
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
  ASSERT_EQ(system -> bus.halt(), false);
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

TEST_F(ControllerTest, testPushPopGPRegs) {
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
  ASSERT_EQ(system -> bus.halt(), false);
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

TEST_F(ControllerTest, testPushPopAddrRegs) {
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
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 57);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(si -> getValue(), 0x1234);
  ASSERT_EQ(di -> getValue(), 0x5678);
  ASSERT_EQ(sp -> getValue(), 0x2000);
  ASSERT_EQ((*mem)[0x2000], 0x34);
  ASSERT_EQ((*mem)[0x2001], 0x12);
  ASSERT_EQ((*mem)[0x2002], 0x78);
  ASSERT_EQ((*mem)[0x2003], 0x56);
}

const byte jmp_basic[] = {
  /* 8000 */ JMP, 0x06, 0x80,
  /* 8003 */ MOV_A_CONST, 0x37,
  /* 8005 */ HLT,
  /* 8006 */ MOV_A_CONST, 0x42,
  /* 8008 */ HLT,
};

TEST_F(ControllerTest, testJmp) {
  mem -> initialize(ROM_START, 9, jmp_basic);
  ASSERT_EQ((*mem)[START_VECTOR], JMP);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // jmp            7 cycles
  // mov a, #xx     4 cycles
  // hlt            3 cycles
  // total         14
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 14);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
}

const byte jc[] = {
  /* 8000 */ JC, 0x06, 0x80,
  /* 8003 */ MOV_A_CONST, 0x37,
  /* 8005 */ HLT,
  /* 8006 */ MOV_A_CONST, 0x42,
  /* 8008 */ HLT,
};

TEST_F(ControllerTest, testJcCarrySet) {
  system -> bus.setFlag(SystemBus::ProcessorFlags::C);
  mem -> initialize(ROM_START, 9, jc);
  ASSERT_EQ((*mem)[START_VECTOR], JC);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // jc             7 cycles
  // mov a, #xx     4 cycles
  // hlt            3 cycles
  // total         14
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 14);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
}

TEST_F(ControllerTest, testJcCarryNotSet) {
  system -> bus.clearFlag(SystemBus::ProcessorFlags::C);
  mem -> initialize(ROM_START, 9, jc);
  ASSERT_EQ((*mem)[START_VECTOR], JC);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // jc             6 cycles
  // mov a, #xx     4 cycles
  // hlt            3 cycles
  // total         13
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 13);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x37);
}

TEST_F(ControllerTest, testBusFlagManip) {
  system -> bus.clearFlags();
  system -> bus.setFlag(SystemBus::ProcessorFlags::C);
  system -> bus.setFlag(SystemBus::ProcessorFlags::Z);

  ASSERT_TRUE(system -> bus.isSet(SystemBus::ProcessorFlags::C));
  ASSERT_TRUE(system -> bus.isSet(SystemBus::ProcessorFlags::Z));
  ASSERT_FALSE(system -> bus.isSet(SystemBus::ProcessorFlags::V));

  system -> bus.clearFlag(SystemBus::ProcessorFlags::C);

  ASSERT_FALSE(system -> bus.isSet(SystemBus::ProcessorFlags::C));
  ASSERT_TRUE(system -> bus.isSet(SystemBus::ProcessorFlags::Z));
  ASSERT_FALSE(system -> bus.isSet(SystemBus::ProcessorFlags::V));
}

const byte jnz[] = {
  /* 8000 */ JNZ, 0x06, 0x80,
  /* 8003 */ MOV_A_CONST, 0x37,
  /* 8005 */ HLT,
  /* 8006 */ MOV_A_CONST, 0x42,
  /* 8008 */ HLT,
};

TEST_F(ControllerTest, testJnzZeroNotSet) {
  system -> bus.clearFlag(SystemBus::ProcessorFlags::Z);
  mem -> initialize(ROM_START, 9, jnz);
  ASSERT_EQ((*mem)[START_VECTOR], JNZ);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // jnz            7 cycles
  // mov a, #xx     4 cycles
  // hlt            3 cycles
  // total         14
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 14);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
}

TEST_F(ControllerTest, testJnzZeroSet) {
  system -> bus.setFlag(SystemBus::ProcessorFlags::Z);
  mem -> initialize(ROM_START, 9, jnz);
  ASSERT_EQ((*mem)[START_VECTOR], JNZ);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // jc             6 cycles
  // mov a, #xx     4 cycles
  // hlt            3 cycles
  // total         13
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 13);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x37);
}

const byte jv[] = {
  /* 8000 */ JV, 0x06, 0x80,
  /* 8003 */ MOV_A_CONST, 0x37,
  /* 8005 */ HLT,
  /* 8006 */ MOV_A_CONST, 0x42,
  /* 8008 */ HLT,
};

TEST_F(ControllerTest, testJvCarrySet) {
  system -> bus.setFlag(SystemBus::ProcessorFlags::V);
  mem -> initialize(ROM_START, 9, jv);
  ASSERT_EQ((*mem)[START_VECTOR], JV);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // jc             7 cycles
  // mov a, #xx     4 cycles
  // hlt            3 cycles
  // total         14
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 14);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x42);
}

TEST_F(ControllerTest, testJvCarryNotSet) {
  system -> bus.clearFlag(SystemBus::ProcessorFlags::V);
  mem -> initialize(ROM_START, 9, jv);
  ASSERT_EQ((*mem)[START_VECTOR], JV);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // jc             6 cycles
  // mov a, #xx     4 cycles
  // hlt            3 cycles
  // total         13
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 13);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x37);
}

// MOV_ADDR_A      = 0x39,
// MOV_ADDR_B      = 0x3B,
// MOV_ADDR_C      = 0x3D,
// MOV_ADDR_D      = 0x3F,

const byte gp_to_absolute_mem[] = {
  MOV_A_CONST, 0x42,
  MOV_B_CONST, 0x43,
  MOV_C_CONST, 0x44,
  MOV_D_CONST, 0x45,
  MOV_ADDR_A, 0x00, 0x20,
  MOV_ADDR_B, 0x01, 0x20,
  MOV_ADDR_C, 0x02, 0x20,
  MOV_ADDR_D, 0x03, 0x20,
  HLT,
};

TEST_F(ControllerTest, testMovGPRegToMem) {
  mem -> initialize(ROM_START, 21, gp_to_absolute_mem);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_A_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov a, #xx     4 cycles x4  16
  // mov (xxxx), a  8 cycles x4  32
  // hlt            3 cycles      3
  // total                       51
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 51);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ((*mem)[0x2000], 0x42);
  ASSERT_EQ((*mem)[0x2001], 0x43);
  ASSERT_EQ((*mem)[0x2002], 0x44);
  ASSERT_EQ((*mem)[0x2003], 0x45);
}

const byte gp_to_rom[] = {
  /* 8000 */ MOV_A_CONST, 0x42,
  /* 8002 */ MOV_ADDR_A, 0x06, 0x80, // mov (8006), a
  /* 8005 */ HLT,
};

TEST_F(ControllerTest, testCantMovGPRegToROM) {
  mem -> initialize(ROM_START, 21, gp_to_rom);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_A_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  ASSERT_NE((*mem)[0x8006], 0x42);
  system -> run();
  ASSERT_EQ(system -> error, ProtectedMemory);
  ASSERT_NE((*mem)[0x8006], 0x42);
}

const byte gp_to_unmapped[] = {
  /* 8000 */ MOV_A_CONST, 0x42,
  /* 8002 */ MOV_ADDR_A, 0x06, 0x10, // mov (1006), a
  /* 8005 */ HLT,
};

TEST_F(ControllerTest, testCantMovGPRegToUnmappedMem) {
  mem -> initialize(ROM_START, 21, gp_to_unmapped);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_A_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  system -> run();
  ASSERT_EQ(system -> error, ProtectedMemory);
}

const byte gp_to_di_indirect[] = {
  MOV_A_CONST, 0x42,
  MOV_B_CONST, 0x43,
  MOV_C_CONST, 0x44,
  MOV_D_CONST, 0x45,
  MOV_DI_CONST, 0x00, 0x20,
  MOV_DI_A,
  MOV_DI_B,
  MOV_DI_C,
  MOV_DI_D,
  HLT,
};

TEST_F(ControllerTest, testMovGPRegToDiIndirect) {
  mem -> initialize(ROM_START, 16, gp_to_di_indirect);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_A_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov a, #xx     4 cycles x4  16
  // mov di, #xxxx  6 cycles      6
  // mov (di), a    4 cycles x4  16
  // hlt            3 cycles      3
  // total                       41
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 41);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ((*mem)[0x2000], 0x42);
  ASSERT_EQ((*mem)[0x2001], 0x43);
  ASSERT_EQ((*mem)[0x2002], 0x44);
  ASSERT_EQ((*mem)[0x2003], 0x45);
}

const byte addr_reg_to_absolute_mem[] = {
  MOV_SI_CONST, 0x22, 0x11,
  MOV_DI_CONST, 0x44, 0x33,
  MOV_C_CONST, 0x66,
  MOV_D_CONST, 0x55,
  MOV_ADDR_SI, 0x00, 0x20,
  MOV_ADDR_DI, 0x02, 0x20,
  MOV_ADDR_CD, 0x04, 0x20,
  HLT,
};

TEST_F(ControllerTest, testMovAddrRegToMem) {
  mem -> initialize(ROM_START, 21, addr_reg_to_absolute_mem);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_SI_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov si, #xxxx   6 cycles x2  12
  // mov c, #xx      4        x2   8
  // mov (xxxx), si 10        x3  30
  // hlt             3             3
  // total                        53
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 53);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ((*mem)[0x2000], 0x22);
  ASSERT_EQ((*mem)[0x2001], 0x11);
  ASSERT_EQ((*mem)[0x2002], 0x44);
  ASSERT_EQ((*mem)[0x2003], 0x33);
  ASSERT_EQ((*mem)[0x2004], 0x66);
  ASSERT_EQ((*mem)[0x2005], 0x55);
}

const byte cd_to_sidi_indirect[] = {
  MOV_SI_CONST, 0x00, 0x20,
  MOV_DI_CONST, 0x10, 0x20,
  MOV_C_CONST, 0x42,
  MOV_D_CONST, 0x37,
  MOV_SI_CD_,
  MOV_DI_CD_,
  HLT,
};

TEST_F(ControllerTest, testMovCDRegToMemViaSiDiIndirect) {
  mem -> initialize(ROM_START, 21, cd_to_sidi_indirect);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_SI_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov si, #xxxx   6 cycles x2  12
  // mov c, #xx      4        x2   8
  // mov (si), cd    6        x2  12
  // hlt             3             3
  // total                        35
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 35);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ((*mem)[0x2000], 0x42);
  ASSERT_EQ((*mem)[0x2001], 0x37);
  ASSERT_EQ((*mem)[0x2010], 0x42);
  ASSERT_EQ((*mem)[0x2011], 0x37);
}


//
// A R I T H M E T I C
//


typedef std::function<byte(Harness *, byte, byte)> Expect;

static Expect expect[16] = {
  /* 0x0 ADD */ [](Harness *system, byte lhs, byte rhs) {
    return (byte) (lhs + rhs);
  },

  /* 0x1 ADC */ [](Harness *system, byte lhs, byte rhs) {
    return (byte) (lhs + rhs +
           + (byte) (system -> bus.isSet(SystemBus::C)));
  },

  /* 0x2 SUB */ [](Harness *system, byte lhs, byte rhs) {
    return (byte) (lhs - rhs);
  },

  /* 0x3 SBB */ [](Harness *system, byte lhs, byte rhs) {
    return (byte) (lhs - rhs
           + (byte) (system -> bus.isSet(SystemBus::C)));
  },

  /* 0x4 */ nullptr,
  /* 0x5 */ nullptr,
  /* 0x6 */ nullptr,
  /* 0x7 */ nullptr,
  /* 0x8 AND */ [](Harness *system, byte lhs, byte rhs) {
    return lhs & rhs;
  },

  /* 0x9 OR  */ [](Harness *system, byte lhs, byte rhs) {
    return lhs | rhs;
  },

  /* 0xA XOR */ [](Harness *system, byte lhs, byte rhs) {
    return lhs ^ rhs;
  },

  /* 0xB NOT */ [](Harness *system, byte lhs, byte rhs) {
    return ~lhs;
  },

  /* 0xC SHL */ [](Harness *system, byte lhs, byte rhs) {
    byte ret = lhs << 1;
    if (system -> bus.isSet(SystemBus::C)) {
      ret |= 0x01;
    }
    return ret;
  },

  /* 0xD SHR */ [](Harness *system, byte lhs, byte rhs) {
    byte ret = lhs >> 1;
    if (system -> bus.isSet(SystemBus::C)) {
      ret |= 0x80;
    }
    return ret;
  },

  /* 0xE CLR */ [](Harness *system, byte lhs, byte rhs) {
    return (byte) 0;
  },
  /* 0xF */ nullptr,
};

static byte reg2instr[4] {
  MOV_A_CONST,
  MOV_B_CONST,
  MOV_C_CONST,
  MOV_D_CONST,
};

struct OpTest {
  byte            m_value     = 0x1F; // 31 dec
  byte            m_value2    = 0xF8; // 248
  byte            m_op_instr  = NOT_A;
  int             m_reg       = GP_A;
  int             m_reg2      = GP_B;
  ALU::Operations m_op        = ALU::Operations::NOT;

  OpTest(int reg, byte op_instr, ALU::Operations op, int reg2 = GP_B)
    : m_reg(reg), m_op_instr(op_instr), m_op(op), m_reg2(reg2) {
  }

  virtual const byte * bytes() const = 0;
  virtual int          bytesSize() const = 0;
  virtual int          regs() const = 0;
  virtual int          cycleCount() const = 0;

  void value(byte val) {
    m_value = val;
  }

  virtual void execute(Harness *system) {
    auto *mem = dynamic_cast<Memory *>(system -> component(MEMADDR));
    const byte *b = bytes();
    mem -> initialize(RAM_START, bytesSize(), b);
    ASSERT_EQ((*mem)[RAM_START], b[0]);
    (*mem)[0x2000] = reg2instr[m_reg];
    (*mem)[0x2001] = m_value;
    word instrAddr = 0x2002;
    if (regs() > 1) {
      (*mem)[0x2002] = reg2instr[m_reg2];
      (*mem)[0x2003] = m_value2;
      instrAddr = 0x2004;
    }
    (*mem)[instrAddr] = m_op_instr;

    auto *pc = dynamic_cast<AddressRegister *>(system -> component(PC));
    pc -> setValue(RAM_START);
    ASSERT_EQ(pc -> getValue(), RAM_START);

    byte e = expect[m_op](system, m_value, m_value2);
    auto cycles = system -> run();
    ASSERT_EQ(system -> error, NoError);
    ASSERT_EQ(cycles, cycleCount());
    ASSERT_EQ(system -> bus.halt(), false);

    auto *r = dynamic_cast<Register *>(system -> component(m_reg));
    ASSERT_EQ(r -> getValue(), e);
  }
};


// mov a, #xx      4
// not a           4
// hlt             3
// total          11
const byte unary_op[] = {
  /* 2000 */ MOV_A_CONST, 0x1F,
  /* 2002 */ NOP,
  /* 2003 */ HLT,
};

struct UnaryOpTest : public OpTest {
  UnaryOpTest(int reg, byte op_instr, ALU::Operations op)
    : OpTest(reg, op_instr, op) {
  }

  const byte * bytes() const override {
    return unary_op;
  }

  int bytesSize() const override {
    return 4;
  }

  int regs() const override {
    return 1;
  }

  int cycleCount() const override {
    return 11;
  }
};


// mov a, #xx      4        x2   8
// add a, b        5             5
// hlt             3             3
// total                        16
const byte binary_op[] = {
  /* 2000 */ MOV_A_CONST, 0x1F,
  /* 2002 */ MOV_B_CONST, 0xF8,
  /* 2004 */ NOP,
  /* 2005 */ HLT,
};

struct BinaryOpTest : public OpTest {
  BinaryOpTest(int reg, int reg2, byte op_instr, ALU::Operations op)
    : OpTest(reg, op_instr, op, reg2) {
  }

  const byte * bytes() const override {
    return binary_op;
  }

  int bytesSize() const override {
    return 6;
  }

  int regs() const override {
    return 2;
  }

  int cycleCount() const override {
    return 16;
  }

  void values(byte val1, byte val2) {
    m_value = val1;
    m_value2 = val2;
  }
};


TEST_F(ControllerTest, testAddAB) {
  BinaryOpTest t(GP_A, GP_B, ADD_A_B, ALU::Operations::ADD);
  t.execute(system);
}

TEST_F(ControllerTest, testAddABSetCarry) {
  BinaryOpTest t(GP_A, GP_B, ADD_A_B, ALU::Operations::ADD);
  t.values(0xC0, 0xC0);
  t.execute(system);
  ASSERT_TRUE(system -> bus.isSet(SystemBus::C));
}

TEST_F(ControllerTest, testAddABSetOverflow) {
  BinaryOpTest t(GP_A, GP_B, ADD_A_B, ALU::Operations::ADD);
  t.values(100, 50);
  t.execute(system);
  ASSERT_TRUE(system -> bus.isSet(SystemBus::V));
}

TEST_F(ControllerTest, testAddABSetZero) {
  BinaryOpTest t(GP_A, GP_B, ADD_A_B, ALU::Operations::ADD);
  t.values(-20, 20);
  t.execute(system);
  ASSERT_TRUE(system -> bus.isSet(SystemBus::Z));
  ASSERT_TRUE(system -> bus.isSet(SystemBus::C));
}

TEST_F(ControllerTest, testAdcABCarrySet) {
  BinaryOpTest t(GP_A, GP_B, ADC_A_B, ALU::Operations::ADC);
  system -> bus.setFlag(SystemBus::C);
  t.execute(system);
}

TEST_F(ControllerTest, testAdcABCarryNotSet) {
  BinaryOpTest t(GP_A, GP_B, ADC_A_B, ALU::Operations::ADC);
  system -> bus.clearFlags();
  t.execute(system);
}

const byte sub_a_b[] = {
  MOV_A_CONST, 0x20,
  MOV_B_CONST, 0x12,
  SUB_A_B,
  HLT,
};

TEST_F(ControllerTest, testSubAB) {
  BinaryOpTest t(GP_A, GP_B, SUB_A_B, ALU::Operations::SUB);
  t.execute(system);
}

const byte sbb_a_b[] = {
  MOV_A_CONST, 0x20,
  MOV_B_CONST, 0x12,
  SBB_A_B,
  HLT,
};

TEST_F(ControllerTest, testSbbNoCarry) {
  mem -> initialize(ROM_START, 6, sbb_a_b);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_A_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov a, #xx      4        x2   8
  // add a, b        5             5
  // hlt             3             3
  // total                        16
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x20 - 0x12);
}

TEST_F(ControllerTest, testSbbWithCarry) {
  mem -> initialize(ROM_START, 6, sbb_a_b);
  ASSERT_EQ((*mem)[START_VECTOR], MOV_A_CONST);

  pc -> setValue(START_VECTOR);
  ASSERT_EQ(pc -> getValue(), START_VECTOR);

  // mov a, #xx      4        x2   8
  // add a, b        5             5
  // hlt             3             3
  // total                        16
  system -> bus.setFlag(SystemBus::C);
  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x20 - 0x12 + 0x01);
}

// mov a, #xx      4        x2   8
// add a, b        5             5
// hlt             3             3
// total                        16
const byte op_a_b[] = {
  /* 2000 */ MOV_A_CONST, 0x1F,
  /* 2002 */ MOV_B_CONST, 0xF8,
  /* 2004 */ NOP,
  /* 2005 */ HLT,
};

TEST_F(ControllerTest, testAndAB) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2004] = AND_A_B;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x1F & 0xF8);
}

TEST_F(ControllerTest, testOrAB) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2004] = OR_A_B;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x1F | 0xF8);
}

TEST_F(ControllerTest, testXorAB) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2004] = XOR_A_B;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x1F ^ 0xF8);
}

// Register A Unary Operations

TEST_F(ControllerTest, testNotA) {
  UnaryOpTest t(GP_A, NOT_A, ALU::Operations::NOT);
  t.execute(system);
}

TEST_F(ControllerTest, testShlA) {
  UnaryOpTest t(GP_A, SHL_A, ALU::Operations::SHL);
  t.execute(system);
}

TEST_F(ControllerTest, testShrA) {
  UnaryOpTest t(GP_A, SHR_A, ALU::Operations::SHR);
  t.execute(system);
}

TEST_F(ControllerTest, testClrA) {
  UnaryOpTest t(GP_A, CLR_A, ALU::Operations::CLR);
  t.execute(system);
  ASSERT_TRUE(system -> bus.isSet(SystemBus::Z));
}

// Arithmetic A, C.

TEST_F(ControllerTest, testAddAC) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_C_CONST;
  (*mem)[0x2004] = ADD_A_C;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), (byte) (0x1F + 0xF8));
}

TEST_F(ControllerTest, testAdcAC) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_C_CONST;
  (*mem)[0x2004] = ADC_A_C;
  system -> bus.setFlag(SystemBus::C);

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), (byte) (0x1F + 0xF8 + 1));
}

TEST_F(ControllerTest, testSubAC) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_C_CONST;
  (*mem)[0x2004] = SUB_A_C;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), (byte) (0x1F - 0xF8));
}

TEST_F(ControllerTest, testSbbAC) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_C_CONST;
  (*mem)[0x2004] = SBB_A_C;
  system -> bus.setFlag(SystemBus::C);

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), (byte) (0x1F - 0xF8 + 1));
}

TEST_F(ControllerTest, testAndAC) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_C_CONST;
  (*mem)[0x2004] = AND_A_C;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x1F & 0xF8);
}

TEST_F(ControllerTest, testOrAC) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_C_CONST;
  (*mem)[0x2004] = OR_A_C;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x1F | 0xF8);
}

TEST_F(ControllerTest, testXorAC) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_C_CONST;
  (*mem)[0x2004] = XOR_A_C;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x1F ^ 0xF8);
}

// Arithmetic A, D.

TEST_F(ControllerTest, testAddAD) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_D_CONST;
  (*mem)[0x2004] = ADD_A_D;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), (byte) (0x1F + 0xF8));
}

TEST_F(ControllerTest, testAdcAD) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_D_CONST;
  (*mem)[0x2004] = ADC_A_D;
  system -> bus.setFlag(SystemBus::C);

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), (byte) (0x1F + 0xF8 + 1));
}

TEST_F(ControllerTest, testSubAD) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_D_CONST;
  (*mem)[0x2004] = SUB_A_D;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), (byte) (0x1F - 0xF8));
}

TEST_F(ControllerTest, testSbbAD) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_D_CONST;
  (*mem)[0x2004] = SBB_A_D;
  system -> bus.setFlag(SystemBus::C);

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), (byte) (0x1F - 0xF8 + 1));
}

TEST_F(ControllerTest, testAndAD) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_D_CONST;
  (*mem)[0x2004] = AND_A_D;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x1F & 0xF8);
}

TEST_F(ControllerTest, testOrAD) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_D_CONST;
  (*mem)[0x2004] = OR_A_D;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x1F | 0xF8);
}

TEST_F(ControllerTest, testXorAD) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_D_CONST;
  (*mem)[0x2004] = XOR_A_D;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(), 0x1F ^ 0xF8);
}


// Register B Unary Operations

TEST_F(ControllerTest, testNotB) {
  UnaryOpTest t(GP_B, NOT_B, ALU::Operations::NOT);
  t.execute(system);
}

TEST_F(ControllerTest, testShlB) {
  UnaryOpTest t(GP_B, SHL_B, ALU::Operations::SHL);
  t.execute(system);
}

TEST_F(ControllerTest, testShrB) {
  UnaryOpTest t(GP_B, SHR_B, ALU::Operations::SHR);
  t.execute(system);
}

TEST_F(ControllerTest, testClrB) {
  UnaryOpTest t(GP_B, CLR_B, ALU::Operations::CLR);
  t.execute(system);
  ASSERT_TRUE(system -> bus.isSet(SystemBus::Z));
}


// Register C Unary Operations

TEST_F(ControllerTest, testNotC) {
  UnaryOpTest t(GP_C, NOT_C, ALU::Operations::NOT);
  t.execute(system);
}

TEST_F(ControllerTest, testShlC) {
  UnaryOpTest t(GP_C, SHL_C, ALU::Operations::SHL);
  t.execute(system);
}

TEST_F(ControllerTest, testShrC) {
  UnaryOpTest t(GP_C, SHR_C, ALU::Operations::SHR);
  t.execute(system);
}

TEST_F(ControllerTest, testClrC) {
  UnaryOpTest t(GP_C, CLR_C, ALU::Operations::CLR);
  t.execute(system);
  ASSERT_TRUE(system -> bus.isSet(SystemBus::Z));
}


// Register D Unary Operations

TEST_F(ControllerTest, testNotD) {
  UnaryOpTest t(GP_D, NOT_D, ALU::Operations::NOT);
  t.execute(system);
}

TEST_F(ControllerTest, testShlD) {
  UnaryOpTest t(GP_D, SHL_D, ALU::Operations::SHL);
  t.execute(system);
}

TEST_F(ControllerTest, testShrD) {
  UnaryOpTest t(GP_D, SHR_D, ALU::Operations::SHR);
  t.execute(system);
}

TEST_F(ControllerTest, testClrD) {
  UnaryOpTest t(GP_D, CLR_D, ALU::Operations::CLR);
  t.execute(system);
  ASSERT_TRUE(system -> bus.isSet(SystemBus::Z));
}


// SWAP

TEST_F(ControllerTest, testSwpAB) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_B_CONST;
  (*mem)[0x2004] = SWP_A_B;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(),  0xF8);
  ASSERT_EQ(gp_b -> getValue(),  0x1F);
}

TEST_F(ControllerTest, testSwpAC) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_C_CONST;
  (*mem)[0x2004] = SWP_A_C;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(),  0xF8);
  ASSERT_EQ(gp_c -> getValue(),  0x1F);
}

TEST_F(ControllerTest, testSwpAD) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2002] = MOV_D_CONST;
  (*mem)[0x2004] = SWP_A_D;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_a -> getValue(),  0xF8);
  ASSERT_EQ(gp_d -> getValue(),  0x1F);
}

TEST_F(ControllerTest, testSwpBC) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2000] = MOV_B_CONST;
  (*mem)[0x2002] = MOV_C_CONST;
  (*mem)[0x2004] = SWP_B_C;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_b -> getValue(),  0xF8);
  ASSERT_EQ(gp_c -> getValue(),  0x1F);
}

TEST_F(ControllerTest, testSwpBD) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2000] = MOV_B_CONST;
  (*mem)[0x2002] = MOV_D_CONST;
  (*mem)[0x2004] = SWP_B_D;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_b -> getValue(),  0xF8);
  ASSERT_EQ(gp_d -> getValue(),  0x1F);
}

TEST_F(ControllerTest, testSwpCD) {
  mem -> initialize(RAM_START, 6, op_a_b);
  ASSERT_EQ((*mem)[RAM_START], MOV_A_CONST);
  (*mem)[0x2000] = MOV_C_CONST;
  (*mem)[0x2002] = MOV_D_CONST;
  (*mem)[0x2004] = SWP_C_D;

  pc -> setValue(RAM_START);
  ASSERT_EQ(pc -> getValue(), RAM_START);

  auto cycles = system -> run();
  ASSERT_EQ(system -> error, NoError);
  ASSERT_EQ(cycles, 16);
  ASSERT_EQ(system -> bus.halt(), false);
  ASSERT_EQ(gp_c -> getValue(),  0xF8);
  ASSERT_EQ(gp_d -> getValue(),  0x1F);
}
