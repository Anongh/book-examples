typedef unsigned char UCHAR;

typedef struct GO_STR_FILE {
	UCHAR *p0, *p1, *p;
	int dummy;
} GO_FILE;

extern GO_FILE GO_stdin, GO_stdout, GO_stderr;
extern struct GOL_STR_MEMMAN GOL_memman, GOL_sysman;
UCHAR *GOL_work0;

struct bss_alloc {
	UCHAR _stdout[SIZ_STDOUT];
	UCHAR _stderr[SIZ_STDERR];
	UCHAR syswrk[SIZ_SYSWRK];
	UCHAR work[SIZ_WORK];
	UCHAR for_align[16];
};

void GOL_sysabort(UCHAR termcode);
void *GOL_memmaninit(struct GOL_STR_MEMMAN *man, unsigned int size, void *p);
void *GOL_sysmalloc(unsigned int size);
void GOL_callmain0();

void mainCRTStartup(void)
/* 반드시, -o옵션을 붙인다 */
/* 그러나 입력 파일명은 쓴다(표준 입력으로는 size를 측정할 수 없기 때문에) */
{
	struct bss_alloc bss_image;
	struct bss_alloc *bss0 = (void *) ((((int) &bss_image) + 0x0f) & ~0x0f);
	GO_stdout.p0 = GO_stdout.p = bss0->_stdout;
	GO_stdout.p1 = GO_stdout.p0 + SIZ_STDOUT;
	GO_stdout.dummy = ~0;
	GO_stderr.p0 = GO_stderr.p = bss0->_stderr;
	GO_stderr.p1 = GO_stderr.p0 + (SIZ_STDERR - 128); /* 일부러 조금 작게 해 둔다 */
	GO_stderr.dummy = ~0;
	GOL_memmaninit(&GOL_sysman, SIZ_SYSWRK, bss0->syswrk);
	GOL_memmaninit(&GOL_memman, SIZ_WORK, GOL_work0 = bss0->work);
	GOL_callmain0();
}
