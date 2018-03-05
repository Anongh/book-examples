/* "nask.c" */
/* copyright(C) 2003 H.Kawai(카와이 히데미) */
/*   [OSASK 3978], [OSASK 3979]로 光成씨의 지적을 많이 참고로 했습니다 */
/*	小柳씨의 string0에 관한 지적도 참고로 했습니다 */

#include "../include/stdlib.h"	/* malloc/free */

#define	DEBUG			0

int nask_LABELBUFSIZ = 256 * 1024;

#define	UCHAR			unsigned char

#define	OPCLENMAX		8	/* 부족하게 되면 12로 해 주세요 */
#define MAX_SECTIONS	8

#define E_LABEL0		16
int nask_L_LABEL0 = 16384; /* extern 라벨은 16300개 정도 사용할 수 있다 */
int nask_maxlabels = 64 * 1024; /* 64 K개(LL:88*64k) */

static void setdec(unsigned int i, int n, UCHAR *s);
static void sethex0(unsigned int i, int n, UCHAR *s);

static void *cmalloc(int size)
{
	int i;
	char *p = malloc(size);
//	if (p) {
		for (i = 0; i < size; i++)
			p[i] = 0;
//	}
	return p;
}

struct INST_TABLE {
	UCHAR opecode[OPCLENMAX];
	unsigned int support;
	UCHAR param[8];
};

struct STR_SECTION {
	unsigned int dollar_label0; /* $ */
	unsigned int dollar_label1; /* ..$ */
	unsigned int dollar_label2; /* $$ */
	int total_len;
	UCHAR *p0, *p; /* 소트용의 포인터 */
	UCHAR name[17], name_len;
	signed char align0, align1; /* -1은 미설정 */
};

struct STR_OUTPUT_SECTION {
	UCHAR *p, *d0, *reloc_p;
	int addr, relocs;
	UCHAR align, flags;
};

extern int nask_errors;

#define SUP_8086		0x000000ff	/* bit 0 */
#define SUP_80186		0x000000fe	/* bit 1 */
#define SUP_80286		0x000000fc	/* bit 2 */
#define SUP_80286P		0x000000a8	/* bit 3 */
#define SUP_i386		0x000000f0	/* bit 4 */
#define SUP_i386P		0x000000a0	/* bit 5 */
#define SUP_i486		0x000000c0	/* bit 6 */
#define SUP_i486P		0x00000080	/* bit 7 */
#define	SUP_Pentium
#define	SUP_Pentium2
#define	SUP_Pentium3
#define	SUP_Pentium4

#define	PREFIX			0x01	/* param[1]이 프리픽스 번호 */
#define	NO_PARAM		0x02	/* param[1]의 하위 4bit가 작동코드 바이트수 */
#define	OPE_MR			0x03	/* mem/reg, reg형 */ /* [1]:datawidth, [2]:len */
#define	OPE_RM			0x04	/* reg, mem/reg형 */
#define	OPE_M			0x05	/* mem/reg형 */
#define OPE_SHIFT		0x06	/* ROL, ROR, RCL, RCR, SHL, SAL, SHR, SAR */
#define OPE_RET			0x07	/* RET, RETF, RETN */
#define OPE_AAMD		0x08	/* AAM, AAD */
#define OPE_INT			0x09	/* INT */
#define	OPE_PUSH		0x0a	/* INC, DEC, PUSH, POP */
#define	OPE_MOV			0x0b	/* MOV */
#define	OPE_ADD			0x0c	/* ADD, OR, ADC, SBB, AND, SUB, XOR, CMP */
#define	OPE_XCHG		0x0d	/* XCHG */
#define	OPE_INOUT		0x0e	/* IN, OUT */
#define	OPE_IMUL		0x0f	/* IMUL */
#define	OPE_TEST		0x10	/* TEST */
#define	OPE_MOVZX		0x11	/* MOVSX, MOVZX */
#define	OPE_SHLD		0x12	/* SHLD, SHRD */
#define	OPE_LOOP		0x13	/* LOOPcc, JCXZ */
#define	OPE_JCC			0x14	/* Jcc */
#define	OPE_BT			0x15	/* BT, BTC, BTR, BTS */
#define	OPE_ENTER		0x16	/* ENTER */
#define OPE_ALIGN		0x17	/* ALIGN, ALIGNB */
#define	OPE_FPU			0x30
#define	OPE_FPUP		0x31
#define	OPE_FSTSW		0x32
#define	OPE_FXCH		0x33
#define	OPE_ORG			0x3d	/* ORG */
#define	OPE_RESB		0x3e	/* RESB, RESW, RESD, RESQ, REST */
#define	OPE_EQU			0x3f

#define	OPE_JMP			0x40	/* CALL, JMP */
#define OPE_GLOBAL		0x44	/* GLOBAL, EXTERN */
#define	OPE_TIMES		0x47	/* TIMES */
#define	OPE_DB			0x48	/* DB, DW, DD, DQ, DT */
#define	OPE_END			0x49

/* NO_PARAM용 */
#define	OPE16			0x10
#define	OPE32			0x20
#define DEF_DS			0x40
	/* param[1]의 bit4 : ope32 */
	/* param[1]의 bit5 : ope16 */
	/* param[1]의 bit6 : 디폴트 프리픽스 DS */
	/* param[1]의 bit7 : 디폴트 프리픽스 SS */

static UCHAR table_prms[] = {
	0, 0, 0 /* NO_PARAM */, 2 /* OPE_MR */, 2 /* OPE_RM */,
	1 /* OPE_M */, 2 /* OPE_SHIFT */, 9 /* OPE_RET */, 9 /* OPE_AAMD */,
	1 /* OPE_INT */, 1 /* OPE_PUSH */, 2 /* OPE_MOV */, 2 /* OPE_ADD */,
	2 /* OPE_XCHG */, 2 /* OPE_INOUT */, 9 /* OPE_IMUL */, 2 /* OPE_TEST */,
	2 /* OPE_MOVZX */, 3 /* OPE_SHLD */, 9 /* OPE_LOOP */, 1 /* OPE_JCC */,
	2 /* OPE_BT */, 2 /* OPE_ENTER */, 1 /* OPE_ALIGN */, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	9 /* OPE_FPU */, 9 /* OPE_FPUP */, 1 /* OPE_FSTSW */, 9 /* OPE_FXCH */,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1 /* OPE_ORG */, 1 /* OPE_RESB */,
	1 /* OPE_EQU */
};

struct STR_DECODE {
	UCHAR *label, *param;
	struct INST_TABLE *instr;
	unsigned int prm_t[3];
	UCHAR *prm_p[3];
	int prefix;
	int gparam[3], gvalue[3], gp_mem, gp_reg;
	struct STR_SECTION *sectable;
	UCHAR error, flag /* , dollar */;
};
/* flag의 bit0는 mem/reg가 reg인가를 나타낸다 */

struct STR_TERM {
	int term_type;
	int value;
};

struct STR_OFSEXPR {
	int scale[2], disp;
	unsigned char reg[2], dispflag; /* 0xff때, unknown, reg가 127이하라면 스케일 없음 */
	unsigned char err;
};

struct STR_DEC_EXPR_STATUS {
	unsigned int support;
	int glabel_len;
	UCHAR *glabel;
	signed char datawidth; /* -1(default), 1(byte), 2(word), 4(dword) */
	signed char seg_override; /* -1(default), 0~5 */
	signed char range; /* -1(default), 0(short), 1(near), 2(far) */
	char nosplit; /* 0(default), 1(nosplit) */
	char use_dollar;  /* 0(no use), 1(use) */
	char option;
	char to_flag;
	unsigned int dollar_label0; /* $ */
	unsigned int dollar_label1; /* ..$ */
	unsigned int dollar_label2; /* $$ */
};

struct STR_STATUS {
	UCHAR *src1; /* 파일 종단 포인터 */
	unsigned int support, file_len;
	char bits, optimize, format, option;
	struct STR_DEC_EXPR_STATUS expr_status;
	struct STR_OFSEXPR ofsexpr;
	struct STR_TERM *expression, *mem_expr;
	UCHAR *file_p;
};

struct STR_IFDEFBUF {
	/* 조건부 정의용 버퍼 구조체 */
	UCHAR *bp, *bp0, *bp1; /* range-error용 버퍼 */
	UCHAR vb[12]; /* bit0-4:바이트수, bit7:expr 플래그, bit5-6:레인지 체크 */
	int dat[12];
	UCHAR *expr[12];
};

UCHAR *decoder(struct STR_STATUS *status, UCHAR *src, struct STR_DECODE *decode);
UCHAR *putprefix(UCHAR *dest0, UCHAR *dest1, int prefix, int bits, int opt);
void put4b(unsigned int i, UCHAR *p);
unsigned int get4b(UCHAR *p);
struct STR_TERM *decode_expr(UCHAR **ps, UCHAR *s1, struct STR_TERM *expr, int *priority, struct STR_DEC_EXPR_STATUS *status);
void calc_ofsexpr(struct STR_OFSEXPR *ofsexpr, struct STR_TERM **pexpr, char nosplit);
int getparam(UCHAR **ps, UCHAR *s1, int *p, struct STR_TERM *expression, struct STR_TERM *mem_expr, 
	struct STR_OFSEXPR *ofsexpr, struct STR_DEC_EXPR_STATUS *status);
int testmem(struct STR_OFSEXPR *ofsexpr, int gparam, struct STR_STATUS *status, int *prefix);
void putmodrm(struct STR_IFDEFBUF *ifdef, int tmret, int gparam,
	struct STR_STATUS *status, /* struct STR_OFSEXPR *ofsexpr, */ int ttt);
int microcode90(struct STR_IFDEFBUF *ifdef, struct STR_TERM *expr, int *def, signed char dsiz);
int microcode91(struct STR_IFDEFBUF *ifdef, struct STR_TERM *expr, int *def, signed char dsiz);
int microcode94(struct STR_IFDEFBUF *ifdef, struct STR_TERM *expr, int *def);
int defnumexpr(struct STR_IFDEFBUF *ifdef, struct STR_TERM *expr, UCHAR vb, UCHAR def);
int getparam0(UCHAR *s, struct STR_STATUS *status);
int getconst(UCHAR **ps, struct STR_STATUS *status, int *p);
int testmem0(struct STR_STATUS *status, int gparam, int *prefix);
static UCHAR *labelbuf0, *labelbuf;
static UCHAR *locallabelbuf0 /* 256bytes */, *locallabelbuf;
static int nextlabelid;
int label2id(int len, UCHAR *label, int extflag);
UCHAR *id2label(int id);
UCHAR *put_expr(UCHAR *s, struct STR_TERM **pexpr);
UCHAR *flush_bp(int len, UCHAR *buf, UCHAR *dest0, UCHAR *dest1, struct STR_IFDEFBUF *ifdef);
struct STR_TERM *rel_expr(struct STR_TERM *expr, struct STR_DEC_EXPR_STATUS *status);
UCHAR *LL_skip_expr(UCHAR *p);
UCHAR *LL_skipcode(UCHAR *p);

#define	defnumconst(ifdef, imm, virbyte, typecode) ifdef->vb[(virbyte) & 0x07] = typecode; ifdef->dat[(virbyte) & 0x07] = imm

/* 리마크 NL(f8) : 라인 스타트, 4바이트의 길이, 4바이트의 포인터
	바이트열을 늘어놓는다 */
/* 리마크 ADR(e0) : 주소 출력 */
/* 리마크 BY(e1) : 1바이트 출력 */
/* 리마크 WD(e2) : 2바이트 출력 */
/* 리마크 3B(e3) : 3바이트 출력 */
/* 리마크 DW(e4) : 4바이트 출력 */
/* 리마크[BY](e5) : 1바이트 출력[]붙어 있다 */
/* 리마크[WD](e6) : 2바이트 출력[]붙어 있다 */
/* 리마크[3B](e7) : 3바이트 출력[]붙어 있다 */
/* 리마크[DW](e8) : 4바이트 출력[]붙어 있다 */

#define	REM_ADDR		0xe0
//#define	REM_BYTE		0xe1	/* 폐지 */
//#define	REM_WORD		0xe2	/* 폐지 */
//#define	REM_DWRD		0xe4	/* 폐지 */
#define	REM_ADDR_ERR	0xe5
#define	REM_RANGE_ERR	0xe8
#define REM_3B			0xf1
#define REM_4B			0xf2
#define REM_8B			0xf6
#define	SHORT_DB0		0x30
#define	SHORT_DB1		0x31
#define	SHORT_DB2		0x32
#define	SHORT_DB4		0x34

#define	EXPR_MAXSIZ		2048
#define	EXPR_MAXLEN		1000

UCHAR *skipspace(UCHAR *s, UCHAR *t)
{
	while (s < t && *s ! = '\n' && *s <= ' ')
		s++;
	return s;
}

UCHAR *putimm(int i, UCHAR *p)
/* 최대 6바이트 출력 */
{
	UCHAR c = 6;
	if (i >= 0) {
		if (i <= 0xff)
			c = 0x00;
		else if (i <= 0xffff)
			c = 0x02;
		else if (i <= 0xffffff)
			c = 0x04;
	} else {
		if (i >= -0x100)
			c = 0x01;
		else if (i >= -0x10000)
			c = 0x03;
		else if (i >= -0x1000000)
			c = 0x05;
	}
	p[0] = c;
	p[1] = i & 0xff;
	c >>= 1;
	p += 2;
	while (c) {
		i >>= 8;
		c--;
		*p++ = i & 0xff;
	}
	return p;
}

