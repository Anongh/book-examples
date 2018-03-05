/* "obj2bim" */
//	Copyright(C) 2003 H.Kawai
//	usage : obj2bim @(rule file) out:(file) [map:(file)] [stack:#] [(. obj/.lib file) ...]

#include "../include/stdio.h"
#include "../include/stdlib.h"
#include "../include/string.h"
#include "../include/setjmp.h"

/* OBJBUFSIZ는 전파일 합계의 제한 사이즈 */

#define FILEBUFSIZ		(4 * 1024)	/* rulefile의 최대 사이즈 */
#define	OBJBUFSIZ		(512 * 1024)	/* 512KB */
#define	LABELSTRSIZ		(8000)	/* 총 라벨수 (8000*140Bytes) */
#define	OBJFILESTRSIZ		(512)	/* 최대 오브젝트 파일수(512*260bytes)
// #define LINKSTRSIZ		(LABELSTRSIZ * 1)	/* 8000*12Bytes */
#define	MAXSECTION				  8	/* 1개의 .obj 파일 근처의 최대 섹션수 */

/* LINKSTRSIZ가 왠지 효과가 없다. cpp0는 버그를 갖고 있는가?  */
#define LINKSTRSIZ		LABELSTRSIZ

/* 합계 1833.5KB?  */

struct STR_OBJ2BIM {
	UCHAR *cmdlin; /* '\0'로 끝난다 */
	UCHAR *outname; /* '\0'로 끝난다, work의 어딘가에의 포인터 */
	UCHAR *mapname; /* '\0'로 끝난다, work의 어딘가에의 포인터 */
	UCHAR *dest0, *dest1; /* 출력 파일(dest0는 고쳐 쓸 수 있다) */
	UCHAR *map0, *map1; /* 출력 파일(map0는 고쳐 쓸 수 있다) */
	UCHAR *err0, *err1; /* 콘솔 메세지(err0는 고쳐 쓸 수 있다) */
	UCHAR *work0, *work1;
	int errcode;
};

extern GO_FILE GO_stdin, GO_stdout, GO_stderr;
extern struct GOL_STR_MEMMAN GOL_memman, GOL_sysman;
extern int GOL_abortcode;
extern jmp_buf setjmp_env;
void *GOL_memmaninit(struct GOL_STR_MEMMAN *man, size_t size, void *p);
void *GOL_sysmalloc(size_t size);
UCHAR **ConvCmdLine1(int *pargc, UCHAR *p);

#define SIZ_SYSWRK			(4 * 1024)

static int main0(const int argc, const char **argv, struct STR_OBJ2BIM *params);

int obj2bim_main(struct STR_OBJ2BIM *params)
{
//	static char execflag = 0;
	int argc;
	UCHAR **argv, *tmp0;
	UCHAR **argv1, **p;
	GO_stdout.p0 = GO_stdout.p = params->map0;
	GO_stdout.p1 = params->map1; /* stdout는 map */
	GO_stdout.dummy = ~0;
	GO_stderr.p0 = GO_stderr.p = params->err0;
	GO_stderr.p1 = params->err1;
	GO_stderr.dummy = ~0;

	/* 다중 실행 저지 (static를 재초기화하면 할 수 있지만) */
//	if (execflag)
//		return 7;
//	execflag = 1;

	if (setjmp(setjmp_env)) {
		params->err0 = GO_stderr.p;
		return GOL_abortcode;
	}

	if (params->work1 - params->work0 < SIZ_SYSWRK + 16 * 1024)
		return GO_TERM_WORKOVER;
	GOL_memmaninit(&GOL_sysman, SIZ_SYSWRK, params->work0);
	GOL_memmaninit(&GOL_memman, params->work1 - params->work0 - SIZ_SYSWRK, params->work0 + SIZ_SYSWRK);
	argv = ConvCmdLine1(&argc, params->cmdlin);

	params->errcode = main0(argc, argv, params);
	params->map0 = GO_stdout.p;

skip:
	/* 버퍼를 출력 */
	GOL_sysabort(0);
}


