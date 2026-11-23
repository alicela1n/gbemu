#include "linkport.h"
#include "cpu.h"

/* init_link_port(): Initialize and populate link port struct */
void init_link_port(struct CPU *cpu) {
  cpu->lp->out_buffer = new_string(NULL); // Initialize serial buffer
  cpu->lp->lp_out_char = read_memory8(cpu, 0xff01);
  cpu->lp->lp_ctr = read_memory8(cpu, 0xff02);
}

uint8_t read_link_port(struct CPU *cpu) {
  cpu->lp->lp_out_char = read_memory8(cpu, 0xff01);
  cpu->lp->lp_ctr = read_memory8(cpu, 0xff02);
  return cpu->lp->lp_out_char;
}

void update_lp_out_buffer(struct CPU *cpu) {
  if (cpu->lp->lp_ctr) {
    uint8_t c = read_link_port(cpu);
    if (c) {
      append_char(cpu->lp->out_buffer, c);
    }
  }
}
