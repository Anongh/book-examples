#include <stdio.h>
#include <stdlib.h>

typedef unsigned char UCHAR;
int getnum(const UCHAR *p);
int get32(const UCHAR *p);
void put32(UCHAR *p, int i);

#define MAXSIZ	4 * 1024 * 1024

int main(int argc, UCHAR **argv)
{
	UCHAR *fbuf = malloc(MAXSIZ);
	int heap_siz, mmarea, fsiz, dsize, dofs, stksiz, wrksiz, entry, bsssiz;
	int heap_adr, i;
	FILE *fp;
	static UCHAR sign[4] = "Hari";

	/* 파라미터의 취득 */
	if (argc < 4) {
		puts("usage>bim2hrb appname.bim appname.hrb heap-size [mmarea-size]");
		return 1;
	}
	heap_siz = getnum(argv[3]);
	mmarea = 0;
	if (argc >= 5)
		mmarea = getnum(argv[4]);

	/* 파일 read */
	fp = fopen(argv[1], "rb");
	if (fp == NULL) {
err_bim:
		puts("bim file read error");
		return 1;
	}
	fsiz = fread(fbuf, 1, MAXSIZ, fp);
	fclose(fp);
	if (fsiz >= MAXSIZ || fsiz < 0)
		goto err_bim;

	/* 헤더 확인 */
	if (get32(&fbuf[4]) ! = 0x24) {	/* 파일중의 .text 스타트 주소 */
err_form:
		puts("bim file format error");
		return 1;
	}
	if (get32(&fbuf[8]) ! = 0x24)	/* 메모 리로드시의 .text 스타트 주소 */
		goto err_form;
	dsize  = get32(&fbuf[12]);	/* .data 섹션 사이즈 */
	dofs   = get32(&fbuf[16]);	/* 파일의 어디에 .data 섹션이 있을까 */
	stksiz = get32(&fbuf[20]);	/* 스택 사이즈 */
	entry  = get32(&fbuf[24]);	/* 엔트리 포인트 */
	bsssiz = get32(&fbuf[28]);	/* bss 사이즈 */

	/* 헤더 생성 */
	heap_adr = stksiz + dsize + bsssiz;
	heap_adr = (heap_adr + 0xf) & 0xfffffff0; /* 16바이트 단위로 올림 */
	wrksiz = heap_adr + heap_siz;
	wrksiz = (wrksiz + 0xfff) & 0xfffff000; /* 4KB단위로 올림 */
	put32(&fbuf[ 0], wrksiz);
	for (i = 0; i < 4; i++)
		fbuf[4 + i] = sign[i];
	put32(&fbuf[ 8], mmarea);
	put32(&fbuf[12], stksiz);
	put32(&fbuf[16], dsize);
	put32(&fbuf[20], dofs);
	put32(&fbuf[24], 0xe9000000);
	put32(&fbuf[28], entry - 0x20);
	put32(&fbuf[32], heap_adr);

	/* 파일 write */
	fp = fopen(argv[2], "wb");
	if (fp == NULL) {
err_hrb:
		puts("hrb file write error");
		return 1;
	}
	i = fwrite(fbuf, 1, fsiz, fp);
	fclose(fp);
	if (fsiz ! = i)
		goto err_hrb;

	return 0;
}

int getnum(const UCHAR *p)
{
	int i = 0, base = 10, sign = 1;
	UCHAR c;
	if (*p == '-') {
		p++;
		sign = -1;
	}
	if (*p == '0') {
		p++;
		base = 8;
		c = *p;
		if (c >= 'a')
			c -= 'a' - 'A';
		if (c == 'X') {
			p++;
			base = 16;
		}
		if (c == 'O') {
			p++;
			base = 8;
		}
		if (c == 'B') {
			p++;
			base = 2;
		}
	}
	for (;;) {
		c = *p++;
		if ('0' <= c && c <= '9')
			c -= '0'; 
		else if ('A' <= c && c <= 'F')
			c -= 'A' - 10;
		else if ('a' <= c && c <= 'f')
			c -= 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		i = i * base + c;
	}
	if (c >= 'a')
		c -= 'a' - 'A';
	if (c == 'K')
		i <<= 10;
	if (c == 'M')
		i <<= 20;
	if (c == 'G')
		i <<= 30;
	return i * sign;
}

int get32(const UCHAR *p)
{
	return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

void put32(UCHAR *p, int i)
{
	p[0] =  i        & 0xff;
	p[1] = (i >>  8) & 0xff;
	p[2] = (i >> 16) & 0xff;
	p[3] = (i >> 24) & 0xff;
	return;
}

/*

memo

[ .bim 파일의 구조 ]

+ 0 : .text 사이즈
+ 4 : 파일중의 .text 스타트 주소(0 x24)
+ 8 : 메모 리로드시의 .text 스타트 주소(0 x24)
+12 : .data 사이즈
+16 : 파일중의 .data 스타트 주소
+20 : 메모 리로드시의 .data 스타트 주소
+24 : 엔트리 포인트
+28 : bss 영역의 바이트수
+36 : 코드

[ .hrb 파일의 구조 ]

+ 0 : stack+.data+heap 의 크기(4KB의 배수)
+ 4 : 서명 "Hari"
+ 8 : mmarea의 크기(4KB의 배수)
+12 : 스택 초기치＆ .data 전송처
+16 : .data의 사이즈
+20 : .data의 초기치 열이 파일의 어디에 있을까
+24 : 0xe9000000
+28 : 엔트리 주소-0x20
+32 : heap 영역(malloc 영역) 개시 주소

*/
