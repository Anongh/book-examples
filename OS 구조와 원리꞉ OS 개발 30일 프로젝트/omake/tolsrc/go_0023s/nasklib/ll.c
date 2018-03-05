/* "LL.c":Lilte-Linker */
/* Copyright(C) 2002 H.Kawai */

/* 방침：늦어도 괜찮으니까 우선 움직일 것!  */
/* 알고리즘에 열중하는 것은 움직이게 되고 나서 */

#define	DEBUG			0

#include "../include/stdlib.h"

#if (DEBUG)
	#include "../include/stdio.h"
#endif

#define	INVALID_DELTA		0x40000000

extern int nask_maxlabels;

#define	MAXSIGMAS				  4

#define	UCHAR			unsigned char

struct STR_SIGMA {
	int scale;
	unsigned int subsect, terms;
};

struct STR_VALUE {
	int min;
	unsigned int delta, flags;
	int scale[2];
	unsigned int label[2];
	struct STR_SIGMA sigma[MAXSIGMAS];
};

#define	VFLG_ERROR			0x01	/* 에러 */
#define	VFLG_SLFREF			0x02	/* 자기 참조 에러 */
#define	VFLG_UNDEF			0x04	/* 미정의 에러 */
#define	VFLG_EXTERN			0x10	/* 외부 라벨 */
#define	VFLG_CALC			0x20	/* 계산중 */
#define	VFLG_ENABLE			0x40	/* STR_LABEL로 유효한 일을 나타낸다 */

struct STR_LABEL {
	struct STR_VALUE value;
	UCHAR *define; /* 이것이 NULL라고 extlabel */
};

struct STR_SUBSECTION {
	unsigned int min, delta, unsolved; /* unsolved == 0 이라면 최적화할 필요 없음 */
	UCHAR *sect0, *sect1;
};

static struct STR_LABEL *label0;
static struct STR_SUBSECTION *subsect0;

unsigned int solve_subsection(struct STR_SUBSECTION *subsect, char force);
UCHAR *skip_mc30(UCHAR *s, UCHAR *bytes, char flag);
void calcsigma(struct STR_VALUE *value);
void addsigma(struct STR_VALUE *value, struct STR_SIGMA sigma);
UCHAR *LL_skip_expr(UCHAR *p);
UCHAR *LL_skip_mc30(UCHAR *s, UCHAR *bytes, char flag);
UCHAR *LL_skipcode(UCHAR *p);

void init_value(struct STR_VALUE *value)
{
	int i;
	value->min = value->delta = value->scale[0] = value->scale[1] = 0;
	value->flags = 0;
	value->label[0] = value->label[1] = -1;
	for (i = 0; i < MAXSIGMAS; i++) {
		value->sigma[i]. scale = 0;
		value->sigma[i]. subsect = 0;
		value->sigma[i]. terms = 0;
	}
	return;
}

unsigned int get_id(int len, UCHAR **ps, int i)
/* len = 0~3 */
{
	union UINT_UCHAR {
		unsigned int i;
		UCHAR c[4];
	} u;
	UCHAR *s = *ps;
	u.i = i;
	i = 0;
	do {
		u.c[i++] = *s++;
	} while (--len >= 0);
	*ps = s;
	return u.i;
}

void calc_value0(struct STR_VALUE *value, UCHAR **pexpr);

void calc_value(struct STR_VALUE *value, UCHAR **pexpr)
{
	calc_value0(value, pexpr);
	if (value->sigma[0]. scale)
		calcsigma(value);
	return;
}

void enable_label(struct STR_LABEL *label)
{
	struct STR_VALUE value;
	UCHAR *t;

//	if (label->value.flags & VFLG_CALC) {
//		label->value.flags |= VFLG_SLFREF;
//		return;
//	}
//	init_value(&value);
//	value.label[0] = label - label0; /* for EXTERN */
//	value.scale[0] = 1;
//	value.flags |= label->value.flags & VFLG_EXTERN;
	if ((t = label->define) ! = NULL) {
		label->value.flags |= VFLG_CALC;
		calc_value0(&value, &t);
		if (value.flags & VFLG_SLFREF) {
			init_value(&value);
			value.flags |= VFLG_SLFREF;
		}
	} else /* if ((label->value.flags & VFLG_EXTERN) == 0) */ {
		/* EXTERN때는 곧바로 ENABLE이 되므로, 여기에 올 리 없다 */
		init_value(&value);
		value.flags |= VFLG_UNDEF;
	}
	label->value = value;
	label->value.flags |= VFLG_ENABLE;
	return;
}

