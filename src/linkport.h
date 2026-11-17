#ifndef LINKPORT_H
#define LINKPORT_H

#include "helper.h"

/* This is technically the link port, the GameBoy uses its
 * link port as a serial port.
 */
struct LINKPORT {
  string * buffer; // A full buffer of link port output
  uint8_t lp_state;
  uint8_t lp_char; // Last character outputted through the link port
};

struct CPU;

void init_link_port(struct CPU *cpu);
uint8_t read_link_port(struct CPU *cpu);
void update_lp_buffer(struct CPU *cpu);

#endif
