; OS용 어플리 "HELLO.NAS"
; TAB = 4
; copyright(C) 2003 카와이 히데미, KL-01

;	prompt>nask hello.nas hello.hoa hello.lst
; 로 어셈블(assemble) 할 수 있습니다. nask는 tolset05이후에 포함되어 있습니다.
; tolset05는 http://www.imasy.orr/~kawai/osask/developers.html에 있습니다.

[FORMAT "BIN"]
[INSTRSET "i386"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 16]
			ORG		0x0100

			MOV		AX, 1
			MOV		SI, msg
			INT		0x80
			XOR		AX, AX
			XOR		CX, CX
			INT		0x80
msg:
			DB		"hello, world", 10,0
