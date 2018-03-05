/* "wce.c":와일드 카드 전개 */

/* +:on -:off #:esc */
/* default는 on */

/* prompt>wce.exe e arcfile basepath align #b=basepath * > $esar.ini */

/* *:일반 파일과 디렉토리에 match */
/* **:일반 파일에만 match(=slash를 포함하지 않는다) */

/* #f=* 와 #f내의 #w의 서포트를, 나머지##도 */
/*	"#f=copy from:* to:@:" 와 "#f=#wpre *#w"의 차이 */
/*	잠깐 기다림, 자동" 부가 기능과의 균형은?  */
/*	자 이렇게 하면 될까 */
/*	"#f=copy #wfrom:*#w to:@:" */
/*	#w는 자동 판별. #W는 강제 */
/*	디폴트는#f=#w*#w가 된다 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char UCHAR;

UCHAR *outnames(UCHAR *outb0, UCHAR *outb1, UCHAR *outb3, UCHAR flags);
UCHAR *setnames(UCHAR *names0, UCHAR *base0, UCHAR *base1, UCHAR *tmp0, UCHAR *tmp1, UCHAR *ltmp0, UCHAR *ltmp1);
UCHAR *fixbase(UCHAR *base0, UCHAR *base1);
int pathcmp(UCHAR *p0, UCHAR *p1);

int main(int argc, UCHAR **argv)
{
	int i, j, len0, len1;
	UCHAR m = 1, mc = 0, mw, mo = 0, ms = 1, mp = 0;
	UCHAR *p, *base0 = malloc(4096), *base1 = base0 + 8, *p0, *p1, *n;
	UCHAR *names0 = malloc(8 * 1024 * 1024), *names1 = NULL;
	UCHAR *outb0 = malloc(4 * 1024 * 1024), *outb1 = outb0;
	UCHAR *outb2 = malloc(4 * 1024 * 1024), *outb3 = outb2;
	UCHAR *ltmp = malloc(1024 * 1024), *tmp = malloc(4096);

	for (i = 0; i < 8; i++)
		base0[i] = "dir    \x22"[i];

	for (i = 1; i < argc; i++) {
		p = argv[i];
		if (p[0] == '(' && p[1] == '\0') {
			mo++;
			continue;
		}
		if (p[0] == ')' && p[1] == '\0' && mo > 0) {
			mo--;
			goto check_mo;
		}
		if (p[0] == '+' && p[1] == '\0') {
			m = 1;
			continue;
		}
		if (p[0] == '-' && p[1] == '\0') {
			m = 0;
			continue;
		}
		if (p[0] == '#' && p[1] ! = '\0') {
			if (p[1] == 'b' && p[2] == '=') { /* basepath 지정 */
			//	if (p[3])
					p += 3;
			//	else {
			//		i++;
			//		if (i >= argc)
			//			break;
			//		p = argv[i];
			//	}
				base1 = base0 + 8;
				while (*p)
					*base1++ = *p++;
				names1 = NULL;
				continue;
			}
			if (p[1] == 'C' && p[2] == '=') { /* CAP 지정 */
				/* 잘 생각해 보면 본래는 이런 옵션은 있어서는 안된다 */
			//	if (p[3] == '\0')
			//		mc ^= 1;
				if (p[3] == '+' || p[3] == '1')
					mc = 1;
				if (p[3] == '-' || p[3] == '0')
					mc = 0;
				continue;
			}
			if (p[1] == 's' && p[2] == '=') { /* sort 지정 */
			//	if (p[3] == '\0')
			//		ms ^= 1;
				if (p[3] == '+' || p[3] == '1')
					ms = 1;
				if (p[3] == '-' || p[3] == '0')
					ms = 0;
				continue;
			}
			if (p[1] == '! ' && p[2] == '=') { /* 배제 지정 */
				/* 장래적으로는 배제 지정에서도 와일드 카드를 사용할 수 있도록 한다 */
				p0 = outb0;
			//	if (p[3])
					p += 3;
			//	else {
			//		i++;
			//		if (i >= argc)
			//			break;
			//		p = argv[i];
			//	}
				len1 = strlen(p) + 2;
				while (p0 < outb1) {
					len0 = strlen(p0 + 1) + 2;
					if (len0 == len1 && strcmp(p, p0 + 1) == 0) {
						outb1 -= len0;
						p1 = p0;
						while (p1 < outb1) {
							*p1 = *(p1 + len0);
							p1++;
						}
						continue;
					}
					p0 += len0;
				}
				continue;
			}
