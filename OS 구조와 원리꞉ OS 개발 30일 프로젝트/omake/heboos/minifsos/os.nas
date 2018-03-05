; NASK로 만들었다. COM 파일형OS (AT호환기종 전용)
;		mini-FirstStepOS
;
; TAB = 4
; copyright(C) 2003 카와이 히데미, KL-01
;
; .COM 파일은 프로그램이 64KB를 넘지 않는 한 가장 간단
; 대부분의 사람은 최종적으로 OS가 64KB를 넘기 때문에 .COM을 
; 선택하고 싶지 않을 것이다. 그러나 최초부터 .EXE의 노고를 맛볼 필요는 없다.
; 64KB를 넘으면 그 때에 .EXE화하면 좋지 않을까.
; .COM라면 세그먼트(segment)를 신경쓰지 않아도 되기 때문에 편할 것이다.

; prompt>nask os.nas os.com os.lst
; 로 어셈블(assemble)할 수 있습니다. nask는 tolset05 이후에 포함되어 있습니다.
; tolset05는 http://www.imasy.orr/~kawai/osask/developers.html에 있습니다.

[FORMAT "BIN"]
[INSTRSET "i386"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 16]
		ORG		0x0100 ; .COM은 반드시 ORG 0x0100로 시작된다

; 그런데 무엇을 할까. 우선 320x200x256색으로도 해 그라데이션(gradation)을 
; 표시하며 놀아볼까? ("hello"따위는 너무 평범하니까)

;	화면 모드와 팔레트 설정

			MOV 	AX, 0x0013
			INT		0x10
			XOR		BX, BX
PALLOP:
			MOV		AX, 0x1010
			MOV		DH, BL
			MOV		CH, BL
			MOV		CL, 0
			PUSH	BX
			INT		0x10
			POP		BX
			INC		BX
			CMP		BX, 0x3f
			JBE		PALLOP

			MOV		AX, 0x1010
			MOV		BL, 255
			MOV		DH, 0x3f
			MOV		CX, 0x3f3f
			INT		0x10

;	VRAM에 WRITE

			PUSH	DS
			MOV		AX, 0xa000
			MOV		DS, AX
			XOR		BX, BX

;	먼저 8라인을 클리어

			XOR		AX, AX
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
			CMP		AL, 0x3f
			JBE		GRALOP0

			POP		DS

			JMP		$	; 끝(리셋 버튼을 누르세요)
