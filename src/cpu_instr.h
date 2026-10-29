#ifndef CPU_INSTR_H
#define CPU_INSTR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct CPU;

void cpu_toggle_halt(struct CPU *cpu);
bool cpu_check_flag(struct CPU *cpu, uint8_t flag);
void cpu_set_flag(struct CPU *cpu, uint8_t flag, bool set);
void cpu_toggle_flag(struct CPU *cpu, uint8_t flag);
void cpu_ld_reg8(struct CPU *cpu, uint8_t r, uint8_t n);
void cpu_ld_reg16(struct CPU *cpu, uint16_t r, uint16_t n);
void cpu_ld_mem8(struct CPU *cpu, uint16_t addr, uint8_t a);
void cpu_ld_mem16(struct CPU *cpu, uint16_t addr, uint16_t b);
void cpu_inc_reg16(struct CPU *cpu, uint16_t r, uint16_t n);
void cpu_inc_reg8(struct CPU *cpu, uint8_t r, uint8_t n);
void cpu_dec_reg8(struct CPU *cpu, uint8_t r, uint8_t n);
void cpu_dec_reg16(struct CPU *cpu, uint16_t r, uint16_t n);
void cpu_inc_mem8(struct CPU *cpu, uint8_t addr, uint8_t n);
void cpu_dec_mem8(struct CPU *cpu, uint8_t addr, uint8_t n);
void cpu_jp(struct CPU *cpu, uint16_t addr);
void cpu_jr(struct CPU *cpu, uint8_t addr);
void cpu_add8(struct CPU *cpu, uint8_t *a, uint8_t b);
void cpu_adc8(struct CPU *cpu, uint8_t *a, uint8_t *b);
void cpu_add_reg16(struct CPU *cpu, uint16_t rp1, uint16_t rp2);
void cpu_sub8(struct CPU *cpu, uint8_t *a, uint8_t b);
void cpu_sbc8(struct CPU *cpu, uint8_t *a, uint8_t *b);
uint16_t cpu_read_reg16(struct CPU *cpu, uint16_t r);
void cpu_cp8(struct CPU *cpu, uint8_t *a, uint8_t *b);
void cpu_and(struct CPU *cpu, uint8_t *a, uint8_t *b);
void cpu_xor(struct CPU *cpu, uint8_t *a, uint8_t *b);
void cpu_or(struct CPU *cpu, uint8_t *a, uint8_t *b);
void cpu_pop(struct CPU *cpu, uint8_t *a, uint8_t *b);
void cpu_push(struct CPU *cpu, uint8_t *a, uint8_t *b);
void cpu_call(struct CPU *cpu, uint16_t addr);
void cpu_rst(struct CPU *cpu, uint16_t n);
void cpu_ret(struct CPU *cpu);

void cpu_bit_r(struct CPU *cpu, uint8_t r, uint8_t b);
void cpu_bit_mem8(struct CPU *cpu, uint16_t addr, uint8_t b);
void cpu_res_r(struct CPU *cpu, uint8_t r, uint8_t b);
void cpu_res_mem8(struct CPU *cpu, uint16_t addr, uint8_t b);
void cpu_set_r(struct CPU *cpu, uint8_t r, uint8_t b);
void cpu_set_mem8(struct CPU *cpu, uint16_t addr, uint8_t b);
void cpu_rl_r(struct CPU *cpu, uint8_t r);
void cpu_rl_mem8(struct CPU *cpu, uint16_t addr);
void cpu_rr_r(struct CPU *cpu, uint8_t r);
void cpu_rr_mem8(struct CPU *cpu, uint16_t addr);
void cpu_rlc_r(struct CPU *cpu, uint8_t r);
void cpu_rlc_mem8(struct CPU *cpu, uint16_t addr);
void cpu_rrc_r(struct CPU *cpu, uint8_t r);
void cpu_rrc_mem8(struct CPU *cpu, uint16_t addr);
void cpu_sl_r(struct CPU *cpu, uint8_t r);
void cpu_sl_mem8(struct CPU *cpu, uint16_t addr);
void cpu_sr_r(struct CPU *cpu, uint8_t r);
void cpu_sr_mem8(struct CPU *cpu, uint16_t addr);
void cpu_sla_r(struct CPU *cpu, uint8_t r);
void cpu_sla_mem8(struct CPU *cpu, uint16_t addr);
void cpu_srl_r(struct CPU *cpu, uint8_t r);
void cpu_srl_mem8(struct CPU *cpu, uint16_t addr);
void cpu_sra_r(struct CPU *cpu, uint8_t r);
void cpu_swap_r(struct CPU *cpu, uint8_t r);
void cpu_swap_m(struct CPU *cpu, uint8_t addr);
void cpu_sra_mem8(struct CPU *cpu, uint16_t addr);

#endif