#if 0
			if (p[1] == 'a' && p[2] == '=') { /* 문자열 부가(요점 버퍼링) */
				if (p[3])
					p += 3;
				else {
					i++;
					if (i >= argc)
						break;
					p = argv[i];
				}
				if (outb0 >= outb1) {
					*outb1++ = 0x00;
					outb1++;
				}
				outb1--;
				while (*p)
					*outb1++ = *p++;
				*outb1++ = '\0';
				continue;
			}
#endif
			if (p[1] == 'p' && p[2] == '=') { /* print 지정 */
			//	if (p[3] == '\0')
			//		mp ^= 1;
				if (p[3] == '+' || p[3] == '1')
					mp = 1;
				if (p[3] == '-' || p[3] == '0')
					mp = 0;
				continue;
			}
			p++; /* #+, #-, ## */
		}
		if (m == 0) {
through:
			*outb1++ = 0;
			while (*p)
				*outb1++ = *p++;
			*outb1++ = '\0';
			goto check_mo;
		}
	//	if (m == 1) {
			p0 = p;
			while (*p) {
				if (*p++ == '*')
					goto wild;
			}
			p = p0;
			goto through;
	//	}
wild:
		/* 와일드 카드를 분할 */
		/* 전방 일치 부분, 후방 일치 부분, 중간 임의 부분 */
		len0 = 0;
		p = p0;
		while (p0[len0] ! = '*')
			len0++;
		p = &p0[len0] + 1;
		mw = 0;
		if (*p == '*') {
			mw = 1;
			p++;
		}
		len1 = 0;
		p1 = p;
		while (p1[len1] ! = '\0')
			len1++;
		if (names1 == NULL) {
			base1 = fixbase(base0 + 8, base1);
			names1 = setnames(names0, base0, base1, tmp, tmp, ltmp, ltmp + 1024 * 1024);
		}
		n = names0;
		while (n < names1) {
			j = len0;
			while (j) {
				if (n[j - 1] ! = p0[j - 1])
					break;
				j--;
			}
			if (j)
				goto next;
			p = n;
			while (*p)
				p++;
			if (p - n < len0 + len1)
				goto next;
			p -= len1;
			j = len1;
			while (j) {
				if (p[j - 1] ! = p1[j - 1])
					break;
				j--;
			}
			if (j)
				goto next;
			if (mw) {
				j = p - (n + len0);
				while (j) {
					if (n[len0 + j - 1] == '/')
						break;
					j--;
				}
				if (j)
					goto next;
			}
			*outb1++ = mc;
			while (*n)
				*outb1++ = *n++;
			*outb1++ = '\0';
	next:
			while (*n)
				n++;
			n++;			
		}
check_mo:
		if (mo == 0 && outb0 < outb1) {
			outb3 = outnames(outb0, outb1, outb3, ms | mp << 1);
			outb1 = outb0;
		}
	}
	if (mp)
		puts(outb2);
	else
		return system(outb2);
	return 0;
}

UCHAR *outnames(UCHAR *outb0, UCHAR *outb1, UCHAR *outb3, UCHAR flags)
{
	UCHAR *p, *m, *m0, mc;

	p = outb0;
	do {
		mc = *p++;
		while (*p) {
			if (mc ! = 0 && 'a' <= *p && *p <= 'z')
				*p -= 'a' - 'A';
			p++;
		}
		p++;
	} while (p < outb1);

	m0 = outb0;
	for (;;) {
		m = m0;
		while (*m >= 2) {
			m++;
			while (*m)
				m++;
			m++;
			m0 = m;
			if (m >= outb1)
				goto fin;
		}
		p = m;
		if (flags & 1) {
			for (;;) {
				p++;
				while (*p)
					p++;
				p++;
				if (p >= outb1)
					break;
				if (*p >= 2)
					continue;
				if (pathcmp(m + 1, p + 1) > 0)
					m = p;
			}
		}
		*m = 0xff;
		mc = 0;
		p = m + 1;
		while (*p) {
			if (*p <= ' ')
				mc = 1;
			p++;
		}
		if (mc)
			*outb3++ = 0x22;
		p = m + 1;
		while (*p)
			*outb3++ = *p++;
		if (mc)
			*outb3++ = 0x22;
		if (flags & 2)
			*outb3++ = '\n';
		else
			*outb3++ = ' ';
	}
fin:
	return outb3;
}