const int get32b(unsigned char *p)
{
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

const int get32l(unsigned char *p)
{
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

struct LABELSTR {
	unsigned char type, sec, flags, align;
	/* type  0 xff:미사용 */
	/* type  0x01:global/local label */
	/* type  0x02:constant */
	/* flags bit0 : used */
	/* flags bit1 : linked */
	unsigned int name[128 / 4 - 4];
	struct OBJFILESTR *name_obj; /* 로컬 .obj에의 포인터 .public라면 NULL */
	struct OBJFILESTR *def_obj; /* 소속 오브젝트 파일 */
	unsigned int offset;
};

struct OBJFILESTR {
	struct {
		unsigned char *ptr;
		int links, sh_paddr, sectype;
		unsigned int size, addr;
		struct LINKSTR *linkptr;
		signed char align;
		unsigned char flags; /* bit0 : pure-COFF(0) /MS-COFF(1) */
	} section[MAXSECTION];
	unsigned int flags;
	/* flags  0xff : terminator */
	/* flags  bit0 : link */
};

struct LINKSTR {
	unsigned char type, dummy[3];
	int offset;
	struct LABELSTR *label;
	/* type  0x06:absolute */
	/* type  0x14:relative */
};

static unsigned char *skipspace(unsigned char *p);
static void loadlib(unsigned char *p);
static void loadobj(unsigned char *p);
static const int getnum(unsigned char **pp);
static struct LABELSTR *symbolconv0(unsigned char *s, struct OBJFILESTR *obj);
static struct LABELSTR *symbolconv(unsigned char *p, unsigned char *s, struct OBJFILESTR *obj);
static void link0(const int sectype, int *secparam, unsigned char *image);

static struct LABELSTR *label0 = NULL;
static struct OBJFILESTR *objstr0;
static unsigned char *objbuf0;

unsigned char *osain(char *f, int *size);
void osaunmap();

static const int alignconv(int align)
{
	/* align이 2로 해야할지 어떨지는 귀찮아서 확인하고 있지 않다 */
	int i; 
	if ((i = align) >= 1) {
		align = 1;
		while (i >>= 1);
			align++;
	}
	return align;
}

static int main0(const int argc, const char **argv, struct STR_OBJ2BIM *params)
{
	FILE *fp;
	unsigned char *filebuf, *p /* , *filename, *mapname */;
	unsigned char *s, *ps, *t, *image[2];
	struct LABELSTR *labelbuf[16];
	int filesize, i, j, labelbufptr = 0;
	int section_param[12]; /* 최초의 4개가 코드, 다음의 4개는 데이터 */
	struct LABELSTR *label;
	struct OBJFILESTR *obj;
	struct LINKSTR *ls;

	if (argc <= 2) {
		fprintf(stderr,
			"\"obj2bim\" flexible linker for COFF   copyright (C) 2003 H.Kawai\n"
			"usage : \n"
			">obj2bim @(rule file) out:(file) [map:(file)] [stack:#] [(. obj/.lib file) ...]\n"
		);
		return 1;
	}

#if 0

	/* 오브젝트 파일 가공(align 지정) */

	if (strcmp(argv[1], "-fixobj") == 0) {
		/* 한 번에 복수의 파일을 지정하지 않을 것 */
		/* 복수 파일의 가공에는 대응하고 있지 않다 */
		int text_align = -1, data_align = -1, bss_align = -1;
		filebuf = (unsigned char *) malloc(FILEBUFSIZ);
		filename = NULL;
		for (i = 2; i < argc; i++) {
			p = argv[i] + 11;
			if (strncmp(argv[i], "text_align:", 11) == 0) {
			//	p = argv[i] + 11;
				text_align = alignconv(getnum(&p));
			} else if (strncmp(argv[i], "data_align:", 11) == 0) {
			//	p = argv[i] + 11;
				data_align = alignconv(getnum(&p));
			} else if (strncmp(argv[i], "bss_align:", 10) == 0) {
			//	p = argv[i] + 10;
				p--;
				bss_align = alignconv(getnum(&p));
			} else if (filename == NULL && (fp = fopen(filename = argv[i], "rb")) ! = NULL) {
				filesize = fread(filebuf, 1, FILEBUFSIZ, fp);
				fclose(fp);
			} else {
				fprintf(stderr, "Command line error : %s\n", argv[i]);
				free(filebuf);
				return 1;
			}
		}

		if (filebuf[0x00] - 0x4c | filebuf[0x01] - 0x01 | filebuf[0x03] | filebuf[0x10] | filebuf[0x11]) {
			free(filebuf);
			fprintf(stderr, "Unknown . obj file format\n");
			return 2;
		}
		for (i = 0; i < filebuf[0x02]; i++) {
			p = &filebuf[0x14 + i * 0x28];
			j = -1;
			if (p[0x24] == 0x20) {
				/* text section */
				j = text_align;
			} else if (p[0x24] == 0x40) {
				/* data section */
				j = data_align;
			} else if (p[0x24] == 0x80) {
				/* bss section */
				j = bss_align;
			}
			if (j >= 0) {
				p[0x26] &= 0x0f;
				p[0x26] |= j << 4;
			}
		}

		fp = fopen(filename, "wb");
		fwrite(filebuf, 1, filesize, fp);
		fclose(fp);
		free(filebuf);
		return 0;
	}

#endif

	/* 범용 링커 */

	s = (unsigned char *) malloc(1024);

	if (argv[1][0] ! = '@') {
		fprintf(stderr, "The argv[1] must be a rule file : %s\n", argv[1]);
		return 1;
	}

	p = osain(argv[1] + 1, &filesize);
	filebuf = (unsigned char *) malloc(FILEBUFSIZ);
	if (p == NULL || filesize > FILEBUFSIZ - 1) {
		fprintf(stderr, "Can't open rule file\n");
		return 2;
	}
	for (i = 0; i < filesize; i++)
		filebuf[i] = p[i];
	osaunmap();
	p = filebuf;
	filebuf[filesize] = '\0'; /* EOF mark */

	/* (format section) */

	p = skipspace(p);
	if (strncmp(p, "format", 6)) {
err_rule_format0:
		fprintf(stderr, "Rule file error : can't find format section\n");
		free(filebuf);
		return 2;
	}
	p = skipspace(p + 6);
	if (*p ! = ':')
		goto err_rule_format0;
	p = skipspace(p + 1);

//	section_param[0 /* align */ + 0 /* code */] = -1;
//	section_param[1 /* logic */ + 0 /* code */] = -1;
//	section_param[2 /* file */  + 0 /* code */] = -1;
//	section_param[0 /* align */ + 4 /* data */] = -1;
//	section_param[1 /* logic */ + 4 /* data */] = -1;
//	section_param[2 /* file */  + 4 /* data */] = -1;
	for (i = 0; i < 12; i++)
		section_param[i] = -1;

	for (;;) {
		if (*p == '\0' || strncmp(p, "file", 4) == 0 || strncmp(p, "label", 5) == 0)
			break;
		i = -1;
		if (strncmp(p, "code", 4) == 0)
			i = 0;
		if (strncmp(p, "data", 4) == 0)
			i = 4;
		if (i >= 0) {
			p = skipspace(p + 4);
			if (*p ! = '(')
				goto err_rule_format1;
			p++;
			for (;;) {
				if (strncmp(p, "align", 5) == 0) {
					p += 5;
					j = 0;
				} else if (strncmp(p, "logic", 5) == 0) {
					p += 5;
					j = 1;
				} else if (strncmp(p, "file", 4) == 0) {
					p += 4;
					j = 2;
				} else if (*p == ')')
					break;
				else
					goto err_rule_format1;
				p = skipspace(p);
				if (*p ! = ':')
					goto err_rule_format1;
				p = skipspace(p + 1);
				if ('0' <= *p && *p <= '9')
					section_param[i + j] = getnum(&p);
				else if (strncmp(p, "code_end", 8) == 0) {
					section_param[i + j] = -2;
					p += 8;
				} else if (strncmp(p, "data_end", 8) == 0) {
					section_param[i + j] = -3;
					p += 8;
				} else if (strncmp(p, "stack_end", 9) == 0) {
					section_param[i + j] = -4;
					p += 9;
				} else
					goto err_rule_format1;
				p = skipspace(p);
				if (*p == ')')
					break;
				if (*p ! = ', ')
					goto err_rule_format1;
				p = skipspace(p + 1);
			}
			p = skipspace(p + 1);
			if (*p ! = ';')
				goto err_rule_format1;
			p = skipspace(p + 1);
		} else {
err_rule_format1:
			fprintf(stderr, "Rule file error : syntax error in format section\n");
			free(filebuf);
			return 3;
		}
	}

	/* (command line file) */

//	filename = NULL;
//	mapname = NULL;

	for (i = 2; i < argc; i++) {
		ps = s;
		t = argv[i];
		if (strncmp(t, "out:", 4) == 0) {
			/* filename = t + 4; */
			params->outname = t + 4;
			continue;
		}
		if (strncmp(t, "map:", 4) == 0) {
			/* mapname = t + 4; */
			params->mapname = t + 4;
			continue;
		}
		if (strncmp(t, "stack:", 6) == 0) {
			t += 6;
			j = getnum(&t);
			if (section_param[1 /* logic */ + 0 /* code */] == -4 /* stack_end */)
				section_param[1 /* logic */ + 0 /* code */] = j;
			if (section_param[1 /* logic */ + 4 /* data */] == -4 /* stack_end */)
				section_param[1 /* logic */ + 4 /* data */] = j;
			continue;
		}

		while (*ps++ = *t++);
		if (ps[-2] == 0x22 && *s == 0x22) {
			ps[-2] = '\0';
			ps = s + 1;
		} else
			ps = s;
		t = osain(ps, &j);
		if (t == NULL) {
			fprintf(stderr, "Command line error : can't open file : %s\n", ps);
			free(filebuf);
			return 8;
		}
		if (strncmp(t, "! <arch>\x0a/               ", 24) == 0 && (t[0x42] - 0x60 | t[0x43] - 0x0a) == 0)
			loadlib(t);
		else if ((t[0] - 0x4c | t[1] - 0x01) == 0)
			loadobj(t);
		else {
			fprintf(stderr, "Command line error : unknown file format : %s\n", ps);
			free(filebuf);
			return 9;
		}
		osaunmap();
	}

	/* (file section) */

	if (strncmp(p, "file", 4) == 0) {
		p = skipspace(p + 4);
		if (*p ! = ':') {
err_rule_illsec:
			fprintf(stderr, "Rule file error : found a unknown section\n");
			free(filebuf);
			return 4;
		}

		/* 파일을 자꾸자꾸 읽어들여, 해석하고 버퍼에 담는다 */
		for (;;) {
			p = skipspace(p + 1); /* ':'나 ';'를 건너뛴다 */
			if (*p == '\0' || (strncmp(p, "label", 5) == 0 && (p[5] == ':' || p[5] <= ' ')))
				break;
			ps = s;
			while (*p ! = '\0' && *p ! = ';')
				*ps++ = *p++;
			while (ps[-1] == ' ')
				ps--;
			*ps = '\0';
			p = skipspace(p);
			if (*p ! = ';') {
				fprintf(stderr, "Rule file error : syntax error in file section\n");
				free(filebuf);
				return 5;
			}
			if (ps[-1] == 0x22 && *s == 0x22) {
				ps[-1] = '\0';
				ps = s + 1;
			} else
				ps = s;
			t = osain(ps, &j);
			if (t == NULL) {
				fprintf(stderr, "Rule file error : can't open file : %s\n", ps);
				free(filebuf);
				return 6;
			}
			if (strncmp(t, "! <arch>\x0a/               ", 24) == 0 && (t[0x42] - 0x60 | t[0x43] - 0x0a) == 0)
				loadlib(t);
			else if ((t[0] - 0x4c | t[1] - 0x01) == 0)
				loadobj(t);
			else {
				fprintf(stderr, "Rule file error : unknown file format : %s\n", ps);
				free(filebuf);
				return 7;
			}
			osaunmap();
		}
	}

	/* (label section) */

	if (strncmp(p, "label", 5) ! = 0) {
err_rule_label:
		fprintf(stderr, "Rule file error : can't find label section\n");
		free(filebuf);
		return 10;
	}
	p = skipspace(p + 5);
	if (*p ! = ':')
		goto err_rule_label;
	for (;;) {
		p = skipspace(p + 1);
		if (*p == '\0')
			break;
		ps = s;
		for (;;) {
			if (*p == ';' || *p <= ' ' || *p == '/')
				break;
			*ps++ = *p++;
		}
		*ps = '\0';
		label = symbolconv0(s, NULL);
		labelbuf[labelbufptr++] = label;
		if (label->def_obj)
			label->flags |= 0x01; /* used */
		else
			fprintf(stderr, "Warning : can't link %s\n", label->name);
		p = skipspace(p);
		if (*p ! = ';') {
			fprintf(stderr, "Rule file error : syntax error in label section\n");
			free(filebuf);
			return 11;
		}
	}

	/* 필요한 .obj 파일을 선택한다 */
	label = label0;
	for (;;) {
		if (label->type == 0xff)
			break; /* 선택 완료 */
		if ((label->flags & 0x03) ! = 0x01 /* used && not linked */) {
			label++;
			continue;
		}

		obj = label->def_obj;
		if (obj == NULL) {
			label++;
			continue;
		}
		obj->flags |= 0x01;
		for (label = label0; label->type ! = 0xff; label++) {
			if (label->def_obj == obj) {
				label->flags |= 0x02; /* linked */
			}
		}
		for (j = 0; j < MAXSECTION; j++) {
			if (obj->section[j]. size == 0)
				continue;
			ls = obj->section[j]. linkptr;
			for (i = obj->section[j]. links; i > 0; i--, ls++)
				ls->label->flags |= 0x01; /* used */
		}
		label = label0;
	}

	if (objstr0 == NULL)
		return 99;

//	image[0] = filebuf;
//	image[1] = filebuf + FILEBUFSIZ / 2;

	image[0] = params->dest0;
	image[1] = params->dest0 + (params->dest1 - params->dest0) / 2;

	/* . obj의 각 섹션의 논리 주소를 확정하여, 이미지를 구축 */
	section_param[0 /* align */ + 8 /* bss */] = section_param[0 /* align */ + 4 /* data */];
	if (section_param[1 /* logic */ + 0 /* code */] ! = -3 /* data_end */) {
		/* 코드가 앞 */
		link0(0 /* code */, &section_param[0 /* code */], image[0]);
		if (section_param[1 /* logic */ + 4 /* data */] == -2 /* code_end */)
			section_param[1 /* logic */ + 4 /* data */] = section_param[3 /* logic+size */ + 0 /* code */];
		link0(1 /* data */, &section_param[4 /* data */], image[1]);
		section_param[1 /* logic */ + 8 /* bss */] = section_param[3 /* logic+size */ + 4 /* data */];
		p = image[1] + section_param[3 /* logic+size */ + 4 /* data */] - section_param[1 /* logic */ + 4 /* data */];
		link0(2 /* bss */, &section_param[8 /* bss */], p);
	} else {
		/* 데이터가 앞 */
		link0(1 /* data */, &section_param[4 /* data */], image[1]);
		section_param[1 /* logic */ + 8 /* bss */] = section_param[3 /* logic+size */ + 4 /* data */];
		p = image[1] + section_param[3 /* logic+size */ + 4 /* data */] - section_param[1 /* logic */ + 4 /* data */];
		link0(2 /* bss */, &section_param[8 /* bss */], p);
		if (section_param[1 /* logic */ + 0 /* code */] == -3 /* data_end */)
			section_param[1 /* logic */ + 0 /* code */] = section_param[3 /* logic+size */ + 8 /* bss */];
		link0(0 /* code */, &section_param[0 /* code */], image[0]);
	}

	/* 라벨 값을 확정 */
	for (label = label0; label->type ! = 0xff; label++) {
		if ((label->flags & 0x03 /* used | linked */) == 0)
			continue;
		if (label->type ! = 0x01 /* global/local label */)
			continue;
		obj = label->def_obj;
		if (obj == NULL)
			continue;
		label->offset += obj->section[label->sec - 1]. addr;
	}

	/* mapfile의 출력 */
	/* map은 stdout에 */
	if (params->mapname) {
		if ((fp = /* fopen(mapname, "w") */ stdout) == NULL)
			fprintf(stderr, "Warning : can't open mapfile\n");
		else {
			i = section_param[3 /* logic+size */ + 0 /* code */] - section_param[1 /* logic */ + 0 /* code */];
			fprintf(fp, "text size : %6d(0x%05X) \n", i, i);
			i = section_param[3 /* logic+size */ + 4 /* data */] - section_param[1 /* logic */ + 4 /* data */];
			fprintf(fp, "data size : %6d(0x%05X) \n", i, i);
			i = section_param[3 /* logic+size */ + 8 /* bss  */] - section_param[1 /* logic */ + 8 /* bss  */];
			fprintf(fp, "bss  size : %6d(0x%05X) \n\n", i, i);

			/* 이하는 제대로 된 소트를 쓰는 것이 귀찮아서 부실을 하고 있다 */
			for (i = 0; i < 3; i++) {
				unsigned int value = 0, min;
				for (;;) {
					min = 0xffffffff;
					for (label = label0; label->type ! = 0xff; label++) {
						if ((label->flags & 0x03 /* used | linked */) == 0)
							continue;
						if (label->def_obj == NULL /* || label->name_obj ! = NULL */)
							continue;
						if (label->def_obj->section[label->sec - 1]. sectype ! = i || label->type ! = 0x01)
							continue;
						if (label->offset < value)
							continue;
						if (min > label->offset)
							min = label->offset;
					}
					if (min == 0xffffffff)
						break;
					for (label = label0; label->type ! = 0xff; label++) {
						if ((label->flags & 0x03 /* used | linked */) == 0)
							continue;
						if (label->def_obj == NULL /* || label->name_obj ! = NULL */)
							continue;
						if (label->def_obj->section[label->sec - 1]. sectype ! = i || label->type ! = 0x01)
							continue;
						if (label->offset ! = min)
							continue;
						if (label->name_obj)
							fprintf(fp, "0x%08X : (%s) \n", label->offset, label->name);
						else
							fprintf(fp, "0x%08X : %s\n", label->offset, label->name);
					}
					value = min + 1;
				}
			}
		//	fclose(fp);
		}
	}

	/* linking */
	/* 원버전：objbuf부터 filebuf에 전송 */
	/* 이것을 dest로 하도록 변경해야 함 */
	for (obj = objstr0; obj->flags ! = 0xff; obj++) {
		if ((obj->flags & 0x01 /* link */) == 0)
			continue;
		for (i = 0; i < MAXSECTION; i++) {
			unsigned char *p0;
			if (obj->section[i]. size == 0)
				continue;
			if ((j = obj->section[i]. sectype) >= 2)
				continue;
		//	int rel0 = obj->section[i]. sh_paddr - obj->addr[i];
			p0 = image[j] + obj->section[i]. addr - section_param[1 + j * 4];
			ls = obj->section[i]. linkptr;
			for (j = obj->section[i]. links; j > 0; j--, ls++) {
				int value;
				label = ls->label;
				p = p0 + ls->offset;
				if (label->def_obj == NULL && (label->flags & 0x01) ! = 0) {
					fprintf(stderr, "Warning : can't link %s\n", label->name);
					label->flags &= 0xfe;
				}
				value = get32l(p) + label->offset;
				if (ls->type == 0x14) {
					value -= obj->section[i]. addr;
					if (obj->section[i]. flags & 0x01)
						value -= ls->offset + 4;
				}
				p[0] = value         & 0xff;
				p[1] = (value >>  8) & 0xff;
				p[2] = (value >> 16) & 0xff;
				p[3] = (value >> 24) & 0xff;
			}
		}
	}

	/* 파일에 출력 */
	/* 원버전：filebuf→objbuf */
	/* dest→objbuf로 변경(전부 끝나면 objbuf부터 dest에 재전송) */
	filesize = 0;
	p = objbuf0;
	for (i = 0; i < OBJBUFSIZ; i++)
		p[i] = '\0';
	if (section_param[2 /* file */ + 0 /* code */] ! = -3 /* data_end */) {
		/* 코드가 앞 */
		p = objbuf0 + section_param[2 /* file */ + 0 /* code */];
		t = image[0];
		for (i = section_param[3 + 0 /* code */] - section_param[1 + 0 /* code */]; i > 0; i--)
			*p++ = *t++;
		i = p - objbuf0;
		if (filesize < i)
			filesize = i;
		if (section_param[2 /* file */ + 4 /* data */] == -2 /* code_end */) {
			j = section_param[0 /* align */ + 4 /* data */] - 1;
			if (j < 0)
				j = 0;
			while (i & j)
				i++;
			section_param[2 /* file */ + 4 /* data */] = i;
		}
		p = objbuf0 + section_param[2 /* file */ + 4 /* data */];
		t = image[1];
		for (i = section_param[3 + 8 /* bss */] - section_param[1 + 4 /* data */]; i > 0; i--)
			*p++ = *t++;
		i = p - objbuf0;
		if (filesize < i)
			filesize = i;
	} else {
		/* 데이터가 앞 */
		p = objbuf0 + section_param[2 /* file */ + 4 /* data */];
		t = image[1];
		for (i = section_param[3 + 8 /* bss */] - section_param[1 + 4 /* data */]; i > 0; i--)
			*p++ = *t++;
		i = p - objbuf0;
		if (filesize < i)
			filesize = i;
		if (section_param[2 /* file */ + 0 /* data */] == -3 /* data_end */) {
			j = section_param[0 /* align */ + 0 /* code */] - 1;
			if (j < 0)
				j = 0;
			while (i & j)
				i++;
			section_param[2 /* file */ + 0 /* code */] = i;
		}
		p = objbuf0 + section_param[2 /* file */ + 0 /* code */];
		t = image[0];
		for (i = section_param[3 + 0 /* code */] - section_param[1 + 0 /* code */]; i > 0; i--)
			*p++ = *t++;
		i = p - objbuf0;
		if (filesize < i)
			filesize = i;
	}
	p = objbuf0;
	*((int *) p)        = section_param[3 + 0 /* code */] - section_param[1 + 0 /* code */]; /* 코드 사이즈 */
	*((int *) (p +  4)) = section_param[2 + 0 /* code */]; /* 파일중의 개시 주소 */
	*((int *) (p +  8)) = section_param[1 + 0 /* code */]; /* 링크 해결시의 개시 주소 */
	*((int *) (p + 12)) = section_param[3 + 8 /* bss  */] - section_param[1 + 4 /* data */]; /* 데이터 사이즈 */
	*((int *) (p + 16)) = section_param[2 + 4 /* data */]; /* 파일중의 개시 주소 */
	*((int *) (p + 20)) = section_param[1 + 4 /* data */]; /* 링크 해결시의 개시 주소 */
	t = p + 24;
	for (i = 0; i < labelbufptr; i++, t += 4)
		*((int *) t) = labelbuf[i]->offset;

/* 
	free(filebuf);
	fp = NULL;
	if (filename)
		fp = fopen(filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Can't open output file\n");
		return 12;
	}
	fwrite(objbuf0, 1, filesize, fp);
*/
	for (i = 0; i < filesize; i++)
		*(params->dest0)++ = objbuf0[i];


	return 0;
}

static unsigned char *skipspace(unsigned char *p)
{
reloop:
	while (*p ! = '\0' && *p <= ' ')
		p++;
	if (p[0] == '/' && p[1] == '/') {
		while (*p ! = '\0' && *p ! = '\n')
			p++;
		if (*p == '\n')
			p++;
		goto reloop;
	}
	if (p[0] == '/' && p[1] == '*') {
		while (*p ! = '\0' && (p[0] ! = '*' || p[1] ! = '/'))
			p++;
		if (p[0] == '*' && p[1] == '/')
			p += 2;
		goto reloop;
	}
	return p;
}

#if 0

static void loadlib(unsigned char *p)
{
	int *obj, i, j;
	unsigned char *t = &p[0x48];
	obj = malloc(1024 * sizeof (int));
	obj[0] = 0;
	for (j = get32b(&p[0x44]); j > 0; j--) {
		int objofs = get32b(t);
		for (i = 0; obj[i]; i++) {
			if (obj[i] == objofs)
				goto skip;
		}
		obj[i] = objofs;
		obj[i + 1] = 0;
		loadobj(p + objofs + 0x3c);
skip:
		t += 4;
	}
	free(obj);
	return;
}

#endif

static int getdec(unsigned char *p)
{
	int i = 0;
	while (*p == ' ')
		p++;
	while ('0' <= *p && *p <= '9')
		i = i * 10 + (*p++ - '0');
	return i;
}

static void loadlib(unsigned char *p)
{
	unsigned char *t;
	int i, j;
	i = getdec(&p[0x38]) + 0x44;
	if (strncmp(&p[i], "/       ", 8) ! = 0) {
		fprintf(stderr, "Internal error : loadlib(1) \n");
		return;
	}
	t = &p[i + 0x3c];
	for (j = *t; j > 0; j--) {
		t += 4;
		loadobj(p + get32l(t) + 0x3c);
	}
	return;
}

static void loadobj(unsigned char *p)
{
	static struct OBJFILESTR *next_objstr = NULL;
	static unsigned char *next_objbuf;
	static struct LINKSTR *next_linkstr;
	int i, j, k, sec0, sec, value, bss_alloc;
	unsigned char *q;
	struct LABELSTR *label;
	struct OBJFILESTR *objstr;

	if (next_objstr == NULL) {
		objstr0 = next_objstr = malloc(OBJFILESTRSIZ * sizeof (struct OBJFILESTR));
		objbuf0 = next_objbuf = malloc(OBJBUFSIZ);
		next_linkstr = malloc(LINKSTRSIZ * sizeof (struct LINKSTR));
	}

	/* 헤더 체크 */
	if (p[0x00] - 0x4c | p[0x01] - 0x01) {
		fprintf(stderr, "Internal error : loadobj(1) %16. 16s\n", &p[-0x3c]);
		return;
	}

	objstr = next_objstr;

	for (i = 0; i < MAXSECTION; i++) {
		objstr->section[i]. size = 0;
		objstr->section[i]. sectype = 3; /* 공백 */
		objstr->section[i]. flags = 0;
	}
	objstr->flags = 0x00;
	if ((p[0x02] | p[0x03] << 8) > MAXSECTION) {
		/* section수가 너무 많다 */
		fprintf(stderr, "Internal error : loadobj(2) \n");
		return;
	}
	for (i = 0; i < (p[0x02] | p[0x03] << 8); i++) {
		q = p + 0x14 + i * 0x28;
		if (q[0x24] == 0x20) {
			/* text section */
			j = 0;
		} else if (q[0x24] == 0x40) {
			/* data section */
			j = 1;
		} else if (q[0x24] == 0x80) {
			/* bss section */
			j = 2;
		} else {
			objstr->section[i]. sectype = 4; /* unknown */
			continue;
		}
		objstr->section[i]. sectype = j;
		if (q[0x27] & 0xe0)
			objstr->section[i]. flags = 0x01; /* MS-COFF */

		objstr->section[i]. size = get32l(&q[0x10]);
		objstr->section[i]. align = q[0x26] >> 4;
		if (objstr->section[i]. sectype < 2) {	/* text or data */
			unsigned char *s, *t;
			struct LINKSTR *ls;

			/* next_objbuf에 전송 */
			objstr->section[i]. ptr = next_objbuf;
			objstr->section[i]. links = q[0x20] | q[0x21] << 8;
			objstr->section[i]. sh_paddr = get32l(&q[0x08]);
			objstr->section[i]. linkptr = ls = next_linkstr;
			s = next_objbuf;
			t = p + get32l(&q[0x14]);
			for (k = objstr->section[i]. size; k > 0; k--)
				*s++ = *t++;
			next_objbuf = s;

			/* next_linkstr에 전송 */
		//	ls = next_linkstr;
			t = p + get32l(&q[0x18]);
			for (k = objstr->section[i]. links; k > 0; k--, t += 0x0a) {
				ls->offset = get32l(&t[0x00]) /* - objstr->section[i]. sh_paddr */;
				s = p + get32l(&t[0x04]) * 0x12 + get32l(&p[0x08]);
			//	if (strncmp(s, ". text\0\0\0", 8) == 0)
			//		goto link_skip;
			//	if (strncmp(s, ". data\0\0\0", 8) == 0)
			//		goto link_skip;
			//	if (strncmp(s, ". bss\0\0\0\0", 8) == 0)
			//		goto link_skip;
				ls->label = symbolconv(p, s, objstr);
			//	ls->label = label0 + get32l(&t[0x04]);
				if (t[0x08] == 0x06 || t[0x08] == 0x14) {
					ls->type = t[0x08];
					ls++;
				} else {
					fprintf(stderr, "Found a unknown reloc_type 0x%02X.  Skipped\n", t[0x08]);
link_skip:
					objstr->section[i]. links--;
				}
			}
			next_linkstr = ls;
			/* 터미네이터는 있을까?  */
		//	printf("0x%04X 0x%04X 0x%02X\n", get32l(&t[0x00]), get32l(&t[0x04]), t[0x08] | t[0x09] << 8);
			/* 없었다··· */
		}
	}

	/* 심볼 정의 */
	q = p + get32l(&p[0x08]);
	for (i = get32l(&p[0x0c]); i > 0; i -= j, q += j * 0x12) {
		j = q[0x11] /* numaux */ + 1;
		sec = q[0x0c];
		if (sec ! = 0 && sec < 0xf0)
			sec0 = sec;
	//	if ((q[0x0e] | q[0x0f] | q[0x10] - 0x03) == 0 && q[0x11] ! = 0) {
	//		/* section symbols */
	//	//	sec0 = sec;
	//		continue;
	//	}
		value = get32l(&q[0x08]);
		switch(q[0x10]) {
		case 0x02: /* public symbol */
		case 0x03: /* static symbol */
		case 0x06: /* label */
		//	if (q[0x11] /* numaux */)
		//		break;
			if (strncmp(q, "@comp.id", 8) == 0)
				break;
		//	if (strncmp(q, ". text\0\0\0", 8) == 0)
		//		break;
		//	if (strncmp(q, ". data\0\0\0", 8) == 0)
		//		break;
		//	if (strncmp(q, ". bss\0\0\0\0", 8) == 0)
		//		break;
			if (sec == 0xfe /* debugging symbol */)
				break;
			label = symbolconv(p, q, objstr);
			if (sec == 0 /* extern symbol */ && value == 0)
				break;
			if (objstr->section[sec0 - 1]. sectype == 2 /* bss */ && sec == 0 /* extern symbol */) {
				int align = 2, sec_align, sec_size = objstr->section[sec0 - 1]. size;
				while (align <= value)
					align <<= 1;
				align >>= 1;
				if (sec_align = objstr->section[sec0 - 1]. align) {
					k = 1 << (sec_align - 1);
					if (align > k)
						align = k;
				}
				while (sec_size & (k - 1))
					sec_size++;
				k = value;
				value = sec_size;
				objstr->section[sec0 - 1]. size = sec_size + k;
			}
			if (label->def_obj)
				fprintf(stderr, "Warning : redefine %s\n", label->name);
			label->offset = value;
			label->sec = sec0;
			label->type = 1 + (sec == 0xff);
			label->def_obj = objstr;
			break;

		case 0x67: /* file name */
			break; /* 무시해 버린다 */

		default:
			fprintf(stderr, "unknown storage class : %02X\n", q[0x10]);
		}
	}

	objstr++;
	objstr->flags = 0xff;
	next_objstr = objstr;
	return;
}

static const int getnum(unsigned char **pp)
{
	unsigned char *p = *pp;
	int i = 0, j, base = 10;
//	p = skipspace(p);
	if (*p == '0') {
		p++;
		if (*p == 'X' || *p == 'x') {
			base = 16;
			p++;
		} else if (*p == 'O' || *p == 'o') {
			base = 8;
			p++;
		}
	}
	p--;
	for (;;) {
		p++;
		if (*p == '_')
			continue;
		j = 99;
		if ('0' <= *p && *p <= '9')
			j = *p - '0';
		if ('A' <= *p && *p <= 'F')
			j = *p - 'A' + 10;
		if ('a' <= *p && *p <= 'f')
			j = *p - 'a' + 10;
		if (base <= j)
			break;
		i = i * base + j;
	}
	if (*p == 'k' || *p == 'K') {
		i *= 1024;
		p++;
	} else if (*p == 'm' || *p == 'M') {
		i *= 1024 * 1024;
		p++;
	} else if (*p == 'g' || *p == 'G') {
		i *= 1024 * 1024 * 1024;
		p++;
	}
	*pp = p;
	return i;
}

static struct LABELSTR *symbolconv0(unsigned char *s, struct OBJFILESTR *obj)
{
	unsigned char *n;
	struct LABELSTR *label;
	int i, *name;

	name = malloc((128 / 4 - 4) * sizeof (int));

	if (label0 == NULL) {
		label0 = malloc(LABELSTRSIZ * sizeof (struct LABELSTR));
		label0->type = 0xff;
	}

	for (i = 0; i < 128 / 4 - 4; i++)
		name[i] = 0;
	n = (unsigned char *) name;
	while (*n++ = *s++);

	for (label = label0; label->type ! = 0xff; label++) {
		if (name[0] ! = label->name[0])
			continue;
		if (name[1] ! = label->name[1])
			continue;
		if (name[2] ! = label->name[2])
			continue;
		if (obj ! = label->name_obj)
			continue;
		for (i = 3; i < 128 / 4 - 4; i++) {
			if (name[i] ! = label->name[i])
				goto next_label;
		}
		goto fin;
next_label:
		;
	}
	label->type = 0x00;
	label->name_obj = obj;
	label->flags = 0x00;
	label->def_obj = NULL;
	label->offset = 0;
	for (i = 0; i < 128 / 4 - 4; i++)
		label->name[i] = name[i];
	label[1]. type = 0xff;
fin:
	free(name);
	return label;
}

static struct LABELSTR *symbolconv(unsigned char *p, unsigned char *s, struct OBJFILESTR *obj)
{
	unsigned char tmp[12], *n;

	if (s[0x10] == 0x02)
		obj = NULL;	/* external */

	if (s[0x00] | s[0x01] | s[0x02] | s[0x03]) {
		int i;
		for (i = 0; i < 8; i++)
			tmp[i] = s[i];
		tmp[8] = '\0';
		n = tmp;
	} else
		n = p + get32l(&p[0x08]) + get32l(&p[0x0c]) * 0x12 + get32l(&s[0x04]);

	return symbolconv0(n, obj);
}

static void link0(const int sectype, int *secparam, unsigned char *image)
/* .obj의 각 섹션의 논리 주소를 확정시킨다 */
{
	struct OBJFILESTR *obj;
	int addr = secparam[1 /* logic */], i, j;
	unsigned char *p;

	for (obj = objstr0; obj->flags ! = 0xff; obj++) {
		for (j = 0; j < MAXSECTION; j++) {
			if (obj->section[j]. sectype ! = sectype)
				continue;
			if (obj->section[j]. size == 0)
				continue;
			if ((obj->flags & 0x01 /* link */) == 0)
				continue;
			i = obj->section[j]. align;
			if (i == 0) {
				i = secparam[0 /* align */] - 1;
				if (i < 0) {
					static char *secname[3] = { "code", "data", "data" };
					fprintf(stderr, "Warning : please set align for %s\n", secname[sectype]);
					i = 0;
				}
			} else
				i = (1 << i - 1) - 1;
			while (addr & i) {
				addr++;
				*image++ = '\0';
			}
			obj->section[j]. addr = addr;
			if (sectype < 2) { /* text or data */
				p = obj->section[j]. ptr;
				for (i = obj->section[j]. size; i > 0; i--)
					*image++ = *p++;
			} else { /* bss */
				for (i = obj->section[j]. size; i > 0; i--)
					*image++ = '\0';
			}
			addr += obj->section[j]. size;
		}
	}
	secparam[3 /* logic+size */] = addr; /* size라고 하는 것보다 최종 주소 */
}
