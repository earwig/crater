.include	"05.inc1.asm"

.org $0000
main:
	di
	ld	c, $FA
	inc	c
	call	bar
