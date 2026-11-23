#ifndef LINKPORT_H
#define LINKPORT_H

#include "helper.h"

enum ctr_bit {
  serial_enable = 0x80,
  clk_spd = 0x02,
  clk_sel = 0x00
};

/* This is technically the link port, the GameBoy uses its
 * link port as a serial port.
 */
struct LINKPORT {
  string * out_buffer; // A full buffer of link port output
  uint8_t lp_ctr; // Serial control
  uint8_t lp_out_char; // Last character outputted through the link port
};

struct CPU;

void init_link_port(struct CPU *cpu);
uint8_t read_link_port(struct CPU *cpu);
void update_lp_out_buffer(struct CPU *cpu);

#endif
