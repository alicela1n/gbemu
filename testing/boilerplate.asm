SECTION "header", ROM0[$100]
	jp LCDOff
	ds $150 - @, 0

LCDOff:
	;; This code turns off the display
	;; Check line counter until it's equal to 144
	ldh A, [$FF44]
	cp 144
	jp nz, LCDOff ;; Keep jumping to LCDOff until A does not equal 144

	xor A ;; Set A to 0
	ld [$FF40], A ;; Write 0 to LCD control register

	jp nz, Main

Main:
	;; Write code here
	jp nz, LCDOn
	
LCDOn:
	;; Turn display on
	ld A, %10010001
	ld [$FF40], A

.infloop
	jp .infloop
