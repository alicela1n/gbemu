#ifndef PPU_H
#define PPU_H

#include "memory.h"
#include <stdint.h>
#include <stdio.h>

struct CPU;

enum lcd_control_flags {
	bgw_en  = 0x01, // Negative bit (bit-0)
	obj_en  = 0x02, // Object enable (bit-1)
	obj_s   = 0x04, // Object size (bit-2)
	bg_tma  = 0x08, // Background tile map array (bit-3)
	wtile_a = 0x10,	// Background and window tile data area (bit-4)
	w_en    = 0x20, // Window enable flag (bit-5)
	wtile   = 0x40, // Window tile area map (bit-6)
	lcd_en  = 0x80  // LCD enable bit (bit-7)
};

struct PPU {
	uint16_t lcd_ctr; // Memory location of LCD control register
	struct CPU *cpu;
};

#endif
