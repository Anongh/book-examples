/* "osalink1.c":OSASK LINK 프로그램 version 0.1.
	copyright(C) 2003 H.Kawai (카와이 히데미)

	최초가 BASE.EXE, 그 후는 각종 .BIN(.TEK)를 상정하고 있다 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define	OPTIONFILE	"OSALINK1.OPT"
#define	OUTPUTFILE	"OSASK.EXE"
#define	BUFSIZE		2 * 1024 * 1024

void wordstore(char *p, const int w)
{
	p[0] =  w       & 0xff;
	p[1] = (w >> 8) & 0xff;
	return;
}

void dwordstore(char *p, const int d)
{
	p[0] =  d        & 0xff;
	p[1] = (d >>  8) & 0xff;
	p[2] = (d >> 16) & 0xff;
	p[3] = (d >> 24) & 0xff;
	return;
}

const int wordload(const unsigned char *p)
{
	return p[0] | p[1] << 8;
}

const int dwordload(const unsigned char *p)
{
	return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

const int script(char *opt, char *inp, char *out);

static unsigned char buf0[BUFSIZE];

const int main(int argc, char **argv)
{
	FILE *fp0, *fp1;
	int i, j, k, size, totalsize = 0;
	unsigned char fname[32], name[8], c;
	unsigned char *buf = buf0, *buf1, *buf2, *buf3;
	unsigned char *optfile = OPTIONFILE, *outfile = OUTPUTFILE;

	if (argc == 4)
		return script(argv[1], argv[2], argv[3]);
	if (argc >= 2)
		optfile = argv[1];
	if (argc >= 3)
		outfile = argv[2];

	// OPTIONFILE의 read와 각 파일의 사이즈 취득
	fp0 = fopen(optfile, "r");
	if (fp0 == NULL) {
		fprintf(stderr, "Can't open \"%s\". \n", optfile);
		return 1;
	}
	for (i = 0; fscanf(fp0, " %s", fname) == 1; i++) {
		fp1 = fopen(fname, "rb");
		if (fp1 == NULL) {
err1:
			fclose(fp0);
			fprintf(stderr, "Can't open \"%s\". \n", fname);
			return 1;
		}
		buf1 = buf + totalsize;
		size = fread(buf + totalsize, 1, BUFSIZE - totalsize, fp1);
		fclose(fp1);
		if (size <= 0)
			goto err1;
		if (size >= BUFSIZE - totalsize)
			goto err1;
		if (i == 0) {
			buf2 = buf + (wordload(buf + 0x204) << 4) - 16 + 0x220;
			goto skip; /* BASE.EXE는 가공하지 않는다 */
		}

		for (j = 0; j < 8; j++)
			name[j] = ' ';
		for (j = 0; j < 8 && fname[j] ! = '. '; j++)
			name[j] = tolower(fname[j]);
		for (buf3 = buf2; *buf3; buf3 += 16) {
			c = 0;
			for (j = 0; j < 8; j++)
				c |= name[j] ^ buf3[j];
			if (c)
				continue;
			j = size;
			if (size > 20) {
			//	c = 0;
				for (k = 1; k < 16; k++)
					c |= buf1[k] ^ "\x82\xff\xff\xff\x01\x00\x00\x00OSASKCMP"[k];
				if (c == 0) {
					size -= 20;
					j = dwordload(buf1 + 16); /* 전개 후의 사이즈 */
					for (k = 0; k < size; k++)
						buf1[k] = buf1[k + 20];
					buf3[0x0f] = 0x80; /* 압축 플래그를 세운다 */
				}
			}
			dwordstore(buf3 + 0x08, j);
			wordstore(buf3 + 0x0c, (totalsize - 0x200) >> 4);
			break;
		}
skip:
		totalsize += size;
		while (totalsize & 0x0f)
			*(buf + totalsize++) = '\0'; /* 패러그래프 단위의 align */
	}
	fclose(fp0);

	/* 헤더 조정 */
	wordstore(buf + 0x02, totalsize & 0x01ff); // 최종 페이지 사이즈
	wordstore(buf + 0x04, (totalsize + 0x1ff) >> 9); // 파일 페이지수
	wordstore(buf + 0x0e, (totalsize - 0x200) >> 4); // 초기 SS

	if (buf[0x0208] == 0x10 && buf[0x0209] == 0x89 && buf[0x020a] == 0x00) {
		/* OSASK의 KHBIOS용 스크립트를 발견 */
		size = (totalsize + (0x1ff - 0x200)) >> 9;
		buf[0x020b] =  size       & 0xff;
		buf[0x020c] = (size >> 8) & 0xff;
	}

	/* 출력 */
	fp1 = fopen(outfile, "wb");
	fwrite(buf, 1, totalsize, fp1);
	fclose(fp1);
	return 0;
}

char *skipspace(char *s, char *s1)
{
retry:
	while (s < s1 && *s <= ' ')
		s++;
	if (s + 1 < s1 && *s == '/' && *(s + 1) == '*') {
		s += 2;
		do {
			while (s < s1 && *s ! = '*')
				s++;
			if (s + 1 >= s1)
				return s1;
			s += 2;
		} while (*(s - 1) ! = '/');
		goto retry;
	}
	return s;
}