void calc_value0(struct STR_VALUE *value, UCHAR **pexpr)
{
	UCHAR *expr = *pexpr, c, *t;
	int i, j, k;
	struct STR_VALUE tmp, tmp2;
	struct STR_SUBSECTION *subsect;
	c = *expr++;
	init_value(value);
	if (c <= 6) {
		/* 정수 */
		i = 0;
		if (c & 0x01)
			i--;
		value->min = get_id(c >> 1, &expr, i);
		goto fin;
	}
	if (c == 0x07) {
		/* 정수 1 */
		value->min = 1;
		goto fin;
	}
	if (c <= 0x0b) {
		/* 라벨 인용 */
		struct STR_LABEL *label;
		label = &label0[i = get_id(c - 8, &expr, 0)];
		if ((label->value.flags & VFLG_ENABLE) == 0) {
			if (label->value.flags & VFLG_CALC) {
				value->flags |= VFLG_SLFREF;
				goto fin;
			}
			enable_label(label);
		}
		*value = label->value;
		goto fin;
	}
	#if (DEBUG)
		if (c < 0x10)
			goto dberr;
	#endif
	if (c < 0x20) {
		/* 연산자 */
		calc_value0(value, &expr);
		if (c == 0x10)
			goto fin; /* 단항 연산자 + */
		if (c == 0x11) {
			/* 단항 연산자 - */
			value->min = - (int) (value->min + value->delta);
	minus2:
			value->scale[0] *= -1;
 			value->scale[1] *= -1;
			for (i = 0; i < MAXSIGMAS; i++)
				value->sigma[i]. scale *= -1;
			goto fin;
		}
		if (c == 0x12) {
			/* 단항 연산자 ~ */
			/* ~a == - a - 1 */
			value->min = - (int) (value->min + value->delta) - 1;
			goto minus2;
		}
		calc_value0(&tmp, &expr);
		value->flags |= tmp.flags;
		if (c == 0x13) {
			/* 2항연산자 + */
add2:
			for (i = 0; i < 2; i++) {
				if (value->scale[i] == 0)
					break;
			}
			for (j = 0; j < 2; j++) {
				if (tmp.scale[j] == 0)
					break;
				for (k = 0; k < i; k++) {
					if (value->label[k] == tmp.label[j]) {
						/* 동류항 검출 */
						if ((value->scale[k] += tmp.scale[j]) == 0) {
							/* 항의 소멸 */
							i--;
							value->label[k] = 0xffffffff;
							if (k)
								goto next_product;
							value->scale[0] = value->scale[1];
							value->label[0] = value->label[1];
							value->scale[1] = 0;
							value->label[1] = 0xffffffff;
						}
						goto next_product;
					}
				}
				/* 동류항이 없었기 때문에 항의 추가 */
				if (i >= 2) {
					value->flags |= VFLG_ERROR;
					goto fin;
				}
				value->scale[i] = tmp.scale[j];
				value->label[i] = tmp.label[j];
				i++;
next_product:
				;
			}
			for (j = 0; j < MAXSIGMAS; j++) {
				if (tmp.sigma[j]. scale == 0)
					break;
				addsigma(value, tmp.sigma[j]);
			}
			value->min += tmp.min;
			value->delta += tmp.delta;
			goto fin;
		}
		if (c == 0x14) {
			/* 2항연산자 - */
			tmp.min = - (int) (tmp.min + tmp.delta);
			tmp.scale[0] *= -1;
 			tmp.scale[1] *= -1;
			for (i = 0; i < MAXSIGMAS; i++)
				tmp.sigma[i]. scale *= -1;
			goto add2;
		}
		if (c == 0x15) { /* "*" */
			if (value->scale[0])
				goto no_exchange;
			if (tmp.scale[0])
				goto exchange;
		}
		if (c == 0x15 || 0x1d <= c && c <= 0x1f) { /* "*", "&", "|", "^" */
			if (value->delta)
				goto no_exchange;
			if (tmp.delta)
				goto exchange;
			if (value->sigma[0]. scale)
				goto no_exchange;
			if (tmp.sigma[0]. scale) {
		exchange:
				/* 항의 교환 */
				/* 부정치를 1항목 가져오기 위해 교환 */
				tmp.flags = value->flags;
				tmp2 = *value;
				*value = tmp;
				tmp = tmp2;
			}
		}
	no_exchange:
		calcsigma(&tmp);
		if (tmp.delta) {
			/* 부정치를 사용하기 위해 값을 아직 결정할 수 없다 */
			value->flags |= VFLG_ERROR;
			goto fin;
		}
		if (c == 0x15) {
			/* 2항연산자 * (signed) */
			if (tmp.scale[0]) {
	error:
				/* 양쪽 모두 label를 포함하고 있으면 에러 */
				value->flags |= VFLG_ERROR;
				goto fin;
			}
multiply2:
			if (tmp.min == 0) {
				init_value(value);
				goto fin;
			}
			value->scale[0] *= tmp.min;
			value->scale[1] *= tmp.min;
			for (i = 0; i < MAXSIGMAS; i++)
				value->sigma[i]. scale *= tmp.min;
			i = (signed int) value->min * (signed int) tmp.min;
			j = (signed int) (value->min + value->delta) * (signed int) tmp.min;
minmax:
			value->min = i;
			value->delta = j - i;
			if (i > j) {
				value->min = j;
				value->delta = i - j;
			}
			goto fin;
		}
		/* 이 이후는 tmp측의 label은 캔슬된다 */
		if (c == 0x16) {
			/* 2항연산자 << */
			tmp.min = 1 << tmp.min;
			goto multiply2;
		}
		if (c == 0x17) {
			/* 2항연산자 / (unsigned) */
divu2:
			if (tmp.min == 0)
				goto error;
divu_ij:
			i = (unsigned int) value->min / (unsigned int) tmp.min;
			j = (unsigned int) (value->min + value->delta) / (unsigned int) tmp.min;
			for (k = 0; k < MAXSIGMAS; k++) {
				if (value->sigma[k]. scale % tmp.min) {
					calcsigma(value);
					goto divu_ij;
				}
			}
divall:
			if (value->scale[0] % tmp.min)
				goto error;
			if (value->scale[1] % tmp.min)
				goto error;
			value->scale[0] /= tmp.min;
			value->scale[1] /= tmp.min;
			for (k = 0; k < MAXSIGMAS; k++)
				value->sigma[k]. scale /= tmp.min;
			goto minmax;
		}
		if (c == 0x19) {
			/* 2항연산자 / (signed) */
divs2:
			if (tmp.min == 0)
				goto error;
divs_ij:
			i = (signed int) value->min / (signed int) tmp.min;
			j = (signed int) (value->min + value->delta) / (signed int) tmp.min;
			for (k = 0; k < MAXSIGMAS; k++) {
				if (value->sigma[k]. scale % tmp.min) {
					calcsigma(value);
					goto divs_ij;
				}
			}
			goto divall;
		}
		if (c == 0x1b) {
			/* 2항연산자 >> (unsigned) */
			tmp.min = 1 << tmp.min;
			goto divu2;
		}
		if (c == 0x1c) {
			/* 2항연산자 >> (signed) */
			tmp.min = 1 << tmp.min;
			goto divs2;
		}
		value->scale[0] = value->scale[1] = 0; /* 자동 캔슬 */
		calcsigma(value);
		if (c == 0x18) {
			/* 2항연산자 % (unsigned) */
			if (tmp.min == 0)
				goto error;
			value->min = (unsigned int) value->min % (unsigned int) tmp.min;
			if (value->delta) {
				value->min = 0;
				value->delta = tmp.min - 1;
			}
			goto fin;
		}
		if (c == 0x1a) {
			/* 2항연산자 % (signed) */
			if (tmp.min == 0)
				goto error;
			value->min = (signed int) value->min % (signed int) tmp.min;
			if (value->delta) {
				value->delta = tmp.min - 1;
				if (value->min >= 0) {
					value->min = 0;
				} else if ((signed int) (value->min + value->delta) <= 0) {
					value->min = 1 - tmp.min;
				} else {
					value->min = 1 - tmp.min;
					value->delta *= 2;
				}
			}
			goto fin;
		}
		if (c == 0x1d) {
			/* 2항연산자 & (unsigned) */
			if (tmp.min == -1)
				goto fin; /* 그냥 지나침 */
			if (value->delta == 0) {
				value->min &= tmp.min;
				goto fin;
			}
			if (tmp.min & 0x80000000)
				goto error; /* 손을 댈 수 없다 */
			/* 사실은 좀 더 세세하게 할 수 있지만, 귀찮아서 하지않고 있다 */
			value->min = 0;
			value->delta = tmp.min;
			goto fin;
		}
		if (c == 0x1e) {
			/* 2항연산자 | (unsigned) */
			if (tmp.min == 0)
				goto fin; /* 그냥 지나침 */
			if (value->delta == 0) {
				value->min |= tmp.min;
				goto fin;
			}
			if (tmp.min & 0x80000000)
				goto error; /* 손을 댈 수 없다 */
			value->delta += tmp.min;
			if ((value->delta & 0x80000000) == 0)
				goto fin;
			value->delta = 0;
			goto error;
		}
	//	if (c == 0x1f) {
			/* 2항연산자 ^ (unsigned) */
			if (tmp.min == 0)
				goto fin; /* 그냥 지나침 */
			if (value->delta == 0) {
				value->min ^= tmp.min;
				goto fin;
			}
			if (tmp.min & 0x80000000)
				goto error; /* 손을 댈 수 없다 */
			if ((signed int) value->min > (signed int) 0 && value->min >= tmp.min) {
				value->min -= tmp.min;
				value->delta += tmp.min;
				goto fin;
			}
			goto error; /* 손을 댈 수 없다 */
	//	}
	}
	#if (DEBUG)
		if (c < 0x80)
			goto dberr;
	#endif
#if 0
	if (c == 0x80) {
		/* subsect size sum */
		/* sum는, delta가 2 GB라면 에러로 해 버린다(귀찮아서) */
		calc_value(&tmp, &expr);
		i = tmp.min;
		calc_value(&tmp, &expr);
		subsect = subsect0 + tmp.min;
		do {
			value->min += subsect->min;
			value->delta += subsect->delta;
			if (subsect->delta >= INVALID_DELTA)
				value->flags |= VFLG_ERROR;
		} while (--i);
		goto fin;
	}
#endif
	#if (DEBUG)
dberr:
		fprintf(stderr, "calc_value:%02x\n", c);
		GOL_sysabort(GO_TERM_BUGTRAP);
	#endif

fin:
	*pexpr = expr;
	return;
}

