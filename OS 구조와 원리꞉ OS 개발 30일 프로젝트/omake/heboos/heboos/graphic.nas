; OS용 어플리 "GRAPHIC.NAS"
; TAB = 4
; copyright(C) 2003 카와이 히데미, KL-01

; prompt>nask graphic.nas graphic.hoa graphic.lst
; 로 어셈블(assemble)할 수 있습니다. nask는 tolset05 이후에 포함되어 있습니다.
; tolset05는 http://www.imasy.orr/~kawai/osask/developers.html 에 있습니다.

[FORMAT "BIN"]
[INSTRSET "i386"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 16]
			ORG		0x0100

;	팔레트의 설정
;	0-15는 시스템 팔레트로 16이후는 자유롭게 사용할 수 있게 했습니다. (웃음)

			MOV		BX, 64
PALLOP:
			MOV		AX, 0x1010
			MOV		DH, BL
			AND		DH, 0x3f
			MOV		CH, DH
			MOV		CL, 0
			PUSH	BX
			INT		0x10
			POP		BX
			INC		BX
			CMP		BX, 0x7f
			JBE		PALLOP

;	VRAM에 write

			PUSH	DS
			MOV		AX, 0xa000
			MOV		DS, AX
			XOR		BX, BX

;	먼저 8라인을 클리어

			MOV		AX, 0x4040
CLR8LOP:
			MOV		[BX], AX
			ADD		BX, 2
			CMP		BX,320*8
			JB		CLR8LOP

;	그라데이션(gradation) 생성

GRALOP0:
			MOV		CX,320*3/2
GRALOP1:
			MOV		[BX], AX
			ADD		BX, 2
			DEC		CX
			JNZ		GRALOP1
			ADD		AX, 0x0101
			CMP		AL, 0x7f
			JBE		GRALOP0

			POP		DS

;	무엇인가 키를 눌러 준다

			MOV		AH, 0x00
			INT		0x16

;	화면을 클리어하고 OS로 돌아온다

			MOV		AX, 2
			INT		0x80
			XOR		AX, AX
			XOR		CX, CX
			INT		0x80