char *checktoken(char *s, char *s1)
/* 이것을 부르기 전에 skipspace 해 둘 것 */
{
	char *s0 = s;
	if (s < s1) {
		do {
			char c = *s;
			if (c <= ' ')
				break;
			if (c == ', ')
				break;
			if (c == '(')
				break;
			if (c == ')')
				break;
			if (c == '{')
				break;
			if (c == '}')
				break;
			if (c == ';')
				break;
			if (c == '\x22')
				break;
			s++;
		} while (s < s1);
		if (s == s0)
			s++;
	}
	return s;
}

int getnum(char *s, char *s1)
/* 10진수와 16진수만, 게다가 부의 값은 취급하지 않는다 */
/* 에러라면 -1을 반환한다 */
/* s는 skipspace()해 둘 것 */
{
	int i = 0, base = 10, c;
	if (s >= s1)
		goto err;
	if (s + 2 < s1 && *s == '0' && tolower(*(s + 1)) == 'x') {
		s += 2;
		base = 16;
	}
	do {
		c = tolower(*s++);
		if (c < '0')
			goto err;
		c -= '0';
		if (c <= 9)
			goto skip;
		if (c < 'a' - '0')
			goto err;
		c -= 'a' - '0' - 10;
		if (c >= base)
			goto err;
	skip:
		i = i * base + c;
	} while (s < s1);
	return i;
err:
	return -1;
}

const char cmptoken(char *s, char *s1, char *t)
{
	do {
		if (s >= s1)
			break;
		if (*s++ ! = *t)
			break;
		t++;
	} while (*t);
	return *t;
}

char *get2prm(char *s, char *scr1, int *p0, int *p1)
/* "(p0, p1," */
{
	char *s1;

	s = skipspace(s, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if (cmptoken(s, s1, "(") ! = 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if ((*p0 = getnum(s, s1)) < 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if (cmptoken(s, s1, ",") ! = 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if ((*p1 = getnum(s, s1)) < 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if (cmptoken(s, s1, ",") ! = 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	return s;

err:
	return NULL;
}

char *get3prm(char *s, char *scr1, int *p0, int *p1, int *p2)
{
	char *s1;

	s = get2prm(s, scr1, p0, p1);
	if (s == NULL)
		goto err;
	s1 = checktoken(s, scr1);
	if ((*p2 = getnum(s, s1)) < 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if (cmptoken(s, s1, ")") ! = 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if (cmptoken(s, s1, ";") ! = 0)
		goto err;
	return s1;

err:
	return NULL;
}

const int script(char *opt, char *inp, char *out)
/* 스크립트가 4KB를 넘으면 다운됩니다 */
{
	unsigned char *buf = buf0 + 4 * 1024, *scr0 = buf0, *scr1, *s, *s1;
	FILE *fp;
	int size, memofs, filofs;

	fp = fopen(opt, "r");
	if (fp == NULL)
		goto err;
	scr1 = scr0 + fread(scr0, 1, 4 * 1024, fp);
	fclose(fp);

	s = scr0;
	for (;;) {
		s = skipspace(s, scr1);
		if (s >= scr1)
			break;
		s1 = checktoken(s, scr1);
		if (cmptoken(s, s1, ";") == 0)
			continue;
		if (cmptoken(s, s1, "load") == 0) {
			s = get3prm(s1, scr1, &size, &memofs, &filofs);
			if (s == NULL)
				goto err;
			fp = fopen(inp, "rb");
			if (fp == NULL)
				goto err;
			fseek(fp, filofs, SEEK_SET);
			if (fread(buf + memofs, 1, size, fp) ! = size)
				goto err;
			fclose(fp);
			continue;
		}
		if (cmptoken(s, s1, "store") == 0) {
			s = get3prm(s1, scr1, &size, &memofs, &filofs);
			if (s == NULL)
				goto err;
			fp = fopen(out, "wb");
			if (fp == NULL)
				goto err;
			fseek(fp, filofs, SEEK_SET);
			if (fwrite(buf + memofs, 1, size, fp) ! = size)
				goto err;
			fclose(fp);
			continue;
		}
		if (cmptoken(s, s1, "fill") == 0) {
			s = get3prm(s1, scr1, &size, &memofs, &filofs);
			if (s == NULL)
				goto err;
			while (size > 0) {
				buf[memofs++] = (char) filofs;
				size--;
			}
			continue;
		}

#if 0
		if (cmptoken(s, s1, "if") == 0) {

			s = skipspace(s1, scr1);
			if (s >= scr1)
				goto err;
			s1 = checktoken(s, scr1);
			if (cmptoken(s, s1, "(") ! = 0) {
				goto err;
			s = skipspace(s1, scr1);
			if (s >= scr1)
				goto err;

		}
#endif

err:
		return 1;
	}
	return 0;
}
