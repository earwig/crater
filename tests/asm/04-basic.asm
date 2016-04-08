.org $0000
main:
	di
	ld	a, $23
	inc	a
	call	foo

foo:
	push	bc
	xor	c
	ret	z
	rrca
	ret
