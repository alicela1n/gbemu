#include "ppu.h"

void ppu_init(struct PPU *ppu, struct CPU *cpu) {
	ppu->lcd_ctr = 0xFF40;
	ppu->cpu = cpu;
}
