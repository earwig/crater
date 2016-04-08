.org $0100

.include	"05.inc2.asm"
.include	"05.inc3.asm"

bar:
	push	de
	call	blah
	xor	d
	ret	pe
	call	testfunc
	exx
	ret
