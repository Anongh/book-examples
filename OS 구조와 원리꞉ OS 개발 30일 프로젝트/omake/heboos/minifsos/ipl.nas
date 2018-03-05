; NASK로 만든 OS전용 IPL(AT호환기종용)
; TAB = 4
; copyright(C) 2003 카와이 히데미, KL-01
; OS는 512바이트 미만이라고 가정
; OS는 실린더 0, 헤드 0, 섹터 2가 들어가 있다고 상정

;	prompt>nask ipl.nas ipl.bin ipl.lst
; 로 어셈블(assemble) 할 수 있습니다. nask는 tolset05 이후에 포함되어 있습니다.
; tolset05는 http://www.imasy.orr/~kawai/osask/developers.html 에 있습니다.

; 디스크 이미지를 만들려면 다음과 같이 합니다.
;	prompt>copy /B ipl.bin+os.com fdimage.bin

; 이것을 디스크에 write하려면 다음과 같습니다.
; 우선 포맷이 끝난 디스크를 A:에 넣는다.
;	prompt>imgtol w a: fdimage.bin
; imgtol도 tolset05이후에 포함되어 있습니다.


[FORMAT "BIN"]
[INSTRSET "i386"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 16]
			ORG		0x7c00

			JMP		START
			NOP
			DB		'hogehoge'

; FAT를 준비하고 있지 않기 때문에 BPB를 만들어 FAT12라고 해 둔다
; 이것을 없애면, 디스크를 포맷 하지 않으면 imgtol로 read, write하는 것은 불가능하므로
; 어쩔 수 없이 넣어 둔다

			DW		512 	; 섹터 길이(바이트 단위)
			DB		1	; 클러스터 길이(섹터 단위)
			DW		1	; boot sector 길이(섹터 단위)
			DB		2	; FAT의 수
			DW		0x00e0	; root directory entries.
			DW		2880	; 총 섹터수
			DB		0xf0	; media descriptor byte.
			DW		9	; FAT의 길이(섹터 단위)
			DW		18	; 1트랙에 포함된 섹터수
			DW		2	; 헤드수
			DD		0	; 눈에 보이지 않는 섹터수
			DD		2880	; 총 섹터수
			DB		0, 0	; /* unknown */
			DB		0x29
			DD		0xffffffff
			DB		"hogehoge   "
			DB		"FAT12   "

; 이 디스크는 DOS나 Win에서는 read, write할 수 없기 때문에 잘 부탁드립니다

START:
			MOV		AX, 0x0201
			MOV		CX, 0x0002
			XOR		DX, DX
			MOV		BX, 0x0800
			MOV		ES, BX
			MOV		BX, 0x0100
			INT		0x13
			JC		ERR

; .COM 파일로 DOS호환용 레지스터 상태로 해 준다

			MOV		AX, 0x0800
			MOV		SS, AX
			MOV		SP, 0xfffe
			MOV		DS, AX
			MOV		ES, AX
			JMP		0x0800:0x0100
ERR:
			INT		0x18	; ROM-BASIC에 (웃음)

			RESB	0x7dfe-$	; 0x7dfe까지 0x00로 채운다

			DB		0x55, 0xaa
			