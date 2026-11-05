#include "../cpu.h"
#include "../rom.h"
#include "unity.h"
#include <stdio.h>
#include <assert.h>

void setUp(void) {} //Nothing to do yet
void tearDown(void) {} //Nothing to do yet

void test_flags(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  cpu_set_flag(cpu, flag_c, true);
  bool c = cpu_check_flag(cpu, flag_c);
  cpu_set_flag(cpu, flag_h, true);
  bool h = cpu_check_flag(cpu, flag_h);
  cpu_set_flag(cpu, flag_z, true);
  bool z = cpu_check_flag(cpu, flag_z);
  cpu_set_flag(cpu, flag_n, true);
  bool n = cpu_check_flag(cpu, flag_n);

  TEST_ASSERT(c == true);
  TEST_ASSERT(cpu->registers[f] & flag_c);
  TEST_ASSERT(h == true);
  TEST_ASSERT(cpu->registers[f] & flag_h);
  TEST_ASSERT(z == true);
  TEST_ASSERT(cpu->registers[f] & flag_z);
  TEST_ASSERT(n == true);
  TEST_ASSERT(cpu->registers[f] & flag_n);

  cpu_cleanup(cpu);
}

int main(void) {
  UNITY_BEGIN();

  printf("main(): Beginning test\n");
  RUN_TEST(test_flags);
  
  return UNITY_END();
}
