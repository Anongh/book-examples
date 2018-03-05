/* dtk계의 샘플 */

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char UCHAR;

int tek_checkformat(int siz, UCHAR *p); /* 전개 후의 사이즈를 돌려준다 */
	/* -1:非osacmp */
	/* -2:osacmp이지만 대응할 수 없다 */
int tek_decode(int siz, UCHAR *p, UCHAR *q); /* 성공하면 0 */
	/* 定의 값은 포맷의 이상·미대응, 負의 값은 메모리 부족 */
	/* 메모리 부족은 보조 버퍼 이용시 이외는 발생하지 않는다 */

int main(int argc, UCHAR **argv)
/* 출력 파일을 nul로 하면, 전개 속도 측정 모드가 된다(이른바 테스트) */
{
	FILE *fp;
	int tsiz, dsiz, st;
	UCHAR *tbuf, *dbuf = NULL, flag_nul = 0;
	if (argc ! = 3) {
		puts("usage>tstdstk5 input-file output-file");
		return 1;
	}
	tbuf = argv[2];
	if (tbuf[0] == 'n' && tbuf[1] == 'u' && tbuf[2] == 'l' && tbuf[3] == '\0')
		flag_nul = 1;
	fp = fopen(argv[1], "rb");
	if (fp == NULL) {
		puts("can't open input-file");
		return 1;
	}
	tsiz = 8 * 1024 * 1024 + 1024;
	if (flag_nul == 0) {
		fseek(fp, 0, SEEK_END);
		tsiz = ftell(fp);
		fseek(fp, 0, SEEK_SET);
	}
	tbuf = malloc(tsiz);
	if (tbuf == NULL) {
		puts("malloc error");
		return 1;
	}
	tsiz = fread(tbuf, 1, tsiz, fp);
	fclose(fp);
	dsiz = tek_checkformat(tsiz, tbuf);
	if (dsiz == -2) {
		puts("unsupported format");
		return 1;
	}
	if (dsiz == -1) {
		/* 무압축파일 */
		dsiz = tsiz;
		dbuf = tbuf;
		tbuf = NULL;
	} else if (dsiz >= 0) {
		dbuf = malloc(dsiz);
		if (dbuf == NULL) {
			free(tbuf);
			puts("malloc error");
			return 1;
		}
		st = tek_decode(tsiz, tbuf, dbuf);
		if (st > 0) {
			free(tbuf);
			puts("unsupported format");
			return 1;
		}
		if (st < 0) {
			free(tbuf);
			puts("malloc error");
			return 1;
		}
	}
	if (flag_nul == 0) {
		fp = fopen(argv[2], "wb");
		if (fp == NULL) {
			puts("can't open output-file");
			return 1;
		}
		if (dsiz)
			fwrite(dbuf, 1, dsiz, fp);
		fclose(fp);
	}
	if (tbuf)
		free(tbuf);
	free(dbuf);
	return 0;
}

//void puthex(unsigned int i)
//{
//	printf("%08X ", i);
//	return;
//}
