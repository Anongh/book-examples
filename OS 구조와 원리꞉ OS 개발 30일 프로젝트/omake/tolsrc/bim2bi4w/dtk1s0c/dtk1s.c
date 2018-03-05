typedef unsigned char UCHAR;

int tek_checkformat(int siz, UCHAR *p); /* 전개 후의 사이즈를 돌려준다 */
	/* -1:非osacmp */
	/* -2:osacmp이지만 대응할 수 없다 */

int tek_decode(int siz, UCHAR *p, UCHAR *q); /* 성공하면 0 */
	/* 正의 값은 포맷의 이상·미대응, 負의 값은 메모리 부족 */
	/* 메모리 부족은 보조 버퍼 이용시 이외는 발생하지 않는다 */

static unsigned int tek_getnum_s7s(UCHAR **pp);
int tek_lzrestore_stk1(int srcsiz, UCHAR *src, int outsiz, UCHAR *outbuf);
	/* osaskgo가 0.18[sec] */

int tek_checkformat(int siz, UCHAR *p)
{
	static UCHAR header[] = "\xff\xff\xff\x01\x00\x00\x00" "OSASKCMP";
	int i;
	if (siz < 17)
		return -1;
	for (i = 0; i < 15; i++) {
		if (p[i + 1] ! = header[i])
			return -1;
	}
	if (p[0] ! = 0x83)
		return -2;
	p += 16;
	return tek_getnum_s7s(&p);
}

int tek_decode(int siz, UCHAR *p, UCHAR *q)
{
	int dsiz, hed, bsiz;
	UCHAR *p1 = p + siz;
	p += 16;
	if ((dsiz = tek_getnum_s7s(&p)) > 0) {
		hed = tek_getnum_s7s(&p);
		bsiz = 1 << (((hed >> 1) & 0x0f) + 8);
		if (dsiz > bsiz || (hed & 0x21) ! = 0x01)
			return 1;
		if (hed & 0x40)
			tek_getnum_s7s(&p); /* 옵션 정보의 포인터를 건너뛴다 */
		if (tek_getnum_s7s(&p) ! = 0)
			return 1; /* 보조 버퍼 사용 */
		return tek_lzrestore_stk1(p1 - p, p, dsiz, q);
	}
	return 0;
}

static unsigned int tek_getnum_s7s(UCHAR **pp)
/* 이것은 반드시 big-endian */
/* 내용을 건드리기 쉽다 */
{
	unsigned int s = 0;
	UCHAR *p = *pp;
	do {
		s = s << 7 | *p++;
	} while ((s & 1) == 0);
	s >>= 1;
	*pp = p;
	return s;
}

int tek_lzrestore_stk1(int srcsiz, UCHAR *src, int outsiz, UCHAR *q)
{
	int by, lz, cp, ds;
	UCHAR *q1 = q + outsiz, *s7ptr = src, *q0 = q;
	do {
		if ((by = (lz = *s7ptr++) & 0x0f) == 0)
			by = tek_getnum_s7s(&s7ptr);
		if ((lz >>= 4) == 0)
			lz = tek_getnum_s7s(&s7ptr);
		do {
			*q++ = *s7ptr++;
		} while (--by);
		if (q >= q1)
			break;
		do {
			ds = (cp = *s7ptr++) & 0x0f;
			if ((ds & 1) == 0) {
				do {
					ds = ds << 7 | *s7ptr++;
				} while ((ds & 1) == 0);
			}
			ds = ~(ds >> 1);
			if ((cp >>= 4) == 0) {
				do {
					cp = cp << 7 | *s7ptr++;
				} while ((cp & 1) == 0);
				cp >>= 1;
			} /* 0이 오지 않는 것을 목표로 한다 */
			cp++;
			if (q + ds < q0)
				goto err;
			if (q + cp > q1)
				cp = q1 - q;
			do {
				*q = *(q + ds);
				q++;
			} while (--cp);
		} while (--lz);
	} while (q < q1);
	return 0;
err:
	return 1;
}
