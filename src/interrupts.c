#include "interrupts.h"
#include "cpu.h"

/* cpu_set_ie_flag(): Set a flag in the interrupt enable register,
 * passing 1 to set sets the flag, passing 0 unsets it
 */
void cpu_set_ie_flag(struct CPU *cpu, uint8_t interrupt, bool set) {
  if (set)
    cpu->ie_r |= interrupt;
  else
    cpu->ie_r &= ~(interrupt);

  write_memory8(cpu, cpu->ie_r, 0xFF0F);
}

/* cpu_set_if_flag(): Set a flag in the interrupt flag register,
 * passing 1 to set sets the flag, passing 0 unsets it
 */
void cpu_set_if_flag(struct CPU *cpu, uint8_t interrupt, bool set) {
  if (set)
    cpu->if_r |= interrupt;
  else
    cpu->if_r &= ~(interrupt);

  write_memory8(cpu, cpu->if_r, 0xFFFF);
}

/* handle_interrupts(): Handle CPU interrupts
 */
void handle_interrupts(struct CPU *cpu) {
  cpu->if_r = read_memory8(cpu, 0xFF0F);
  cpu->ie_r = read_memory8(cpu, 0xFFFF);
  uint8_t irq_flags = (cpu->if_r & cpu->ie_r);

  if (cpu->ime) {
    if (irq_flags & int_vblank) {
      cpu_push(cpu, cpu->pc);
      cpu->pc = 0x0040;
      cpu_set_ie_flag(cpu, int_vblank, false);
      return;
    }

    if (irq_flags & int_lcd) {
      cpu_push(cpu, cpu->pc);
      cpu->pc = 0x0048;
      cpu_set_ie_flag(cpu, int_lcd, false);
      return;
    }

    if (irq_flags & int_timer) {
      cpu_push(cpu, cpu->pc);
      cpu->pc = 0x0050;
      cpu_set_ie_flag(cpu, int_timer, false);
      return;
    }
    
    if (irq_flags & int_serial) {
      cpu_push(cpu, cpu->pc);
      cpu->pc = 0x0058;
      cpu_set_ie_flag(cpu, int_serial, false);
      return;
    }

    if (irq_flags & int_joypad) {
      cpu_push(cpu, cpu->pc);
      cpu->pc = 0x0060;
      cpu_set_ie_flag(cpu, int_joypad, false);
      return;
    }
  }
}