UCHAR *nask(UCHAR *src0, UCHAR *src1, UCHAR *dest0, UCHAR *dest1)
/* dest1를 돌려준다(NULL라면 overflow) */
{
	int i, j, k, prefix_def, tmret;
	UCHAR buf[2 * 8], *bp; /* buf는 적당하게 malloc하는 것이 좋을지도 */
	UCHAR *src, c, *s, *labelflags, *dest00 = dest0;
	struct STR_STATUS *status;
	struct STR_DECODE *decode;
	struct INST_TABLE *itp;
	struct STR_IFDEFBUF *ifdef;
	struct STR_TERM *expr;
	static int tbl_o16o32[4] =
		{ 0, 0x10000000 /* O16(암묵) */, 0, 0x20000000 /* O32(암묵) */ };
	struct STR_SECTION *sectable, *section;
	nextlabelid = nask_L_LABEL0;
	status = malloc(sizeof (*status));
	decode = malloc(sizeof (*decode));
	ifdef = malloc(sizeof (*ifdef));
	status->expression = malloc(EXPR_MAXLEN * sizeof (struct STR_TERM));
	status->mem_expr = malloc(EXPR_MAXLEN * sizeof (struct STR_TERM));
	sectable = cmalloc(MAX_SECTIONS * sizeof (struct STR_SECTION));
	ifdef->bp0 = malloc(256);
	ifdef->bp1 = ifdef->bp0 + 256;
	labelbuf = labelbuf0 = malloc(nask_LABELBUFSIZ);
	locallabelbuf = locallabelbuf0 = malloc(256);
	for (i = 0; i < 9; i++)
		ifdef->expr[i] = malloc(EXPR_MAXSIZ);
	labelflags = malloc(nask_maxlabels);
	for (i = 0; i < nask_maxlabels; i++)
		labelflags[i] = 0;
	for (i = 0; i < MAX_SECTIONS; i++) {
	//	sectable[i]. name[0] = '\0';
	//	sectable[i]. total_len = 0;
		sectable[i]. align0 = -1;
		sectable[i]. align1 = 1;
		sectable[i]. dollar_label2 = 0xffffffff;
	}
	decode->sectable = section = sectable;
	section->name[0] = '. ';
	section->name[1] = '. ';
	section->name[2] = '\0';
	section->p = dest0;

	status->src1 = src1;
	status->support = status->expr_status.support = 1; /* 1:8086 */
	status->bits = 16;
	status->optimize = 0;
	status->format = 0; /* BIN */
	status->option = 0; /* 거의 NASM 호환 */
	status->expr_status.option = 0;
	status->file_len = 0;

	if (dest0 + 5 > dest1)
		dest0 = NULL;
	if (dest0 == NULL)
		goto overrun;

	dest0[0] = REM_3B;
	dest0[1] = 0; /* start section */
	dest0[2] = 0;
	dest0[3] = 0x68; /* 68-00 Intel-endian */
	dest0[4] = 0x00;
//	dest0[5] = 0x58; /* ORG */
//	dest0[6] = 0x00; /* 0 */
//	dest0[7] = 0x00;
	dest0 += 5;

	status->expr_status.dollar_label2 = 0xffffffff;
	while (src0 < src1) {
		if (status->expr_status.dollar_label2 == 0xffffffff) {
			status->expr_status.dollar_label2 = nextlabelid++;
			status->expr_status.dollar_label1 = status->expr_status.dollar_label2;
		}
		status->expr_status.dollar_label0 = status->expr_status.dollar_label1;
		status->expr_status.dollar_label1 = 0xffffffff;
		bp = buf;
		ifdef->vb[8] = 0; /* for TIMES */
		src = decoder(status, src0, decode);
		/* 라인 스타트 출력 */
		/* f7, src - src0, src0 */
		if (dest0 + 9 + 6 /* $의 분 */ > dest1)
			dest0 = NULL;
		if (dest0 == NULL)
			goto overrun;
		dest0[0] = 0xf7; /* line start */
		put4b(src - src0, &dest0[1]);
		put4b((int) src0, &dest0[5]);
		dest0 += 9;
		ifdef->bp = ifdef->bp0;
	//	if (decode->dollar ! = 0 && status->expr_status.dollar_label0 == 0xffffffff)
	//		status->expr_status.dollar_label0 = nextlabelid++;
		if ((i = status->expr_status.dollar_label0) ! = 0xffffffff) {
			if (labelflags[i] == 0) {
				dest0[0] = 0x0e;
				labelflags[i] = 0x01;
				dest0 = putimm(i, &dest0[1]);
			}
		}
		if (decode->label) {
			/* 라벨 정의 */
			bp[0] = 0x0e; /* 프로그램 카운터를 라벨 정의하는 커맨드 */
 			if (decode->instr ! = NULL && decode->instr->param[0] == OPE_EQU)
				bp[0] = 0x2d; /* EQU */
			s = decode->label;
			do {
				c = *s;
				if (c <= ' ')
					break;
				if (c == ':')
					break;
				if (c == ';')
					break;
				if (c == ', ')
					break;
			} while (++s < src1);
			i = label2id(s - decode->label, decode->label, 0);
			if (labelflags[i]) {
				*bp++ = 0xe7;
				c = 0; /* mod nnn r/m 없음 */
				goto outbp;
			}
			labelflags[i] = 0x01;
			c = bp[0];
			bp = putimm(i, &bp[1]);
			if (c == 0x0e) {
				if ((dest0 = flush_bp(bp - buf, buf, dest0, dest1, ifdef)) == NULL)
					goto overrun;
				bp = buf;
			}
			if (*decode->label ! = '. ') {
				if (! (decode->instr ! = NULL && decode->instr->param[0] == OPE_EQU)) {
					i = s - decode->label;
					locallabelbuf = locallabelbuf0;
					s = decode->label;
					while (i) {
						i--;
						*locallabelbuf++ = *s++;
					}
				}
			}
		}
times_skip:
		if (decode->error) {
err:
			/* 에러 출력 */
			buf[0] = decode->error | 0xe0;
			bp = buf + 1;
			c = 0; /* mod nnn r/m 없음 */
			goto outbp;
		}
		c = 0; /* mod nnn r/m 없음 */
		prefix_def = status->bits; /* 디폴트 상태 */
		if ((itp = decode->instr) ! = 0) {
			switch (itp->param[0]) {
			case NO_PARAM:
				/* 프리픽스 */
				j = itp->param[1];
				if (j & OPE16)
					decode->prefix |= 0x10000000; /* O16(암묵) */
				if (j & OPE32)
					decode->prefix |= 0x20000000; /* O32(암묵) */
				if (j & DEF_DS)
					prefix_def |= 0x01; /* DS */
				for (i = 0; i < (j & 0x0f); i++) {
					bp[0] = SHORT_DB1; /* 0x31 */
					bp[1] = itp->param[2 + i];
					bp += 2;
				}
			//	c = 0; /* mod nnn r/m 없음 */
				break;

			case OPE_M:
			ope_m:
				if ((i = decode->gparam[0]) & 0xe0) /* reg도 mem도 아닌 || range가 붙으면 에러 */
					goto err4; /* 데이터 타입 에러 */
				decode->flag = 0;
				if ((i & 0x10) == 0) {
					decode->flag = 1;
					if (decode->gvalue[0] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
						goto err4;
				}
				decode->gp_mem = i;
				decode->gp_reg = itp->param[1] << (9 - 4);
				i &= 0x0f;
				goto ope_mr_check0;

			case OPE_MR:
				if ((j = decode->gparam[0]) & 0xe0) /* reg도 mem도 아닌 || range가 붙으면 에러 */
					goto err4;
				decode->flag = 0;
				if ((j & 0x10) == 0) {
					decode->flag = 1;
					if (decode->gvalue[0] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
						goto err4;
				}
				if (decode->gparam[1] & 0x1f0) /* reg가 아닌 || range가 붙으면 에러 || use $도 에러 */
					goto err4;
				if (decode->gvalue[1] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
					goto err4;
				decode->gp_reg = decode->gparam[1];
				if ((j & 0x0f) == 0x0f && (itp->param[1] & 0x80) ! = 0) {
					/* mem의 데이터 사이즈가 부정 && 제2 오퍼랜드에 same0 지정 있음 */
					j = (j & ~0x0f) | (decode->gparam[1] & 0x0f);
				}
				decode->gp_mem = decode->gparam[0] = j;
				goto ope_mr2;

			case OPE_RM:
				if (decode->gparam[0] & 0x1f0) /* reg가 아닌 || range가 붙으면 에러 || use $도 에러 */
					goto err4;
				if (decode->gvalue[0] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
					goto err4;
				decode->gp_reg = decode->gparam[0];
				if ((j = decode->gparam[1]) & 0xe0) /* reg도 mem도 아닌 || range가 붙으면 에러 */
					goto err4;
				decode->flag = 0;
				if ((j & 0x10) == 0) {
					decode->flag = 1;
					if (decode->gvalue[1] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
						goto err4;
				}
				if ((j & 0x0f) == 0x0f && (itp->param[1] & 0x80) ! = 0) {
					/* mem의 데이터 사이즈가 부정 && 제2 오퍼랜드에 same0 지정 있음 */
					j = (j & ~0x0f) | (decode->gparam[0] & 0x0f);
				}
				decode->gp_mem = decode->gparam[1] = j;

		ope_mr2:
				/* 오퍼랜드 사이즈 체크 */
				k = itp->param[1] >> 4;
				i = decode->gparam[0] & 0x0f;
				j = decode->gparam[1] & 0x0f;
				if (k == 0)
					goto ope_mr_check0;
				if (k & 8) {
					if (i == j)
						goto ope_mr_check0;
				}
				if (j > 4) {
			err3:
					decode->error = 3; /* data size error */
					goto err;
				}
			//	if (j == 3)
			//		goto err3;
				if ((k & j) == 0)
					goto err3;

		ope_mr_check0:
				k = itp->param[1] & 0x0f;
				if (k == 0)
					goto ope_mr_mem;
				if (k & 8) {
					if (i == 0xf)
						goto ope_mr_mem;
				}
				if (i > 4)
					goto err3;
			//	if (i == 3)
			//		goto err3;
				if ((k & i) == 0)
					goto err3;
		ope_mr_mem:
				j = decode->flag & 1;
				if (itp->param[2] & 0x40) { /* no-reg */
					if (j ! = 0) {
			err4:
						decode->error = 4; /* data type error */
						goto err;
					}
				}
				if (itp->param[2] & (UCHAR) 0x80) { /* no-mem */
					if (j == 0)
						goto err4;
				}
			//	if (j == 0) {
			//		tmret = testmem0(status, decode->gp_mem, &decode->prefix);
			//		if (tmret == 0)
			//			goto err5; /* addressing error */
			//		prefix_def |= tmret & 0x03;
			//	}
				j = itp->param[2] & 0x07;
				for (i = 0; i < j; i++) {
					bp[0] = SHORT_DB1; /* 0x31 */
					bp[1] = itp->param[3 + i];
					bp += 2;
				}
				if ((itp->param[2] & 0x30) ! = 0x20) {
					/* 데이터 사이즈를 확정 */
					i = decode->gparam[0];
					if (itp->param[2] & 0x08)
						i = decode->gparam[1];
					i &= 0x0f;
					if ((itp->param[2] & 0x20) == 0) {
						decode->prefix |= (tbl_o16o32 - 1)[i];
					//	if (i == 2)
					//		decode->prefix |= 0x10000000; /* O16(암묵) */
					//	if (i == 4)
					//		decode->prefix |= 0x20000000; /* O32(암묵) */
					}
					if (itp->param[2] & 0x10) {
						if (i ! = 1)
							bp[-1] |= 0x01;
					}
				}
				if (status->optimize >= 1) {
					if (itp->param[3] == 0x8d) /* LEA */
						decode->prefix &= ~0x07e0; /* bit5-10 */
				}
				bp[0] = 0x78; /* mod nnn r/m */
				bp[1] = 0x79; /* sib */
				bp[2] = 0x7a; /* disp */
				bp += 3;
	setc:
				c = 3 ^ decode->flag; /* mod nnn r/m 있어 */ 				
				break;

			case OPE_SHIFT: /* mem/reg, imm8|CL */
				if ((j = decode->gparam[0]) & 0xe0) /* reg도 mem도 아닌 || range가 붙으면 에러 */
					goto err4;
				decode->gp_mem = j;
				decode->gp_reg = itp->param[1] << 9; /* TTT */
				decode->flag = 0; /* mem */
				if ((j & 0x10) == 0) {
					/* reg */
					decode->flag = 1;
					if (decode->gvalue[0] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
						goto err4;
				}
				/* 데이터 사이즈를 확정 */
				i = decode->gparam[0] & 0x0f;
				if (i == 0)
					goto err3;
				if (i > 4)
					goto err3;
				decode->prefix |= (tbl_o16o32 - 1)[i];
			//	if (i == 2)
			//		decode->prefix |= 0x10000000; /* O16(암묵) */
			//	if (i == 4)
			//		decode->prefix |= 0x20000000; /* O32(암묵) */
				j = 0;
				if (i ! = 1)
					j++; /* j = 1; */
				if (decode->gparam[1] == 0x2201) { /* CL */
					bp[0] = SHORT_DB1; /* 0x31 */
					bp[1] = 0xd2 | j;
					bp[2] = 0x78; /* mod nnn r/m */
					bp[3] = 0x79; /* sib */
					bp[4] = 0x7a; /* disp */
				} else {
					static int mcode[] = {
						0x0154, SHORT_DB1, 0xc0, 0x98, 0, /* default */
						0x01,   SHORT_DB1, 0xd0, SHORT_DB0, 0 /* if == 1 */
					};
					mcode[2] = 0xc0 | j;
					mcode[7] = 0xd0 | j;
					if ((decode->gparam[1] & 0xf0) ! = 0x20) /* imm가 아닌 || range가 붙어 있었다 */
						goto err4; /* data type error */
					if ((decode->gparam[1] & 0x0f) == 0x0f) {
						if (microcode94(ifdef, status->expression, mcode))
							goto err2;
					} else if ((decode->gparam[1] & 0x0f) == 0x01) {
						defnumconst(ifdef, 0xc0 | j, 0x74, 0x01 /* UCHAR, const */);
						if (defnumexpr(ifdef, status->expression, 0x75 & 0x07, 0x98 & 0x07))
							goto err2;
					} else
						goto err3; /* WORD나 DWORD가 지정되었다 */
					bp[0] = 0x7c; /* 1100000w || 1101000w */
					bp[1] = 0x78; /* mod nnn r/m */
					bp[2] = 0x79; /* sib */
					bp[3] = 0x7a; /* disp */
					bp[4] = 0x7d; /* imm8 || none */
				}
				bp += 5;
			//	c = 3 ^ decode->flag; /* mod nnn r/m 있음 */ 				
			//	break;
				goto setc;

			case OPE_RET: /* RET, RETF, RETN */
				bp[0] = SHORT_DB1; /* 0x31 */
			//	c = 0; /* mod nnn r/m 없음 */
				if (decode->flag == 0) {
					/* 오퍼랜드 없음 */
				//	bp[0] = SHORT_DB1; /* 0x31 */
					bp[1] = itp->param[1] | 0x01;
					bp += 2;
					break;
				}
				if (decode->flag > 1)
					goto err2; /* 파라미터 에러 */
				if ((decode->gparam[0] & 0xf0) ! = 0x20)
					goto err2; /* imm가 아닌 || range가 붙어 있었다 */
				if ((decode->gparam[0] & 0x0f) == 2)
					goto OPE_RET_notopt; /* WORD 지정 있음 */
				if ((decode->gparam[0] & 0x0f) ! = 0x0f)
					goto err3; /* BYTE나 DWORD가 지정되었다 */
				if (status->optimize == 0) {
					/* 최적화하지 않는다 */
		OPE_RET_notopt:
					if (defnumexpr(ifdef, status->expression, 0x75 & 0x07, 0x9a & 0x07))
						goto err2;
				//	bp[0] = SHORT_DB1; /* 0x31 */
					bp[1] = itp->param[1];
					bp += 2;
				} else {
					static int mcode[] = {
						0x0154, SHORT_DB1, 0, 0x9a, 0, /* default */
						0x00,   SHORT_DB1, 0, SHORT_DB0, 0 /* if == 0 */
					};
					mcode[2] = itp->param[1];
					mcode[7] = itp->param[1] | 0x01;
					/* 최적화한다 */
					if (microcode94(ifdef, status->expression, mcode))
						goto err2;
					*bp++ = 0x7c; /* 자동 선택된 작동코드 */
				}
				*bp++ = 0x7d; /* imm16 || none */
				break;

			case OPE_AAMD: /* AAM, AAD */
				if (decode->flag == 0) {
					defnumconst(ifdef, itp->param[2], 0x74, 0x01 /* UCHAR, const */);
				} else if (decode->flag == 1) {
					if ((decode->gparam[0] & 0xf0) ! = 0x20)
						goto err2; /* imm가 아닌 || range가 붙어 있었다 */
					if ((decode->gparam[0] & 0x0f) ! = 0x01 && (decode->gparam[0] & 0x0f) ! = 0x0f)
						goto err3; /* WORD나 DWORD가 붙어 있었다 */
					if (defnumexpr(ifdef, status->expression, 0x74 & 0x07, 0x98 & 0x07))
						goto err2;
				} else
					goto err2; /* 파라미터 에러 */
				bp[0] = SHORT_DB1; /* 0x31 */
				bp[1] = itp->param[1];
				bp[2] = 0x7c; /* 오퍼랜드(디폴트 :itp->param[2]) */
				bp += 3;
			//	c = 0; /* mod nnn r/m 없음 */
				break;

			case OPE_INT: /* INT */
				if ((decode->gparam[0] & 0xf0) ! = 0x20)
					goto err2; /* imm가 아닌 || range가 붙어 있었다 */
				if ((decode->gparam[0] & 0x0f) == 1)
					goto OPE_INT_notopt; /* BYTE 지정 있음 */
				if ((decode->gparam[0] & 0x0f) ! = 0x0f)
					goto err3; /* WORD나 DWORD가 붙어 있었다 */
				if (status->optimize == 0) {
		OPE_INT_notopt:
					/* 최적화하지 않는다 */
					if (defnumexpr(ifdef, status->expression, 0x75 & 0x07, 0x98 & 0x07))
						goto err2;
					bp[0] = SHORT_DB1; /* 0x31 */
					bp[1] = 0xcd;
					bp += 2;
				} else {
					static int mcode[] = {
						0x0154, SHORT_DB1, 0xcd, 0x98, 0, /* default */
						0x03,   SHORT_DB1, 0xcc, SHORT_DB0, 0 /* if == 3 */
					};
					/* 최적화한다 */
					if (microcode94(ifdef, status->expression, mcode))
						goto err2;
					*bp++ = 0x7c; /* 자동 선택된 작동 코드 */
				}
				*bp++ = 0x7d; /* imm8 || none */
			//	c = 0; /* mod nnn r/m 없음 */
				break;

			case OPE_PUSH: /* PUSH, POP, INC, DEC */
				if (decode->gparam[0] & 0xc0)
					goto err2; /* range가 붙어 있었다 */
			//	c = 0; /* mod nnn r/m 없음 */
				decode->gp_mem = decode->gparam[0];
				decode->gp_reg = (itp->param[1] & 0x07) << 9;
				bp[0] = SHORT_DB1; /* 0x31 */
				switch (decode->gparam[0] & 0x30) {
				case 0x00: /* reg */
					if (decode->gvalue[0] < 16) {
						/* reg16/reg32 */
						decode->prefix |= (tbl_o16o32 - 1)[decode->gparam[0] & 0x0f];
					//	i = 0x10000000; /* O16(암묵) */
					//	if (decode->gvalue[0] < 8) {
					//	//	i = 0x20000000; /* O32(암묵) */
					//		i <<= 1;
					//	}
					//	decode->prefix |= i;
						bp[1] = itp->param[2] | (decode->gvalue[0] & 0x07);
						bp += 2;
						goto outbp;
					}
					if (decode->gvalue[0] < 24) {
						/* reg8 */
						if (itp->param[1] & 0x08)
							goto err3; /* PUSH, POP */
						bp[1] = itp->param[3];
						c = 2; /* mod nnn r/m 있음 */
			ope_push_mem:
						bp[2] = 0x78;
						bp[3] = 0x79;
						bp[4] = 0x7a;
						bp += 5;
						goto outbp;
					}
					if ((itp->param[1] & 0x08) == 0)
						goto err2; /* INC, DEC */
					if (decode->gvalue[0] < 28) {
						/* ES, CS, SS, DS */
						/* NASK는"POP CS"를 에러로 하지 않는다(8086이기 때문에) */
						bp[1] = itp->param[4] | (decode->gvalue[0] & 0x03) << 3;
						bp += 2;
						goto outbp;
					}
					if (decode->gvalue[0] < 30) {
						/* FS, GS */
						bp[1] = 0x0f;
						bp[2] = SHORT_DB1; /* 0x31 */
						bp[3] = itp->param[5] | (decode->gvalue[0] & 0x03) << 3;
						bp += 4;
						goto outbp;
					}
					goto err2;
				case 0x10: /* mem */
				//	tmret = testmem0(status, decode->gp_mem, &decode->prefix);
				//	if (tmret == 0)
				//		goto err5; /* addressing error */
				//	prefix_def |= tmret & 0x03;
					c = decode->gparam[0] & 0x0f;
					bp[1] = 0;
					if (itp->param[1] & 0x08) {
						/* PUSH, POP */
						if (c == 0x01)
							goto err3;
						if (c == 0x0f)
							c = 1;
					} else {
						/* INC, DEC */
						if (c == 0x0f)
							goto err3;
						if (c ! = 1)
							bp[1] = 1;
					}
					bp[1] |= itp->param[3];
					decode->prefix |= (tbl_o16o32 - 1)[c];
					c = 3; /* mod nnn r/m 있음 */
					goto ope_push_mem;
				case 0x20: /* imm */
					if ((itp->param[1] & 0x10) == 0)
						goto err2;
					/* PUSH */
					{
						static int mcode[] = {
							0x54,	0x01 /* UCHAR, const */, 0x68 /* 16bit/32bit */,
									0x01 /* UCHAR, const */, 0x6a /* 8bit */
						};
						c = decode->gparam[0] & 0x0f;
						mcode[0] = 0x54; /* word/byte mode */
						if (c <= 4)
							decode->prefix |= (tbl_o16o32 - 1)[c];
						if (c == 4 || c == 0x0f && (prefix_def & 32) ! = 0)
							mcode[0] = 0x54 | 8 /* D-bit */;
						if ((decode->error = microcode90(ifdef, status->expression, mcode, c)) ! = 0)
							goto err;
						bp[0] = 0x7d;
						bp[1] = 0x7c;
						bp += 2;
						c = 0; /* mod nnn r/m 없음 */
						goto outbp;
					}
				}
				goto err2;

			case OPE_MOV: /* MOV */
				if (decode->gparam[0] & 0xc0)
					goto err4; /* range가 붙어 있음, data type error */
				if (decode->gparam[1] & 0xc0)
					goto err4; /* range가 붙어 있음, data type error */
				if ((decode->gparam[1] & 0x30) == 0x20) {
					/* imm */
					static char typecode[4] = { 0x9e & 0x07, 0x9b & 0x07, 0, 0x9d & 0x07 };
					c = decode->gparam[0] & decode->gparam[1] & 0x0f;
					if (c == 0)
						goto err3;
					if (c == 0x0f)
						goto err3;
					decode->prefix |= (tbl_o16o32 - 1)[c];
					if (defnumexpr(ifdef, status->expression, 0x74 & 0x07, typecode[c - 1]))
						goto err2; /* parameter error */
					bp[0] = SHORT_DB1; /* 0x31 */
					if ((j = decode->gparam[0] & 0x30) == 0x00) {
						if (decode->gvalue[0] >= 24)
							goto err4; /* data type error */
						bp[1] = 0xb0 | (decode->gvalue[0] & 0x07);
						bp[2] = 0x7c; /* imm */
						if (c ! = 1)
							bp[1] |= 0x08;
						bp += 3;
						c = 0; /* mod nnn r/m 없음 */
						goto outbp;
					}
					if (j ! = 0x10)
						goto err2;
					/* mem, imm */
					decode->gp_mem = decode->gparam[0];
				//	tmret = testmem0(status, decode->gp_mem = decode->gparam[0], &decode->prefix);
				//	if (tmret == 0)
				//		goto err5; /* addressing error */
				//	prefix_def |= tmret & 0x03;
				//	decode->flag = 0;
					bp[1] = 0xc6;
					decode->gp_reg = 0x00 << 9;
					bp[2] = 0x78;
					bp[3] = 0x79;
					bp[4] = 0x7a;
					bp[5] = 0x7c; /* imm */
					if (c ! = 1)
						bp[1] |= 0x01;
					bp += 6;
					c = 3; /* mod nnn r/m 있음 */
					goto outbp;
				}
				i = 0; /* direction-bit */
				if ((decode->gparam[1] & 0x30) == 0x10)
					goto mov_swap; /* memory */
				if ((decode->gparam[0] & 0x30) == 0x00 && decode->gvalue[0] >= 24) {
		mov_swap:
					i++;
				}
				tmret = 0;
				decode->flag = 1;
				if (((decode->gp_mem = decode->gparam[i]) & 0x30) == 0x10) {
					/* memory */
					tmret = testmem0(status, decode->gp_mem, &decode->prefix);
					if (tmret == 0)
						goto err5; /* addressing error */
				//	prefix_def |= tmret & 0x03;
					decode->flag = 0;
				} else if ((decode->gp_mem & 0x30) ! = 0x00)
					goto err4; /* imm가 와선 안 된다 */
				else if ((decode->gp_mem >> 9) >= 24)
					goto err4; /* reg8/reg16/reg32 이외가 와선 안 된다 */
				j = decode->gp_reg = decode->gparam[i ^ 1];
				c = decode->gp_mem & 0x0f;
				if ((j & 0x30) ! = 0x00)
					goto err4; /* reg는 아님, data type error */
				if (24 <= (j >> 9) && (j >> 9) < 30 && decode->flag ! = 0) {
					if (c == 1)
						goto err3; /* data size error */
				} else if (c ! = 0x0f && (j & 0x0f) ! = c)
					goto err3; /* data size error */
				decode->gparam[i] = (decode->gparam[i] & ~0x0f) | (j & 0x0f);
				if (j == 0x0004 /* EAX */ || j == 0x1002 /* AX */ || j == 0x2001 /* AL */) {
					if (tmret & 0x08) { /* disp-only */
						decode->prefix |= (tbl_o16o32 - 1)[j & 0x0f];
						c = 0xa0 | (i ^ 1) << 1;
						if (j ! = 0x2001 /* AL */)
							c |= 0x01;
						bp[0] = SHORT_DB1; /* 0x31 */
						bp[1] = c;
						bp[2] = 0x7a; /* disp */
						bp += 3;
					//	c = 3 ^ decode->flag; /* mod nnn r/m 있음 */
					//	goto outbp;
						goto setc;
					}
				}
				if ((j = decode->gp_reg >> 9) < 24) { /* mem/reg, reg */
					itp->param[2] = 0x11; /* w0 */
					itp->param[3] = 0x88 | i << 1;
					goto ope_mr_mem;
				}
				if (j < 30) { /* mem/reg, sreg */
					if (i == 0 && decode->flag ! = 0) {
						/* (i == 0) 한편 reg라면 O16/O32 있음 */
						decode->prefix |= (tbl_o16o32 - 1)[c];
					}
					itp->param[2] = 0x21; /* no-w no-o16/o32 */
					itp->param[3] = 0x8c | i << 1;
					goto ope_mr_mem;
				}
				if (j < 40)
					goto err2; /* 그런 레지스터는 모르기 때문에 파라미터 에러 */
				if (j < 64) {
					c = (j - 40) >> 3;
					if (c == 2)
						c = 4;
					c |= 0x20; /* 20, 21, 24 */
					itp->param[2] = 0xa2; /* reg-only no-w no-o16/o32 no-mem */
					itp->param[3] = 0x0f;
					itp->param[4] = c | i << 1;
					goto ope_mr_mem;
				}
	err2:
				decode->error = 2; /* 파라미터 에러 */
				goto err;

			case OPE_ADD: /* ADD */
				itp->param[3] &= 0x38;
				if (decode->gparam[0] & 0xc0)
					goto err4; /* range가 붙어 있음, data type error */
				if (decode->gparam[1] & 0xc0)
					goto err4; /* range가 붙어 있음, data type error */
				if ((decode->gparam[1] & 0x30) == 0x20) {
					/* imm */
					if ((c = decode->gparam[0] & 0x0f) == 0x0f)
						goto err3; /* data size error */
					if (c > 4)
						goto err3; /* data size error */
					if ((decode->gparam[1] & 0x0f) ! = 0x0f) {
						if (c < (decode->gparam[1] & 0x0f))
							goto err3; /* data size error */
					}
					decode->prefix |= (tbl_o16o32 - 1)[c];
					if ((j = decode->gparam[0]) & 0x20) /* reg도 mem도 아니면 에러 */
						goto err4;
					decode->flag = 0;
					if ((j & 0x10) == 0) {
						decode->flag = 1;
						if (decode->gvalue[0] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
							goto err4;
						if ((decode->gvalue[0] & 0x07) == 0) {
							/* EAX, AX, AL */
							static int mcode[] = {
								0x5c,	0x01 /* UCHAR, const */, 0x05, 0x00 /* null */,         0x00 /* 32bit */,
										0x01 /* UCHAR, const */, 0x83, 0x01 /* UCHAR, const */, 0xc0 /* 8bit */
							};
							if (c <= 2) {
								/* AL, AX이므로 이야기는 간단 */
								bp[0] = SHORT_DB1;
								bp[1] = itp->param[3] | 0x04;
								if (c == 2)
									bp[1] |= 0x01;
								bp[2] = 0x7c;
								bp += 3;
							//	c == 1 >> 9e(6);
							//	c == 2 >> 9b(3);
							//	9 - c * 3
								if (defnumexpr(ifdef, status->expression, 0x7c & 0x07, 9 - c * 3))
									goto err2; /* 파라미터 에러 */
								c = 0; /* mod nnn r/m 없음 */
								goto outbp;
							}
							/* EAX */
							mcode[2] = itp->param[3] | 0x05;
							mcode[8] = itp->param[3] | 0xc0;

							bp[0] = 0x7d;
							bp[1] = 0x7e;
							bp[2] = 0x7c; /* imm */
							bp += 3;

							if (microcode91(ifdef, status->expression, mcode, decode->gparam[1] & 0x0f))
								goto err2; /* 파라미터 에러 */
							c = 0; /* mod nnn r/m 없음 */
							goto outbp;
						}
					}
					decode->gp_mem = decode->gparam[0];
					decode->gp_reg = itp->param[3] << (9 - 3);
					if (c == 1) {
						/* 1바이트이므로 이야기는 간단 */
						bp[0] = SHORT_DB1;
						bp[1] = 0x80;
						bp += 2;
						if (defnumexpr(ifdef, status->expression, 0x7c & 0x07, 0x9e & 0x07))
							goto err2; /* 파라미터 에러 */
					} else {
						static int mcode[] = {
							0x54,	0x01 /* UCHAR, const */, 0x81 /* 16bit/32bit */,
									0x01 /* UCHAR, const */, 0x83 /* 8bit */
						};
						mcode[0] = 0x54; /* 16bit */
						if (c == 4)
							mcode[0] = 0x5c; /* 32bit */
						*bp++ = 0x7d;
						if (microcode90(ifdef, status->expression, mcode, decode->gparam[1] & 0x0f))
							goto err2; /* 파라미터 에러 */
					}
					bp[0] = 0x78;
					bp[1] = 0x79;
					bp[2] = 0x7a;
					bp[3] = 0x7c;
					bp += 4;
				//	c = 3 ^ decode->flag; /* mod nnn r/m 있음 */
				//	goto outbp;
					goto setc;
				}
				i = 0; /* direction-bit */
				if ((decode->gparam[1] & 0x30) == 0x10)
					i++;
				if ((j = decode->gparam[i]) & 0x20) /* reg도 mem도 아니면 에러 */
					goto err4;
				decode->flag = 0;
				if ((j & 0x10) == 0) {
					decode->flag = 1;
					if (decode->gvalue[i] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
						goto err4;
				}
				if (decode->gparam[i ^ 1] & 0x30) /* reg는 아니면 에러 */
					goto err4;
				if (decode->gvalue[i ^ 1] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
					goto err4;
				decode->gp_reg = decode->gparam[i ^ 1];
				if ((j & 0x0f) == 0x0f /* && (itp->param[1] & 0x80) ! = 0 */ ) {
					/* mem의 데이터 사이즈가 부정 && 제2 오퍼랜드에 same0 지정 있음 */
					j = (j & ~0x0f) | (decode->gp_reg & 0x0f);
				}
				decode->gp_mem = decode->gparam[i] = j;
				itp->param[3] = (itp->param[3] & ~0x02) | i << 1;
				goto ope_mr2;

			case OPE_XCHG: /* XCHG */
				/* 메모리를 제1 오퍼랜드에 .EAX를 제2 오퍼랜드에 */
				/* 그리고 reg16/reg32, eAX라면 특별 형식 */
				/* 그 이외는 MR형 */
				i = 0;
				if ((decode->gparam[1] & 0x30) == 0x10)
					goto xchg_swap; /* memory */
				if (decode->gparam[0] == 0x0004 /* EAX */ || decode->gparam[0] == 0x1002 /* AX */) {
		xchg_swap:
					i++;
				}
				if ((j = decode->gparam[i]) & 0xe0) /* reg도 mem도 아닌 || range가 붙으면 에러 */
					goto err4;
				decode->flag = 0;
				if ((j & 0x10) == 0) {
					decode->flag = 1;
					if (decode->gvalue[i] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
						goto err4;
				}
				if (decode->gparam[i ^ 1] & 0x1f0) /* reg가 아닌 || range가 붙으면 에러 || use $도 에러 */
					goto err4;
				if (decode->gvalue[i ^ 1] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
					goto err4;
				decode->gp_reg = decode->gparam[i ^ 1];
				if ((j & 0x0f) == 0x0f /* && (itp->param[1] & 0x80) ! = 0 */) {
					/* mem의 데이터 사이즈가 부정 && 제2 오퍼랜드에 same0 지정 있음 */
					j = (j & ~0x0f) | (decode->gp_reg & 0x0f);
				}
				decode->gp_mem = decode->gparam[i] = j;
				if ((decode->gp_reg & 0x0f) ! = (decode->gp_mem & 0x0f))
					goto err3; /* data size error */
				if (decode->gp_reg == 0x0004 /* EAX */ ||  decode->gp_reg == 0x1002 /* AX */) {
					if (decode->flag) {
						bp[0] = SHORT_DB1; /* 0x31 */
						bp[1] = 0x90 | ((decode->gp_mem >> 9) & 0x07);
						bp += 2;
						decode->prefix |= (tbl_o16o32 - 1)[decode->gp_reg & 0x0f];
					//	c = 0; /* mod nnn r/m 없음 */
						goto outbp;
					}
				}
				goto ope_mr2;

			case OPE_INOUT: /* IN, OUT */
				j = 0;
				if ((c = itp->param[1]) == 0xe6 /* OUT */)
					j++; /* j = 1; */
				i = 0x10000000; /* O16(암묵) */
				if (decode->gparam[j] == 0x0004 || decode->gparam[j] == 0x1002) {
					/* EAX 나 AX */
					decode->prefix |= (tbl_o16o32 - 1)[decode->gparam[j] & 0x0f];
					c++;
				} else if (decode->gparam[j] ! = 0x2001)
					goto err2; /* 파라미터 에러 */
				j = getparam0(decode->prm_p[j ^ 0x01], status);
				bp[0] = SHORT_DB1; /* 0x31 */
				if (j == 0x1402) { /* DX */
					bp[1] = c | 0x08;
					bp += 2;
				} else {
					bp[1] = c;
					bp[2] = 0x7c;
					bp += 3;
					c = j & 0x0f;
					if (c ! = 0xf && c ! = 0x01)
						goto err3; /* data size error */
					if (j & 0xc0)
						goto err2; /* 파라미터 에러(range 검출) */
					if ((j & 0x30) ! = 0x20)
						goto err2; /* 파라미터 에러(reg/mem 검출) */
					if (defnumexpr(ifdef, status->expression, 0x7c & 0x07, 0x98 & 0x07))
						goto err2; /* 파라미터 에러 */
				}
				c = 0; /* mod nnn r/m 없음 */
				goto outbp;

			case OPE_IMUL:
				/* mem/reg			1111011w   mod-101-r/m */
				/* reg, mem/reg		00001111   10101111   mod-reg-r/m */
				/* reg, mem/reg, imm	011010s1   mod-reg-r/m   imm */
				/* reg, imm >> reg, reg, imm에 읽어 대체 */
				if (decode->flag == 0)
					goto err2; /* parameter error */
				if (decode->flag > 3)
					goto err2; /* parameter error */
				if (decode->flag == 1)
					goto ope_m;
				if (decode->gparam[0] & 0xf1) /* 짝수 사이즈의 reg만 */
					goto err4; /* data type error */
				if (decode->gvalue[0] >= 24)
					goto err4; /* data type error */
				decode->gp_reg = decode->gparam[0];
				if (decode->flag == 2) {
					if ((decode->gparam[1] & 0x20) == 0) {
						/* mem/reg */
						bp[0] = SHORT_DB1; /* 0x31 */
						bp[1] = 0x0f;
						bp[2] = SHORT_DB1; /* 0x31 */
						bp[3] = 0xaf;
						bp[4] = 0x78; /* mod nnn r/m */
						bp[5] = 0x79; /* sib */
						bp[6] = 0x7a; /* disp */
						bp += 7;
		imul2:
						if (decode->gparam[1] & 0xe0) /* range가 붙어 있던 || imm */
							goto err4; /* data type error */
						decode->gp_mem = decode->gparam[1];
						decode->flag = 0; /* mem */
						if ((decode->gp_mem & 0x10) == 0) {
							/* reg */
							decode->flag = 1;
							if (decode->gvalue[1] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
								goto err4;
						}
						if ((j = decode->gp_mem & decode->gp_reg & 0x0f) == 0)
							goto err3; /* data size error */
						decode->prefix |= (tbl_o16o32 - 1)[j];
					//	c = 3 ^ decode->flag; /* mod nnn r/m 있음 */ 				
					//	goto outbp;
						goto setc;
					}
					/* imm */
					decode->gparam[2] = decode->gparam[1];
					decode->gparam[1] = decode->gparam[0];
					decode->gvalue[1] = decode->gvalue[0];
				//	decode->flag = 3;
				}
				{
					/* reg, mem/reg, imm형 */
					static int mcode[] = {
						0x54,	0x01 /* UCHAR, const */, 0x69 /* 16bit/32bit */,
								0x01 /* UCHAR, const */, 0x6b /* 8bit */
					};
					if ((decode->gparam[2] & 0xf0) ! = 0x20) /* not imm */
						goto err4; /* data type error */
					mcode[0] &= 0x54;
					if (decode->gp_reg & 4)
						mcode[0] |= 0x5c;
					if (microcode90(ifdef, status->expression, mcode, decode->gparam[2] & 0x0f))
						goto err2; /* 파라미터 에러 */
					bp[0] = 0x7d; /* 011010s1 */
					bp[1] = 0x78; /* mod nnn r/m */
					bp[2] = 0x79; /* sib */
					bp[3] = 0x7a; /* disp */
					bp[4] = 0x7c; /* imm */
					bp += 5;
					goto imul2;
				}

			case OPE_TEST: /* mem/reg, mem/reg|imm8 */
				decode->gp_mem = decode->gparam[0];
				decode->gp_reg = decode->gparam[1];
				if ((decode->gp_reg & 0x30) == 0x10) {
					decode->gp_mem = decode->gparam[1];
					decode->gp_reg = decode->gparam[0];
				}
				if ((j = decode->gp_mem) & 0xe0) /* reg도 mem도 아닌 || range가 붙으면 에러 */
					goto err4; /* data type error */
				decode->flag = 0; /* mem */
				if ((j & 0x10) == 0) {
					/* reg */
					decode->flag = 1;
					if ((j >> 9) >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
						goto err4;
				}
				j &= 0x0f;
				bp[0] = SHORT_DB1; /* 0x31 */
				s = bp;
				if ((i = decode->gp_reg & 0xf0) == 0x00) {
					/* mem/reg, reg */
					j &= decode->gp_reg;
					if ((decode->gp_reg >> 9) >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
						goto err4; /* data type error */
					bp[1] = 0x84;
					bp[2] = 0x78; /* mod nnn r/m */
					bp[3] = 0x79; /* sib */
					bp[4] = 0x7a; /* disp */
					bp += 5;
				} else {
					static UCHAR table[] = {
						0x9e /* dummy */ & 0x07, 0x9e /* byte */ & 0x07, 0x9b /* word */ & 0x07,
						0x9b /* dummy */ & 0x07, 0x9d /* dword */ & 0x07
					};
					if (i ! = 0x20) /* imm가 아닌지, range가 붙어 있었다 */
						goto err4; /* data type error */
					if ((decode->gp_mem & 0x0ef0) == 0) {
						/* EAX, AX, AL */
						bp[1] = 0xa8;
						bp += 2;
					} else {
						if (j > 4)
							goto err3; /* data size error */
						bp[1] = 0xf6;
						decode->gp_reg = 0 << 9;
						bp[2] = 0x78; /* mod nnn r/m */
						bp[3] = 0x79; /* sib */
						bp[4] = 0x7a; /* disp */
						bp += 5;
					}
					*bp++ = 0x7c;
					if (defnumexpr(ifdef, status->expression, 0x7c & 0x07, table[j]))
						goto err2; /* 파라미터 에러 */
				}
				if (j == 0)
					goto err3; /* data size error */
				if (j ! = 1)
					s[1] |= 0x01;
				decode->prefix |= (tbl_o16o32 - 1)[j];
			//	c = 3 ^ decode->flag; /* mod nnn r/m 있음 */ 				
			//	goto outbp;
				goto setc;

			case OPE_MOVZX:
				if (decode->gparam[0] & 0xf9) /* reg가 아닌 || range가 붙으면 에러 || reg8 */
					goto err4; /* data type error */
				if (decode->gvalue[0] >= 24)
					goto err4; /* data type error */
				if (decode->gparam[1] & 0xe4) /* reg도 mem도 아닌 || range가 붙으면 에러 || dword나 사이즈 부정 */
					goto err4; /* data type error */
				decode->flag = 0; /* mem */
				if ((decode->gparam[1] & 0x10) == 0) {
					/* reg */
					decode->flag = 1;
					if (decode->gvalue[1] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
						goto err4;
				}
				decode->gp_reg = decode->gparam[0];
				decode->gp_mem = decode->gparam[1];
				decode->prefix |= (tbl_o16o32 - 1)[decode->gparam[0] & 0x0f];
				bp[0] = SHORT_DB1; /* 0x31 */
				bp[1] = 0x0f;
				bp[2] = SHORT_DB1; /* 0x31 */
				bp[3] = itp->param[1] ^ (decode->gparam[1] & 0x01);
				bp[4] = 0x78; /* mod nnn r/m */
				bp[5] = 0x79; /* sib */
				bp[6] = 0x7a; /* disp */
				bp += 7;
				goto setc;

			case OPE_SHLD: /* mem/reg, reg, imm8|CL */
				if ((j = decode->gparam[0]) & 0xe0) /* reg도 mem도 아닌 || range가 붙으면 에러 */
					goto err4; /* data type error */
				decode->gp_mem = j;
				decode->gp_reg = decode->gparam[1];
				if ((decode->gparam[1] & 0xf0) ! = 0x00) /* reg가 아닌 || range가 붙었다 */
					goto err4; /* data type error */
				decode->flag = 0; /* mem */
				if ((j & 0x10) == 0) {
					/* reg */
					decode->flag = 1;
					if (decode->gvalue[0] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
						goto err4;
				}
				/* 데이터 사이즈를 확정 */
				i = decode->gparam[1] & 0x0f;
				if (i <= 1)
					goto err3; /* data size error */
				if (i > 4)
					goto err3; /* data size error */
				if ((decode->gparam[0] & i) == 0)
					goto err3; /* data size error */
				decode->prefix |= (tbl_o16o32 - 1)[i];
				bp[0] = SHORT_DB1; /* 0x31 */
				bp[1] = 0x0f;
				bp[2] = SHORT_DB1; /* 0x31 */
				bp[3] = itp->param[1];
				bp[4] = 0x78; /* mod nnn r/m */
				bp[5] = 0x79; /* sib */
				bp[6] = 0x7a; /* disp */
				if (decode->gparam[2] == 0x2201) { /* CL */
					bp[3] |= 0x01;
					bp += 7;
				} else {
					if ((decode->gparam[2] & 0xf0) ! = 0x20) /* imm가 아닌 || range가 붙어 있었다 */
						goto err4; /* data type error */
					bp[7] = 0x7c; /* imm8 */
					if (defnumexpr(ifdef, status->expression, 0x7c & 0x07, 0x98 & 0x07 /* UCHAR */))
						goto err2; /* 파라미터 에러 */
					bp += 8;
				}
			//	c = 3 ^ decode->flag; /* mod nnn r/m 있음 */ 				
			//	goto outbp;
				goto setc;

			case OPE_LOOP:
				if (itp->param[2]) {
					if (decode->flag ! = 1)
						goto err2; /* parameter error */
					decode->prefix |= (tbl_o16o32 - 1)[itp->param[2] >> 3];
				} else {
					if (decode->flag == 2) {
						i = decode->gparam[1];
						if (i == 0x0204 /* ECX */ || i == 0x1202 /* CX */)
							decode->prefix |= (tbl_o16o32 - 1)[i & 0x07];
						else
							goto err4; /* data type error */
					} else if (decode->flag ! = 1)
						goto err2;
				}
				if ((decode->gparam[0] & 0x30) ! = 0x20) /* imm는 아니다 */
					goto err4;
				c = decode->gparam[0] & 0xc0;
				if (c == 0x40) /* NEAR */
					goto err4;
				if (c == 0x80) /* FAR */
					goto err4;
				c = decode->gparam[0] & 0x0f;
				if (c == 4)
					goto err4;
				if (c == 2)
					goto err4;
				getparam0(decode->prm_p[0], status);
				bp[0] = SHORT_DB1; /* 0x31 */
				bp[1] = itp->param[1];
				rel_expr(status->expression, &status->expr_status);
				bp[2] = 0x7c;
				bp += 3;
				if (defnumexpr(ifdef, status->expression, 0x7c & 0x07, 0x99 & 0x07 /* SCHAR */))
					goto err2; /* 파라미터 에러 */
				c = 0; /* mod nnn r/m 없음 */
				goto outbp;

			case OPE_JCC:
				if ((decode->gparam[0] & 0x30) ! = 0x20) /* imm는 아니다 */
					goto err4;
				i = 0x7;
				c = decode->gparam[0] & 0xc0;
				if (c == 0x80) {
					/* NEAR */
					i &= 2 + 4;
				}
				if (c == 0xc0)
					goto err4; /* FAR */
				if (c == 0x40) {
					/* SHORT */
					i &= 1;
				}
				i &= decode->gparam[0];
				if (i >= 6) {
					/* DWORD인지 WORD인지를 모를 때 */
					if ((prefix_def & 32) == 0)
						i &= ~4;
					if ((prefix_def & 16) == 0)
						i &= ~2;
				}
				if (i) {
					static int mcode[] = {
						0x00,	0x01 /* UCHAR, const */, 0x0f, 0x01 /* UCHAR, const */, 0x00 /* 16/32bit */,
								0x01 /* UCHAR, const */, 0x00, 0x00 /* null */,         0x00 /* 8bit */
					};
					c = 2;
					mcode[0] = 0x54; /* word/byte mode */
					if (i & 4) {
						c = 4;
						mcode[0] = 0x5c; /* dword/byte mode */
					}
					if (i == 1)
						c = 1;
					decode->prefix |= (tbl_o16o32 - 1)[c];
					c = 0x0f;
					if ((i & (i - 1)) == 0)
						c = i;
					mcode[4] = itp->param[1] | 0x80;
					mcode[6] = itp->param[1] | 0x70;
					rel_expr(status->expression, &status->expr_status);
					if ((decode->error = microcode91(ifdef, status->expression, mcode, c)) ! = 0)
						goto err;
					bp[0] = 0x7d;
					bp[1] = 0x7e;
					bp[2] = 0x7c;
					bp += 3;
					c = 0; /* mod nnn r/m 없음 */
					goto outbp;
				}
				goto err3; /* data size error */

			case OPE_BT: /* mem/reg, imm8|reg */
				/* 0x0f 0xba mod-TTT-r/m imm8 */
				/* 0x0f 10-TTT-011 mod-reg-r/m */
				/* NASM나 MASM에서는 mem, imm 때에 주소 조정을 하지 않았기 때문에 거기에 맞추었다 */
				if ((j = decode->gparam[0]) & 0xe0) /* reg도 mem도 아닌 || range가 붙으면 에러 */
					goto err4;
				decode->gp_mem = j;
				decode->gp_reg = decode->gparam[1];
				decode->flag = 0; /* mem */
				if ((j & 0x10) == 0) {
					/* reg */
					decode->flag = 1;
					if (decode->gvalue[0] >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
						goto err4;
				}
				bp[0] = SHORT_DB1; /* 0x31 */
				bp[1] = 0x0f;
				bp[2] = SHORT_DB1; /* 0x31 */
				bp[4] = 0x78; /* mod nnn r/m */
				bp[5] = 0x79; /* sib */
				bp[6] = 0x7a; /* disp */
				if ((i = decode->gparam[1] & 0xf0) == 0) {
					/* reg */
					if (decode->gvalue[1] >= 24)
						goto err4; /* data type error */
					bp[3] = itp->param[1] | 0x83;
					j &= decode->gparam[1];
					bp += 7;
				} else {
					/* imm8 */
					if (i ! = 0x20) /* imm가 아닌 || range가 붙어 있었다 */
						goto err4; /* data type error */
					decode->gp_reg = itp->param[1] << (9 - 3);
					bp[3] = 0xba;
					bp[7] = 0x7c; /* imm8 */
					bp += 8;
					if (defnumexpr(ifdef, status->expression, 0x7c & 0x07, 0x98 & 0x07 /* UCHAR */))
						goto err2; /* 파라미터 에러 */
				}
				/* 데이터 사이즈를 확정 */
				j &= 0x0f;
				if (j <= 1)
					goto err3; /* data size error */
				if (j == 0x0f)
					j &= 0x01; /* 그 때의 BITS에 맞춘다 */
				if (j > 4)
					goto err3; /* data size error */
				decode->prefix |= (tbl_o16o32 - 1)[j];
			//	c = 3 ^ decode->flag; /* mod nnn r/m 있음 */ 				
			//	goto outbp;
				goto setc;

			case OPE_ENTER: /* imm16, imm8 */
				/* 11001000  imm16  imm8 */
				if ((decode->gparam[0] & 0xf0) ! = 0x20)
					goto err4; /* data type error */
				if ((decode->gparam[1] & 0xf0) ! = 0x20)
					goto err4; /* data type error */
				if ((decode->gparam[0] & 0x02) ! = 0x02)
					goto err3; /* data size error */
				if ((decode->gparam[1] & 0x01) ! = 0x01)
					goto err3; /* data size error */
				bp[0] = SHORT_DB1; /* 0x31 */
				bp[1] = 0xc8;
				bp[2] = 0x7c;
				bp[3] = 0x7d;
				bp += 4;
				if (defnumexpr(ifdef, status->expression, 0x7d & 0x07, 0x98 & 0x07 /* UCHAR */))
					goto err2; /* 파라미터 에러 */
				getparam0(decode->prm_p[0], status);
				if (defnumexpr(ifdef, status->expression, 0x7c & 0x07, 0x9a & 0x07 /* USHORT */))
					goto err2; /* 파라미터 에러 */
			//	c = 0; /* mod nnn r/m 없음 */
				goto outbp;

			case OPE_ALIGN: /* ALIGN, ALIGNB */
				if ((decode->gparam[0] & 0x02f0) ! = 0x0020)
					goto err2; /* 정수식은 아니면 에러 */
				ifdef->vb[8] = 0x84;
				/* ALIGN x 는 다음과 같이 변환한다. TIMES (x-(($-$$) %x)) %x DB y */
				/* mod(sub(x, mod(sub($, $$), x)), x)  ->  mod sub x mod sub $ $$ x x */
				expr = status->expression;
				expr[0]. term_type = 1; /* operator */
				expr[0]. value = 8; /* unsigned mod */
				expr[1]. term_type = 1; /* operator */
				expr[1]. value = 5; /* signed/unsinged sub */
				expr[2]. term_type = 0; /* constant */
				expr[2]. value = decode->gvalue[0];
				expr[3]. term_type = 1; /* operator */
				expr[3]. value = 8; /* unsigned mod */
				expr[4]. term_type = 1; /* operator */
				expr[4]. value = 5; /* signed/unsinged sub */
				expr[5]. term_type = 3; /* label */
				expr[5]. value = status->expr_status.dollar_label0;
				expr[6]. term_type = 3; /* label */
				expr[6]. value = status->expr_status.dollar_label2;
				expr[7]. term_type = 0; /* constant */
				expr[7]. value = decode->gvalue[0];
				expr[8]. term_type = 0; /* constant */
				expr[8]. value = decode->gvalue[0];
				ifdef->dat[8] = put_expr(ifdef->expr[8], &expr) - ifdef->expr[8];
				bp[0] = 0x59; /* TIMES microcode */
				bp[1] = 0x06; /* len [정정수(4바이트)] */
				put4b(1, &bp[2]); /* len = 1 */
				bp[6] = SHORT_DB1; /* 0x31 */
				bp[7] = itp->param[1];
				bp += 8;
				i = decode->gvalue[0];
				c = 0;
				while (i) {
					c++;
					i >>= 1;
				}
				if (section->align1 < c)
					section->align1 = c;
				c = 0;
				goto outbp;

			case OPE_FPU:
				if (decode->flag == 0)
					goto err2; /* 파라미터 에러 */
				if (decode->flag == 1) {
					if (decode->gparam[0] & 0xe0)
						goto err4; /* data type error */
					decode->gp_mem = decode->gparam[0];
					if (decode->gparam[0] & 0x10) {
						/* 메모리 */
						static unsigned char sizelist[16] = {
							0xff, 0xff, 0, 0xff, 1, 0xff, 0xff, 0xff,
							2, 0xff, 3, 0xff, 0xff, 0xff, 0xff, 0xfe
						};
					//	tmret = testmem0(status, decode->gp_mem, &decode->prefix);
					//	if (tmret == 0)
					//		goto err5; /* addressing error */
					//	prefix_def |= tmret & 0x03;
						c = sizelist[decode->gp_mem & 0x0f];
						if (c == 0xff)
							goto err3; /* data size error */
						if (c == 0xfe) {
							if ((itp->param[1] & 0x04) == 0)
								goto err3; /* data size error */
							c = itp->param[1] & 0x03;
						}
					} else {
						/* 레지스터 */
						if ((i = decode->gvalue[0] & ~0x07) == 72)
							c = 4;
						else if (i == 64)
							c = 5;
						else
							goto err2; /* 파라미터 에러 */
					}
				}
				if (decode->flag == 2) {
					if (itp->param[1] & 0x08)
						goto err2; /* 파라미터 에러 */
					for (i = 0; i < 2; i++) {
						if (decode->gparam[i] & 0xf0)
							goto err4; /* data type error */
						if ((decode->gvalue[i] & ~0x07) ! = 72)
							goto err4; /* data type error */
					}
					if (decode->gvalue[0] == 72 /* ST0 */) {
						decode->gp_mem = decode->gparam[1];
						c = 4;
					} else if (decode->gvalue[1] == 72 /* ST0 */) {
						decode->gp_mem = decode->gparam[0];
						c = 5;
					} else
						goto err2; /* 파라미터 에러 */
				}
				if (decode->flag > 2)
					goto err2; /* 파라미터 에러 */
				decode->flag = 0;
				if (c >= 4)
					decode->flag = 1;
				if (((c = itp->param[2 + c]) & 0x80) == 0)
					goto err4; /* data type error */
				bp[0] = SHORT_DB1; /* 0x31 */
				bp[1] = (c & 0x07) | 0xd8;
				decode->gp_reg = (int) c << (9 - 3);
				bp[2] = 0x78;
				bp[3] = 0x79;
				bp[4] = 0x7a;
				bp += 5;
			//	c = 3 ^ decode->flag; /* mod nnn r/m 있음 */
			//	goto outbp;
				goto setc;

			case OPE_FPUP:
				if (decode->flag == 2 && decode->gparam[1] == 0x900a /* ST0 */)
					decode->flag = 1;
				if (decode->flag ! = 1)
					goto err2; /* paramter error */
				if (decode->gparam[0] & 0xf0)
					goto err4; /* data type error */
				if ((decode->gvalue[0] & ~0x07) ! = 72)
					goto err4; /* data type error */
				decode->gp_mem = decode->gparam[0];
				decode->gp_reg = (int) itp->param[1] << (9 - 3);
				bp[0] = SHORT_DB1; /* 0x31 */
				bp[1] = (itp->param[1] & 0x07) | 0xd8;
				bp[2] = 0x78;
				bp += 3;
				c = 2; /* mod nnn r/m 있어(reg) */
				goto outbp;

			case OPE_FSTSW:
				if (decode->gparam[0] ! = 0x1002 /* AX */)
					goto ope_m;
				bp[0] = SHORT_DB1; /* 0x31 */
				bp[1] = itp->param[5];
				bp[2] = SHORT_DB1; /* 0x31 */
				bp[3] = itp->param[6];
				bp += 4;
				if (itp->param[2] & 0x02) {
					bp[0] = SHORT_DB1;
					bp[1] = itp->param[7];
					bp += 2;
				}
			//	c = 0; /* mod nnn r/m 없음 */
				goto outbp;

			case OPE_ORG:
				if ((decode->gparam[0] & 0xf0) ! = 0x20)
					goto err4; /* data type error */
			//	if (status->optimize == 0)
			//		dest0 = putprefix(dest0, dest1, decode->prefix, prefix_def, 0);
				if ((dest0 = flush_bp(bp - buf, buf, dest0, dest1, ifdef)) == NULL)
					goto overrun;
			//	bp = buf;
				if (dest0 + EXPR_MAXSIZ + 1 > dest1)
					dest0 = NULL;
				if (dest0 == NULL)
					goto overrun;
				*dest0++ = 0x58;
				if (status->expr_status.dollar_label1 /* ..$ */ == 0xffffffff)
					status->expr_status.dollar_label1 = nextlabelid++;
				status->expr_status.dollar_label2 = status->expr_status.dollar_label1;
				goto equ_put_expr;

			case OPE_FXCH:
				if (decode->flag == 0) {
					decode->gparam[0] = 0x920a /* ST1 */;
					decode->flag = 1;
				}
				if (decode->flag == 2) {
					if (decode->gparam[1] == 0x900a /* ST0 */)
						goto fxch1;
					if (decode->gparam[0] ! = 0x900a /* ST0 */)
						goto err2; /* 파라미터 에러 */
					decode->gparam[0] = decode->gparam[1];
			fxch1:
					decode->flag = 1;
				}
				if (decode->flag ! = 1)
					goto err2; /* 파라미터 에러 */
				if ((decode->gparam[0] & ~0x0e00) ! = 0x900a)
					goto err4; /* data type error */
				bp[0] = SHORT_DB1; /* 0x31 */
				bp[1] = 0xd9;
				bp[2] = SHORT_DB1; /* 0x31 */
				bp[3] = 0xc8 + ((decode->gparam[0] >> 9) & 0x07);
				bp += 4;
			//	c = 0; /* mod nnn r/m 없음 */
				goto outbp;

			case OPE_RESB:
				if ((decode->gparam[0] & 0xff) ! = 0x2f)
					goto err4; /* data type error */
				if (ifdef->vb[8]) {
					if (itp->param[1] ! = 1)
						goto err6; /* TIMES error */
					/* 귀찮아서 그 외의 체크는 하지 않고 있다 */
					bp[0] = SHORT_DB1; /* 0x31 */
					bp[1] = 0x00;
					bp += 2;
					goto outbp;
				}
				if ((i = itp->param[1]) > 7)
					goto err4; /* data type error */
				bp[0] = 0x59; /* TIMES microcode */
				bp[1] = 0x06; /* len [정정수(4바이트)] */
				put4b(i, &bp[2]); /* len */
				bp[6] = 0x30 | itp->param[1];
				bp += 7;
				do {
					*bp++ = 0x00;
				} while (--i);
				ifdef->vb[8] = 0x84;
				expr = status->expression;
				if ((decode->gparam[0] & 0x200) == 0) {
					expr->term_type = 0; /* constant */
					expr->value = decode->gvalue[0];
				}
				ifdef->dat[8] = put_expr(ifdef->expr[8], &expr) - ifdef->expr[8];
				goto outbp;

			case OPE_EQU:
				if (decode->label == NULL)
					goto err2; /* 파라미터 에러 */
				if ((decode->gparam[0] & 0xf0) ! = 0x20)
					goto err4; /* data type error */
			//	if (status->optimize == 0)
			//		dest0 = putprefix(dest0, dest1, decode->prefix, prefix_def, 0);
				if ((dest0 = flush_bp(bp - buf, buf, dest0, dest1, ifdef)) == NULL)
					goto overrun;
			//	bp = buf;
				if (dest0 + EXPR_MAXSIZ > dest1)
					dest0 = NULL;
				if (dest0 == NULL)
					goto overrun;
		equ_put_expr:
				expr = status->expression;
				if ((decode->gparam[0] & 0x200) == 0) {
					expr->term_type = 0; /* constant */
					expr->value = decode->gvalue[0];
				}
				dest0 = put_expr(dest0, &expr);
				goto skip_equ;

			case OPE_JMP:
				/* JMP:  (r/m) near:FF /4, far:FF /5, (imm) near:E9, far:EA, short:EB */
				/* CALL: (r/m) near:FF /2, far:FF /3, (imm) near:E8, far:9A */
				/* itp->param[1] : r/m-near */
				/* itp->param[2] : imm-near */
				/* itp->param[3] : imm-far */
				/* itp->param[4] : imm-short */
				s = decode->param;
				if (! (s < status->src1 && *s ! = '\n' && *s ! = ';'))
					goto err2;
				j = getparam(&s, status->src1, &i, status->expression,
					status->mem_expr, &status->ofsexpr, &status->expr_status);
				c = 0;
				if (s < status->src1)
					c = *s++;
				if ((j & 0x20) == 0) {
					/* reg/mem */
					if (c ! = 0 && c ! = '\n' && c ! = ';')
						goto err2; /* 후속 검출 */
					decode->gp_mem = j;
					decode->gp_reg = itp->param[1] << (9 - 3);
					c = j & 0x06;
					if (c == 0)
						goto err3; /* data size error */
					if (c == 6)
						c = 1;
					decode->prefix |= (tbl_o16o32 - 1)[c];
					c = j & 0xc0;
					if (c == 0x40)
						goto err4; /* data type error */
					if (c == 0xc0)
						decode->gp_reg |= 1 << 9; /* far */
					decode->flag = 0; /* mem */
					if ((j & 0x10) == 0) {
						/* reg */
						decode->flag = 1;
						if (i >= 24) /* reg이지만 reg8/reg16/reg32는 아니다 */
							goto err4;
					}
					bp[0] = SHORT_DB1; /* 0x31 */
					bp[1] = 0xff;
					bp[2] = 0x78;
					bp[3] = 0x79;
					bp[4] = 0x7a;
					bp += 5;
					goto setc;
					/* "CALL/JMP FAR reg16/reg32"라고 하는 것도 할 수 있지만, 동작은 보증하지 않는다 */
				}
				/* imm */
				if (c ! = ':' && c ! = ', ') {
					if (c ! = 0 && c ! = '\n' && c ! = ';')
						goto err2; /* 후속 검출 */
					c = j & 0xc0;
					if (c == 0xc0 /* far */)
						goto err4; /* data type error */
					if (c == 0x40)
						c = 0x01; /* short */
					else if (c == 0x80)
						c = 0x06; /* near */
					else
						c = 0x07;
					c &= j;
					if (c >= 0x06) {
						/* DWORD인가 WORD인가 모를 때 */
						if ((prefix_def & 32) == 0)
							c &= ~4;
						if ((prefix_def & 16) == 0)
							c &= ~2;
					}
					if (c == 0)
						goto err4; /* data type error */
					if (c & 0x06)
						decode->prefix |= (tbl_o16o32 - 1)[c & 0x06];
					rel_expr(status->expression, &status->expr_status);
					if (itp->param[4]) {
						/* JMP SHORT/NEAR */
						static int mcode[] = {
							0x54,	0x01 /* UCHAR, const */, 0xe9 /* 16bit/32bit */,
									0x01 /* UCHAR, const */, 0xeb /* 8bit */
						};
						mcode[0] &= 0x54;
						if (c & 4)
							mcode[0] |= 0x5c;
						if (c & (c - 1))
							c = 0x0f; /* c가 2의 하지 않는 경우 */
						if (microcode90(ifdef, status->expression, mcode, c))
							goto err2; /* 파라미터 에러 */
						bp[0] = 0x7d; /* e9/eb */
						bp[1] = 0x7c; /* imm */
						bp += 2;
						c = 0; /* mod nnn r/m 없음 */
						goto outbp;
					}
					/* CALL NEAR */
					if ((c &= ~1) == 0)
						goto err4; /* data type error */
					if (defnumexpr(ifdef, status->expression, 0x7c & 0x07,
						(0x99 & 0x07) + c /* non-over SHORT/INT */))
						goto err2; /* 파라미터 에러 */
					bp[0] = SHORT_DB1; /* 0x31 */
					bp[1] = 0xe8;
					bp[2] = 0x7c;
					bp += 3;
					c = 0; /* mod nnn r/m 없음 */
					goto outbp;
				}
				/* far */
				c = j & 0xc0;
				if (c == 0x40) /* short */
					goto err4;
				if (c == 0x80) /* near */
					goto err4;
				c = j & 0x06;
				if (defnumexpr(ifdef, status->expression, 0x7c & 0x07, 0x9b & 0x07 /* non-over SHORT */))
					goto err2; /* 파라미터 에러 */
			//	s = skipspace(s, status->src1);
				j = getparam(&s, status->src1, &i, status->expression,
					status->mem_expr, &status->ofsexpr, &status->expr_status);
				if ((j & 0x30) ! = 0x20)
					goto err4;
				if (s < status->src1 && *s ! = '\n' && *s ! = ';')
					goto err2; /* 후속 있음 */
				c &= j;
				if (c >= 0x06) {
					/* DWORD인지 WORD인지 모를 때 */
					if ((prefix_def & 32) == 0)
						c &= ~4;
					if ((prefix_def & 16) == 0)
						c &= ~2;
				}
				if (c == 0)
					goto err3; /* data size error */
				decode->prefix |= (tbl_o16o32 - 1)[c];
				if (defnumexpr(ifdef, status->expression, 0x7d & 0x07,
					(0x99 & 0x07) + c /* non-over SHORT/INT */))
					goto err2; /* 파라미터 에러 */
				bp[0] = SHORT_DB1; /* 0x31 */
				bp[1] = itp->param[3];
				bp[2] = 0x7d;
				bp[3] = 0x7c;
				bp += 4;
				c = 0; /* mod nnn r/m 없음 */
				goto outbp;

			case OPE_GLOBAL: /* GLOBAL, EXTERN */
				s = decode->param;
				if (! (s < status->src1 && *s ! = '\n' && *s ! = ';'))
					goto err2;
				dest0 = putprefix(dest0, dest1, decode->prefix, prefix_def, status->optimize);
					/* 주소 출력 마크도 출력 */
					/* 필요하면 에러도 출력한다 */
				if ((dest0 = flush_bp(bp - buf, buf, dest0, dest1, ifdef)) == NULL)
					goto overrun;
				for (;;) {
					/* 라벨을 추출 */
					s = skipspace(s, status->src1);
					bp = s;
					if (s >= status->src1)
						goto err2;
					if (*s == ', ')
						goto err2;
					if (*s == ';')
						goto err2;
					if (*s == '\n')
						goto err2;
					do {
						s++;
						if (s >= status->src1)
							break;
						if (*s == ', ')
							break;
						if (*s == ';')
							break;
					} while (*s > ' ');
					i = s - bp;
					j = label2id(i, bp, itp->param[1] == 2);
					/* 로컬 라벨도 사용할 수 있지만. ...으로부터 시작되는 이름인 채 GLOBAL이 되므로 주의 */
					/* 만약 여러 차례 GLOBAL/EXTERN 하는 위험성을 회피하고 싶으면, 플래그를 만들어 체크 */
					if (dest0 + 15 > dest1) {
						dest0 = NULL;
						goto overrun;
					}
					dest0[0] = REM_8B; /* 0xf6 */
					dest0[1] = itp->param[1];
					dest0[2] = i & 0xff;
					dest0[3] = (i >> 8) & 0xff;
					put4b((int) bp, &dest0[4]);
					if (itp->param[1] == 1) { /* GLOBAL */
						dest0[8] = 0x0f;
						dest0[9] = 3;
						bp = &dest0[10];
						put4b(j, &dest0[11]);
						*bp = 0x07;
						dest0 += 11;
						do {
							(*bp)++;
							dest0++;
						} while (j >>= 8);
					} else { /* EXTERN */
						dest0[8] = 0x2c; /* 직전의 REM_8B에 대응하는 EXTERN 정보 */
						dest0[9] = 0x02; /* 2바이트 imm */
						dest0[10] = j & 0xff;
						dest0[11] = (j >> 8) & 0xff;
						if (j < nask_L_LABEL0)
							dest0 += 12;
					}
					s = skipspace(s, status->src1);
					if (s >= status->src1)
						goto skip_equ;
					if (*s ! = ', ')
						break;
					s++;
				}
				if (*s ! = '\n' && *s ! = ';')
					goto err2;
				goto skip_equ;

			case OPE_TIMES: /* virtual byte 0x80를 사용한다 */
				if (ifdef->vb[8]) {
		err6:
					decode->error = 6; /* TIMES error */
					goto err;
				}
				s = decode->param;
				if (! (s < status->src1 && *s ! = '\n' && *s ! = ';'))
					goto err2;
				j = getparam(&s, status->src1, &i, status->expression,
					status->mem_expr, &status->ofsexpr, &status->expr_status);
			//	if (j == 0)
			//		goto err2;
				if ((j & 0xf0) ! = 0x20)
					goto err2;
				ifdef->vb[8] = 0x84;
				expr = status->expression;
				if ((j & 0x0200) == 0) {
					/* 정수식 */
					expr->term_type = 0; /* constant */
					expr->value = i;
				}
				ifdef->dat[8] = put_expr(ifdef->expr[8], &expr) - ifdef->expr[8];
				bp[0] = 0x59; /* TIMES microcode */
				bp[1] = 0x06; /* len [정정수(4바이트)] */
				put4b(1, &bp[2]); /* len = 1 */
				bp += 6;
				src = decoder(status, s, decode);
				if (decode->label)
					goto err6;
				goto times_skip;

			case OPE_DB:
				s = decode->param;
				if (! (s < status->src1 && *s ! = '\n' && *s ! = ';'))
					goto err2;
				if (itp->param[2] == 0)
					goto err4;
				dest0 = putprefix(dest0, dest1, decode->prefix, prefix_def, status->optimize);
					/* 주소 출력 마크도 출력 */
					/* 필요하면 에러도 출력한다 */
				if ((dest0 = flush_bp(bp - buf, buf, dest0, dest1, ifdef)) == NULL)
					goto overrun;
			//	bp = buf;
				for (;;) {
					s = skipspace(s, status->src1);
					if (s < status->src1) {
						c = *s;
						if (c ! = 0x22 && c ! = 0x27)
							goto ope_db_expr;
						bp = s;
						do {
							bp++;
							if (bp >= status->src1)
								goto ope_db_expr;
							if (*bp == '\n')
								goto ope_db_expr;
						} while (*bp ! = c);
						bp = skipspace(bp + 1, status->src1);
						if (bp < status->src1) {
							if (*bp ! = ', ' && *bp ! = '\n' && *bp ! = ';')
								goto ope_db_expr;
						}
						/* 캐릭터 라인 검출 */
						s++;
						k = 0;
						for (;;) {
							if (*s == c)
								break;
							if (dest0 + 5 > dest1)
								dest0 = NULL;
							if (dest0 == NULL)
								goto overrun;
							if (k == 0) {
								*dest0++ = 0x30 | itp->param[1];
							}
							k++;
							*dest0++ = *s++;
							if (k == itp->param[1])
								k = 0;
						}
						if (k) {
							do {
								*dest0++ = 0x00;
							} while (++k < itp->param[1]);
						}
						s = bp;
						goto ope_db_skip;
					}
			ope_db_expr:
					j = getparam(&s, status->src1, &i, status->expression,
						status->mem_expr, &status->ofsexpr, &status->expr_status);
				//	if (j == 0)
				//		goto err2;
					if ((j & 0xf0) ! = 0x20)
						goto err2;
					if (defnumexpr(ifdef, status->expression, 0x7c & 0x07, itp->param[2] & 0x07))
						goto err2; /* 파라미터 에러 */
					/* 직접 출력 */
					k = ifdef->dat[0x7c & 0x07];
					bp = ifdef->expr[0x7c & 0x07];
					c = ifdef->vb[0x7c & 0x07];
					if ((c & 0x80) == 0 /* const */) {
						if (dest0 + c + 1 > dest1)
							dest0 = NULL;
						if (dest0 == NULL)
							goto overrun;
						*dest0++ = (c &= 0x1f) | 0x30;
						do {
							*dest0++ = k & 0xff;
							k >>= 8;
						} while (--c);
					} else {
						/* expr */
						if (dest0 + k + 2 > dest1)
							dest0 = NULL;
						if (dest0 == NULL)
							goto overrun;
						dest0[0] = (c & 0x1f) + 0x37; /* 38~3b */
						dest0[1] = (c >> 5) & 0x03;
						dest0 += 2;
						do {
							*dest0++ = *bp++;
						} while (--k);
					}
			ope_db_skip:
					if (s >= status->src1)
						goto skip_equ; /* break; */
					c = *s;
					if (c == '\n')
						goto skip_equ; /* break; */
					if (c == ';')
						goto skip_equ; /* break; */
					if (c ! = ', ')
						goto err2;
					s = skipspace(s + 1, status->src1);
				}
			//	goto skip_equ;

			case OPE_END:
				src = src1;
			//	c = 0; /* mod nnn r/m 없음 */
				goto outbp;

			case 0xe7: /* SECTION */
				goto flush_ifdefbuf;
			}
		}
outbp:
		if (c & 0x01) { /* mod nnn r/m 있어 */
			tmret = testmem0(status, decode->gp_mem, &decode->prefix);
			if (tmret == 0) {
		err5:
				decode->error = 5; /* addressing error */
				goto err;
			}
			prefix_def |= tmret & 0x03;
		}
		dest0 = putprefix(dest0, dest1, decode->prefix, prefix_def, status->optimize);
			/* 주소 출력 마크도 출력 */
			/* 필요하면 에러도 출력한다 */

		if (c) { /* mod nnn r/m 있어 */
			putmodrm(ifdef, tmret, decode->gp_mem, status,
				/* &status->ofsexpr, */ decode->gp_reg >> 9);
		}

flush_ifdefbuf:
		/* ifdefbuf를 출력 */
		i = ifdef->bp - ifdef->bp0;
		if (dest0 + i > dest1)
			dest0 = NULL;
		if (dest0 == NULL)
			goto overrun;
		for (j = 0; j < i; j++)
			dest0[j] = ifdef->bp0[j];
		dest0 += i;

		if ((dest0 = flush_bp(bp - buf, buf, dest0, dest1, ifdef)) == NULL)
			goto overrun;
	//	bp = buf;

		if (itp ! = NULL && itp->param[0] == 0xe7) {
			/* section */
			if (dest0 + 14 > dest1)
				dest0 = NULL;
			if (dest0 == NULL)
				goto overrun;
			dest0[0] = REM_3B; /* 0xf1 */
			dest0[1] = 0; /* change_section */
			dest0[2] = itp->param[1];
			dest0 += 3;
			section->total_len += dest0 - section->p;
			section->dollar_label0 = status->expr_status.dollar_label0;
			section->dollar_label1 = status->expr_status.dollar_label1;
			section->dollar_label2 = status->expr_status.dollar_label2;
			section = &sectable[itp->param[1]];
			section->p = dest0;
			status->expr_status.dollar_label0 = section->dollar_label0;
			status->expr_status.dollar_label1 = section->dollar_label1;
			status->expr_status.dollar_label2 = section->dollar_label2;
			if (section->total_len == 0) {
				dest0[0] = REM_3B;
				dest0[1] = 1; /* start section */
				dest0[2] = itp->param[1];
				dest0 += 3;
				if (status->expr_status.dollar_label1 /* ..$ */ == 0xffffffff)
					status->expr_status.dollar_label1 = nextlabelid++;
				status->expr_status.dollar_label2 = status->expr_status.dollar_label1;
				if (status->format == 0) { /* BIN */
	org0:
					dest0[0] = 0x58; /* ORG */
					dest0[1] = 0; /* const 0 (ORG 0) */
					dest0[2] = 0;
					dest0 += 3;
				} else { /* WCOFF */
					c = 0;
					if (section->name[0] ! = '. ')
						goto org0;
					if (section->name[1] == 't')
						c = 1; /* . text */
					if (section->name[1] == 'd')
						c = 2; /* . data */
					if (section->name[1] == 'b')
						c = 3; /* . bss */
					if (c == 0)
						goto org0;
					dest0[0] = 0x2c; /* define extern label */
					dest0[1] = 0x02;
					dest0[2] = c;
					dest0[3] = 0;
					dest0[4] = 0x58; /* ORG */
					dest0[5] = 0x08;
					dest0[6] = c;
					dest0 += 7;
				}
			}
		}
skip_equ:
		src0 = src;
	//	if (dest0 == NULL)
	//		goto overrun;
	}
skip_end:
	if (dest0 + (6 + 3) * MAX_SECTIONS + 9 > dest1)
		dest0 = NULL;
	if (dest0 == NULL)
		goto overrun;

	/* 라인 터미네이터 출력 */
	/* f7, 0, 0 */
	dest0[0] = 0xf7; /* line start */
	put4b(0, &dest0[1]);
	put4b(0, &dest0[5]);
	dest0 += 9;

//	section->dollar_label0 = status->expr_status.dollar_label0;
	section->dollar_label1 = status->expr_status.dollar_label1;
//	section->dollar_label2 = status->expr_status.dollar_label2;

	for (i = 0; i < MAX_SECTIONS; i++) {
		if (sectable[i]. name[0] == '\0')
			break;
		if ((j = sectable[i]. dollar_label1) ! = 0xffffffff) {
			if (labelflags[j] == 0) {
				dest0[0] = REM_3B; /* 0xf1 */
				dest0[1] = 0; /* change_section */
				dest0[2] = i;
				dest0 += 3;
				section->total_len += dest0 - section->p;
				section = &sectable[i];
				section->p = dest0;
				dest0[0] = 0x0e;
				labelflags[j] = 0x01;
				dest0 = putimm(j, &dest0[1]);
			}
		}
	}
	section->total_len += dest0 - section->p;
	src = malloc(i = dest0 - dest00);
	for (j = 0; j < i; j++)
		src[j] = dest00[j];
	sectable[0]. p0 = sectable[0]. p = dest00;
	for (j = 1; j < MAX_SECTIONS; j++)
		sectable[j]. p0 = sectable[j]. p = sectable[j - 1]. p0 + sectable[j - 1]. total_len;
	src1 = src + i;
	section = &sectable[0];
	s = section->p;
	do {
		c = *src;
		if (c == REM_3B && src[1] == 0) {
			s[0] = REM_3B;
			s[1] = 0;
			s[2] = src[2];
			s += 3;
			section->p = s;
			section = &sectable[src[2]];
			s = section->p;
			src += 3;
			continue;
		}
		bp = LL_skipcode(src);
		do {
			*s++ = *src++;
		} while (src < bp);
	} while (src < src1);
	free(src1 - i);
	for (j = 0; j < MAX_SECTIONS; j++) {
		if (sectable[j]. name[0] == '\0')
			break;
		c = sectable[j]. align0;
		if (c == 0xff)
			c = sectable[j]. align1;
		if (dest0 + 8 > dest1) {
			dest0 == NULL;
			goto overrun;
		}
		dest0[0] = REM_4B;
		dest0[1] = 0; /* set section align */
		dest0[2] = j; /* section */
		dest0[3] = c;
		c = 0;
		if (sectable[j]. name[0] == '. ') {
			if (sectable[j]. name[1] == 't')
				c = 1; /* . text */
			if (sectable[j]. name[1] == 'd')
				c = 2; /* . data */
			if (sectable[j]. name[1] == 'b')
				c = 3; /* . bss */
			if (sectable[j]. name[1] == 'a')
				c = 0x70; /* . absolute */
		}
		dest0[4] = REM_4B;
		dest0[5] = 1; /* set section flags */
		dest0[6] = j; /* section */
		dest0[7] = c;
		dest0 += 8;
	}
	if (dest0 + 11 > dest1) {
		dest0 == NULL;
		goto overrun;
	}
	if (status->file_len > 18 * 255 - 1)
		status->file_len = 18 * 255 - 1;
	dest0[0] = REM_3B;
	dest0[1] = 2; /* set format */
	dest0[2] = status->format;
	dest0[3] = REM_8B;
	dest0[4] = 0; /* file */
	dest0[5] = status->file_len;
	dest0[6] = 0;
	put4b((int) status->file_p, &dest0[7]);
	dest0 += 11;

overrun:
	free(decode);
	free(labelbuf0);
	free(locallabelbuf0);
	free(status->expression);
	free(status->mem_expr);
	free(status);
	free(ifdef->bp0);
	for (i = 0; i < 9; i++)
		free(ifdef->expr[i]);
	free(ifdef);
	free(labelflags);
	free(sectable);
	return dest0;
}

UCHAR *flush_bp(int len, UCHAR *buf, UCHAR *dest0, UCHAR *dest1, struct STR_IFDEFBUF *ifdef)
{
	int j, k;
	UCHAR *s, c;
	if (dest0 + len > dest1)
		dest0 = NULL;
	if (dest0 == NULL)
		goto fin;
	for (j = 0; j < len; ) {
		c = buf[j++];
		if (c == 0x2d || c == 0x0e) {
			/* label define */
			dest0[0] = c;
			dest0[1] = c = buf[j];
			dest0[2] = buf[j + 1];
			j += 2;
			dest0 += 3;
			while (c) {
				c -= 2;
				*dest0++ = buf[j++];
			}
			continue;
		}

	//	if (c == 0x30)
	//		continue;
		if (0x31 <= c && c <= 0x37) {
			*dest0++ = c;
			c -= 0x30;
			do {
				*dest0++ = buf[j++];
			} while (--c);
			continue;
		}
		if (c == 0x59) {
			/* TIMES microcode */
			dest0[0] = 0x59;
			s = ifdef->expr[8];
			k = ifdef->dat[8];
			if (dest0 + len + k + 4 > dest1)
				dest0 = NULL;
			if (dest0 == NULL)
				goto fin;
			put4b(-1, dest0 + 1); /* 길이 부정 */
			dest0 += 5;
			c = 5; /* len 출력 */
			do {
				*dest0++ = buf[j++];
			} while (--c);
			do {
				*dest0++ = *s++;
			} while (--k);
			continue;
		}
		if (0x78 <= c && c <= 0x7f) {
			c &= 0x07;
			k = ifdef->dat[c];
			s = ifdef->expr[c];
			if (ifdef->vb[c] == 0x7f) {
				*dest0++ = c | 0x78;
				continue;
			}
			c = ifdef->vb[c];
			if ((c & 0x1f) == 0x00)
				continue;
			if ((c & 0x80) == 0 /* const */) {
				if (dest0 + len + (c & 0x1f) > dest1)
					dest0 = NULL;
				if (dest0 == NULL)
					goto fin;
				*dest0++ = (c &= 0x1f) | 0x30;
				do {
					*dest0++ = k & 0xff;
					k >>= 8;
				} while (--c);
				continue;
			}
			/* expr */
			if (dest0 + len + k > dest1)
				dest0 = NULL;
			if (dest0 == NULL)
				goto fin;
			dest0[0] = (c & 0x1f) + 0x37; /* 38~3b */
			dest0[1] = (c >> 5) & 0x03;
			dest0 += 2;
			do {
				*dest0++ = *s++;
			} while (--k);
			continue;
		}
		if (0xe0 <= c && c <= 0xef) {
			/* 1바이트 리마크 */
			*dest0++ = c;
			continue;
		}
		#if (DEBUG)
			/* error */
			fprintf(stderr, "flush_bp:%02X\n", c);
			break;
		#endif
	}
fin:
	return dest0;
}


#define MAX_LISTLEN		32

UCHAR *output(UCHAR *src0, UCHAR *src1, UCHAR *dest0, UCHAR *dest1, UCHAR *list0, UCHAR *list1)
/* dest1를 돌려준다(NULL라면 overflow) */
/* list가 넘쳐도 속행 */
{
	int len, linecount = 0, srcl, i, addr, secno, file_len, g_symbols = 0, e_symbols = 0;
	struct STR_OUTPUT_SECTION *sectable = malloc(MAX_SECTIONS * sizeof (struct STR_OUTPUT_SECTION));
	UCHAR *srcp, *file_p, *string0, *dest = dest0;
	UCHAR *lbuf0 = malloc(1024), *lbuf;
	UCHAR *ebuf0 = malloc(32), *ebuf; /* 에러 버퍼 */

	UCHAR c, status, adrflag, cc, format, file_aux;
		/* 0:최초, 1:주소 출력전, 2:주소 출력 후(바이트열출력중), 3:바이트열출력중&소스 출력이 끝난 상태 */

	/* align 정보 검색 */
	srcp = src0;
	secno = 0;
	for (i = 0; i < MAX_SECTIONS; i++) {
		sectable[i]. relocs = 0;
		sectable[i]. flags = 0; /* invalid */
	}
	do {
		if (srcp[0] == REM_4B) {
			if (srcp[1] == 0)
				sectable[srcp[2]]. align = srcp[3]; /* set section align */
			if (srcp[1] == 1)
				sectable[srcp[2]]. flags = srcp[3]; /* set section flags */
		}
		if (srcp[0] == REM_3B) {
			if (srcp[1] == 1) {
				/* start section */
				secno = srcp[2];
			}
			if (srcp[1] == 2) {
				/* set format */
				format = srcp[2];
			}
		}
		if (srcp[0] == REM_8B) {
			if (srcp[1] == 0) { /* file */
				file_len = srcp[2];
				file_p = (UCHAR *) get4b(&srcp[4]);
				file_aux = (file_len + (1 + 17)) / 18;
			}
			if (srcp[1] == 1)
				g_symbols++; /* GLOBAL */
			if (srcp[1] == 2) { /* EXTERN */
				e_symbols++;
				/* 번호와 COFF 심볼 번호와의 대응표를 만든다...필요는 없다 */
				/* EXTERN 심볼은, 라벨 번호로부터 정수를 빼는 것만으로 COFF 심볼 번호가 된다 */
			}
		}
		if (0x2e <= srcp[0] && srcp[0] <= 0x2f) {
			/* need relocation */
			sectable[secno]. relocs++;
		}
		srcp = LL_skipcode(srcp);
	} while (srcp < src1);

	/* 바이너리(binary) 출력 */
	if (format == 1) { /* WCOFF */
		static UCHAR header[140] = {
			/* file header */
			0x4c, 0x01, /* signature */
			0x03, 0x00, /* sections == 3 */
			0, 0, 0, 0, /* time & date */
			0, 0, 0, 0, /* +0x08: symboltable */
			0, 0, 0, 0, /* +0x0c: sizeof (symboltable) / 18 */
			0x00, 0x00, /* no optional header */
			0x00, 0x00, /* flags */

			/* section header (. text) */
			'. ', 't', 'e', 'x', 't', 0, 0, 0, /* name */
			0, 0, 0, 0, /* paddr (section_text - section_text) */
			0, 0, 0, 0, /* vaddr == 0 */
			0, 0, 0, 0, /* +0x24: sizeof (section_text) */
			0, 0, 0, 0, /* +0x28: section_text */
			0, 0, 0, 0, /* +0x2c: reloctab_text */
			0, 0, 0, 0, /* line number == 0 */
			0, 0, /* +0x34: sizeof (reloctab_text) / 10 */
			0, 0, /* sizeof (line_number) == 0 */
			0x20, 0x00, 0x10, 0x60, /* +0x38: flags, default_align = 1 */

			/* section header (. data) */
			'. ', 'd', 'a', 't', 'a', 0, 0, 0, /* name */
			0, 0, 0, 0, /* paddr (section_data - section_text) */
			0, 0, 0, 0, /* vaddr == 0 */
			0, 0, 0, 0, /* +0x4c: sizeof (section_data) */
			0, 0, 0, 0, /* +0x50: section_data */
			0, 0, 0, 0, /* +0x54: reloctab_data */
			0, 0, 0, 0, /* line number == 0 */
			0, 0, /* +0x5c: sizeof (reloctab_data) / 10 */
			0, 0, /* sizeof (line_number) == 0 */
			0x40, 0x00, 0x10, 0xc0, /* +0x60: flags, default_align = 1 */

			/* section header (. bss) */
			'. ', 'b', 's', 's', 0, 0, 0, 0, /* name */
			0, 0, 0, 0, /* paddr (section_bss - section_text) */
			0, 0, 0, 0, /* vaddr == 0 */
			0, 0, 0, 0, /* +0x74: sizeof (section_bss) */
			0, 0, 0, 0, /* section_bss == 0 */
			0, 0, 0, 0, /* reloctab_bss == 0 */
			0, 0, 0, 0, /* line number == 0 */
			0, 0, /* sizeof (reloctab_data) / 10 == 0 */
			0, 0, /* sizeof (line_number) == 0 */
			0x80, 0x00, 0x10, 0xc0 /* +0x88: flags, default_align = 1 */
		};
		if (dest + sizeof (header) > dest1) {
			dest = NULL;
			goto error;
		}
		for (i = 0; i < sizeof (header); i++)
			dest[i] = header[i];
		dest += sizeof (header);
	}
	srcp = src0;
	secno = 0;
	len = 0;
	do {
		c = *srcp;
		if (SHORT_DB1 <= c && c <= SHORT_DB4) {
			srcp++;
			c -= SHORT_DB0;
			if (dest + 8 > dest1) {
				dest = NULL;
				goto error;
			}
			if (format == 0 /* BIN */)
				goto dest_out_skip;
			if (format == 1 && 1 <= sectable[secno]. flags && sectable[secno]. flags <= 2) {
dest_out_skip:
				do {
					*dest++ = *srcp++;
				} while (--c);
				continue;
			}
			if (format == 1 && sectable[secno]. flags == 3)
				len += c; /* bss */
			srcp += c; /* bss or absolute */
			continue;
		}
		if (c == REM_3B && srcp[1] == 1) {
			/* start section */
			if (format == 1) { /* WCOFF */
				if (1 <= sectable[secno]. flags && sectable[secno]. flags <= 2)
					put4b(dest - sectable[secno]. d0, &dest0[sectable[secno]. flags * 40 - 4]);
			//	if (sectable[secno]. flags == 3)
			//		put4b(len, &dest0[0x74]);
			}
			sectable[secno = srcp[2]]. p = srcp;
			sectable[secno]. d0 = dest;
			if (format == 0) { /* BIN */
				if (sectable[secno]. align > 0) {
					i = 1 << (sectable[secno]. align);
					while (((int) dest) & (i - 1)) {
						if (dest >= dest1) {
							dest = NULL;
							goto error;
						}
						*dest++ = '\0';
					}
				}
			}
			if (format == 1) { /* WCOFF */
				if (1 <= sectable[secno]. flags && sectable[secno]. flags <= 3) {
					if (sectable[secno]. flags ! = 3)
						put4b(dest - dest0, &dest0[sectable[secno]. flags * 40]);
					dest0[sectable[secno]. flags * 40 + 18] = sectable[secno]. align << 4;
				}
			}
		}
		srcp = LL_skipcode(srcp);
	} while (srcp < src1);
	if (format == 1) { /* WCOFF */
		static UCHAR common_symbols0[18 * 1 - 1] = {
			'. ', 'f', 'i', 'l', 'e', 0, 0, 0, /* name */
			0, 0, 0, 0, /* value */
			0xfe, 0xff, /* debugging symbol */
			0, 0, /* T_NULL */
			103 /* , 0 */ /* file name, numaux = 0 */
		};
		static UCHAR common_symbols1[18 * 6] = {
			'. ', 't', 'e', 'x', 't', 0, 0, 0, /* name */
			0, 0, 0, 0, /* value */
			0x01, 0x00, /* section 1 */
			0, 0, /* T_NULL */
			3, 1, /* private symbol, numaux = 1 */
			0, 0, 0, 0, /* +0x12: sizeof (section_text) */
			0, 0, /* +0x16: sizeof (reloctab_text) / 10 */
			0, 0, /* sizeof (line_number) == 0 */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

			'. ', 'd', 'a', 't', 'a', 0, 0, 0, /* name */
			0, 0, 0, 0, /* value */
			0x02, 0x00, /* section 2 */
			0, 0, /* T_NULL */
			3, 1, /* private symbol, numaux = 1 */
			0, 0, 0, 0, /* +0x36: sizeof (section_text) */
			0, 0, /* +0x3a: sizeof (reloctab_text) / 10 */
			0, 0, /* sizeof (line_number) == 0 */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

			'. ', 'b', 's', 's', 0, 0, 0, 0, /* name */
			0, 0, 0, 0, /* value */
			0x03, 0x00, /* section 3 */
			0, 0, /* T_NULL */
			3, 1, /* private symbol, numaux = 1 */
			0, 0, 0, 0, /* +0x5a: sizeof (section_bss) */
			0, 0, /* sizeof (reloctab_text) / 10 == 0 */
			0, 0, /* sizeof (line_number) == 0 */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};
		/* 마지막 섹션의 사이즈 write */
		if (1 <= sectable[secno]. flags && sectable[secno]. flags <= 2)
			put4b(dest - sectable[secno]. d0, &dest0[sectable[secno]. flags * 40 - 4]);
		put4b(len, &dest0[0x74]);

		/* relocation tables (allocate only) */
		/* relocation 정보를 가지는 섹션은 2개 밖에 없다 */
		for (i = 0; i < MAX_SECTIONS; i++) {
			if (1 <= sectable[i]. flags && sectable[i]. flags <= 2) {
				sectable[i]. reloc_p = dest;
				put4b(dest - dest0, &dest0[sectable[i]. flags * 40 + 4]); /* reloctab */
				dest0[sectable[i]. flags * 40 + 12] = sectable[i]. relocs & 0xff;
				dest0[sectable[i]. flags * 40 + 13] = (sectable[i]. relocs >> 8) & 0xff;
				dest += sectable[i]. relocs * 10;
			}
		}
		if (dest > dest1) {
			dest = NULL;
			goto error;
		}

		/* symbol table */
		put4b(dest - dest0, &dest0[0x08]);
		put4b(i = file_aux + 7 + e_symbols + g_symbols, &dest0[0x0c]);
		if (dest + i * 18 > dest1) {
			dest = NULL;
			goto error;
		}
		for (i = 0; i < sizeof (common_symbols0); i++)
			dest[i] = common_symbols0[i];
		dest[17] = file_aux;
		for (i = 0; i < 18; i++)
			dest[file_aux * 18 + i] = '\0';
		for (i = 0; i < file_len; i++)
			dest[18 + i] = file_p[i];
		dest += file_aux * 18 + 18;
		for (i = 0; i < sizeof (common_symbols1); i++)
			dest[i] = common_symbols1[i];
		put4b(get4b(&dest0[0x24]), &dest[0x12]);
		put4b(get4b(&dest0[0x34]), &dest[0x16]);
		put4b(get4b(&dest0[0x4c]), &dest[0x36]);
		put4b(get4b(&dest0[0x5c]), &dest[0x3a]);
		put4b(get4b(&dest0[0x74]), &dest[0x5a]);
		file_p = dest + sizeof (common_symbols1);
		dest = file_p + (e_symbols + g_symbols) * 18;
		string0 = dest;
		dest += 4;
		/* 이하의 루프로 이전과 심볼 테이블을 만들 수 있다 */
		srcp = src0;
		secno = 0;
		addr = 0;
		g_symbols = e_symbols * 18;
		do {
			c = *srcp;
			if (SHORT_DB1 <= c && c <= SHORT_DB4) {
				srcp++;
				c -= SHORT_DB0;
				srcp += c;
				addr += c;
				continue;
			}
			if (c == REM_3B && srcp[1] == 1) {
				/* start section */
				secno = srcp[2];
				addr = 0;
			}
			if (srcp[0] == REM_8B) {
				if (1 <= srcp[1] && srcp[1] <= 2) {
					len = srcp[2] | srcp[3] << 8;
					lbuf = (UCHAR *) get4b(&srcp[4]);
					ebuf = file_p + ((srcp[10] | srcp[11] << 8) - E_LABEL0) * 18; /* EXTERN */
					if (srcp[1] == 1) { /* GLOBAL */
						ebuf = file_p + g_symbols;
						g_symbols += 18;
					}
					for (i = 0; i < 18; i++)
						ebuf[i] = '\0';
					ebuf[16] = 2; /* GLOBAL or EXTERN */
					if (len <= 8) {
						for (i = 0; i < len; i++)
							ebuf[i] = lbuf[i];
					} else {
						put4b(dest - string0, &ebuf[4]);
						if (dest + len + 1 > dest1) {
							dest = NULL;
							goto error;
						}
						do {
							*dest++ = *lbuf++;
						} while (--len);
						*dest++ = '\0';
					}
					if (srcp[1] == 1) { /* GLOBAL */
						ebuf[ 8] = srcp[12];
						ebuf[ 9] = srcp[13];
						ebuf[10] = srcp[14];
						ebuf[11] = srcp[15];
						ebuf[12] = srcp[10];
						ebuf[13] = srcp[11];
					}
				}
			}
			/* 이전 */
			if (0x2e <= srcp[0] && srcp[0] <= 0x2f &&
				1 <= sectable[secno]. flags && sectable[secno]. flags <= 2) {
				/* 0x2e : 절대 보정 요구, 0x2f : 상대 보정 요구 */
				put4b(addr, sectable[secno]. reloc_p);
				i = srcp[2] | srcp[3] << 8;
				/* . text == file_aux + 1 */
				/* . data == file_aux + 3 */
				/* . bss  == file_aux + 5 */
				/* ext0 == file_aux + 7 */
				if (i >= E_LABEL0)
					i += 7 - E_LABEL0;
				else
					i = i * 2 - 1;
				put4b(i + file_aux, sectable[secno]. reloc_p + 4);
				sectable[secno]. reloc_p[8] = 0x06; /* absolute */
				sectable[secno]. reloc_p[9] = 0;
				if (srcp[0] == 0x2f) {
					sectable[secno]. reloc_p[8] = 0x14; /* relative */
					i = get4b(&dest0[sectable[secno]. flags * 40]) + addr;
					#if (DEBUG)
						if (i < 0 || i + 4 > dest - lbuf) {
							fprintf(stderr, "output:relative relocation error(1)!  i = 0x%08X\n", i);
							goto skip_relative_relocation;
						}
					#endif
					put4b(get4b(&dest0[i]) + addr + 4, &dest0[i]);
				}
skip_relative_relocation:
				sectable[secno]. reloc_p += 10;
			}
			srcp = LL_skipcode(srcp);
		} while (srcp < src1);
		put4b(dest - string0, string0);
	}

	/* 리스트 출력 */
	status = 0;
	secno = 0;
	addr = 0;
	ebuf = ebuf0;
	for (;;) {
		lbuf = lbuf0;
		c = *src0;
		if (c == REM_3B && src0[1] == 0) {
			sectable[secno]. p = src0;
			sectable[secno]. addr = addr;
			secno = src0[2];
			src0 = sectable[secno]. p;
			addr = sectable[secno]. addr;
		}

		if (c == 0xf7) {	/* switch로 쓰자 lcc-win32이 다운 */
			/* line start */
			if (status == 1) {
				len = -9;
				status = 2;
			}
			if (status == 2) {
				/* (MAX_LISTLEN - len)개의 스페이스를 출력 */
				if (list0 + (MAX_LISTLEN - len + srcl) >= list1) {
					*list0 = '\0';
					list0 = NULL;
				}
				if (list0) {
					do {
						*list0++ = ' ';
						len++;
					} while (len < MAX_LISTLEN);
					while (srcl--)
						*list0++ = *srcp++;
				}
			}
			if (status == 3)
				*lbuf++ = '\n';
			if ((len = ebuf - ebuf0) ! = 0) {
				/* 에러 출력 */
				static char *errmsg[] = {
					"      >> [ERROR #001] syntax error. \n",
					"      >> [ERROR #002] parameter error. \n",
					"      >> [ERROR #003] data size error. \n",
					"      >> [ERROR #004] data type error. \n",
					"      >> [ERROR #005] addressing error. \n",
					"      >> [ERROR #006] TIMES error. \n",
					"      >> [ERROR #007] label definition error. \n",
					"      >> [ERROR #008] data range error. \n",
					"      >> [ERROR #009] expression error. \n",	/* 부정치 에러(delta ! = 0) */
					"      >> [ERROR #010] expression error. \n",
					"      >> [ERROR #011] expression error. \n",
					"      >> [ERROR #012] expression error. \n" /* 미정의 라벨 참조 */
				};
				nask_errors += len;
				for (i = 0; i < len; i++) {
					ebuf = errmsg[ebuf0[i] - 0xe1];
					while ((*lbuf++ = *ebuf++) ! = '\n');
				}
				ebuf = ebuf0;
			}
			srcl = get4b(&src0[1]);
			srcp = (UCHAR *) get4b(&src0[5]);
			if (srcl) {
				setdec(++linecount, 6, &lbuf[0]);
				lbuf[6] = ' ';
				lbuf += 7;
			}
			len = 0;
			src0 += 9;
			status = 1;
			adrflag = 0;
		} else if (c == 0x5a) {
			/* ORG */
			addr = get4b(&src0[1]);
			src0 += 5;
		} else if (c == 0x68) {
			src0 += 2; /* 건너뛴다 */
		} else if (c == REM_ADDR) {
			status = 2;
			sethex0(addr, 8, &lbuf[0]);
			lbuf[8] = ' ';
			lbuf += 9;
			src0++;
			adrflag = 1;
		} else if (SHORT_DB1 <= c && c <= SHORT_DB4) {
			if (status == 1) {
				for (i = 0; i < 9; i++)
					*lbuf++ = ' ';
				status = 2;
			}
			if (len + 1 + (c - 0x30) * 2 > MAX_LISTLEN) {
			//	if (status == 1) {
			//		len = -9;
			//		status = 2;
			//	}
				if (status == 2) {
					/* (MAX_LISTLEN - len)개의 스페이스를 출력 */
					if (list0 + (MAX_LISTLEN - len + srcl) >= list1) {
						*list0 = '\0';
						list0 = NULL;
					}
					if (list0) {
						do {
							*list0++ = ' ';
							len++;
						} while (len < MAX_LISTLEN);
						while (srcl--)
							*list0++ = *srcp++;
					}
				} else
					*lbuf++ = '\n';
				for (i = 0; i < 7 + 9; i++)
					lbuf[i] = ' ';
				if (adrflag)
					sethex0(addr, 8, &lbuf[7]);
				lbuf += 9 + 7;
				status = 3;
				len = 0;
			}
			src0++;
			for (i = c - SHORT_DB1 /* 0x31 */; i >= 0; i--) {
				sethex0(src0[i], 2, &lbuf[0]);
				lbuf += 2;
				len += 2;
				addr++;
			}
			src0 += c - SHORT_DB0; /* - 0x30 */
			*lbuf++ = ' ';
			len++;
		} else if (0x2e <= c && c <= 0x2f && 1 <= sectable[secno]. flags && sectable[secno]. flags <= 2) { /* reloc data */
			if (status == 1) {
				for (i = 0; i < 9; i++)
					*lbuf++ = ' ';
				status = 2;
			}
			if (len + 11 > MAX_LISTLEN) {
			//	if (status == 1) {
			//		len = -9;
			//		status = 2;
			//	}
				if (status == 2) {
					/* (MAX_LISTLEN - len)개의 스페이스를 출력 */
					if (list0 + (MAX_LISTLEN - len + srcl) >= list1) {
						*list0 = '\0';
						list0 = NULL;
					}
					if (list0) {
						do {
							*list0++ = ' ';
							len++;
						} while (len < MAX_LISTLEN);
						while (srcl--)
							*list0++ = *srcp++;
					}
				} else
					*lbuf++ = '\n';
				for (i = 0; i < 7 + 9; i++)
					lbuf[i] = ' ';
				if (adrflag)
					sethex0(addr, 8, &lbuf[7]);
				lbuf += 9 + 7;
				status = 3;
				len = 0;
			}
			src0 += 9;
			i = get4b(&dest0[sectable[secno]. flags * 40]) + addr;
			lbuf[0] = '[';
			sethex0(get4b(&dest0[i]), 8, &lbuf[1]);
			lbuf[9] = ']';
			lbuf[10] = ' ';
			lbuf += 11;
			len += 11;
			addr += 4;
		} else if (c == 0x0c) {
			/* EQU (1) */
			/* 반드시 status == 1 */
			lbuf[0] = ' ';
			lbuf[1] = '=';
			lbuf[2] = ' ';
			sethex0(get4b(&src0[1]), 8, &lbuf[3]);
			lbuf[11] = ' ';
			len = -9 + 12;
			lbuf += 12;
			src0 += 5;
			status = 2;
		} else if (c == 0x0d) {
			/* EQU (2) */
			/* 반드시 status == 1 */
			lbuf[0] = ' ';
			lbuf[1] = '=';
			lbuf[2] = ' ';
			lbuf[3] = '[';
			sethex0(get4b(&src0[1]), 8, &lbuf[4]);
			lbuf[12] = ']';
			lbuf[13] = ' ';
			len = -9 + 14;
			lbuf += 14;
			src0 += 5;
			status = 2;
		} else if (0xe1 <= c && c <= 0xec) {
			/* 에러 코드 */
			*ebuf++ = c;
			src0++;
		} else if (c == 0x30 /* SHORT_DB0 */) {
			src0++; /* 건너뛴다 */
		} else if (0xf0 <= c && c <= 0xf7) {
			src0 += c - (0xf0 - 2);
		} else if (c == 0x2c) {
			src0 += 4;
		} else {
			#if (DEBUG)
				fprintf(stderr, "output:%02X\n", c);
			#endif
			src0++;
		}
		i = lbuf - lbuf0;
		if (list0 + i >= list1) {
			*list0 = '\0';
			list0 = NULL;
		}
		if (list0) {
			lbuf = lbuf0;
			while (i--)
				*list0++ = *lbuf++;
		}
		if (c == 0xf7) {
			cc = 0;
			for (i = -8; i < 0; i++)
				cc |= src0[i]; 
			if (cc == 0)
				break;
		}
	}
error:
	list1[1] = 1; /* over */
	if (list0) {
		*list0 = '\0';
		list1[1] = 0; /* enough */
	}
	free(lbuf0);
	free(ebuf0);
	free(sectable);
	return dest;
}

UCHAR *putprefix(UCHAR *dest0, UCHAR *dest1, int prefix, int bits, int opt)
{
	/* prefix:
		bit 0:REP/REPZ
		bit 1:REPNZ
		bit 2:LOCK
		bit 3:ADRSIZ
		bit 4:OPRSIZ
		bit 5:ES
		bit 6:CS
		bit 7:SS
		bit 8:DS
		bit 9:FS
		bit10:GS
		bit16:A16(명시)
		bit17:A32(명시)
		bit20:O16(명시)
		bit21:O32(명시)
		bit24:A16(암묵)
		bit25:A32(암묵)
		bit28:O16(암묵)
		bit29:O32(암묵)
	*/
	/* bits는 16이나 32. bit0는 DS, bit1는 SS */
	/*
		우선 암묵의 프리픽스를 bits를 사용해 말소
		opt >= 1이라면 SS나 DS도 말소
		만약 명시 프리픽스가 있으면 대응하는 종류의 암묵 프리픽스는 캔슬
		명시 프리픽스가 없으면 암묵을 명시에 카피
		명시를 사용해 ADRSIZ, OPRSIZ를 반전
		주소 출력 마크를 출력
		하위로부터 참조해 프리픽스 열을 출력(bit0-10)
	*/
	UCHAR buf[3 * 16 + 1], *bp;
	static UCHAR code[11] = {
		0xf3 /* bit 0:REP/REPZ */,
		0xf2 /* bit 1:REPNZ */,
		0xf0 /* bit 2:LOCK */,
		0x67 /* bit 3:ADRSIZ */,
		0x66 /* bit 4:OPRSIZ */,
		0x26 /* bit 5:ES */,
		0x2e /* bit 6:CS */,
		0x36 /* bit 7:SS */,
		0x3e /* bit 8:DS */,
		0x64 /* bit 9:FS */,
		0x65 /* bit10:GS */
	};
	int i, j;
	if (opt >= 1) {
		if (bits & 1)
			prefix &= ~0x00000100; /* DS */
		if (bits & 2)
			prefix &= ~0x00000080; /* SS */
	}
	if ((prefix & 0x00030000) == 0) /* A16(명시), A32(명시) */
		prefix |= (prefix & 0x03000000) >> 8;
	if ((prefix & 0x00300000) == 0) /* O16(명시), O32(명시) */
		prefix |= (prefix & 0x30000000) >> 8;
	if (bits & 16)
		prefix &= ~0x00110000; /* A16(명시), O16(명시) */
	if (bits & 32)
		prefix &= ~0x00220000; /* A32(명시), O32(명시) */
	if (prefix & 0x00030000) /* A16(명시), A32(명시) */
		prefix ^= 0x00000008; /* ADRSIZ */
	if (prefix & 0x00300000) /* O16(명시), O32(명시) */
		prefix ^= 0x00000010; /* OPRSIZ */
	buf[0] = REM_ADDR; /* 0xe0 */
	bp = &buf[1];
	for (i = 0; i < 11; i++) {
		if (prefix & (1 << i)) {
			bp[0] = SHORT_DB1; /* 0x31 */
			bp[1] = code[i];
			bp += 2;
		}
	}
	i = bp - buf;
	if (dest0 + i > dest1)
		dest0 = NULL;
	if (dest0) {
		for (j = 0; j < i; j++)
			dest0[j] = buf[j];
		dest0 += i;
	}
	return dest0;
}

static struct INST_TABLE instruction[] = {
	{ "AAA",		SUP_8086,	NO_PARAM, 1, 0x37 },
	{ "AAD",		SUP_8086,	OPE_AAMD, 0xd5, 10 },
	{ "AAS",		SUP_8086,	NO_PARAM, 1, 0x3f },
	{ "AAM",		SUP_8086,	OPE_AAMD, 0xd4, 10 },
	{ "ADC",		SUP_8086,	OPE_ADD, 0x87, 0x11, 0x02 << 3 },
	{ "ADD",		SUP_8086,	OPE_ADD, 0x87, 0x11, 0x00 << 3 },
	{ "AND",		SUP_8086,	OPE_ADD, 0x87, 0x11, 0x04 << 3 },
	{ "ALIGN",		SUP_8086,	OPE_ALIGN, 0x90 },
	{ "ALIGNB",		SUP_8086,	OPE_ALIGN, 0x00 },
	{ "ARPL",		SUP_80286P,	OPE_MR, 0xa2, 0x21, 0x63 }, /* w, ws no-w no-o16/o32 */
	{ "BOUND",		SUP_80186,	OPE_RM, 0x86, 0x01, 0x62 }, /* dw, s no-w */
	{ "BSF",		SUP_i386,	OPE_RM, 0x86, 0x02, 0x0f, 0xbc }, /* dw, s no-w */
	{ "BSR",		SUP_i386,	OPE_RM, 0x86, 0x02, 0x0f, 0xbd }, /* dw, s no-w */
	{ "BSWAP",		SUP_i486,	OPE_M, 0x14, 0xa1, 0x0f }, /* d no-w no-o16/o32 no-mem */
	{ "BT",			SUP_i386,	OPE_BT, 0x04 << 3 },
	{ "BTC",		SUP_i386,	OPE_BT, 0x07 << 3 },
	{ "BTR",		SUP_i386,	OPE_BT, 0x06 << 3 },
	{ "BTS",		SUP_i386,	OPE_BT, 0x05 << 3 },
	{ "CALL",		SUP_8086,	OPE_JMP, 0x02 << 3, 0xe8, 0x9a, 0x00 },
	{ "CBW",		SUP_8086,	NO_PARAM, 1 | OPE16, 0x98 },
	{ "CDQ",		SUP_i386,	NO_PARAM, 1 | OPE32, 0x99 },
	{ "CLC",		SUP_8086,	NO_PARAM, 1, 0xf8 },
	{ "CLD",		SUP_8086,	NO_PARAM, 1, 0xfc },
	{ "CLI",		SUP_8086,	NO_PARAM, 1, 0xfa },
	{ "CLTS",		SUP_80286P,	NO_PARAM, 2, 0x0f, 0x06 },
	{ "CMC",		SUP_8086,	NO_PARAM, 1, 0xf5 },
	{ "CMP",		SUP_8086,	OPE_ADD, 0x87, 0x11, 0x07 << 3 },
	{ "CMPSB",		SUP_8086,	NO_PARAM, 1 | DEF_DS, 0xa6 },
	{ "CMPSD",		SUP_i386,	NO_PARAM, 1 | OPE32 | DEF_DS, 0xa7 },
	{ "CMPSW",		SUP_8086,	NO_PARAM, 1 | OPE16 | DEF_DS, 0xa7 },
	{ "CMPXCHG",	SUP_i486,	OPE_MR, 0x87, 0x12, 0x0f, 0xb0 }, /* bwd, s w0 */
	{ "CS",			SUP_8086,	PREFIX,	0x06 },
	{ "CWD",		SUP_8086,	NO_PARAM, 1 | OPE16, 0x99 },
	{ "CWDE",		SUP_i386,	NO_PARAM, 1 | OPE32, 0x98 },
	{ "DAA",		SUP_8086,	NO_PARAM, 1, 0x27 },
	{ "DAS",		SUP_8086,	NO_PARAM, 1, 0x2f },
	{ "DB",			SUP_8086,	OPE_DB, 1, 0x9e },
	{ "DD",			SUP_8086,	OPE_DB,	4, 0x9d },
	{ "DEC",		SUP_8086,	OPE_PUSH, 0x01, 0x48, 0xfe },
	{ "DIV",		SUP_8086,	OPE_M, 0x67, 0x11, 0xf6 }, /* bwd w0 */
	{ "DQ",			SUP_8086,	OPE_DB, 8 },
	{ "DS",			SUP_8086,	PREFIX, 0x08 },
	{ "DT",			SUP_8086,	OPE_DB, 10 },
	{ "DW",			SUP_8086,	OPE_DB, 2, 0x9b },
	{ "END",		SUP_8086,	OPE_END },
	{ "ENTER",		SUP_80186,	OPE_ENTER },
	{ "EQU",		SUP_8086,	OPE_EQU },
	{ "ES",			SUP_8086,	PREFIX, 0x05 },
	{ "EXTERN",		SUP_8086,	OPE_GLOBAL, 2 },
	{ "F2XM1",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xf0 },
	{ "FABS",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xE1 },
	{ "FADD",		SUP_8086,	OPE_FPU, 0, 0, 0x80 /* D8 /0 */, 0x84 /* DC /0 */, 0, 0x80 /* D8 /0 */, 0x84 /* DC /0 */ },
	{ "FADDP",		SUP_8086,	OPE_FPUP, 0x06 /* DE /0 */ },
	{ "FBLD",		SUP_8086,	OPE_FPU, 0x07 /* mem10 */, 0, 0, 0, 0xa7 /* DF /4 */ },
	{ "FBSTP",		SUP_8086,	OPE_FPU, 0x07 /* mem10 */, 0, 0, 0, 0xb7 /* DF /6 */ },
	{ "FCHS",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xe0 },
	{ "FCLEX",		SUP_8086,	NO_PARAM, 3, 0x9b, 0xdb, 0xe2 },
	{ "FCOM",		SUP_8086,	OPE_FPU, 0, 0, 0x90 /* D8 /2 */, 0x94 /* DC /2 */, 0, 0x90 /* D8 /2 */, 0 },
	{ "FCOMP",		SUP_8086,	OPE_FPU, 0, 0, 0x98 /* D8 /3 */, 0x9c /* DC /3 */, 0, 0x98 /* D8 /3 */, 0 },
	{ "FCOMPP",		SUP_8086,	NO_PARAM, 2, 0xde, 0xd9 },
	{ "FCOS",		SUP_i386,	NO_PARAM, 2, 0xd9, 0xff },
	{ "FDECSTP",	SUP_8086,	NO_PARAM, 2, 0xd9, 0xf6 },
	{ "FDISI",		SUP_8086,	NO_PARAM, 3, 0x9b, 0xdb, 0xe1 },
	{ "FDIV",		SUP_8086,	OPE_FPU, 0, 0, 0xb0 /* D8 /6 */, 0xb4 /* DC /6 */, 0, 0xb0 /* D8 /6 */, 0xbc /* DC /7 */ },
	{ "FDIVP",		SUP_8086,	OPE_FPUP, 0x3e /* DE /7 */ },
	{ "FDIVR",		SUP_8086,	OPE_FPU, 0, 0, 0xb8 /* D8 /7 */, 0xbc /* DC /7 */, 0, 0xb8 /* D8 /7 */, 0xb4 /* DC /6 */ },
	{ "FDIVRP",		SUP_8086,	OPE_FPUP, 0x36 /* DE /6 */ },
	{ "FENI",		SUP_8086,	NO_PARAM, 3, 0x9b, 0xdb, 0xe0 },
	{ "FFREE",		SUP_8086,	OPE_FPU, 0x08, 0, 0, 0, 0, 0x85 /* DD /0 */ },
	{ "FIADD",		SUP_8086,	OPE_FPU, 0, 0x86 /* DE /0 */, 0x82 /* DA /0 */ },
	{ "FICOM",		SUP_8086,	OPE_FPU, 0, 0x96 /* DE /2 */, 0x92 /* DA /2 */ },
	{ "FICOMP",		SUP_8086,	OPE_FPU, 0, 0x9e /* DE /3 */, 0x9a /* DA /3 */ },
	{ "FIDIV",		SUP_8086,	OPE_FPU, 0, 0xb6 /* DE /6 */, 0xb2 /* DA /6 */ },
	{ "FIDIVR",		SUP_8086,	OPE_FPU, 0, 0xbe /* DE /7 */, 0xba /* DA /7 */ },
	{ "FILD",		SUP_8086,	OPE_FPU, 0, 0x87 /* DF /0 */, 0x83 /* DB /0 */, 0xaf /* DF /5 */ },
	{ "FIMUL",		SUP_8086,	OPE_FPU, 0, 0x8e /* DE /1 */, 0x8a /* DA /1 */ },
	{ "FINCSTP",	SUP_8086,	NO_PARAM, 2, 0xd9, 0xf7 },
	{ "FINIT",		SUP_8086,	NO_PARAM, 3, 0x9b, 0xdb, 0xe3 },
	{ "FIST",		SUP_8086,	OPE_FPU, 0, 0x97 /* DF /2 */, 0x93 /* DB /2 */ },
	{ "FISTP",		SUP_8086,	OPE_FPU, 0, 0x9f /* DF /3 */, 0x9b /* DB /3 */, 0xbf /* DF /7 */ },
	{ "FISUB",		SUP_8086,	OPE_FPU, 0, 0xa6 /* DE /4 */, 0xa2 /* DA /4 */ },
	{ "FISUBR",		SUP_8086,	OPE_FPU, 0, 0xae /* DE /5 */, 0xaa /* DA /5 */ },
	{ "FLD",		SUP_8086,	OPE_FPU, 0x08, 0, 0x81 /* D9 /0 */, 0x85 /* DD /0 */, 0xab /* DB /5 */, 0x81 /* D9 /0 */ },
	{ "FLD1",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xe8 },
	{ "FLDCW",		SUP_8086,	OPE_FPU, 0x04 /* mem2 */, 0xa9 /* D9 /5 */ },
	{ "FLDENV",		SUP_8086,	OPE_M, 0x48, 0x61, 0xd9 }, /* s no-w no-o16/o32 no-reg */
	{ "FLDL2E",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xea },
	{ "FLDL2T",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xe9 },
	{ "FLDLG2",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xec },
	{ "FLDLN2",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xed },
	{ "FLDPI",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xeb },
	{ "FLDZ",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xee },
	{ "FMUL",		SUP_8086,	OPE_FPU, 0, 0, 0x88 /* D8 /1 */, 0x8c /* DC /1 */, 0, 0x88 /* D8 /1 */, 0x8c /* DC /1 */ },
	{ "FMULP",		SUP_8086,	OPE_FPUP, 0x0e /* DE /1 */ },
	{ "FNCLEX",		SUP_8086,	NO_PARAM, 2, 0xdb, 0xe2 },
	{ "FNDISI",		SUP_8086,	NO_PARAM, 2, 0xdb, 0xe1 },
	{ "FNENI",		SUP_8086,	NO_PARAM, 2, 0xdb, 0xe0 },
	{ "FNINIT",		SUP_8086,	NO_PARAM, 2, 0xdb, 0xe3 },
	{ "FNOP",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xd0 },
	{ "FNSAVE",		SUP_8086,	OPE_M, 0x68, 0x61, 0xdd }, /* s no-w no-o16/o32 no-reg */
	{ "FNSTCW",		SUP_8086,	OPE_M, 0x7a, 0x61, 0xd9 }, /* ws no-w no-o16/o32 no-reg */
	{ "FNSTENV",	SUP_8086,	OPE_M, 0x68, 0x61, 0xd9 }, /* s no-w no-o16/o32 no-reg */
	{ "FNSTSW",		SUP_8086,	OPE_FSTSW, 0x0a, 0x61, 0xdd, 0, 0xdf, 0xe0 }, /* ws no-w no-o16/o32 no-reg */
	{ "FPATAN",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xf3 },
	{ "FPTAN",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xf2 },
	{ "FPREM",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xf8 },
	{ "FPREM1",		SUP_i386,	NO_PARAM, 2, 0xd9, 0xf5 },
	{ "FRNDINT",	SUP_8086,	NO_PARAM, 2, 0xd9, 0xfc },
	{ "FRSTOR",		SUP_8086,	OPE_M, 0x48, 0x61, 0xdd }, /* s no-w no-o16/o32 no-reg */
	{ "FS",			SUP_i386,	PREFIX, 0x09 },
	{ "FSAVE",		SUP_8086,	OPE_M, 0x68, 0x62, 0x9b, 0xdd }, /* s no-w no-o16/o32 no-reg */
	{ "FSCALE",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xfd },
	{ "FSETPM",		SUP_80286P,	NO_PARAM, 2, 0xdb, 0xe4 },
	{ "FSIN",		SUP_i386,	NO_PARAM, 2, 0xd9, 0xfe },
	{ "FSINCOS",	SUP_i386,	NO_PARAM, 2, 0xd9, 0xfb },
	{ "FSQRT",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xfa },
	{ "FST",		SUP_8086,	OPE_FPU, 8, 0, 0x91 /* D9 /2 */, 0x95 /* DD /2 */, 0, 0x95 /* DD /2 */ },
	{ "FSTCW",		SUP_8086,	OPE_M, 0x7a, 0x62, 0x9b, 0xd9 }, /* ws no-w no-o16/o32 no-reg */
	{ "FSTENV",		SUP_8086,	OPE_M, 0x68, 0x62, 0x9b, 0xd9 }, /* s no-w no-o16/o32 no-reg */
	{ "FSTP",		SUP_8086,	OPE_FPU, 8, 0, 0x99 /* D9 /3 */, 0x9d /* DD /3 */, 0xbb /* DB /7 */, 0x9d /* DD /3 */ },
	{ "FSTSW",		SUP_8086,	OPE_FSTSW, 0x0a, 0x62, 0x9b, 0xdd, 0x9b, 0xdf, 0xe0 }, /* ws no-w no-o16/o32 no-reg */
	{ "FSUB",		SUP_8086,	OPE_FPU, 0, 0, 0xa0 /* D8 /4 */, 0xa4 /* DC /4 */, 0, 0xa0 /* D8 /4 */, 0xac /* DC /5 */ },
	{ "FSUBP",		SUP_8086,	OPE_FPUP, 0x2e /* DE /5 */ },
	{ "FSUBR",		SUP_8086,	OPE_FPU, 0, 0, 0xa8 /* D8 /5 */, 0xac /* DC /5 */, 0, 0xa8 /* D8 /5 */, 0xa4 /* DC /4 */ },
	{ "FSUBRP",		SUP_8086,	OPE_FPUP, 0x26 /* DE /4 */ },
	{ "FTST",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xe4 },
	{ "FUCOM",		SUP_i386,	OPE_FPU, 0, 0, 0, 0, 0, 0xa5 /* DD /4 */ },
	{ "FUCOMP",		SUP_i386,	OPE_FPU, 0, 0, 0, 0, 0, 0xad /* DD /5 */ },
	{ "FUCOMPP",	SUP_i386,	NO_PARAM, 2, 0xda, 0xe9 },
	{ "FXAM",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xe5 },
	{ "FXCH",		SUP_8086,	OPE_FXCH },
	{ "FXTRACT",	SUP_8086,	NO_PARAM, 2, 0xd9, 0xf4 },
	{ "FYL2X",		SUP_8086,	NO_PARAM, 2, 0xd9, 0xf1 },
	{ "FYL2XP1",	SUP_8086,	NO_PARAM, 2, 0xd9, 0xf9 },
	{ "GLOBAL",		SUP_8086,	OPE_GLOBAL, 1 },
	{ "GS",			SUP_i386,	PREFIX, 0x0a },
	{ "HLT",		SUP_8086,	NO_PARAM, 1, 0xf4 },
	{ "IDIV",		SUP_8086,	OPE_M, 0x77, 0x11, 0xf6 }, /* bwd w0 */
	{ "IMUL",		SUP_8086,	OPE_IMUL, 0x57, 0x11, 0xf6  }, /* bwd w0 */
	{ "IN",			SUP_8086,	OPE_INOUT, 0xe4 },
	{ "INC",		SUP_8086,	OPE_PUSH, 0x00, 0x40, 0xfe },
	{ "INSB",		SUP_8086,	NO_PARAM, 1, 0x6c },
	{ "INSD",		SUP_i386,	NO_PARAM, 1 | OPE32, 0x6d },
	{ "INSW",		SUP_8086,	NO_PARAM, 1 | OPE16, 0x6d },
	{ "INT",		SUP_8086,	OPE_INT },
	{ "INT3",		SUP_8086,	NO_PARAM, 1, 0xcc },
	{ "INTO",		SUP_8086,	NO_PARAM, 1, 0xce },
	{ "INVD",		SUP_i486,	NO_PARAM, 2, 0x0f, 0x08 },
	{ "INVLPG",		SUP_i486,	OPE_M, 0x7e, 0x62, 0x0f, 0x01 }, /* wds no-w no-o16/o32 no-reg */
	{ "IRET",		SUP_8086,	NO_PARAM, 1, 0xcf },
	{ "IRETD",		SUP_i386,	NO_PARAM, 1 | OPE32, 0xcf },
	{ "IRETW",		SUP_8086,	NO_PARAM, 1 | OPE16, 0xcf },
	{ "JA",			SUP_8086,	OPE_JCC, 0x07 },
	{ "JAE",		SUP_8086,	OPE_JCC, 0x03 },
	{ "JB",			SUP_8086,	OPE_JCC, 0x02 },
	{ "JBE",		SUP_8086,	OPE_JCC, 0x06 },
	{ "JC",			SUP_8086,	OPE_JCC, 0x02 },
	{ "JCXZ",		SUP_8086,	OPE_LOOP, 0xe3, 16 },
	{ "JE",			SUP_8086,	OPE_JCC, 0x04 },
	{ "JECXZ",		SUP_i386,	OPE_LOOP, 0xe3, 32 },
	{ "JG",			SUP_8086,	OPE_JCC, 0x0f },
	{ "JGE",		SUP_8086,	OPE_JCC, 0x0d },
	{ "JL",			SUP_8086,	OPE_JCC, 0x0c },
	{ "JLE",		SUP_8086,	OPE_JCC, 0x0e },
	{ "JMP",		SUP_8086,	OPE_JMP, 0x04 << 3, 0xe9, 0xea, 0xeb },
	{ "JNA",		SUP_8086,	OPE_JCC, 0x06 },
	{ "JNAE",		SUP_8086,	OPE_JCC, 0x02 },
	{ "JNB",		SUP_8086,	OPE_JCC, 0x03 },
	{ "JNBE",		SUP_8086,	OPE_JCC, 0x07 },
	{ "JNC",		SUP_8086,	OPE_JCC, 0x03 },
	{ "JNE",		SUP_8086,	OPE_JCC, 0x05 },
	{ "JNG",		SUP_8086,	OPE_JCC, 0x0e },
	{ "JNGE",		SUP_8086,	OPE_JCC, 0x0c },
	{ "JNL",		SUP_8086,	OPE_JCC, 0x0d },
	{ "JNLE",		SUP_8086,	OPE_JCC, 0x0f },
	{ "JNO",		SUP_8086,	OPE_JCC, 0x01 },
	{ "JNP",		SUP_8086,	OPE_JCC, 0x0b },
	{ "JNS",		SUP_8086,	OPE_JCC, 0x09 },
	{ "JNZ",		SUP_8086,	OPE_JCC, 0x05 },
	{ "JO",			SUP_8086,	OPE_JCC, 0x00 },
	{ "JP",			SUP_8086,	OPE_JCC, 0x0a },
	{ "JPE",		SUP_8086,	OPE_JCC, 0x0a },
	{ "JPO",		SUP_8086,	OPE_JCC, 0x0b },
	{ "JS",			SUP_8086,	OPE_JCC, 0x08 },
	{ "JZ",			SUP_8086,	OPE_JCC, 0x04 },
	{ "LAHF",		SUP_8086,	NO_PARAM, 1, 0x9f },
	{ "LAR",		SUP_80286P,	OPE_RM, 0xa6, 0x02, 0x0f, 0x02 }, /* wd, ws no-w */
	{ "LDS",		SUP_8086,	OPE_RM, 0x86, 0x41, 0xc5 }, /* wd, s no-w no-reg */
	{ "LEA",		SUP_8086,	OPE_RM, 0x06, 0x41, 0x8d }, /* wd, all no-w no-reg (cancel seg:) */
	{ "LEAVE",		SUP_80186,	NO_PARAM, 1, 0xc9 },
	{ "LES",		SUP_8086,	OPE_RM, 0x86, 0x41, 0xc4 }, /* wd, s no-w no-reg */
	{ "LFS",		SUP_i386,	OPE_RM, 0x86, 0x42, 0x0f, 0xb4 }, /* wd, s no-w no-reg */
	{ "LGDT",		SUP_80286P,	OPE_M, 0x2e, 0x62, 0x0f, 0x01 }, /* wds no-w no-o16/o32 no-reg */
	{ "LGS",		SUP_i386,	OPE_RM, 0x86, 0x42, 0x0f, 0xb5 }, /* wd, s no-w no-reg */
	{ "LIDT",		SUP_80286P,	OPE_M, 0x3e, 0x62, 0x0f, 0x01 }, /* wds no-w no-o16/o32 no-reg */
	{ "LLDT",		SUP_80286P,	OPE_M, 0x2a, 0x22, 0x0f, 0x00 }, /* ws no-w no-o16/o32 */
	{ "LMSW",		SUP_80286P, OPE_M, 0x6a, 0x22, 0x0f, 0x01 }, /* ws no-w no-o16/o32 */
	{ "LOCK",		SUP_8086,	PREFIX, 0x02 },
	{ "LODSB",		SUP_8086,	NO_PARAM, 1 | DEF_DS, 0xac },
	{ "LODSD",		SUP_i386,	NO_PARAM, 1 | OPE32 | DEF_DS, 0xad },
	{ "LODSW",		SUP_8086,	NO_PARAM, 1 | OPE16 | DEF_DS, 0xad },
	{ "LOOP",		SUP_8086,	OPE_LOOP, 0xe2, 0 },
	{ "LOOPE",		SUP_8086,	OPE_LOOP, 0xe1, 0 },
	{ "LOOPNE",		SUP_8086,	OPE_LOOP, 0xe0, 0 },
	{ "LOOPNZ",		SUP_8086,	OPE_LOOP, 0xe0, 0 },
	{ "LOOPZ",		SUP_8086,	OPE_LOOP, 0xe1, 0 },
	{ "LSL",		SUP_80286P,	OPE_RM, 0xa6, 0x02, 0x0f, 0x03 }, /* wd, ws no-w */
	{ "LSS",		SUP_8086,	OPE_RM, 0x86, 0x42, 0x0f, 0xb2 }, /* wd, s no-w no-reg */
	{ "LTR",		SUP_80286P,	OPE_M, 0x3a, 0x22, 0x0f, 0x00 }, /* ws no-w no-o16/o32 */
	{ "MOV",		SUP_8086,	OPE_MOV },
	{ "MOVSB",		SUP_8086,	NO_PARAM, 1 | DEF_DS, 0xa4 },
	{ "MOVSD",		SUP_i386,	NO_PARAM, 1 | OPE32 | DEF_DS, 0xa5 },
	{ "MOVSW",		SUP_8086,	NO_PARAM, 1 | OPE16 | DEF_DS, 0xa5 },
	{ "MOVSX",		SUP_i386,	OPE_MOVZX, 0xbf },
	{ "MOVZX",		SUP_i386,	OPE_MOVZX, 0xb7 },
	{ "MUL",		SUP_8086,	OPE_M, 0x47, 0x11, 0xf6 }, /* bwd w0 */
	{ "NEG",		SUP_8086,	OPE_M, 0x37, 0x11, 0xf6 }, /* bwd w0 */
	{ "NOP",		SUP_8086,	NO_PARAM, 1, 0x90 },
	{ "NOT",		SUP_8086,	OPE_M, 0x27, 0x11, 0xf6 }, /* bwd w0 */
	{ "OR",			SUP_8086,	OPE_ADD, 0x87, 0x11, 0x01 << 3 },
	{ "ORG",		SUP_8086,	OPE_ORG },
	{ "OUT",		SUP_8086,	OPE_INOUT, 0xe6 },
	{ "OUTSB",		SUP_8086,	NO_PARAM, 1 | DEF_DS, 0x6e },
	{ "OUTSD",		SUP_i386,	NO_PARAM, 1 | OPE32 | DEF_DS, 0x6f },
	{ "OUTSW",		SUP_8086,	NO_PARAM, 1 | OPE16 | DEF_DS, 0x6f },

	{ "POP",		SUP_8086,	OPE_PUSH, 0x08, 0x58, 0x8f, 0x07, 0xa1 },
	{ "POPA",		SUP_80186,	NO_PARAM, 1, 0x61 },
	{ "POPAD",		SUP_i386,	NO_PARAM, 1 | OPE32, 0x61 },
	{ "POPAW",		SUP_80186,	NO_PARAM, 1 | OPE16, 0x61 },
	{ "POPF",		SUP_8086,	NO_PARAM, 1, 0x9d },
	{ "POPFD",		SUP_i386,	NO_PARAM, 1 | OPE32, 0x9d },
	{ "POPFW",		SUP_8086,	NO_PARAM, 1 | OPE16, 0x9d },
	{ "PUSH",		SUP_8086,	OPE_PUSH, 0x1e, 0x50, 0xff, 0x06, 0xa0 },
	{ "PUSHA",		SUP_80186,	NO_PARAM, 1, 0x60 },
	{ "PUSHAD",		SUP_i386,	NO_PARAM, 1 | OPE32, 0x60 },
	{ "PUSHAW",		SUP_80186,	NO_PARAM, 1 | OPE16, 0x60 },
	{ "PUSHF",		SUP_8086,	NO_PARAM, 1, 0x9c },
	{ "PUSHFD",		SUP_i386,	NO_PARAM, 1 | OPE32, 0x9c },
	{ "PUSHFW",		SUP_8086,	NO_PARAM, 1 | OPE16, 0x9c },

	{ "RCL",		SUP_8086,	OPE_SHIFT, 0x02 },
	{ "RCR",		SUP_8086,	OPE_SHIFT, 0x03 },
	{ "REP",		SUP_8086,	PREFIX, 0x00 },
	{ "REPE",		SUP_8086,	PREFIX, 0x00 },
	{ "REPNE",		SUP_8086,	PREFIX, 0x01 },
	{ "REPNZ",		SUP_8086,	PREFIX, 0x01 },
	{ "REPZ",		SUP_8086,	PREFIX, 0x00 },
	{ "RESB",		SUP_8086,	OPE_RESB, 1 },
	{ "RESD",		SUP_8086,	OPE_RESB, 4 },
	{ "RESQ",		SUP_8086,	OPE_RESB, 8 },
	{ "REST",		SUP_8086,	OPE_RESB, 10 },
	{ "RESW",		SUP_8086,	OPE_RESB, 2 },
	{ "RET",		SUP_8086,	OPE_RET, 0xc2 },
	{ "RETF",		SUP_8086,	OPE_RET, 0xca },
	{ "RETN",		SUP_8086,	OPE_RET, 0xc2 },
	{ "ROL",		SUP_8086,	OPE_SHIFT, 0x00 },
	{ "ROR",		SUP_8086,	OPE_SHIFT, 0x01 },
	{ "SAHF",		SUP_8086,	NO_PARAM, 1, 0x9e },
	{ "SAL",		SUP_8086,	OPE_SHIFT, 0x04 },
	{ "SAR",		SUP_8086,	OPE_SHIFT, 0x07 },
	{ "SBB",		SUP_8086,	OPE_ADD, 0x87, 0x11, 0x03 << 3 },
	{ "SCASB",		SUP_8086,	NO_PARAM, 1, 0xae },
	{ "SCASD",		SUP_i386,	NO_PARAM, 1 | OPE32, 0xaf },
	{ "SCASW",		SUP_8086,	NO_PARAM, 1 | OPE16, 0xaf },
	{ "SETA",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x97 }, /* bs no-w no-o16/o32 */
	{ "SETAE",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x93 }, /* bs no-w no-o16/o32 */
	{ "SETB",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x92 }, /* bs no-w no-o16/o32 */
	{ "SETBE",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x96 }, /* bs no-w no-o16/o32 */
	{ "SETC",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x92 }, /* bs no-w no-o16/o32 */
	{ "SETE",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x94 }, /* bs no-w no-o16/o32 */
	{ "SETG",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x9f }, /* bs no-w no-o16/o32 */
	{ "SETGE",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x9d }, /* bs no-w no-o16/o32 */
	{ "SETL",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x9c }, /* bs no-w no-o16/o32 */
	{ "SETLE",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x9e }, /* bs no-w no-o16/o32 */
	{ "SETNA",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x96 }, /* bs no-w no-o16/o32 */
	{ "SETNAE",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x92 }, /* bs no-w no-o16/o32 */
	{ "SETNB",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x93 }, /* bs no-w no-o16/o32 */
	{ "SETNBE",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x97 }, /* bs no-w no-o16/o32 */
	{ "SETNC",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x93 }, /* bs no-w no-o16/o32 */
	{ "SETNE",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x95 }, /* bs no-w no-o16/o32 */
	{ "SETNG",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x9e }, /* bs no-w no-o16/o32 */
	{ "SETNGE",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x9c }, /* bs no-w no-o16/o32 */
	{ "SETNL",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x9d }, /* bs no-w no-o16/o32 */
	{ "SETNLE",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x9f }, /* bs no-w no-o16/o32 */
	{ "SETNO",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x91 }, /* bs no-w no-o16/o32 */
	{ "SETNP",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x9b }, /* bs no-w no-o16/o32 */
	{ "SETNS",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x99 }, /* bs no-w no-o16/o32 */
	{ "SETNZ",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x95 }, /* bs no-w no-o16/o32 */
	{ "SETO",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x90 }, /* bs no-w no-o16/o32 */
	{ "SETP",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x9a }, /* bs no-w no-o16/o32 */
	{ "SETPE",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x9a }, /* bs no-w no-o16/o32 */
	{ "SETPO",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x9b }, /* bs no-w no-o16/o32 */
	{ "SETS",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x98 }, /* bs no-w no-o16/o32 */
	{ "SETZ",		SUP_i386,	OPE_M, 0x09, 0x32, 0x0f, 0x94 }, /* bs no-w no-o16/o32 */
	{ "SGDT",		SUP_80286P,	OPE_M, 0x0e, 0x62, 0x0f, 0x01 }, /* wds no-w no-o16/o32 no-reg */
	{ "SHL",		SUP_8086,	OPE_SHIFT, 0x04 },
	{ "SHLD",		SUP_i386,	OPE_SHLD, 0xa4 },
	{ "SHR",		SUP_8086,	OPE_SHIFT, 0x05 },
	{ "SHRD",		SUP_i386,	OPE_SHLD, 0xac },
	{ "SIDT",		SUP_80286P,	OPE_M, 0x1e, 0x62, 0x0f, 0x01 }, /* wds no-w no-o16/o32 no-reg */
	{ "SLDT",		SUP_80286P,	OPE_M, 0x0a, 0x22, 0x0f, 0x00 }, /* ws no-w no-o16/o32 */
	{ "SMSW",		SUP_80286P,	OPE_M, 0x4a, 0x22, 0x0f, 0x01 }, /* ws no-w no-o16/o32 */
	{ "SS",			SUP_8086,	PREFIX, 0x07 },
	{ "STC",		SUP_8086,	NO_PARAM, 1, 0xf9 },
	{ "STD",		SUP_8086,	NO_PARAM, 1, 0xfd },
	{ "STI",		SUP_8086,	NO_PARAM, 1, 0xfb },
	{ "STOSB",		SUP_8086,	NO_PARAM, 1, 0xaa },
	{ "STOSD",		SUP_i386,	NO_PARAM, 1 | OPE32, 0xab },
	{ "STOSW",		SUP_8086,	NO_PARAM, 1 | OPE16, 0xab },
	{ "STR",		SUP_80286P,	OPE_M, 0x1a, 0x22, 0x0f, 0x00 }, /* ws no-w no-o16/o32 */
	{ "SUB",		SUP_8086,	OPE_ADD, 0x87, 0x11, 0x05 << 3 },
	{ "TEST",		SUP_8086,	OPE_TEST },
	{ "TIMES",		SUP_8086,	OPE_TIMES },
	{ "VERR",		SUP_80286P,	OPE_M, 0x4a, 0x22, 0x0f, 0x00 }, /* ws no-w no-o16/o32 */
	{ "VERW",		SUP_80286P, OPE_M, 0x5a, 0x22, 0x0f, 0x00 }, /* ws no-w no-o16/o32 */
	{ "WAIT",		SUP_8086,	NO_PARAM, 1, 0x9b },
	{ "WBINVD",		SUP_i486,	NO_PARAM, 1, 0x0f, 0x09 },
	{ "XADD",		SUP_i486,	OPE_MR, 0x87, 0x12, 0x0f, 0xc0 }, /* bwd, s w0 */
	{ "XCHG",		SUP_8086,	OPE_XCHG, 0x87, 0x11, 0x86 }, /* bwd, s w0 */
	{ "XLATB",		SUP_8086,	NO_PARAM, 1 | DEF_DS, 0xd7 },
	{ "XOR",		SUP_8086,	OPE_ADD, 0x87, 0x11, 0x06 << 3 },
	{ "", 0, 0 }
};
/* 이 테이블은 반드시 대문자로 */

static struct INST_TABLE setting_table[] = {
	{ "BITS",		SUP_8086, 0xe0 }, /* 섹션이 바뀌면 연동한다(섹션외에서 하면 디폴트) */
	{ "INSTRSET",	SUP_8086, 0xe1 },
	{ "OPTIMIZE",	SUP_8086, 0xe2 }, 
	{ "FORMAT",		SUP_8086, 0xe3 }, /* BIN, COFF */
	{ "PADDING",	SUP_8086, 0xe4 }, /* set, len, byte, byte, byte,... */
	{ "PADSET",		SUP_8086, 0xe5 }, /* 섹션이 바뀌면 연동한다(섹션외에서 하면 디폴트) */
	{ "OPTION",		SUP_8086, 0xe6 },
	{ "SECTION",	SUP_8086, 0xe7 },
	{ "ABSOLUTE",	SUP_8086, 0xe8 },
	{ "FILE",		SUP_8086, 0xe9 },
	{ "", 0, 0 }
};

UCHAR *setinstruct(UCHAR *s, UCHAR *t, UCHAR *inst)
{
	UCHAR c, *i1 = inst + OPCLENMAX;
	while (s < t) {
		c = *s;
		if (c <= ' ')
			break;
		s++;
		if ('a' <= c && c <= 'z')
			c += 'A' - 'a';
		if (inst >= i1)
			return NULL;
		*inst++ = c;
	}
	while (inst < i1)
		*inst++ = '\0';
	return skipspace(s, t);
}

static unsigned char *cpu_name[] = {
	"8086", "80186", "80286", "80286p", "i386", "i386p", "i486", "i486p", /* 0~7 */
	"Pentium", "PentiumPro", "PentiumMMX", "Pentium2", "Pentium3", "Pentium4", /* 8~13 */
	NULL
};

static unsigned char *format_type[] = { "BIN", "WCOFF", NULL };

UCHAR *decoder(struct STR_STATUS *status, UCHAR *src, struct STR_DECODE *decode)
/* NASK의 문법에 근거해, 일문을 분해한다 */
/* 다음 행에 포인터를 돌려준다 */
{
	int i, j, k;
	struct INST_TABLE *itp;
	UCHAR instruct[OPCLENMAX], *p, **pq, *q, c, cc;

	decode->instr = NULL; /* 공행 혹은 주석행 */
	decode->error = 0;
	decode->prefix = 0;
	decode->label = NULL;
//	decode->dollar = 0;
setting:
	src = skipspace(src, status->src1);
	if (src >= status->src1)
		goto fin;
	if (*src == '[') {
		/* BITS 지정 등 */
		src++;
		if ((p = setinstruct(skipspace(src, status->src1), status->src1, instruct)) ! = NULL) {
			for (itp = setting_table; itp->opecode[0]; itp++) {
				c = 0;
				for (i = 0; i < OPCLENMAX; i++)
					c |= itp->opecode[i] ^ instruct[i];
				if (c == 0) {
					/* 의사 명령 발견 */
					switch (itp->param[0]) {
					case 0xe0:
						/* BITS */
						if (getconst(&p, status, &i))
								goto error2;
						if (i ! = 16 && i ! = 32)
								goto error2;
						status->bits = i;
						goto skipline; /* 후속을 분명하게 체크하고 있지 않다 */

					case 0xe1:
						/* INSTRSET */
						pq = cpu_name;
			format2:
						if (p + 4 >= status->src1) { /* INSTRSET는 최악이어도 2문자는 있을 것이다 */
			error2:
							decode->error = 2; /* parameter error */
							goto skipline;
						}
						c = *p++;
						if (c ! = 0x22 && c ! = 0x27)
							goto error2;
						src = p;
						i = 0;
						do {
							q = *pq++;
							do {
								if (p >= status->src1)
									goto next_cpu;
								if (*p++ ! = *q++)
									goto next_cpu;
							} while (*q);
							if (*p == c) {
								if (itp->param[0] == 0xe1)
									status->support = status->expr_status.support = 1 << i;
								else
									status->format = i;
								goto skipline; /* 후속을 분명하게 체크하고 있지 않다 */
							}
			next_cpu:
							p = src;
							i++;
						} while (*pq);
						goto error2;

					case 0xe2:
						/* OPTIMIZE */
						if (getconst(&p, status, &i))
							goto error2;
						status->optimize = i;
						goto skipline; /* 후속을 분명하게 체크하고 있지 않다 */

					case 0xe3:
						/* FORMAT */
						pq = format_type;
						goto format2;

					case 0xe6:
						/* OPTION */
						if (getconst(&p, status, &i))
							goto error2;
						status->expr_status.option = status->option = i;
						goto skipline; /* 후속을 분명하게 체크하고 있지 않다 */

					case 0xe7:
						/* SECTION */
						/* 섹션명과 align를 인식 */
						for (j = 0; &p[j] < status->src1; j++) {
							if (p[j] <= ' ')
								break;
							if (p[j] == ']')
								break;
						}
						if (j > 16)
							goto error1; /* 섹션명이 너무 길다 */
						if (&p[j] >= status->src1)
							goto error1; /* 문법이 이상하다 */
						for (i = 0; i < MAX_SECTIONS; i++) {
							if (decode->sectable[i]. name[0] == '\0') {
								decode->sectable[i]. name_len = j;
								decode->sectable[i]. name[j] = '\0';
								k = j;
								while (--k >= 0)
									decode->sectable[i]. name[k] = p[k];
								break;
							}
							if (decode->sectable[i]. name_len ! = j)
								continue;
							c = 0;
							for (k = 0; k < j; k++)
								c |= p[k] ^ decode->sectable[i]. name[k];
							if (c == 0)
								break;
						}
						if (i >= MAX_SECTIONS)
							goto error1; /* 섹션수가 너무 많다 */
						p = skipspace(p + j, status->src1);
						c = 0xff;
						cc = 0;
						if (p >= status->src1)
							goto error1; /* 문법이 이상하다 */
						if (*p == 'a' || *p == 'A') {
							/* align 지정 */
							/* getconst()는 skipspace를 포함한다 */
							p += 5;
							if (p >= status->src1)
								goto error1; /* 문법이 이상하다 */
							p = skipspace(p, status->src1);
							if (p >= status->src1)
								goto error1; /* 문법이 이상하다 */
							if (*p ! = '=')
								goto error1; /* 문법이 이상하다 */
							p++;
							if ((cc = getconst(&p, status, &j)) == 0) {
								c = 0;
								while (j) {
									c++;
									j >>= 1;
								}
							}
						}
						if (decode->sectable[i]. align0 == -1 && c ! = 0xff)
							decode->sectable[i]. align0 = c;
						decode->instr = itp;
						itp->param[1] = i;
						if (cc)
							goto error2;
						goto skipline; /* 우선 건너뛴다 */

					case 0xe9:
						/* FILE */
						c = *p;
						if (c ! = 0x22 && c ! = 0x27)
							goto error2;
						status->file_p = ++p;
						while (p < status->src1 && *p ! = c && *p ! = '\n')
							p++;
						status->file_len = p - status->file_p;
						if (p >= status->src1)
							goto error2;
						if (*p ! = c)
							goto error2;
						goto skipline; /* 우선 건너뛴다 */
					}
				}
			}
		}
		goto error1;
	}
	if (*src == ';') {
		c = 1;
		if (++src + 5 < status->src1) {
			c = 0;
			for (i = 0; i < 5; i++)
				c |= src[i] - "%NASK"[i];
		}
		if (c == 0) {
			src += 5;
			goto setting; /* 확장 옵션 검출 */
		}

skipline:
		/* 전문주석 */
		while (src < status->src1 && *src ! = '\n')
			src++;
fin:
		if (src < status->src1 && *src == '\n')
			src++;
		return src;
	}
	if (*src == '\n')
		goto fin; /* 공행 */
	if (*src == '#')
		goto skipline; /* I.Tak. 씨의 요망 [OSASK 5543] */

	/* 일반 형식 */
research:
	if ((p = setinstruct(src, status->src1, instruct)) ! = 0) {
		for (itp = instruction; itp->opecode[0]; itp++) {
			c = 0;
			for (i = 0; i < OPCLENMAX; i++)
				c |= itp->opecode[i] ^ instruct[i];
			if (c == 0 && (itp->support & status->support) ! = 0) {
				decode->instr = itp;
				decode->param = p;
				/* 간이 판정 */
				if (status->expr_status.dollar_label0 == 0xffffffff) {
					if (itp->param[0] == OPE_ALIGN)
						goto need_dollar0;
					for (q = p; q < status->src1 && *q ! = '\n'; q++) {
						if (*q == '$') {
			need_dollar0:
							status->expr_status.dollar_label0 = nextlabelid++;
					//		decode->dollar = 1;
							break;
						}
					}
				}
				if ((c = itp->param[0]) ! = 0) {
				//	src = skipspace(p, status->src1);
					src = p;
					if (c == PREFIX) {
						decode->instr = NULL;
						decode->prefix |= 1 << itp->param[1];
					//	src = p;
						if (src < status->src1 && *src ! = '\n' && *src ! = ';')
							goto research; /* 무엇인가가 계속되고 있으면, 한층 더 검색 */
						goto skipline;
					}
					if (c < 0x40) {
						/* 통상 명령, 파라미터는 최대로 3개 */
						i = 0;
						if (src < status->src1 && *src ! = '\n' && *src ! = ';') {
							/* 무엇인가가 계속되고 있다 */
							for (;;) {
								decode->prm_p[i] = src;
								j = getparam(&src, status->src1, &decode->gvalue[i], status->expression,
									status->mem_expr, &status->ofsexpr, &status->expr_status);
								if (j == 0)
									goto error2;
								decode->gparam[i++] = j;
								if (src >= status->src1)
									break;
								cc = *src;
								if (cc == '\n')
									break;
								if (cc == ';')
									break;
								if (cc ! = ', ')
									goto error2;
								src = skipspace(src + 1, status->src1);
								if (i == 3)
									goto error1; /* 불필요한 기술이 있다 */
							}
						}
						decode->flag = i;
						cc = table_prms[c];
						if (cc < 4) {
							if (cc ! = decode->flag)
								goto error2;
						}
					}
					goto skipline;
				}
				goto error1;
			}
		}
	}
	if (decode->label == NULL) {
		decode->label = src; /* 라벨 발견 */
		while (*src > ' ' && src < status->src1)
			src++;
	//	c = src[-1];
		src = skipspace(src, status->src1);
		if (src >= status->src1 || *src == '\n' || *src == ';') {
		//	if (c == ':')
				goto skipline; /* 라벨 정의 */
		//	goto error1;
		}
		goto research;
	}
error1:
	decode->error = 1; /* 불필요한 기술이 있다(공행이어야 하는 것인데) */
	goto skipline;
}

/* "label: hoge" 는 라벨을 등록하지만, 에러 1 */
/* "label  hoge" 도 라벨을 등록하지만, 에러 1 */
/* "hoge"는 라벨을 등록하지 않고 에러 1 */

void put4b(unsigned int i, UCHAR *p)
{
	p[0] =  i        & 0xff;
	p[1] = (i >>  8) & 0xff;
	p[2] = (i >> 16) & 0xff;
	p[3] = (i >> 24) & 0xff;
	return;
}

unsigned int get4b(UCHAR *p)
{
	return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

struct STR_TERM *decode_expr(UCHAR **ps, UCHAR *s1, struct STR_TERM *expr, int *priority, struct STR_DEC_EXPR_STATUS *status)
{
	UCHAR *s = *ps, c, d, *t, buf8[8];
	struct STR_TERM *expr0 = expr, *pterm;
	int prio0 = 0, prio1, i, j, k;
	static char symbols[] = "\"'+-*/%&|^(){}[]<>,;:";
	static struct STR_OPELIST {
		char str[2], prio, num;
	} opelist0[] = {
		{ "|>", 12, 18 }, { "&>", 12, 17 },
		{ "<<", 12, 16 }, { ">>", 12, 17 },
		{ "//", 14,  9 }, { "%%", 14, 10 },
		{ "+",  13,  4 }, { "-",  13,  5 },
		{ "*",  14,  6 }, { "/",  14,  7 },
		{ "%",  14,  8 }, { "^",   7, 14 },
		{ "&",   8, 12 }, { "|",   6, 13 },
		{ "",    0,  0 }
	}, opelist1[] = {
		{ "|>", 12, 18 }, { "&>", 12, 17 },
		{ "<<", 12, 16 }, { ">>", 12, 18 },
		{ "//", 14,  7 }, { "%%", 14,  8 },
		{ "+",  13,  4 }, { "-",  13,  5 },
		{ "*",  14,  6 }, { "/",  14,  9 },
		{ "%",  14, 10 }, { "^",   7, 14 },
		{ "&",   8, 12 }, { "|",   6, 13 },
		{ "",    0,  0 }
	};
	struct STR_OPELIST *popelst, *opelist = opelist0;
	static struct STR_KEYWORD {
		int support;
		char keyword[8][8];
	} keywordlist[] = {
		SUP_i386,	"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI",
		SUP_8086,	"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI",
		SUP_8086,	"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH",
		SUP_8086,	"ES", "CS", "SS", "DS", "", "", "", "",
		SUP_i386,	"FS", "GS", "", "", "", "", "", "",
		SUP_i386,	"CR0", "CR1", "CR2", "CR3", "CR4", "CR5", "CR6", "CR7",
		SUP_i386,	"DR0", "DR1", "DR2", "DR3", "DR4", "DR5", "DR6", "DR7",
		SUP_i386,	"TR0", "TR1", "TR2", "TR3", "TR4", "TR5", "TR6", "DR7",
		SUP_8086,	"BYTE", "WORD", "SHORT", "NEAR", "FAR", "NOSPLIT", "$", "$$",
		SUP_8086,	"DWORD", "", "", "", "QWORD", "..$", "TWORD", "TO",
		SUP_8086,	"ST0", "ST1", "ST2", "ST3", "ST4", "ST5", "ST6", "ST7",	/* 80-87 */
	//	SUP_MMX,	"MM0", "MM1", "MM2", "MM3", "MM4", "MM5", "MM6", "MM7",	/* 88-95 */
		0, 			"", "", "", "", "", "", "", ""
	};
	struct STR_KEYWORD *pkw;

	if (priority)
		prio0 = *priority;
	if (status->option & 1)
		opelist = opelist1;

single:
	s = skipspace(s, s1);
	c = '\0';
	if (s < s1)
		c = *s++;
	if (c == '+') {
		/* 단항 플러스 */
		expr->value = 0; /* s+ */
single1:
		expr->term_type = 1; /* operator */
		expr++;
		goto single;
	}
	if (c == '-') {
		/* 단항 마이너스 */
		expr->value = 1; /* s- */
		goto single1;
	//	expr->term_type = 1; /* operator */
	//	expr++;
	//	goto single;
	}
	if (c == '~') {
		/* 단항 NOT */
		expr->value = 2; /* s~ */
		goto single1;
	//	expr->term_type = 1; /* operator */
	//	expr++;
	//	goto single;
	}

	/* 제1항 */
	for (i = 0; (unsigned int) i < sizeof symbols; i++) {
		if (c == symbols[i])
			goto symbol;
	}
	if (c == 0)
		goto error;

	/* number, keyword, label */
	t = s - 1;
	while (s < s1) {
		if ((c = *s) <= ' ')
			break;
		for (i = 0; (unsigned int) i < sizeof symbols; i++) {
			if (c == symbols[i])
				goto token_end;
		}
		s++;
	}

token_end:
	for (i = 0; i < 8; i++) {
		d = 0;
		if (&t[i] < s) {
			d = t[i];
			if ('a' <= d && d <= 'z')
				d += 'A' - 'a';
		}
		buf8[i] = d;
	}
	j = 0;
	for (pkw = keywordlist; pkw->support; pkw++, j += 8) {
		if ((status->support & pkw->support) == 0)
			continue;
		for (i = 0; i < 8; i++) {
			d = 0;
			for (k = 0; k < 8; k++)
				d |= buf8[k] ^ pkw->keyword[i][k];
			if (d == 0) {
				i += j;
				goto find_keyword;
			}
		}
	}

	/* 라벨인가, 숫자 */
	k = s - t;
	d = t[0];
	if ('0' <= d && d <= '9') {
		if (k >= 2) {
			if (t[1] == 'X' || t[1] == 'x') {
				t += 2;
				if (k -= 2)
					goto num_hex;
				goto error;
			}
			d = s[-1];
			if ('a' <= d && d <= 'z')
				d += 'A' - 'a';
			k--;
			if (d == 'H')
				goto num_hex;
			j = 2;
			if (d == 'B')
				goto num_all;
			j = 8;
			if (d == 'Q')
				goto num_all;
			k++;
		}
		j = 10;
		goto num_all;
	}
	if (d == '$' && k >= 2 /* && '0' <= t[1] && t[1] <= '9' */) {
		t++;
		k--;
num_hex:
		j = 16;
num_all:
		i = 0;
		do {
			d = *t++;
			if ('0' <= d && d <= '9')
				d -= '0';
			else if ('A' <= d && d <= 'Z')
				d += 10 - 'A';
			else if ('a' <= d && d <= 'z')
				d += 10 - 'a';
			else if (d == '_')
				continue;
			else
				goto error;
			if (d >= j)
				goto error;
			i = i * j + d;
		} while (--k);
constant:
		expr->term_type = 0; /* constant */
		expr->value = i;
		expr++;
		goto search_oper;
	}

	/* label */
	expr->term_type = 3 /* label */;
	expr->value = label2id(k, t, 0);
	expr++;
	goto search_oper;

find_keyword:
	if (i == 70) { /* $ */
		expr->term_type = 3; /* label */
		expr->value = status->dollar_label0;
		expr++;
		goto search_oper;
	}
	if (i == 71) { /* $$ */
		expr->term_type = 3; /* label */
		expr->value = status->dollar_label2;
		expr++;
		goto search_oper;
		goto search_oper;
	}
	if (i == 77) { /* ..$ */
		if (status->dollar_label1 == 0xffffffff)
			status->dollar_label1 = nextlabelid++;
		expr->term_type = 3; /* label */
		expr->value = status->dollar_label1;
		expr++;
		goto search_oper;
	}
	if (32 <= i && i <= 33) {
		/* FS, GS */
		i -= 4;
	} 
	if (i < 24) {
		/* reg32, reg16, reg8 */
term_reg:
		expr->term_type = 2; /* reg */
		expr->value = i;
		expr++;
		goto search_oper;
	}
	if (i < 36) {
		/* sreg */
		if (c ! = ':')
			goto term_reg;
		s++;
		if (status->seg_override ! = -1)
			goto error;
		status->seg_override = i - 24;
		goto single;
	}
	if (i < 64)
		goto term_reg;
//	if (i == 64 || i == 65)
//		i += 36 - 64;
	if (i < 66) {
		/* datawidth */
datawidth:
		if (status->datawidth ! = -1)
			goto error;
		status->datawidth = i - 63;
		goto single;
	}
	if (i < 69) {
		if (status->range ! = -1)
			goto error;
		status->range = i - 66; /* 66:SHORT, 67:NEAR, 68:FAR */
		goto single;
	}
	if (i == 69) {
		if (status->nosplit)
			goto error;
		status->nosplit = 1;
		goto single;
	}
//	if (i < 72) {
//		/* $, $$ */
//		expr->term_type = 3; /* label */
//		expr->value = - 16 + 70 - i; /* -16:$, -17:$$ */
//		expr++;
//		if (i == 70)
//			status->use_dollar = 1;
//		goto search_oper;
//	}
	if (i < 78) {
		i += 63 + 4 - 72;
		goto datawidth;
	}
	if (i == 79) { /* TO */
		if (status->to_flag)
			goto error;
		status->to_flag++;
		goto single;
	}
//	if (i < 96) {
		/* STx, MMx */
		i += 72 - 80;
		goto term_reg;
//	}

symbol:
	if (c == '(') {
		/* 괄호 */
		expr = decode_expr(&s, s1, expr, NULL, status);
	//	s = skipspace(s, s1);
		c = '\0';
		if (s < s1)
			c = *s++;
		if (c ! = ')')
			expr = NULL;
		if (expr == NULL)
			return NULL;
	} else if (c == 0x22 || c == 0x27) {
		/* " ' */
		i = 0;
		k = 0;
		for (;;) {
			if (s >= s1)
				goto error;
			if (*s == c) {
				s++;
				goto constant;
			}
			if (k == 4)
				goto error;
			((char *) &i)[k++] = *s++;
		}
	} else {
		s--;
error:
		expr = NULL;
		goto fin;
	//	*ps = s;
	//	return NULL;
	}

search_oper:
	s = skipspace(s, s1);
	c = d = '\0';
	if (s < s1)
		c = *s++;
	if (s < s1)
		d = *s;
	if (c == 0)
		goto fin;

	for (popelst = opelist; ; popelst++) {
		if (popelst->str[0] == '\0') {
			*ps = s - 1;
			return expr;
		}
		if (c == popelst->str[0]) {
			if (popelst->str[1] == '\0')
				break;
			if (d == popelst->str[1]) {
				s++;
				break;
			}
		}
	}
	prio1 = popelst->prio;
	c = popelst->num;

new_operator:
	if (prio0 >= prio1) {
	//	if (priority)
			*priority = - prio1; /* 부모의 연산자 변경을 통지 */
		expr->value = c;
		goto fin;
	//	*ps = s;
	//	return expr; /* 제일 마지막 뒤가 신연산자 */
	}

	/* 연산자를 앞에 두고 낸다 */
	pterm = expr++;
	do {
		*(pterm + 1) = *pterm;
		pterm--;
	} while (expr0 <= pterm);
	expr0->term_type = 1; /* operator */
	expr0->value = c;

	/* 제2항 */
	expr = decode_expr(&s, s1, expr, &prio1, status);
//	s = skipspace(s, s1);
	if (prio1 < 0) {
		prio1 = - prio1;
		c = expr->value;
		goto new_operator;
	}
fin:
	*ps = s;
	return expr;
}

/*

라벨이 섞이면 불명
4개의 컨디션
하나의 int, 2개의 계수
이것을 계산하는 것으로, 베이스, 스케이르드인젝스, disp가 요구된다.

라벨 계산시：최대와 최소를 낸다. 레지스터는 0취급으로 소거.

*/

void init_ofsexpr(struct STR_OFSEXPR *ofsexpr)
{
	ofsexpr->scale[0] = ofsexpr->scale[1] = ofsexpr->disp = 0;
//	ofsexpr->extlabel = 0;
	ofsexpr->reg[0] = ofsexpr->reg[1] = 0xff;
	ofsexpr->dispflag = ofsexpr->err = 0;
	return;
}

void calc_ofsexpr(struct STR_OFSEXPR *ofsexpr, struct STR_TERM **pexpr, char nosplit)
{
	struct STR_TERM *expr = *pexpr;
	struct STR_OFSEXPR tmp, tmp2;
	int i, j;

	j = expr->term_type;
	i = expr->value;
	*pexpr = ++expr;
	switch (j) {
	case 0: /* constant number */
		init_ofsexpr(ofsexpr);
		ofsexpr->disp = i;
		return;
	case 1: /* operator */
		calc_ofsexpr(ofsexpr, pexpr, nosplit);
		if (i >= 4) {
			calc_ofsexpr(&tmp, pexpr, nosplit);
			j = tmp.disp;
			if (i == 6 /* 2항 * */ || (12 <= i && i <= 14 /* 2항 &, |, ^ */)) {
				if (tmp.reg[0] ! = 0xff || (ofsexpr->reg[0] == 0xff && ofsexpr->dispflag == 0 && tmp.dispflag ! = 0)) {
					if (ofsexpr->reg[0] ! = 0xff)
						goto err1;
					tmp2 = tmp;
					tmp = *ofsexpr;
					*ofsexpr = tmp2;
					j = tmp.disp;
				}
			}
			ofsexpr->err |= tmp.err;
			ofsexpr->dispflag |= tmp.dispflag;
		//	if (i >= 5) {
		//		/* - * % // %% & | << >> */
		//		if (tmp.extlabel)
		//			goto err1;
		//	}
			if (i >= 6) {
				/* * / % // %% & | ^ << >> */
				if (tmp.reg[0] ! = 0xff)
					goto err1;
				if (tmp.dispflag == 0) {
					if (((i == 6 || i == 7 || i == 9) && j == 1)
						|| (i == 12 && j == -1) || (i >= 13 && j == 0)) {
						/* *1, /1, //1, &-1, |0, ^0, <<0, >>0 */
						ofsexpr->reg[0] |= 0x80;
						ofsexpr->reg[1] |= 0x80;
						return;
					}
					if (((i == 6 || i == 12) && j == 0) || (i == 13 && j == -1)) {
						/* *0, &0, |-1 */
						ofsexpr->disp = j;
						ofsexpr->scale[0] = ofsexpr->scale[1] = 0;
						ofsexpr->reg[0] = ofsexpr->reg[1] = 0xff;
					//	ofsexpr->extlabel = 0;
						ofsexpr->dispflag = 0;
						return;
					}
					if (7 <= i && i <= 10 && j == 0)
						goto err1; /* /0, %0, //0, %%0 */
					/* ^ 의 상쇄는 판정이 복잡해서 하지 않음 */
				//	if (ofsexpr->extlabel)
				//		goto err1;
					if (i == 8 || i == 10 || i == 12 || i == 13 || i == 14) {
						/* % %% & | ^ */
						if (ofsexpr->reg[0] ! = 0xff)
							goto err1;
					}
				}
			}
		}
		if (i >= 6 && tmp.dispflag ! = 0) {
			/* 라벨을 포함한 식의 취급 */
			if (ofsexpr->reg[0] ! = 0xff)
				goto err1;
			return;
		}
		switch (i) {
		case 0: /* 단항 + */
			break;
		case 1: /* 단항 - */
			ofsexpr->scale[0] *= -1;
			ofsexpr->scale[1] *= -1;
			ofsexpr->disp *= -1;
			break;
		//	if (ofsexpr->extlabel) {
	err1:
				ofsexpr->err |= 1;
		//	}
			break;
		case 2: /* 단항 ~ */
			ofsexpr->disp ^= -1;
			if ((ofsexpr->reg[0] & ofsexpr->reg[1]) ! = 0xff /* || ofsexpr->extlabel ! = 0 */)
				goto err1;
			break;
		case 4: /* 2항 + */
		addition:
			for (i = 0; i < 2; i++) {
				if (tmp.reg[i] ! = 0xff) {
					for (j = 0; ; j++) {
						if (j >= 2)
							goto err1;
						if (ofsexpr->reg[j] == 0xff) {
							ofsexpr->reg[j] = tmp.reg[i];
							ofsexpr->scale[j] = tmp.scale[i];
							break;
						}
						if (nosplit == 0 && (ofsexpr->reg[j] & 0x7f) == (tmp.reg[i] & 0x7f)) {
							ofsexpr->reg[j] |= 0x80;
							if ((ofsexpr->scale[j] += tmp.scale[i]) == 0) {
								if (j == 0) {
									ofsexpr->scale[0] = ofsexpr->scale[1];
									ofsexpr->reg[0] = ofsexpr->reg[1];
								}
								ofsexpr->scale[1] = 0;
								ofsexpr->reg[1] = 0xff;
							}
							break;
						}
					}
				}
			}
			ofsexpr->disp += tmp.disp;
		//	if (tmp.extlabel) {
		//		if (ofsexpr->extlabel)
		//			goto err1;
		//		ofsexpr->extlabel = tmp.extlabel;
		//	}
			break;
		case 5: /* 2항 - */
			tmp.scale[0] *= -1;
			tmp.scale[1] *= -1;
			tmp.disp *= -1;
			goto addition;
		case 6: /* 2항 * */
			ofsexpr->scale[0] *= j;
			ofsexpr->scale[1] *= j;
			ofsexpr->disp *= j;
	reg_operated:
			ofsexpr->reg[0] |= 0x80;
			ofsexpr->reg[1] |= 0x80;
			break;
		case 7: /* 2항 / */
		div_unsigned:
			if ((unsigned int) ofsexpr->scale[0] % (unsigned int) j)
				goto err1;
			ofsexpr->scale[0] /= (unsigned int) j;
			if ((unsigned int) ofsexpr->scale[1] % (unsigned int) j)
				goto err1;
			ofsexpr->scale[1] /= (unsigned int) j;
			ofsexpr->disp /= (unsigned int) j;
			goto reg_operated;
		case 8: /* 2항 % */
			ofsexpr->disp %= (unsigned int) j;
			break;
		case 9: /* 2항 // */
		div_signed:
			if ((signed int) ofsexpr->scale[0] % (signed int) j)
				goto err1;
			ofsexpr->scale[0] /= (signed int) j;
			if ((signed int) ofsexpr->scale[1] % (signed int) j)
				goto err1;
			ofsexpr->scale[1] /= (signed int) j;
			ofsexpr->disp /= (signed int) j;
			goto reg_operated;
		case 10: /* 2항 %% */
			ofsexpr->disp %= (signed int) j;
			break;
		case 12: /* 2항 & */
			ofsexpr->disp &= j;
			break;
		case 13: /* 2항 | */
			ofsexpr->disp |= j;
			break;
		case 14: /* 2항 ^ */
			ofsexpr->disp ^= j;
			break;
		case 16: /* 2항 << */
			ofsexpr->scale[0] <<= j;
			ofsexpr->scale[1] <<= j;
			ofsexpr->disp <<= j;
			while (ofsexpr->scale[0] == 0 && ofsexpr->reg[0] ! = 0xff) {
				ofsexpr->scale[0] = ofsexpr->scale[1];
				ofsexpr->reg[0] = ofsexpr->reg[1];
				ofsexpr->scale[1] = 0;
				ofsexpr->reg[1] = 0xff;
			}
			if (ofsexpr->scale[1] == 0) {
				ofsexpr->scale[1] = 0;
				ofsexpr->reg[1] = 0xff;
			}
			goto reg_operated;
		case 17: /* 2항 &> */
			j = 1 << j;
			goto div_unsigned;
		case 18: /* 2항 |> */
			j = 1 << j;
			goto div_signed;
		}
		break;
	case 2: /* register */
		init_ofsexpr(ofsexpr);
	//	if (i >= 64)
	//		goto err1;
		ofsexpr->reg[0] = i;
		ofsexpr->scale[0] = 1;
		break;

	case 3: /* label */
		init_ofsexpr(ofsexpr);
		ofsexpr->dispflag = 1;
		/* 무조건 불명하게 하는 것이 아니라, 확정하고 있는 것에 대해서는 인정해도 괜찮을 것이다 */
		/* extlabel인가 어떤가 판정할 필요도 있다 */
		break;
	}
	return;
}

int getparam(UCHAR **ps, UCHAR *s1, int *p, struct STR_TERM *expression, struct STR_TERM *mem_expr, 
	struct STR_OFSEXPR *ofsexpr, struct STR_DEC_EXPR_STATUS *status)
/* 반대 값에 대해
	bit0-3:datawidth(바이트수, 15는 부정)
	bit4-5:type (0:reg, 1:mem, 2:imm)
	bit6-7:range (0:default, 1:short, 2:near, 3:far)
	bit8:use_dollar
	all 0은 에러를 의미한다
	bit9-11:(mem):disp의 datawidth (1:byte, 2:word, 4:dword, 7:default)
	bit12-14:(mem) :seg (7:default, 0~5:seg)
	bit15:(mem) :nosplit
	bit9-15:(reg):레지스터 번호
	bit9:(imm):extlabel나 식의 해석에 실패했으므로 *p는 무효
*/
{
	struct STR_TERM *pe, *expr;
	int i;
	UCHAR ret, ret2, rethigh, *s = *ps;

/*	status->support = 
	status->glabel_len = 0;
	status->glabel = NULL;
*/
	status->datawidth = -1; /* -1(default), 1(byte), 2(word), 4(dword) */
	status->seg_override = -1; /* -1(default), 0~5 */
	status->range = -1; /* -1(default), 0(short), 1(near), 2(far) */
	status->nosplit = 0; /* 0(default), 1(nosplit) */
	status->use_dollar = 0; /* 0(no use), 1(use) */
	status->to_flag = 0;

	expression[0]. term_type = -1;
	pe = decode_expr(&s, s1, expression, NULL, status);
	ret = (status->range + 1) << 6 | (status->datawidth & 0x0f);
	rethigh = status->use_dollar;
	if (status->nosplit)
		goto err;
	if (pe == NULL && expression[0]. term_type == -1 && s < s1 && *s == '[') {
		/* 메모리 검출 */
		/* datawidth와 range를 가까이 둔다 */
		/* seg_override는 공통 */
		if (mem_expr == NULL)
			goto err;
		ret = (status->range + 1) << 6 | (status->datawidth & 0x0f) | 0x10;
		s++;
		status->datawidth = -1;
		status->range = -1;
		pe = decode_expr(&s, s1, mem_expr, NULL, status);
		if (pe == NULL)
			goto err;
		if (status->range ! = -1)
			goto err;
		if (s >= s1)
			goto err;
		if (*s ! = ']')
			goto err;
		rethigh |= status->use_dollar;
		rethigh |= (status->datawidth & 0x07) << 1 | status->nosplit << 7
			| (status->seg_override & 0x07) << 4;
		s = skipspace(s + 1, s1);
		goto fin;
	}

	if (pe == NULL)
		goto err;

	/* 레지스터나 값 */
	expr = expression;
	calc_ofsexpr(ofsexpr, &expr, 0);
	if (ofsexpr->err)
		goto err; /* 에러 */
	if (status->to_flag) {
		if ((ofsexpr->reg[0] & ~0x07) ! = 72)
			goto err;
		ofsexpr->reg[0] -= 8;
		status->to_flag = 0;
	}
	if (ofsexpr->reg[0] == 0xff) {
		/* 정수 */
		i = ofsexpr->disp;
	//	if (status->datawidth == -1) {
	//		ret &= 0xf0;
	//		if (-128 <= i && i <= 127)
	//			ret |= 0x01;
	//		else if (-0x8000 <= i && i <= 0x7fff)
	//			ret |= 0x02;
	//		else
	//			ret |= 0x04;
	//	} else {
			ret |= status->datawidth & 0x0f;
	//	}
		ret |= 0x20;
		if (ofsexpr->dispflag ! = 0 /* || ofsexpr->extlabel ! = 0 */)
			rethigh |= 0x02;
		goto fin;
	}
	if (ofsexpr->reg[1] ! = 0xff || ofsexpr->scale[0] ! = 1
		|| ofsexpr->disp ! = 0 /* || ofsexpr->extlabel ! = 0 */) {
		/* 에러 */
err:
		ret = 0x00;
		rethigh = 0;
		goto fin;
	}
	/* 레지스터 */
	i = ofsexpr->reg[0] & 0x7f;
	if (i < 8)
		ret2 = 0x04; /* reg32 */
	else if (i < 16)
		ret2 = 0x02; /* reg16 */
	else if (i < 24)
		ret2 = 0x01; /* reg8 */
	else if (i < 40)
		ret2 = 0x02; /* sreg */
	else if (i < 64)
		ret2 = 0x04; /* sysreg */
	else if (i < 80)
		ret2 = 0x0a; /* stx */
	else
		ret2 = 0x08; /* mmx */
	if (status->datawidth == -1)
		ret = (ret & 0xf0) | ret2;
	else if ((ret & 0x0f) > ret2)
		goto err;
	else if ((ret & 0x0f) < ret2) {
		if (! (i < 4 || (8 <= i && i < 12)))
			goto err;
		i = (i & 0x03) | 0x08;
		ret &= 0xf0 | ret2;
		if (ret2 == 0x01)
			i += 8;
	}
	rethigh |= i << 1;
fin:
	if (status->to_flag) {
		ret = 0x00;
		rethigh = 0;
	}
	*p = i;
	*ps = s;
	return ret | rethigh << 8;
}

int testmem(struct STR_OFSEXPR *ofsexpr, int gparam, struct STR_STATUS *status, int *prefix)
/* mem의 a16인지 a32인지를 조사하는 함수(디폴트를 준다) */
/*	bit0:default DS
	bit1:default SS
	bit3:disp only
	bit4:need a16
	bit5:need a32
	bit7:no-base
	0:error
	bit4와 bit5가 1일때, reg
	bit8-10:base
	bit11-13:index
	bit14-15:(a32) :scale factor
*/
{
	struct STR_TERM *expr = status->mem_expr;
	UCHAR nosplit, reg[2], tmp;
	int seg, ret = 0, scale[2], i;

//	if ((gparam & 0x30) == 0) {
//		/* reg */
//		return 0x30;
//	}
	nosplit = (gparam >> 15) & 0x01;
	seg = (1 << ((gparam >> 12) & 0x07)) & 0x7f; /* bit12-14:(mem) :seg (7:default, 0~5:seg) */
	calc_ofsexpr(ofsexpr, &expr, nosplit);
	reg[0] = ofsexpr->reg[0];
	reg[1] = ofsexpr->reg[1];
	if (reg[0] ! = 0xff)
		reg[0] &= 0x7f;
	if (reg[1] ! = 0xff)
		reg[1] &= 0x7f;
	scale[0] = ofsexpr->scale[0];
	scale[1] = ofsexpr->scale[1];
	i = (gparam >> 9) & 0x07;
	if (reg[0] == 0xff) {
		ret |= 0x09; /* default DS, disp-only */
		if (i == 1)
			goto error;
		if (i == 7) {
			if (status->bits & 0x10)
				goto disponly_a16;
			goto disponly_a32;
		}
		if (i == 2) {
disponly_a16:
			ret |= 0x10; /* need a16 */
			*prefix |= 0x01000000; /* bit24:A16(암묵) */
		}
		if (i == 4) {
disponly_a32:
			ret |= 0x20; /* need a32 */
			*prefix |= 0x02000000; /* bit25:A32(암묵) */
		}
	} else if (reg[0] < 8) {
		if (i == 2)
			goto error;
		ret |= 0x20; /* need a32 */
		*prefix |= 0x02000000; /* bit25:A32(암묵) */
		if (reg[1] == 0xff) {
			if (nosplit == 0) {
				if (scale[0] ! = 1) {
					if (reg[0] == 4 /* ESP */)
						goto error;
					scale[1] = scale[0];
					reg[1] = reg[0];
					reg[0] = 0xff;
					if (scale[0] == 2 || scale[0] == 3 || scale[0] == 5 || scale[0] == 9) {
						reg[0] = reg[1];
						scale[1]--;
					}
				}
			} else if (ofsexpr->reg[0] & 0x80) { /* reg[0]은 스케일 되고 있다 */
				reg[1] = reg[0];
				scale[1] = scale[0];
				reg[0] = 0xff;
			}
		} else { /* 2개의 레지스터가 존재한다 */
			if (reg[1] >= 8)
				goto error;
			if (scale[0] ! = 1 || scale[1] ! = 1) {
				if (scale[0] ! = 1) {
					i = scale[0];
					tmp = reg[0];
					scale[0] = scale[1];
					reg[0] = reg[1];
					scale[1] = i;
					reg[1] = tmp;
				}
				if (scale[0] ! = 1)
					goto error;
			} else {
				/* 양쪽 모두의 scale이 1 */
			//	tmp = 0xff;
				if ((ofsexpr->reg[0] & 0x80) ! = 0 && (ofsexpr->reg[1] & 0x80) == 0) {
					/* reg[0]은 스케일 되고 있어, reg[1]은 되어 있지 않다 */
					tmp = reg[0];
					reg[0] = reg[1];
					reg[1] = tmp;
				}
			//	if ((ofsexpr->reg[0] & 0x80) == 0 && (ofsexpr->reg[1] & 0x80) ! = 0)
			//		tmp = 0;
				/* 제멋대로인 교환을 저지하고 싶으면, NOSPLIT를 써라 */
				if (status->optimize >= 1 && nosplit == 0) {
					/* EBP가 베이스라면 교환 */
					if (/* tmp == 0xff && */ reg[0] == 5 && ofsexpr->disp == 0 &&
						ofsexpr->dispflag == 0 /* && ofsexpr->extlabel == 0 */) {
						reg[0] = reg[1];
						reg[1] = 5;
					}
					/* reg[1]으로 seg를 지울 수 있다면, 한층 더 교환 */
					i = 1 << 3; /* DS */
					if (reg[1] == 4 || reg[1] == 5)
						/* i = 1 << 2; */ i >>= 1; /* SS */
					if (seg == i) {
						tmp = reg[0];
						reg[0] = reg[1];
						reg[1] = tmp;
					}
				}
				if (reg[1] == 4) { /* ESP가 index가 되어 버렸으므로 또 교환 */
					reg[1] = reg[0];
					reg[0] = 4;
				}
			}
			if (reg[1] == 4)
				goto error;
		}
	} else if (reg[0] < 16) {
		if (i == 4)
			goto error;
		if (scale[0] ! = 1)
			goto error;
		if (reg[1] ! = 0xff && scale[1] ! = 1)
			goto error;
		if (reg[0] >= 14) { /* reg[0] == SI, DI */
			tmp = reg[0];
			reg[0] = reg[1];
			reg[1] = tmp;
		}
		if (reg[0] ! = 11 && reg[0] ! = 13 && reg[0] ! = 0xff)
			goto error; /* BX, BP */
		if (reg[1] ! = 14 && reg[1] ! = 15 && reg[1] ! = 0xff)
			goto error; /* SI, DI */
 		ret |= 0x10; /* need a16 */
		*prefix |= 0x01000000; /* bit24:A16(암묵) */
	} else {
error:
		ret = 0; /* error */
		goto fin;
	}
	if (reg[0] == 0xff)
		ret |= 0x81; /* no-base, default DS */
	else {
		reg[0] &= 0x07;
		ret |= reg[0] << 8;
		ret |= 0x01; /* default DS */
		if (reg[0] == 4) /* ESP */
			ret ^= 0x03; /* default SS */
		if (reg[0] == 5) /* EBP, BP */
			ret ^= 0x03; /* default SS */
	}
	if (reg[1] == 0xff) {
		reg[1] = 4; /* ESP(no-index) */
		scale[1] = 1;
	}
	tmp = 0;
	if (ret & 0x20) {
		tmp = 4;
		if (scale[1] == 1)
			tmp = 0;
		if (scale[1] == 2)
			tmp = 1;
		if (scale[1] == 4)
			tmp = 2;
		if (scale[1] == 8)
			tmp = 3;
		if (tmp == 4)
			goto error;
	}
	ret |= (int) tmp << 14 | (int) (reg[1] & 0x07) << 11;

fin:
	*prefix |= seg << 5;
	return ret;
}

/* mem/reg를 a16 혹은 a32에 근거해 전개하는 함수(TTT 필드도 있다) */

void putmodrm(struct STR_IFDEFBUF *ifdef, int tmret, int gparam,
	struct STR_STATUS *status, /* struct STR_OFSEXPR *ofsexpr, */ int ttt)
{
	UCHAR width, mod2 = 0x80;
	UCHAR mod, tmret1 = (tmret >> 8) & 0xff, nobase = tmret & 0x80;
//	struct STR_TERM *expr = status->mem_expr;
	static int mcode[] = {
		0x82,	0x01 /* UCHAR, const */, 0x80 /* 16bit/32bit */,
				0x01 /* UCHAR, const */, 0x40 /* 8bit */,
				0x01 /* UCHAR, const */, 0x00 /* 0bit */
	};

	ifdef->vb[0 /* 0x78(mod r/m) */] = 0x01; /* UCHAR, const */
	ifdef->vb[1 /* 0x79(sib) */] = 0x00; /* null */
	ifdef->vb[2 /* 0x7a(disp) */] = 0x00; /* null */
	ttt &= 0x07;
	ttt <<= 3;
	if ((gparam & 0x30) == 0) {
		/* reg */
		ifdef->dat[0 /* 0x78(mod r/m) */] = 0xc0 | ttt | ((gparam >> 9) & 0x07);
		goto fin;
	}
	/* mem */
//	calc_ofsexpr(ofsexpr, &expr, 0);
	width = (gparam >> 9) & 0x07;
	mod = 0x82; /* word/byte/zero */
	if (tmret & 0x10) {
		/* a16 */
		if (tmret & 0x08)
			width = 2; /* disp-only */
		else if ((tmret1 & 0x3f) == 0x25) {
			/* base:BP, no-index */
			mod = 0x02; /* word/byte */
		}
	} else {
		/* a32 */
		mod = 0x8a; /* dword/byte/zero */
		if ((tmret & 0x08) ! = 0 || nobase ! = 0)
			width = 4;
		else if ((tmret1 & 0x07) == 0x05) {
			/* base:EBP */
			mod = 0x0a; /* dword/byte */
		}
	}
	mcode[0] = mod;

	if (tmret & 0x08) {
		/* disp-only */
base_ebp:
		tmret1 &= ~0x07;
		tmret1 |= 0x05; /* base:EBP/BP */
		mod2 = 0x00;
	} else if ((tmret & 0x20) ! = 0 && nobase ! = 0)
		goto base_ebp; /* a32로 base가 없는 경우 */

	mod = 0;
	if (tmret & 0x10) {
		/* a16 */
		if ((tmret1 & 0x38) ! = 0x20) {
			/* SI, DI 있음 */
			if (nobase == 0) {
				if ((tmret1 & 0x07) == 0x05)
					mod |= 0x02; /* BP */
			} else
				mod |= 0x04; /* no-base */
			if ((tmret1 & 0x38) == 0x38)
				mod |= 0x01; /* + DI */
		} else {
			/* SI, DI없음 */
			mod |= 0x06;
			if ((tmret1 & 0x07) == 0x03)
				mod |= 0x01; /* BX */
		}
	} else {
		/* a32 */
		if ((tmret1 & 0x38) == 0x20 && (tmret1 & 0x07) ! = 0x04) {
			/* sib 없음(no-index && base ! = ESP) */
			mod |= tmret1 & 0x07;
		} else {
			/* sib 있음 */
			ifdef->vb[1 /* 0x79(sib) */] = 0x01; /* UCHAR, const */
			ifdef->dat[1 /* 0x79(sib) */] = tmret1;
			mod |= 0x04;
		}
	}
	mod |= ttt;
	mcode[2] = mod | mod2;
	mcode[4] = mod | 0x40;
	mcode[6] = mod;
	if (microcode90(ifdef, status->mem_expr, mcode, width)) {
		*(ifdef->bp)++ = REM_ADDR_ERR; /* 0xe5 */
	}
fin:
	return;
}

static char dsiz2mc98[] = { 0x99 & 0x07, 0x9b & 0x07, 0, 0x9d & 0x07 };

int microcode90(struct STR_IFDEFBUF *ifdef, struct STR_TERM *expr, int *def, signed char dsiz)
/* typ? 은 00~04를 서포트 */
/* 식중의 레지스터 필드는 완전하게 무시한다 */
/* def[0] : virtual-byte-code, zero-flag */
/* def[1-2] : typ2, dat2 */
/* def[3-4] : typ1, dat1 */
/* def[5-6] : typ0, dat0 */

/* dsiz == 0x0f : 자동 선택에 맡긴다 */
/* dsiz == 0, 1, 2, 4 : 유저 지정 */
/* [zero-bbb-w/d-aaa] w/d == 1으로 D */

/* 어드레싱에서는 DWORD지정이 없으면 16bit이므로 고민할 것은 없다 */
/* 이외는 전부 imm의 선택에 지나지 않기 때문에, zero는 사용하지 않고 w/d인지는 이미 정해져 있다 */

/* 91은 abc타입. c = b + 1; b, c의 타입은 SHORT_DB1에 고정
	...그래서 안된 것일까. SHORT_DB0일지도 모르니까.  */
{
	struct STR_OFSEXPR ofsexpr;
	struct STR_TERM *expr0 = expr;
	UCHAR idx, vbc = def[0], c;
	int vba = vbc & 0x07, vbb = (vbc >> 4) & 0x07;
	UCHAR *bp = ifdef->bp;
	calc_ofsexpr(&ofsexpr, &expr, 0); /* 식의 평가 */
	if (ofsexpr.err) {
err2:
		return 2; /* 파라미터 에러 */
	}
	if ((dsiz == 2 && (vbc & 0x08) ! = 0) || (dsiz == 4 && (vbc & 0x08) == 0)
		|| (dsiz == 0 && (vbc & 0x80 /* zero-bit */) == 0))
		return 3; /* data size error */
	if (ofsexpr.dispflag ! = 0) {
		if (dsiz <= 4) {
			ifdef->vb[vba] = dsiz; /* zero/UCHAR/USHORT/UINT, const */
			if (dsiz ! = 0) {
			//	ifdef->bp = bp;
				if (defnumexpr(ifdef, expr0, vba, (dsiz2mc98 - 1)[dsiz]))
					goto err2; /* 파라미터 에러 */
				bp = ifdef->bp;
			}
			goto set_vbb;
		}
		bp[0] = 0x90;
		bp[1] = 0x0f; /* 미결정 */
		bp[2] = 0x00;
		bp[3] = vbc;
		bp = put_expr(bp + 4, &expr0);
		idx = 3;
		def++;
		ifdef->vb[vba] = 0x7f;
		ifdef->vb[vbb] = 0x7f;
		do {
			if (idx == 1 && (vbc & 0x80 /* zero-bit */) == 0)
				continue;
			*bp++ = (c = def[0]) | 0x30;
			vba = def[1];
			while (c) {
				*bp++ = vba & 0xff;
				vba >>= 8;
				c--;
				def += 2;
			}
		} while (--idx);
		goto fin;
	}
	ifdef->dat[vba] = ofsexpr.disp;
	if ((dsiz == 2 && (vbc & 0x08) ! = 0) || (dsiz == 4 && (vbc & 0x08) == 0)
		|| (dsiz == 0 && (vbc & 0x80 /* zero-bit */) == 0))
		return 3; /* data size error */
	if ((dsiz == 0 && ofsexpr.disp ! = 0) || (dsiz == 1 && ! (-128 <= ofsexpr.disp && ofsexpr.disp <= 127))
		|| (dsiz == 2 && ! (-0x10000 <= ofsexpr.disp && ofsexpr.disp <= 0xffff)))
		*bp++ = REM_RANGE_ERR;
	if (dsiz > 4) {
		dsiz = 2;
		if (vbc & 0x08 /* d-bit */)
			dsiz = 4;
		if (-128 <= ofsexpr.disp && ofsexpr.disp <= 127)
			dsiz = 1;
		if ((vbc & 0x80 /* zero-bit */) ! = 0 && ofsexpr.disp == 0)
			dsiz = 0;
	}
	ifdef->vb[vba] = dsiz; /* zero/UCHAR/USHORT/UINT, const */
set_vbb:
	idx = 1;
	if (dsiz == 1)
		idx = 3;
	if (dsiz == 0)
		idx = 5;
	ifdef->vb[vbb] = def[idx + 0];
	ifdef->dat[vbb] = def[idx + 1];
fin:
	ifdef->bp = bp;
	return 0;
}

int microcode91(struct STR_IFDEFBUF *ifdef, struct STR_TERM *expr, int *def, signed char dsiz)
/* typ? 은 00~04를 서포트 */
/* 식중의 레지스터 필드는 완전하게 무시한다 */
/* def[0] : virtual-byte-code, zero-flag */
/* def[1-4] : typ2(b), dat2(b), typ2(c), dat2(c) */
/* def[5-8] : typ1(b), dat1(b), typ1(c), dat1(c) */

/* 91은 abc타입. c = b + 1; b, c의 타입은 SHORT_DB1에 고정
	...그래서 안된 것일까. SHORT_DB0일지도 모르니까.  */
{
	struct STR_OFSEXPR ofsexpr;
	struct STR_TERM *expr0 = expr;
	UCHAR idx, vbc = def[0], c, d;
	int vba = vbc & 0x07, vbb = (vbc >> 4) & 0x07;
	UCHAR *bp = ifdef->bp;
	calc_ofsexpr(&ofsexpr, &expr, 0); /* 식의 평가 */
	if (ofsexpr.err) {
err2:
		return 2; /* 파라미터 에러 */
	}
	if ((dsiz == 2 && (vbc & 0x08) ! = 0) || (dsiz == 4 && (vbc & 0x08) == 0)
		|| (dsiz == 0 && (vbc & 0x80 /* zero-bit */) == 0))
		return 3; /* data size error */
	if (ofsexpr.dispflag ! = 0) {
		if (dsiz <= 4) {
			ifdef->vb[vba] = dsiz; /* zero/UCHAR/USHORT/UINT, const */
			if (dsiz ! = 0) {
			//	ifdef->bp = bp;
				if (defnumexpr(ifdef, expr0, vba, (dsiz2mc98 - 1)[dsiz]))
					goto err2; /* 파라미터 에러 */
				bp = ifdef->bp;
			}
			goto set_vbb;
		}
		bp[0] = 0x91;
		bp[1] = 0x0f; /* 미결정 */
		bp[2] = 0x00;
		bp[3] = vbc;
		bp = put_expr(bp + 4, &expr0);
		idx = 3;
		def++;
		ifdef->vb[vba] = 0x7f;
		ifdef->vb[vbb] = 0x7f;
		ifdef->vb[vbb + 1] = 0x7f;
		do {
			if (idx == 1 && (vbc & 0x80 /* zero-bit */) == 0)
				continue;
			d = 2;
			do {
				*bp++ = (c = def[0]) | 0x30;
				vba = def[1];
				while (c) {
					*bp++ = vba & 0xff;
					vba >>= 8;
					c--;
				}
				def += 2;
			} while (--d);
		} while (--idx);
		goto fin;
	}

	ifdef->dat[vba] = ofsexpr.disp;
	if ((dsiz == 0 && ofsexpr.disp ! = 0) || (dsiz == 1 && ! (-128 <= ofsexpr.disp && ofsexpr.disp <= 127))
		|| (dsiz == 2 && ! (-0x10000 <= ofsexpr.disp && ofsexpr.disp <= 0xffff)))
		*bp++ = REM_RANGE_ERR;
	if (dsiz > 4) {
		dsiz = 2;
		if (vbc & 0x08 /* d-bit */)
			dsiz = 4;
		if (-128 <= ofsexpr.disp && ofsexpr.disp <= 127)
			dsiz = 1;
		if ((vbc & 0x80 /* zero-bit */) ! = 0 && ofsexpr.disp == 0)
			dsiz = 0;
	}
	ifdef->vb[vba] = dsiz; /* zero/UCHAR/USHORT/UINT, const */
set_vbb:
	idx = 1;
	if (dsiz == 1)
		idx = 5;
	if (dsiz == 0)
		idx = 9;
	ifdef->vb[vbb] = def[idx + 0];
	ifdef->dat[vbb] = def[idx + 1];
	ifdef->vb[vbb + 1] = def[idx + 2];
	ifdef->dat[vbb + 1] = def[idx + 3];
fin:
	ifdef->bp = bp;
	return 0;

}

static char mc98_typ[7] = { 0x01, 0x41, 0x02, 0x62, 0x04, 0x64, 0x61 };
static int mc98_min[7] = { 0,    -128, 0,      -0x10000, 0x80000000, 0x80000000, -0x100 };
static int mc98_max[7] = { 0xff, 0x7f, 0xffff, 0xffff,   0x7fffffff, 0x7fffffff, 0xff   };

int microcode94(struct STR_IFDEFBUF *ifdef, struct STR_TERM *expr, int *def)
/* typ? 은 30~34를 서포트 */
/* 식중의 레지스터 필드는 완전하게 무시한다 */
/* extlabel도 무시 */
/* def[0] : virtual-byte-code, len */
/* def[1-4] : (else) def0, def1 */
/* def[5-9] : case, def0, def1 */
/* 98~9 d나 서포트 */
{
	struct STR_OFSEXPR ofsexpr;
	int j = def[0], vb[2];
	UCHAR idx;
	int i = j >> 8, k;
	vb[0] = j;
	vb[1] = j >> 4;
	calc_ofsexpr(&ofsexpr, &expr, 0); /* 식의 평가 */
	if (ofsexpr.err) {
err:
		return 1;
	}
	if (ofsexpr.dispflag ! = 0)
		goto err; /* 지금은 라벨을 사용할 수 없으니까 */
	idx = 0;
	j = 5;
	do {
		if (def[j] == ofsexpr.disp)
			idx = j;
		j += 5;
	} while (--i);
	def = &def[idx + 1];
	for (i = 0; i < 2; i++) {
		if ((j = vb[i] & 0x0f) < 8) {
			ifdef->vb[j] = def[0] & 0x07; /* for 30~37 */
			ifdef->dat[j] = def[1];
			if ((k = def[0] - 0x98) >= 0) {
				ifdef->vb[j] = mc98_typ[k];
				ifdef->dat[j] = ofsexpr.disp;
				if (ofsexpr.disp < mc98_min[k] || mc98_max[k] < ofsexpr.disp)
					*(ifdef->bp)++ = REM_RANGE_ERR;
			}
		}
		def += 2;
	}
	return 0;
}

int defnumexpr(struct STR_IFDEFBUF *ifdef, struct STR_TERM *expr, UCHAR vb, UCHAR def)
/* 레지스터는 무시한다 */
{
	struct STR_OFSEXPR ofsexpr;
	struct STR_TERM *expr0 = expr;
	calc_ofsexpr(&ofsexpr, &expr, 0); /* 식의 평가 */
	vb &= 0x07;
	def &= 0x07;
	if (ofsexpr.err)
		return 1;
	ifdef->vb[vb] = mc98_typ[def];
	if (ofsexpr.dispflag ! = 0) {
		/* 라벨 검출 */
	//	if (ofsexpr.scale[0]) {
	//		return 1; /* 레지스터를 포함하고 있으면 에러 */
	//	}
		ifdef->vb[vb] |= 0x80;
		expr = expr0;
		ifdef->dat[vb] = put_expr(ifdef->expr[vb], &expr) - ifdef->expr[vb];
		return 0;
	}
	ifdef->dat[vb] = ofsexpr.disp;
	if (ofsexpr.disp < mc98_min[def] || mc98_max[def] < ofsexpr.disp)
	*(ifdef->bp)++ = REM_RANGE_ERR;
	return 0;
}

int getparam0(UCHAR *s, struct STR_STATUS *status)
{
	int i;
	return getparam(&s, status->src1, &i, status->expression,
		status->mem_expr, &status->ofsexpr, &status->expr_status);
}

int getconst(UCHAR **ps, struct STR_STATUS *status, int *p)
{
	int ret;
	ret = getparam(ps, status->src1, p, status->expression,
		status->mem_expr, &status->ofsexpr, &status->expr_status);
	return ret ! = 0x002f;
}

int testmem0(struct STR_STATUS *status, int gparam, int *prefix)
{
	return testmem(&status->ofsexpr, gparam, status, prefix);
}

/* 라벨 구조 */
/* id(4bytes), len(1bytes), ... */

int label2id(int len, UCHAR *label, int extflag)
{
	int i;
	UCHAR *s, c;
	static int extlabelnext = E_LABEL0;

	if (label[0] == '. ' && ((len >= 2 && label[1] ! = '. ') || len == 1)) {
		/* local label */
	//	s = locallabelbuf;
		i = len;
	//	s += *s;
		len += locallabelbuf - locallabelbuf0;
		do {
			i--;
			locallabelbuf[i] = label[i];
		} while (i > 0);
		label = locallabelbuf0;
	}

	for (s = labelbuf0; s < labelbuf; s += s[-1]) {
		s += 5;
		if (len ! = s[-1])
			continue;
		c = 0;
		i = len - 1;
		do {
			c |= label[i] ^ s[i];
		} while (--i >= 0);
		if (c == 0)
			goto fin;
	}
	if (extflag)
		i = extlabelnext++;
	else
		i = nextlabelid++;
	s[0] = (i      ) & 0xff;
	s[1] = (i >>  8) & 0xff;
	s[2] = (i >> 16) & 0xff;
	s[3] = (i >> 24) & 0xff;
	s[4] = len;
	s += 5;
	i = len - 1;
	do {
		s[i] = label[i];
	} while (--i >= 0);
	labelbuf = s + len;
fin:
	return get4b(s - 5);
}

UCHAR *id2label(int id)
{
	UCHAR *s;
	for (s = labelbuf0; s < labelbuf; s += *s + 1) {
		int i = get4b(s);
		s += 4;
		if (i == id)
			goto fin;
	}
	s = NULL;
fin:
	return s;
}

UCHAR *put_expr(UCHAR *s, struct STR_TERM **pexpr)
{
	static char ll_ope_list[] = {
		0x10 * 0, 0x11, 0x12, 0, /* s+, s-, s~, null */
		0x13, 0x14, 0x15, 0x17, /* +, -, *, /u */
		0x18, 0x19, 0x1a, 0, /* %u, /s, %s, null */
		0x1d, 0x1e, 0x1f, 0, /* &, |, ^, null */
		0x16, 0x1b, 0x1c, 0, /* <<, &>, |>, null */
	};
	struct STR_TERM *expr = *pexpr;
	int i, j;
	UCHAR c;

skip_single_plus:
	j = expr->term_type;
	i = expr->value;
	expr++;
	switch (j) {
	case 0: /* constant number */
		s = putimm(i, s);
		break;

	case 1: /* operator */
		if (i == 0)
			goto skip_single_plus; /* 단항 + */
		*s++ = ll_ope_list[i];
		s = put_expr(s, &expr);
		if (i >= 4)
			s = put_expr(s, &expr); /* 2항연산자 */
		break;

	case 2: /* register */
		s[0] = 0x00; /* const 0 */
		s[1] = 0x00;
		s += 2;
		break;

	case 3: /* label */
		c = 0x0b;
		if (i <= 0xff)
			c = 0x08;
		else if (i <= 0xffff)
			c = 0x09;
		else if (i <= 0xffffff)
			c = 0x0a;
		s[0] = c;
		s[1] = i & 0xff;
		s += 2;
		c &= 0x03;
		while (c) {
			i >>= 8;
			c--;
			*s++ = i & 0xff;
		}
	//	break;
	}
	*pexpr = expr;
	return s;
}

/*
이것과는 별도로, id로부터 각종 속성을 요구할 수도 있다.
defined-flag, public-flag, external-flag, const-flag, opt-flag, define-ptr, external-base
소속 섹션에 관한 정보는 없다. 값은 값이다.
COFF의 경우 opt가 1이라면 define-ptr가 섹션 번호를 나타낸다.
external-base가―1이라면 []은 붙지 않는다.

*/

static struct STR_TERM *skip_expr(struct STR_TERM *expr)
{
	if (expr++->term_type == 1 /* operator */) {
		int optype = expr->value;
		expr = skip_expr(expr);
		if (optype >= 4)
			expr = skip_expr(expr); /* 2항연산자 */
	}
	return expr;
}

struct STR_TERM *rel_expr(struct STR_TERM *expr, struct STR_DEC_EXPR_STATUS *status)
{
	struct STR_TERM *expr1 = skip_expr(expr), *p;
	p = expr1;
	do {
		*p = *(p - 1);
		p--;
	} while (p ! = expr);
	expr->term_type = 1; /* operator */
	expr->value = 5; /* 2항 - */
	if (status->dollar_label1 == 0xffffffff)
		status->dollar_label1 = nextlabelid++;
	(expr1 + 1)->term_type = 3; /* label */
	(expr1 + 1)->value = status->dollar_label1;
	return expr1 + 2;
}

static void setdec(unsigned int i, int n, UCHAR *s)
{
	s += n;
	do {
		*--s = (i % 10) + '0';
		if (--n == 0)
			break;
	} while (i /= 10);
	do {
		*--s = ' ';
	} while (--n);
	return;
}

static void sethex0(unsigned int i, int n, UCHAR *s)
{
	s += n;
	do {
		*--s = "0123456789ABCDEF"[i & 0xf];
		i >>= 4;
	} while (--n);
	return;
}