static UCHAR *skip_expr(UCHAR *expr)
{
	return LL_skip_expr(expr);
}





/* 라벨의 정의 방법:
	 일반식
	80 variable-sum, 0000bbaa(aa:항수-1, bb:최초의 번호), 
	84~87 sum, i - 1, expr, expr, ...

  ·80~8 f:LL가 내부 처리용으로 사용한다
	80~83:variable 참조(1~4바이트)
	88~8f:sum(variable), (1~4, 1~4) : 처음은 항수-1, 다음은 최초의 번호
		{ "|>", 12, 18 }, { "&>", 12, 17 },
		{ "<<", 12, 16 }, { ">>", 12, 17 },
		{ "//", 14,  9 }, { "%%", 14, 10 },
		{ "+",  13,  4 }, { "-",  13,  5 },
		{ "*",  14,  6 }, { "/",  14,  7 },
		{ "%",  14,  8 }, { "^",   7, 14 },
		{ "&",   8, 12 }, { "|",   6, 13 },
		{ "",    0,  0 }

	s+
	s-
	s~

	+, -, *, <<, /u, %u, /s, %s
	>>u, >>s, &, |, ^

*/






#if 0

	< 문법 >

처음은 헤더.
·헤더 사이즈(DW) = 12
·버전 코드(DW)
·라벨 총수(DW)


  ·38:식의 값을 DB로 설치
  ·39:식의 값을 DW로 설치
  ·3a:식의 값을 3바이트로 설치
  ·3b:식의 값을 DD로 설치
  이하, ·3f까지 있다.
  ·40~47:블록 커맨드. if문등의 후속문을 블록화한다(2~9).
  ·48:바이트 블록(블록 길이가 바이트로 후속).
  ·49:워드 블록.
  ·4a:바이트 블록.
  ·4b:더블 워드 블록.
  ·4c:배타적 if개시.
  ·4d:선택적 if개시.
  ·4e:선택적 바운더리 if개시. 변수 설정의 직후, 바운더리 값이 계속된다.
  ·4f:endif.
  배타적 if는, endif가 올 때까지 얼마든지 늘어놓을 수 있다. endif가 올 때까지,
  모두 else-if로 다루어진다. 마지막에 else를 만들고 싶다면 조건을 정수 1이든.
  ·터미네이터는 라벨 정의로 0xffffffff.

  ·58:ORG

  ·60:아라인. 바이트의 묻는 방법은 개별적으로 설정한다. ···이것은 배타적 if에서도 기술할 수 있다.

  ·70~77:가변장 바이트 선언(문법상에서는 40~4b가 후속 하는 것을 허락하지만, 서포트하고 있지 않다. 허락되는 것은 30~3f)
  ·78~7f:가변장 바이트 참조

  ·80~8f:LL가 내부 처리용으로 사용한다
	80~83:variable 참조(1~4바이트)
	88~8f:sum(variable), (1~4, 1~4) : 처음은 항수-1, 다음은 최초의 번호


  if문중에서는 가변장 바이트 선언 밖에 할 수 없다.

	ORG에 대해. 본래 인수는 식이어도 좋지만, 이 버전에서는 정수식을 가정하고 있다.




#endif

/* ibuf, obuf의 기초 구조 */
/* 시그네처 8바이트, 총장 4바이트, 리저브 4바이트 */
/* 메인 데이터 길이 4B, 메인 데이터 스타트 4B */

/* ↑이런 까다로운 것은 하지 않는다 */
/* 스킵 커맨드를 만들어 대처한다 */

static UCHAR table98typlen[] = { 0x38, 0x38, 0x39, 0x39, 0x3b, 0x3b, 0x38 };
static UCHAR table98range [] = { 0x00, 0x02, 0x00, 0x03, 0x00, 0x03, 0x03 };

struct STR_LL_VB {
	UCHAR *expr, typlen, range;
};

UCHAR *LL_define_VB(struct STR_LL_VB *virtualbyte, UCHAR *s)
{
	UCHAR c;
	virtualbyte->typlen = c = *s++;
	#if (DEBUG)
		if (c < 0x30)
			goto dberr0;
	#endif
	if (c < 0x38) {
		virtualbyte->expr = s;
		s += c & 0x07;
		goto fin;
	}
	if (c < 0x3c)
		virtualbyte->range = *s++;
	else {
		#if (DEBUG)
			if (c < 0x98)
				goto dberr0;
			if (c >= 0x9f)
				goto dberr0;
		#endif
		virtualbyte->typlen = table98typlen[c & 0x07];
		virtualbyte->range  = table98range [c & 0x07];
	}
	virtualbyte->expr = s;
	s = skip_expr(s);
fin:
	return s;	

	#if (DEBUG)
dberr0:
		fprintf(stderr, "LL_define_VB:%02x\n", c);
		GOL_sysabort(GO_TERM_BUGTRAP);
		return NULL;
	#endif
}

UCHAR *lccbug_LL_mc90_func(UCHAR *src0, struct STR_LL_VB *vbb, struct STR_LL_VB *vbc, char len, char c)
{
	do {
		if (c ! = 0) {
			src0 = skip_mc30(src0, NULL, 1);
			if (vbc)
				src0 = skip_mc30(src0, NULL, 1);
		} else {
			src0 = LL_define_VB(vbb, src0);
			if (vbc)
				src0 = LL_define_VB(vbc, src0);
		}
		c--;
	} while (--len);
	return src0;
}