UCHAR *setnames(UCHAR *names0, UCHAR *base0, UCHAR *base1, UCHAR *tmp0, UCHAR *tmp1, UCHAR *ltmp0, UCHAR *ltmp1)
/* Win2000의 dir : 일시가 전부 0x00이라도, 뭐가 되든, 어떤 일자가 최초로 들어간다 */
/* Win95의 dir : 선두는 8.3 파일명 */
/* WinXP의 dir : 2000으로 매우 비슷하지만 미묘하게 다르다.  */
{
	int i;
	UCHAR *p1, *p = ltmp0, c, d, *q, *r, *s;
	FILE *fp;
	for (i = 0; i < 16; i++)
		base1[i] = "\" > __wce__.tmp"[i];
	i = system(base0);
	fp = fopen("__wce__.tmp", "rb");
	p1 = ltmp0;
	if (fp) {
		p1 = ltmp0 + fread(ltmp0, 1, ltmp1 - ltmp0, fp);
		fclose(fp);
	}
	remove("__wce__.tmp");
	if (i)
		return names0;

	/* Win95, Win2000, WinXP */
	/* 5개의'\n'를 건너 뛰고 읽어라 */
	for (i = 5; i > 0; i--) {
		while (*p ! = '\n')
			p++;
		p++;
	}
	for (;;) {
		/* Win2000, WinXP */
		c = 0;
		for (i = 0; i < 17; i++) {
			d = "####/##/##  ##:##"[i];
			if (d == '#') {
				if (p[i] < '0')
					c = 1;
				if (p[i] > '9')
					c = 1;
			} else if (d ! = p[i])
				c = 1;
		}
		d = 0;
		if (c == 0) {
			p += 17;
			while (*p == ' ')
				p++;
			if (*p == '<' && p[4] == '>' && p[5] == ' ') {
				d = 1;
				p += 6;
			} else if ('0' <= *p && *p <= '9') {
				while (*p ! = ' ')
					p++; 
			}
			while (*p == ' ')
				p++;
		} else {
			/* Win95 */
			for (i = 0; i < 12; i++) {
				if (p[i] < ' ')
					c = 0;
			}
			if (p[0] == ' ')
				c = 0;
			if (p[8] ! = ' ')
				c = 0;
			if (c == 0)
				break;
			if (p[15] == '<' && p[19] == '>')
				d = 1;
			p += 44;
		}

		if (d ! = 0 && p[0] == '. ' && p[1] <= '\r')
			goto skip;	/* .  */
		if (d ! = 0 && p[0] == '. ' && p[1] == '. ' && p[2] <= '\r')
			goto skip;	/* .. */
		if (d == 0 && p[0] == '_' && p[1] == '_' && p[2] == 'w' && p[3] == 'c' && 
			p[4] == 'e' && p[5] == '_' && p[6] == '_' && p[7] == '. ' &&
			p[8] == 't' && p[9] == 'm' && p[10] == 'p' && p[11] <= '\r')
			goto skip;	/* __wce__.tmp */
		q = tmp1;
		while (*p >= ' ')
			*q++ = *p++;
		if (d)
			*q++ = '/';
		*q = '\0';
		r = tmp0;
		while (*r)
			*names0++ = *r++;
		*names0++ = '\0';
		if (d) {
			s = base1;
			r = tmp1;
			while (*r ! = '/')
				*s++ = *r++;
			*s++ = '\\';
			names0 = setnames(names0, base0, s, tmp0, q, p1, ltmp1);
		}
skip:
		while (*p >= ' ')
			p++;
		while (*p < ' ')
			p++;
	}	

	return names0;
}

UCHAR *fixbase(UCHAR *base0, UCHAR *base1)
/* win32전용 */
/* Linux라면 아마 이런 처리는 필요없어서, 말미에 /를 붙이는 것만으로 좋겠지.  */
{
	UCHAR *p = base0;
	while (p < base1) {
		if (*p == '/')
			*p = '\\';
		p++;
	}
	if (base0 < base1 && base1[-1] ! = '\\')
		*base1++ = '\\';
	return base1;
}

int pathcmp(UCHAR *p0, UCHAR *p1)
{
	int i;
	while (*p0 == *p1) {
		if (*p0 == '\0')
			return 0;
		p0++;
		p1++;
	}
	/* 동일하지는 않다 */
	i = *p0 - *p1;
	/* 디렉토리는 어떤 일반 파일보다도 작게 하기 위해서,
		디렉토리라면 문자 코드로부터 256을 줄여 평가한다 */
	while (*p0 ! = '\0' && *p0 ! = '/')
		p0++;
	if (*p0)
		i -= 256;
	while (*p1 ! = '\0' && *p1 ! = '/')
		p1++;
	if (*p1)
		i += 256;
	return i;
}