UCHAR *LL(UCHAR *src0, UCHAR *src1, UCHAR *dest0, UCHAR *dest1)
/* 또한 src에 write를 하므로 요주의 */
/* 새로운 dest1를 돌려준다 */
{
	struct STR_SUBSECTION *subsect1, *subsect;
	unsigned int l0, min, max, unsolved, unsolved0;
	struct STR_LABEL *label;
	struct STR_VALUE *value, *labelvalue;
	struct STR_LL_VB virtualbyte[8], *vba, *vbb, *vbc;
	signed int times_count;
	UCHAR *s, *times_src0, *times_dest0, c, len, range;

	label0 = malloc(nask_maxlabels * sizeof (struct STR_LABEL));
	subsect0 = malloc(nask_maxlabels /* == MAXSUBSETCS */ * sizeof (struct STR_SUBSECTION));
	value = malloc(sizeof (struct STR_VALUE));
	labelvalue = malloc(sizeof (struct STR_VALUE));

	/* 라벨식을 등록한다 */
//	l0 = 0; /* 현재의 주소는 l0부터 lc개 */
//	lc = 0;
//	min = max = 0; /* 다음에 생성하는 것은, variable0[l0 + lc] */

	label = label0;
	for (unsolved = nask_maxlabels; unsolved > 0; label++, unsolved--) {
		label->value.flags = 0; /* enable를 지우기 (위해)때문에 */
		label->define = NULL;
	}

	/* 분리한다 */
	/* 라벨을 검출한다 */
	subsect = subsect0;
	subsect->sect0 = src0;
	init_value(labelvalue); /* 라벨 보관 유지 */
	while (src0 < src1) {
		c = *src0;
		if (c == 0x2d) {
			/* EQU */
			src0++;
			calc_value(value, &src0);
			label = &label0[value->min];
			label->define = src0;
			src0 = skip_expr(src0);
			continue;
		}
		if (c == 0x0e) {
			/* 라벨 정의 */
			struct STR_SIGMA sigma;
			sigma.scale = 1;
			sigma.subsect = subsect - subsect0;
			sigma.terms = 1;
			subsect->sect1 = src0;
			subsect++;
			subsect->sect0 = src0;
			src0++;
			calc_value(value, &src0);
			label = &label0[value->min];
			label->define = src0;
			addsigma(labelvalue, sigma);
			label->value = *labelvalue;
			label->value.flags |= VFLG_ENABLE;
			continue;
		}
		if (c == 0x2c) {
			/* extern 라벨 선언 */
			src0++;
			calc_value(value, &src0);
			label = &label0[value->min];
			init_value(&label->value);
		//	label->define = NULL;
			label->value.flags |= VFLG_EXTERN | VFLG_ENABLE;
			label->value.label[0] = value->min;
			label->value.scale[0] = 1;
			continue;
		}
		if (c == 0x58) {
			/* ORG */
			subsect->sect1 = src0;
			subsect++;
			subsect->sect0 = src0;
			src0++;
			calc_value(labelvalue, &src0);
			continue;
		}
		src0 = LL_skipcode(src0);
	}
	subsect->sect1 = src1;
	subsect1 = subsect + 1;

	for (subsect = subsect0; subsect < subsect1; subsect++) {
		subsect->min = 0;
		subsect->delta = INVALID_DELTA;
		subsect->unsolved = 1;
	}
	len = 0;
	c = 0;
	unsolved0 = 0x7fffffff;
	do {
		for (subsect = subsect0; subsect < subsect1; subsect++) {
			if (subsect->unsolved)
				solve_subsection(subsect, len);
		}
		unsolved = 0;
		for (subsect = subsect1 - 1; subsect >= subsect0; subsect--) {
			if (subsect->unsolved)
				unsolved += solve_subsection(subsect, len);
		}
		c++;
		if (unsolved0 > unsolved) {
			c = 0;
			unsolved0 = unsolved;
		}
		if (c >= 3) {
			len++;
			c = 0;
		}
	} while (unsolved);

	for (subsect = subsect0; subsect < subsect1; subsect++) {
		times_count = 0;
		for (src0 = subsect->sect0; src0 < subsect->sect1; ) {
			c = *src0++;
			if (dest0 + 64 >= dest1) {
	error:
				return NULL; /* buffer overrun */
			}
			if (c == 0x2d) {
				/* EQU */
				calc_value(value, &src0);
				label = &label0[value->min];
				if ((label->value.flags & VFLG_ENABLE) == 0)
					enable_label(label);
				*value = label->value;
				calcsigma(value);
				c = 0x0c;
				if (value->scale[0])
					c++;
				*dest0++ = c;
				len = 4;
				src0 = skip_expr(src0);
				goto putvalue0;
			}
			if (c == 0x0e) {
				src0 = skip_expr(src0);
				continue;
			}
			if (c == 0x0f) {
				dest0[0] = 0xf6; /* REM_8B */
				dest0[1] = *src0++;
				c = *src0++;
				label = &label0[get_id(c - 8, &src0, 0)];
				if ((label->value.flags & VFLG_ENABLE) == 0)
					enable_label(label);
				*value = label->value;
				calcsigma(value);
				dest0[2] = 0; /* base extern label (none) */
				dest0[3] = 0;
				dest0[4] = value->min & 0xff;
				dest0[5] = (value->min >>  8) & 0xff;
				dest0[6] = (value->min >> 16) & 0xff;
				dest0[7] = (value->min >> 24) & 0xff;
				dest0 += 8;
				if (value->scale[0]) {
					dest0[2 - 8] = value->label[0] & 0xff;
					dest0[3 - 8] = (value->label[0] >> 8) & 0xff;
				//	if (value->scale[0] ! = 1 || value->scale[1] ! = 0) {
				//		/* 에러를 출력 */
				//	}
				}
				continue;
			}
			if (c == 0x2c) {
				dest0[0] = 0x2c;
				dest0[1] = src0[0];
				dest0[2] = src0[1];
				dest0[3] = src0[2];
				src0 += 3;
				dest0 += 4;
				continue;
			}
			#if (DEBUG)
				if (c < 0x30)
					goto dberr0;
			#endif
			if (c < 0x37) {
				/* 30~37:쇼트 DB(정수, 0~7바이트) */
				len = c & 0x07;
	copy:
				*dest0++ = c;
				if (len) {
					times_count -= len;
					do {
						*dest0++ = *src0++;
					} while (--len);
	check_times_count:
					if (times_count == 0) {
	times_expand:
						s = skip_expr(times_src0 + 4); /* len */
						calc_value(value, &s); /* 반복 회수 */
						c = 0;
						value->min = times_src0[0] | times_src0[1] << 8
							| times_src0[2] << 16 | times_src0[3] << 24;
						/* ↑force등으로 어긋나는 점이 생기지 않도록 하기 위해 */
						if (value->delta) {
							/* 부정치 에러 */
							c = 1;
							*dest0++ = 0xe9;
						}
						if (value->flags & VFLG_UNDEF) {
							/* 미정의 에러 */
							c = 1;
							*dest0++ = 0xec;
						}
						if (value->flags & VFLG_SLFREF) {
							/* 자기 참조 에러 */
							c = 1;
							*dest0++ = 0xea;
						}
						if (value->flags & VFLG_ERROR) {
							/* 라벨치 일반 에러 */
							c = 1;
							*dest0++ = 0xeb;
						}
						if (c ! = 0 || (unsigned int) value->min >= 0xfffffff0) {
							*dest0++ = 0xe6; /* TIMES error */
							dest0 = times_dest0; /* 반복 회수 0으로 간주한다 */
							continue;
						}
						value->delta = dest0 - times_dest0;
						if (dest0 + value->delta * value->min + 64 >= dest1)
							goto error; /* 1회분 불필요하게 비교하고 있다 */
						for (dest0 = times_src0 = times_dest0; value->min > 0; value->min--) {
							s = times_src0;
							for (times_count = value->delta; times_count > 0; times_count--)
								*dest0++ = *s++;
						}
					}
	times_count0:
					times_count = 0;
				}
				continue;
			}
			if (c < 0x3c) {
				/* 식의 값을 둔다 */
				len = c - 0x37;
				range = *src0++;
				calc_value(value, &src0);
	putvalue:
				if (value->scale[0] && len == 4 && value->delta == 0
					&& (value->flags & (VFLG_UNDEF | VFLG_SLFREF | VFLG_ERROR)) == 0) {
					/* 요점 이전(1인가―1) */
					if (value->delta == 0 && value->flags)
					if (value->scale[0] == -1) {
						value->label[0] = value->label[1];
						value->scale[1] = -1;
					}
					/* 여기서 상정해야 할 케이스 */
					/* scale[0] == 1, scale[1] ==  0 : 절대 보정 */
					/* scale[0] == 1, scale[1] == -1 : 상대 보정 */
					/* 0x2e : 절대 보정 요구, 0x2f : 상대 보정 요구 */
					dest0[0] = 0x2e - value->scale[1];
					dest0[1] = 0x09;
					dest0[2] = value->label[0] & 0xff;
					dest0[3] = (value->label[0] >> 8) & 0xff;
					dest0 += 4;
				}
				*dest0++ = 0x30 | len; /* 31~34 */
				/* 레인지 체크등을 한다 */
				/* scale 체크도 해, 필요하면 링크에 필요한 정보를 붙인다 */
	putvalue0:
				/* len 바이트를 둔다 */
				dest0[0] = value->min         & 0xff;
				dest0[1] = (value->min >>  8) & 0xff;
				dest0[2] = (value->min >> 16) & 0xff;
				dest0[3] = (value->min >> 24) & 0xff;
				dest0 += len;
				if (value->delta) {
					/* 부정치 에러 */
					*dest0++ = 0xe9;
				}
				if (value->flags & VFLG_UNDEF) {
					/* 미정의 에러 */
					*dest0++ = 0xec;
				}
				if (value->flags & VFLG_SLFREF) {
					/* 자기 참조 에러 */
					*dest0++ = 0xea;
				}
				if (value->flags & VFLG_ERROR) {
					/* 라벨치 일반 에러 */
					*dest0++ = 0xeb;
				}
				if ((times_count -= len) == 0)
					goto times_expand;
				goto times_count0;
			//	times_count = 0;
			//	continue;
			}
			if (c == 0x58) {
				/* ORG */
				calc_value(value, &src0);
				*dest0++ = 0x5a;
				len = 4;
				goto putvalue0;
			}
			if (c == 0x59) {
				/* TIMES */
				times_dest0 = dest0;
				times_src0 = src0;
				src0 += 4;
				calc_value(value, &src0); /* len */
				/* len의 부분에 에러는 없다고 하는 것이 되고 있다(부실) */
				times_count = value->min;
				src0 = skip_expr(src0);
				continue;
			}
			if (c == 0x68) {
				/* endian */
				src0++;
				continue;
			}
			#if (DEBUG)
				if (c < 0x70)
					goto dberr0;
			#endif
			if (c < 0x78) {
				/* 0 x70~0x77:virtual byte 정의 */
				src0 = LL_define_VB(&virtualbyte[c & 0x07], src0);
				continue;
			}
			if (c < 0x80) {
				/* 0 x78~0x7f:virtual byte 전개 */
				vba = &virtualbyte[c & 0x07];
				s = vba->expr;
				if (vba->typlen < 0x38) {
					len = vba->typlen & 0x07;
					*dest0++ = vba->typlen;
					if (len) {
						times_count -= len;
						do {
							*dest0++ = *s++;
						} while (--len);
						goto check_times_count;
					}
					continue;
				}
				len = vba->typlen - 0x37;
				range = vba->range;
				calc_value(value, &s);
				goto putvalue;
			}
			#if (DEBUG)
				if (c < 0x90)
					goto dberr0;
			#endif
			if (c <= 0x91) {
				/* microcode90 */
				/* microcode91 */
				len = src0[2]; /* z-bbb-d-aaa */
				vba = &virtualbyte[len & 0x07];
				vbb = &virtualbyte[(len >> 4) & 0x07];
				vbc = vbb + 1;
				if (c == 0x90)
					vbc = NULL;
				vba->typlen = 0x37 + (src0[1] & 0x0f);
				vba->range = 0x03; /* non-overflow */
				if (vba->typlen == 1)
					vba->range = 0x02; /* normal-signed */
				c = src0[0] & 0x0f;
				len = ((len /* z-bbb-d-aaa */ >> 7) & 0x01) + 2;
				vba->expr = src0 + 3;
				src0 = skip_expr(vba->expr);

				src0 = lccbug_LL_mc90_func(src0, vbb, vbc, len, c);
#if 0
				do {
					if (c ! = 0) {
						src0 = skip_mc30(src0, NULL, 1);
						if (vbc)
							src0 = skip_mc30(src0, NULL, 1);
					} else {
						src0 = LL_define_VB(vbb, src0);
						if (vbc)
							src0 = LL_define_VB(vbc, src0);
					}
					c--;
				} while (--len);
#endif
				continue;
			}
			#if (DEBUG)
				if (c < 0x94)
					goto dberr0;
			#endif
			if (c <= 0x95) {
				/* microcode94 */
				/* microcode95 */
				len = src0[2]; /* z-bbb-d-aaa */
				vba = &virtualbyte[len & 0x07];
				vbb = &virtualbyte[(len >> 4) & 0x07];
				vbc = vbb + 1;
				if (c == 0x94)
					vbc = NULL;
				vba->expr = src0 + 3;
				c = src0[0] & 0x0f;
				src0 = skip_expr(vba->expr);
				len = *src0++;
				goto outmc94_skip;
				do {
					c--;
					src0 = skip_expr(src0);
			outmc94_skip:
					if (c == 0) {
						range = *src0++;
						if (range < 0x3c) {
							vba->typlen = range;
							vba->range = *src0++;
						} else {
							vba->typlen = table98typlen[range & 0x07];
							vba->range = table98range[range & 0x07];
						}
						src0 = LL_define_VB(vbb, src0);
					} else {
						src0 = skip_mc30(src0, NULL, 0);
						src0 = skip_mc30(src0, NULL, 1);
					}
					if (vbc) {
						if (c == 0)
							src0 = LL_define_VB(vbc, src0);
						else
							src0 = skip_mc30(src0, NULL, 1);
					}
				} while (--len);
				continue;
			}
			#if (DEBUG)
				if (c < 0x98)
					goto dberr0;
			#endif
			if (c < 0x9f) {
				len = table98typlen[c & 0x07] & 0x07;
				range = table98range[c & 0x07];
				calc_value(value, &src0);
				goto putvalue;
			}
			#if (DEBUG)
				if (c < 0xe0)
					goto dberr0;
			#endif
			if (c < 0xf0) {
				/* 1바이트 리마크 */
				*dest0++ = c;
				continue;
			}
			if (c < 0xf8) {
				/* n바이트 리마크 */
				len = c - 0xef;
				goto copy;
			}
			#if (DEBUG)
	dberr0:
				fprintf(stderr, "LL:%02x\n", c);
				GOL_sysabort(GO_TERM_BUGTRAP);
			#endif
		}
	}

	free(label0);
	free(subsect0);
	free(value);
	free(labelvalue);

	return dest0;
}


UCHAR *skip_mc30(UCHAR *s, UCHAR *bytes, char flag)
{
	return LL_skip_mc30(s, bytes, flag);
}

unsigned int solve_subsection(struct STR_SUBSECTION *subsect, char force)
/* unsolved수를 돌려준다 */
/* subsect->unsolved == 0 이라면 call하지 말 것 */
{
	struct {
		signed char min, max;
	} vb[8], *vba, *vbb, *vbc;
	unsigned int unsolved = 0, min = 0, delta = 0;
	int i;
	struct STR_VALUE value, tmp;
	UCHAR *s, *t, c, d, e, f, g, h;

	for (s = subsect->sect0; s < subsect->sect1; ) {
		c = *s++;
		if (c == 0x2d) {
			/* EQU 라벨 정의 */
			s = skip_expr(s);
			s = skip_expr(s);
			continue;
		}
		if (c == 0x0e) {
			/* 비EQU 라벨의 정의 */
			/* 지금은 부실이므로 서로 빼앗지 못하고 무시 */
			s = skip_expr(s);
			continue;
			/* 본래는 이것은 오지 않는다(? ) */
		}
		if (c == 0x0f) {
			s += s[1] - (0x08 - 3); /* GLOBAL용 변환 */
			continue;
		}
		if (c == 0x2c) {
			s = skip_expr(s);
			continue;
		}
		#if (DEBUG)
			if (c < 0x30)
				goto dberr0;
		#endif
		if (c < 0x3c) {
skipmc30:
			/* 30~37:쇼트 DB(정수, 0~7바이트) */
			/* 식을 DB~DD로 둔다 */
			s = skip_mc30(s - 1, &c, 1);
			min += c;
			continue;
		}
		if (c == 0x58) {
			/* ORG */
			s = skip_expr(s);
			continue;
		}
		if (c == 0x59) {
			/* TIMES */
			t = s;
			i = s[0] | s[1] << 8 | s[2] << 16 | s[3] << 24;
			s += 4;
			calc_value(&value, &s); /* len */
			/* len에 에러는 없다는 전제(프로그램 카운터를 포함하지 않는 정수식) */
			if ((unsigned int) i < 0xfffffff0) {
				min += (i - 1) * value.min; /* i는 반복 회수 */
				s = skip_expr(s);
				continue;
			}
			i = value.min;
			calc_value(&value, &s); /* 반복 회수 */
			if (value.flags & (VFLG_SLFREF | VFLG_UNDEF)) /* 자기 참조 에러 */
				goto mc59_force;
			if (value.flags & VFLG_ERROR) {
				/* 라벨치 일반 에러 */
				if (force >= 2)
					goto mc59_force;
				delta = INVALID_DELTA;
				unsolved++;
				continue;
			}
			min += i * (value.min - 1);
			if (value.delta == 0) {
				t[0] = value.min         & 0xff;
				t[1] = (value.min >>  8) & 0xff;
				t[2] = (value.min >> 16) & 0xff;
				t[3] = (value.min >> 24) & 0xff;
				continue;
			}
			if (force < 2) {
				delta += i * value.delta;
				unsolved++;
				continue;
			}
	mc59_force:
			t[0] = 0xfe; /* force */
		//	t[1] = 0xff;
		//	t[2] = 0xff;
		//	t[3] = 0xff;
			continue;
		}
		if (c == 0x68) {
			/* endian */
			s++;
			continue;
		}
		#if (DEBUG)
			if (c < 0x70)
				goto dberr0;
		#endif
		if (c < 0x78) {
			/* 0 x70~0x77:virtual byte 정의 */
			vba = &vb[c & 0x07];
			s = skip_mc30(s, &vba->min, 1);
			vba->max = vba->min;
			continue;
		}
		if (c < 0x80) {
			/* 0 x78~0x7f:virtual byte 전개 */
			vba = &vb[c & 0x07];
			min += vba->min;
			delta += vba->max - vba->min;
			continue;
		}
		#if (DEBUG)
			if (c < 0x90)
				goto dberr0;
		#endif
		if (c <= 0x91) {
			/* microcode90, microcode91 */
			/* [90 decision z-bbb-d-aaa expr] [3x (d, w)] [3x (b)] ([3x (z)]) */
			/* [91 decision z-bbb-d-aaa expr] [3x 3x (d, w)] [3x 3x (b)] ([3x 3x (z)]) */
			/* 조건식은 vba에 */
			/* decision는 2바이트. 선택 번호와 bbbb-aaaa */
			d = c;
			c = s[2]; /* z-bbb-d-aaa */
			vba = &vb[c        & 0x07];
			vbb = &vb[(c >> 4) & 0x07];
			vbc = vbb + 1;
			if (d == 0x90)
				vbc = NULL;
			if (((e = s[0]) & 0x0f) ! = 0x0f) {
				/* 이미 해결 지음 */
				d = s[1];
				s = skip_expr(s + 3);
				vba->min = vba->max = d        & 0x0f;
				vbb->min = vbb->max = (d >> 4) & 0x0f;
			//	if (vbc)
			//		vbc->min = vbc->max = (e >> 4) & 0x0f;
				d = 2;
				if (c & 0x80)
					d = 3;
				if (vbc) {
					vbc->min = vbc->max = (e >> 4) & 0x0f;
					d *= 2;
				}
				do {
					s = skip_mc30(s, NULL, 1);
				} while (--d);
				continue;
			}
			t = s;
			s += 3;
			calc_value(&value, &s);
			d = 1 | 2 | 4; /* unknown */
			if ((value.flags & VFLG_ERROR) == 0) {
				i = value.min + value.delta; /* max */
				if (value.scale[0])
					d = 1; /* word(dword) */
				else if (value.min == 0 && i == 0)
					d = 4; /* zero */
				else if (-128 <= value.min && i <= 127) {
					d = 2; /* byte */
					if (value.min <= 0 && 0 <= i)
						d = 2 | 4; /* byte | zero */
				} else {
					d = 1; /* word(dword) */
					if (value.min <= 127 && -128 <= i) {
						d = 1 | 2; /* word | byte */
						if (value.min <= 0 && 0 <= i)
							d = 1 | 2 | 4; /* word | byte | zero */
					}
				}
			}
			if (force) {
				if (d & 1)
					d = 1;
				else
					d = 2;
			}
			if ((c & 0x80) == 0) { /* zero 없음 */
				if ((d &= (1 | 2)) == 0)
					d = 2; /* byte */
			}
			vba->min = vbb->min =  127;
			vba->max = vbb->max = -128;
			if (vbc) {
				vbc->min =  127;
				vbc->max =  -128;
			}
			for (e = 1; e <= 4; e <<= 1) {
				if (e == 4 && (c & 0x80) == 0)
					break;
				s = skip_mc30(s, &f, 1);
				if (vbc)
					s = skip_mc30(s, &g, 1);
				if (d & e) {
					if (vbb->min > f)
						vbb->min = f;
					if (vbb->max < f)
						vbb->max = f;
					if (vbc) {
						if (vbc->min > g)
							vbc->min = g;
						if (vbc->max < g)
							vbc->max = g;
					}
					f = 2;
					if (c & 0x08)
						f = 4;
					if (e == 2)
						f = 1;
					if (e == 4)
						f = 0;
					if (vba->min > f)
						vba->min = f;
					if (vba->max < f)
						vba->max = f;
				}
			}
			if ((d & (d - 1)) == 0) {
				c = 0;
				if (d == 2)
					c = 1;
				if (d == 4)
					c = 2;
				t[0] = c;
				if (vbc)
					t[0] |= vbc->min << 4;
				t[1] = vba->min | vbb->min << 4;
			} else
				unsolved++;
			continue;
		}
		#if (DEBUG)
			if (c < 0x94)
				goto dberr0;
		#endif
		if (c <= 0x95) {
			/* microcode94, microcode95 */
			/* [94 decision 0-bbb-0-aaa expr] [len] [def-a def-b] ([expr def-a def-b] ...) */
			/* [95 decision 0-bbb-0-aaa expr] [len] [def-a def-b def-c] ([expr def-a def-b def-c] ...) */
			/* def-a 는 0 x98~0x9e (값은 조건식) */
			/* len는 최저 1(case 없음), 처음은 무조건 정의, 후속은 일치하면 정의, 일치가 여러번 있으면 최후가 유효 */
			d = c;
			c = s[2]; /* 0-bbb-0-aaa */
			vba = &vb[c        & 0x07];
			vbb = &vb[(c >> 4) & 0x07];
			vbc = vbb + 1;
			if (d == 0x94)
				vbc = NULL;
			if (((g = s[0]) & 0x0f) ! = 0x0f) {
				/* 이미 해결 지음 */
				d = s[1];
				s = skip_expr(s + 3);
				e = *s++; /* len */
				vba->min = vba->max = d        & 0x0f;
				vbb->min = vbb->max = (d >> 4) & 0x0f;
				if (vbc)
					vbc->min = vbc->max = (g >> 4) & 0x0f;
				s = skip_mc30(s, NULL, 0);
				s = skip_mc30(s, NULL, 1);
				if (vbc)
					s = skip_mc30(s, NULL, 1);
				while (--e) {
					s = skip_expr(s);
					s = skip_mc30(s, NULL, 0);
					s = skip_mc30(s, NULL, 1);
					if (vbc)
						s = skip_mc30(s, NULL, 1);
				}
				continue;
			}
			t = s;
			s += 3;
			calc_value(&value, &s);
			c = *s++; /* len */
			s = skip_mc30(s, &vba->min, 0);
			s = skip_mc30(s, &vbb->min, 1);
			vba->max = vba->min;
			vbb->max = vbb->min;
			if (vbc) {
				s = skip_mc30(s, &vbc->min, 1);
				vbc->max = vbc->min;
			}
			f = 0;
			if (force == 0) {
				if (value.delta ! = 0 || (value.flags & VFLG_ERROR) ! = 0)
					f = 0xff;
				for (g = 1; g < c; g++) {
					/* 어떻게 동일한 것을 지켜보는지?  */
					/* 귀찮아서 모두가 확정할 때까지 decide하지 않는다 */
					calc_value(&tmp, &s);
					if (tmp.delta ! = 0 || (tmp.flags & VFLG_ERROR) ! = 0)
						f = 0xff;
					s = skip_mc30(s, &d, 0);
					s = skip_mc30(s, &e, 1);
					if (vbc)
						s = skip_mc30(s, &h, 1);
					if (f ! = 0xff) {
						if ((value.min ^ tmp.min |
							value.scale[0] ^ tmp.scale[0] | value.label[0] ^ tmp.label[0] |
							value.scale[1] ^ tmp.scale[1] | value.label[1] ^ tmp.label[1]) == 0) {
							vba->min = vba->max = d;
							vbb->min = vbb->max = e;
							if (vbc)
								vbc->min = vbc->max = h;
							f = g;
						}
					} else {
						if (vba->min > d)
							vba->min = d;
						if (vba->max < d)
							vba->max = d;
						if (vbb->min > e)
							vbb->min = e;
						if (vbb->max < e)
							vbb->max = e;
						if (vbc) {
							if (vbb->min > h)
								vbb->min = h;
							if (vbb->max < h)
								vbb->max = h;
						}
					}
				}
			}
			if (f ! = 0xff) {
				t[0] = f;
				if (vbc)
					t[0] |= vbc->min << 4;
				t[1] = vba->min | vbb->min << 4;
			} else
				unsolved++;
			continue;
		}
		#if (DEBUG)
			if (c < 0x98)
				goto dberr0;
		#endif
		if (c < 0x9f)
			goto skipmc30;
		#if (DEBUG)
			if (c < 0xe0)
				goto dberr0;
		#endif
		if (c < 0xf0)
			continue; /* 1바이트 리마크 */
		if (c < 0xf8) {
			/* n바이트 리마크 */
			s += c - 0xef;
			continue;
		}
		#if (DEBUG)
dberr0:
			fprintf(stderr, "solve_subsect:%02X\n", c);
			GOL_sysabort(GO_TERM_BUGTRAP);
		#endif


/*
  [90(91도 동등)] [disp식] [ba] : a는 0~7, b도 0~7 : virtual byte 번호 : B=8은 미사용을 의미한다
  a의 bit3는, D비트. a에 대해서는, 식이 들어간다. 레인지 체크는 03 타입.
  [90] [disp식] [09] [31, 00-nnn-r/m] [31 01-nnn-r/m] [31 10-nnn-r/m]
  [bits 32]에서의 PUSH는 이러한:[91] [imm식] [3a] [31 6a] [31 68]
  94도 비슷한 형식으로 변경(a의 자동 정의는 없다).
  [94] [식] [ba] [case] [def0] [def1] [case] [def0] [def1] ...
  또 이하를 정의
  [98] = [38 00 08 00] UCHAR
  [99] = [38 02 08 00] SCHAR
  [9a] = [39 00 08 00] USHORT
  [9b] = [39 03 08 00] non-overflow SHORT
  [9c] = [3b 00 08 00] UINT
  [9d] = [3b 03 08 00] non-overflow INT
  [9e] = [38 03 08 00] non-overflow CHAR




  ·70~77:가변장 바이트 선언(문법상에서는 40~4b가 후속 하는 것을 허락하지만, 서포트하고 있지 않다. 허락되는 것은 30~3f)
  ·78~7f:가변장 바이트 참조

  ·80~8f:LL가 내부 처리용으로 사용한다
	80~83:variable 참조(1~4바이트)
	88~8f:sum(variable), (1~4, 1~4) : 처음은 항수-1, 다음은 최초의 번호

  ·90:0-B-else 바운더리 if
  ·91:B-else 바운더리 if
  ·92:0BWD 바운더리 if
  ·93:else
  ·선택적 정수 if
*/

	}
	if (delta > INVALID_DELTA)
		delta = INVALID_DELTA;
	subsect->min = min;
	subsect->delta = delta;
	subsect->unsolved = unsolved;

	return unsolved;
}

void calcsigma(struct STR_VALUE *value)
/* sigma를 모두 전개해 버린다 */
{
	int i, j, s, t, min;
	unsigned int delta;
	struct STR_SUBSECTION *subsect;
	for (i = 0; i < MAXSIGMAS; i++) {
		if ((s = value->sigma[i]. scale) == 0)
		//	continue;
			break;
		value->sigma[i]. scale = 0;
		if ((t = value->sigma[i]. terms) == 0)
			continue;
		value->sigma[i]. terms = 0;
		subsect = &subsect0[value->sigma[i]. subsect];
		value->sigma[i]. subsect = 0;
		min = delta = 0;
		do {
			min += subsect->min;
			delta += subsect->delta;
			if (delta > INVALID_DELTA)
				delta = INVALID_DELTA;
			subsect++;
		} while (--t);
		if (delta >= INVALID_DELTA) {
			value->delta = INVALID_DELTA;
		}
		if (value->delta >= INVALID_DELTA) {
			value->flags |= VFLG_ERROR;
			continue;
		}
		if (s > 0) {
			value->min += s * min;
			value->delta += s * delta;
		} else {
			value->min += s * (min + delta);
			value->delta -= s * delta;
		}
	}
	return;
}

void addsigma(struct STR_VALUE *value, struct STR_SIGMA sigma)
{
	int i, j;
retry:
	for (i = 0; i < MAXSIGMAS && value->sigma[i]. scale ! = 0; i++) {
		if (sigma.subsect + sigma.terms <= value->sigma[i]. subsect)
			break;
		if (value->sigma[i]. subsect + value->sigma[i]. terms <= sigma.subsect)
			continue;
	add:
		if (value->sigma[i]. subsect == sigma.subsect && value->sigma[i]. terms <= sigma.terms) {
	add2:
			sigma.terms -= value->sigma[i]. terms;
			sigma.subsect += value->sigma[i]. terms;
			if ((value->sigma[i]. scale += sigma.scale) == 0) {
	delete:
				for (j = i; j <= MAXSIGMAS - 2; j++)
					value->sigma[j] = value->sigma[j + 1];
				value->sigma[MAXSIGMAS - 1]. scale = 0;
				value->sigma[MAXSIGMAS - 1]. subsect = 0;
				value->sigma[MAXSIGMAS - 1]. terms = 0;
			}
	check:
			do {
				if (value->sigma[i]. scale ! = 0) {
					if (i > 0) {
						if (value->sigma[i - 1]. scale == value->sigma[i]. scale) {
							if (value->sigma[i - 1]. subsect + value->sigma[i - 1]. terms == value->sigma[i]. subsect) {
								value->sigma[i - 1]. terms += value->sigma[i]. terms;
								goto delete;
							}
						}
					}
				}
			} while (++i < MAXSIGMAS);
			if (sigma.terms > 0)
				goto retry;
			return;
		}
		if (value->sigma[MAXSIGMAS - 1]. scale ! = 0) {
			calcsigma(value);
			goto retry;
		}
		for (j = MAXSIGMAS - 2; i <= j; j--)
			value->sigma[j + 1] = value->sigma[j];
		if (value->sigma[i]. subsect == sigma.subsect) {
			value->sigma[i]. terms = sigma.terms;
			value->sigma[i + 1]. subsect += sigma.terms;
			value->sigma[i + 1]. terms -= sigma.terms;
			goto add2;
		}
#if 0
		if (sigma.subsect < value->sigma[i]. subsect) {
			value->sigma[i]. terms = j = value->sigma[i]. subsect - sigma.subsect;
			value->sigma[i]. subsect = sigma.subsect;
			value->sigma[i]. scale = sigma.scale;
			sigma.terms -= j;
			sigma.subsect += j;
			i++;
			value->sigma[i]. subsect = sigma.subsect;
			value->sigma[i]. terms -= j;
			goto add;
		}
	//	if (value->sigma[i]. subsect < sigma.subsect) {
			value->sigma[i]. terms = j = sigma.subsect - value->sigma[i]. subsect;
			i++;
			value->sigma[i]. subsect = sigma.subsect;
			value->sigma[i]. terms -= j;
			goto add;
	//	}
#endif
		if (sigma.subsect < value->sigma[i]. subsect) {
			value->sigma[i]. terms = j = value->sigma[i]. subsect - sigma.subsect;
			value->sigma[i]. subsect = sigma.subsect;
			value->sigma[i]. scale = sigma.scale;
			sigma.terms -= j;
			sigma.subsect += j;
		} else {
			value->sigma[i]. terms = j = sigma.subsect - value->sigma[i]. subsect;
		}
		i++;
		value->sigma[i]. subsect = sigma.subsect;
		value->sigma[i]. terms -= j;
		goto add;
	}
	if (value->sigma[MAXSIGMAS - 1]. scale ! = 0) {
		calcsigma(value);
		goto retry;
	}
	for (j = MAXSIGMAS - 2; i <= j; j--)
		value->sigma[j + 1] = value->sigma[j];
	value->sigma[i] = sigma;
	sigma.terms = 0;
	goto check;
}
