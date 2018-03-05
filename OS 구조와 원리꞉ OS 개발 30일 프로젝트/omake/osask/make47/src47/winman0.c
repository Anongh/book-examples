/* "winman0.c":사양 윈도우 매니저 ver. 3.9
		copyright(C) 2004 카와이 히데미, I.Tak., 코야나기아명, KIYOTO, nikq
    stack:64k malloc:4272k file:4160k */

/* 2004.08. 13 screen shot를 BMP화
   8,15,16,32 bpp로 정상적인 화상이 잡히는……듯 by I.Tak.  */

/* 2004.09. 10 화면 관계의 global 변수를 구조체로 하겠어
 * 2004.09. 11 디코더를 DLL 사용으로 하겠어……pokon이 죽었어 tss=4000 EIP=2 f10로
 *            page fault야……mmarea분이 부족
 *            PICTURE0.BIN가 SCRNSHOT.BMP (8bpp)를 디코드할 수 없다(T_T
 */
#define int3 asm(". byte 0xcc")
#include <guigui00.h>
#include <sysgg00.h>
/* sysgg는, 일반 어플리케이션이 이용해서는 안 되는 라이브러리
   사양도 꽤 유동적 */
#include <stdlib.h>

/* 프리 프로세서의 옵션으로 -DPCAT인가 -DTOWNS를 지정하는 것 */

/* 하드 코드. gas2nask 버전 업 대기 */
#define lib_readCSd(ofs)  ({ int _ret;\
 __asm__(". byte 0x2e\n movl %1, %0" : "=r" (_ret) : "m" (*(int*)(ofs)));\
 _ret;})

static inline void call_dll0207_48(int *env, int *cmd)
{
  __asm__ 
    ("pushl %1\n"
     "pushl %0\n"
     ". byte 0x9a\n"
     ". long 0x48\n"
     ". word 0x207\n"		/* "lcall $0x0207, $0x00000048\n" */
     "addl $8, %%esp"		/* 이것을 최적화할 수 없는 것이 유감 */
     : : "g" (env), "g" (cmd), "m" (*cmd) :"%ecx","%edx","memory");
}

static void call_dll0207_48i(void *env, int cmd, ...)
{
  __asm__
    ("pushl %1\n"
     "pushl %0\n"
     ". byte 0x9a\n"
     ". long 0x48\n"
     ". word 0x207\n"		/* "lcall $0x0207, $0x00000048\n" */
     "addl $8, %%esp"
     : : "g" (env), "g" (&cmd), "m" (cmd) :"%ecx","%edx","memory");
}

#if (defined(TOWNS))
	/* 마우스 존재 flag (FMR<<4)|(R<<2) |L, 0:none, 1:mouse, 2:pad, 3:6pad */
	static char townsmouse = 0x04;/* right mouse only */
	#if (defined(VMODE) || defined(CLGD543X))
	  #undef TWVSW        /* 1024가 아니면 인터레이스 할 수 없다 */
	  #define TWVSW 1024  /* 인터레이스하지 않으면 문제 없겠지만
                               * CLGD에는 1024의 파라미터 밖에 만들지 않았다 */
	#endif
	#define FMRMOUSE 1    /* FMRMOUSE에 대응 2004.04. 12 by I.Tak.  */
	#define KROM 1        /* font file이 없을 때에 ROM으로 대용한다 2004.04. 12 by I.Tak.  */
	#if (! defined(TWVSW))
		#define	TWVSW		1024
	#endif
	#define DEFAULTCOLDEP 1
#else
	#define DEFAULTCOLDEP 0
#endif

/* NEC PC-98 에서는 변화하지 않는 변수에 const를 붙여 최적화를 꾀한다 
 * 변수 그 자체는 없어지지 않지만 (포인터 인수가 있을 수 있기 때문에),
 * 참조 회수는 줄어들 것이다……이런 것이 의미 있는 것인가 */
#if (defined(PCAT)) || (defined(TOWNS))
	#define CONST98
#elif (defined(NEC98))
	#define CONST98 const
#endif

typedef struct str_screen {
  CONST98 unsigned char vbecoldep, driver;
  unsigned char wallpaper_name[13];
  unsigned char wallpaper_exist, *wallpaper;
  CONST98 int x2, y2;
  int backcolors[5];		/* default color array for bpp 4,8,16,24,32 */
  CONST98 int backcolor;	/* in use */
  int moveunits[5];
  CONST98 int moveunit;
} SCREEN;

static SCREEN screen = {
  DEFAULTCOLDEP, 0, "OSASK   . BMP", 0, (char*) 0,
  640,400,
#if (defined(WIN31))
  {8, 8, 0xc618, 0x00c0c0c0, 0x00c0c0c0}, 8,
#else
  {6, 6, 0x0410, 0x00008080, 0x00008080}, 6,
#endif
  {8, 4, 2, 4, 1}, 8
};

#if (defined(WIN9X))
	#define	RESERVELINE0		   0
	#define	RESERVELINE1		  28
	#if (defined(PCAT) || defined(TOWNS))
		#define TIMEX				-192	/* 8의 배수 */
		#define TIMEY				 -20
		#define TIMEC				   0
		#define TIMEBC				   8
		#define ERRMSGX				  80
		#define ERRMSGY				 -20
		#define ERRMSGC				   0
		#define ERRMSGBC			   8
		#define ERRMSGCC			   8
	#endif
#elif (defined(TMENU))
	#define	RESERVELINE0		  20
	#define	RESERVELINE1		  28
	#if (defined(PCAT)) || (defined(TOWNS))
		#define TIMEX				-240
		#define TIMEY				   2
		#define TIMEC				   0
		#define TIMEBC				   7
		#define ERRMSGX				 104
		#define ERRMSGY				 -20
		#define ERRMSGC				   0
		#define ERRMSGBC			   7
		#define ERRMSGCC			   7
	#endif
#elif (defined(CHO_OSASK))
	#define	RESERVELINE0		   0
	#define	RESERVELINE1		  20
	#if (defined(PCAT)) || (defined(TOWNS))
		#define TIMEX				-192	/* 8의 배수 */
		#define TIMEY				 -16
		#define TIMEC				  15
		#define TIMEBC				   7
		#define ERRMSGX				   8
		#define ERRMSGY				 -16
		#define ERRMSGC				   0
		#define ERRMSGBC			   7
		#define ERRMSGCC			   7
	#endif
#elif (defined(NEWSTYLE))
	#define	RESERVELINE0		   0
	#define	RESERVELINE1		   0
#elif (defined(WIN31))
	#define	RESERVELINE0		   0
	#define	RESERVELINE1		   0
#endif

#define WALLPAPERMAXSIZE	(4 * 1024 * 1024)
#define	SCRNSHOTMAXSIZ		2048 * 1024
#define MAXWINDEF			16

//static int MALLOC_ADDR;
#define MALLOC_ADDR			j
#define malloc(bytes)		(void *) (MALLOC_ADDR -= ((bytes) + 7) & ~7)
#define free(addr)			for (;;); /* free가 있어서는 곤란하므로 영구적인 루프 */

#define	AUTO_MALLOC			   0
#define NULL				   0
#define	MAX_WINDOWS		 	  80		// 8.1KB
#define JOBLIST_SIZE		 256		// 1KB
#define	MAX_SOUNDTRACK		  16		// 0.5KB
#define	DEFSIGBUFSIZ		2048
#define	MOSWINSIGS			 128		/* 4KB */

#define WINFLG_MUSTREDRAW		0x80000000	/* bit31 */
#define WINFLG_MUSTREDRAWDIF	0x40000000	/* bit30 */
#define WINFLG_OVERRIDEDISABLED	0x01000000	/* bit24 */
#define WINFLG_NOWINDOW			0x00000400	/* bit10 */
#define	WINFLG_WAITREDRAW		0x00000200	/* bit 9 */
#define	WINFLG_WAITDISABLE		0x00000100	/* bit 8 */

//	#define	DEBUG	1

#define	sgg_debug00(opt, bytes, reserve, src_ofs, src_sel, dest_ofs, dest_sel) \
	sgg_execcmd0(0x8010, (int) (opt), (int) (bytes), (int) (reserve), \
	(int) (src_ofs), (int) (src_sel), (int) (dest_ofs), (int) (dest_sel), \
	0x0000)

struct DEFINESIGNAL { // 32bytes
	int win, opt, dev, cod, len, sig[3];
};

struct WM0_WINDOW {	// total 108bytes
//	struct DEFINESIGNAL defsig[29]; // 928bytes
	struct SGG_WINDOW sgg; // 68bytes
//	struct DEFINESIGNAL *ds1;
	int condition, x0, y0, x1, y1, job_flag0, job_flag1;
	int tx0, ty0, tx1, ty1; /* 윈도우 이동을 위한 탭 */
	int flags;
	struct WM0_WINDOW *up, *down;
};

struct SOUNDTRACK {
	int sigbox, sigbase, slot, reserve[5];
};

struct MOSWINSIG { /* 32bytes */
	int flags, sig[6];
	struct WM0_WINDOW *win;
	/* flags의 하위 4 bit는 len */
	/* sig[4], sig[5]는 경우에 따라서는 x0, y0 */
};

static struct STR_JOB {
	int now, movewin4_ready, fontflag;
	int *list, free, *rp, *wp;
	int count, int0;
	int movewin_x, movewin_y, movewin_x0, movewin_y0; /* 이동처와 이동원 */
	int readCSd10;
	void (*func)(int, int);
	struct WM0_WINDOW *win;
	int fonttss, sig;
} job = { 0 /* now */, 0 /* movewin4_ready */, /* fontflag */ };

#if (defined(PCAT))
	struct STR_VBEMODE { /* 16bytes */
		unsigned int linear, linebytes;
		unsigned short x_res, y_res, mode;
		unsigned char flag, dummy;
	};
	static char flag_vbe2 = 0;
	static struct STR_VBEMODE vbelist[128];
	static int f3mode, f4mode;
	static unsigned int vbeoverride[3];
#elif (defined(TOWNS)) && (defined(CLGD543X))
	static int pf13mode = -1; /* (^^; */
#endif

struct WM0_WINDOW *window, *top = NULL, *unuse = NULL, *iactive = NULL, *pokon0 = NULL;
int mx = 0x80000000, my = 1, mbutton = 0, mxx = 1;
int fromboot = 0, winmanerr_time = 0;
struct {
	int x, y;
} windef[MAXWINDEF];
int mouseaccel = 2;	/* 이것보다 크면 가속도가 배로 증가 */
int mousescale = 3; /* 가속 스케일로 하자 */
struct SOUNDTRACK *sndtrk_buf, *sndtrk_active = NULL;
struct DEFINESIGNAL *defsigbuf;
struct MOSWINSIG *moswinsig;
int mws_sensitivecount = 0;
struct WM0_WINDOW *mws_mousewin = NULL;

void init_screen(const int x, const int y);
struct WM0_WINDOW *handle2window(const int handle);
void chain_unuse(struct WM0_WINDOW *win);
struct WM0_WINDOW *get_unuse();
void mousesignal(const unsigned int header, int dx, int dy);
void mousesignal_sub0(int _mx, int _my);
void mousesignal_sub1(int _mx, int _my);

int writejob_n(int n, int p0,...);
void runjobnext();
void job_openwin0(struct WM0_WINDOW *win);
void redirect_input(struct WM0_WINDOW *win);
void job_activewin0(struct WM0_WINDOW *win);
void job_movewin0(struct WM0_WINDOW *win);
void job_movewin1(const int cmd, const int handle);
void job_movewin2();
void job_movewin3();
void job_movewin4(int sig);
void job_movewin4m(int x, int y);
int job_movewin5();
void job_movewin6(const int cmd, const int handle);
void job_closewin0(struct WM0_WINDOW *win0);
void job_general1();
void job_general2(const int cmd, const int handle);
void job_openvgadriver(const int drv);
void job_setvgamode0(const int mode);
void job_setvgamode1(const int cmd, const int handle);
void job_setvgamode2();
void job_setvgamode3(const int sig, const int result);
void job_loadfont0(int fonttype, int tss, int sig);
void job_loadfont1(int flag, int dmy);
void job_loadfont2();
void job_loadfont3(int flag, int dmy);
void moswinsig_flagset();
struct WM0_WINDOW *searchwin(int x, int y);
int lock_v86();
void unlock_v86();

#if (defined(PCAT) || defined(TOWNS))
	void job_savevram0(void);
	void job_savevram1(int flag, int dmy);
	void job_savevram2(int flag, int dmy);
#endif
void job_openwallpaper(void);
void job_loadwallpaper(int flag, int dmy);
void putwallpaper(int x0, int y0, int x1, int y1);

#if (defined(PCAT))
	void job_vesacheck0();
	void job_vesacheck1(int sig, int result);
	void job_vesacheck2();
#endif

//void free_sndtrk(struct SOUNDTRACK *sndtrk);

struct SOUNDTRACK *alloc_sndtrk();
void send_signal2dw(const int sigbox, const int data0, const int data1);
void send_signal3dw(const int sigbox, const int data0, const int data1, const int data2);
void send_signal4dw(const int sigbox, const int data0, const int data1, const int data2, int data3);

void lib_drawletters_ASCII(const int opt, const int win, const int charset, const int x0, const int y0,
	const int color, const int backcolor, const char *str);
void debug_bin2hex(unsigned int i, unsigned char *s);

void sgg_wm0_definesignal3(const int opt, const int device, int keycode,
	const int sig0, const int sig1, const int sig2);
void sgg_wm0_definesignal3sub(const int keycode);
void sgg_wm0_definesignal3sub2(const int rawcode, const int shiftmap);
void sgg_wm0_definesignal3sub3(int rawcode, const int shiftmap);
// void sgg_wm0_definesignal3com();

void write_time();
void winmanerr(const unsigned char *s);
void winmanerr_clr();

/* 키 조작：
      F9:맨 밑의 윈도우로
      F10:위로부터 2번째의 윈도우를 선택
      F11:윈도우의 이동
      F12:윈도우 클로우즈 */

//int allclose = 0;

static int tapisigvec[] = {
	0x006c, 6 * 4, 0x011c /* cmd fot tapi */, 0, 0, 0x0000, 0x0000
};

#define SYSTEM_TIMER		0x01c0
#define SIG_WRITE_TIME		0x0060

#define	NOSHIFT		0	/* 0x0000c070 */
#define	SHIFT		1	/* 0x0010c070 */
#define	IGSHIFT		2	/* 0x0000c060 */
#define	CAPLKON		3	/* 0x0004c074, 0x0010c074 */
#define	CAPLKOF		4	/* 0x0000c074, 0x0014c074 */
#define	NUMLKON		5	/* 0x0002c072, 0x0010c072 */
#define	NUMLKOF		6	/* 0x0000c072, 0x0012c072 */
#define ALT			7   /* 0x0040c070 */

/* 입력 방법 테이블(2방법까지 서포트) */
static struct KEYTABLE {
	unsigned char rawcode0, shifttype0;
	unsigned char rawcode1, shifttype1;
} keybindtable[] = {
	#if (defined(PCAT))
		{ 0x39, IGSHIFT, 0xff, 0xff    } /* ' ' */,
		{ 0x02, SHIFT,   0xff, 0xff    } /* '! ' */,
		{ 0x03, SHIFT,   0xff, 0xff    } /* '\x22' */,
		{ 0x04, SHIFT,   0xff, 0xff    } /* '#' */,
		{ 0x05, SHIFT,   0xff, 0xff    } /* '%' */,
		{ 0x06, SHIFT,   0xff, 0xff    } /* '$' */,
		{ 0x07, SHIFT,   0xff, 0xff    } /* '&' */,
		{ 0x08, SHIFT,   0xff, 0xff    } /* '\x27' */,
		{ 0x09, SHIFT,   0xff, 0xff    } /* '(' */,
		{ 0x0a, SHIFT,   0xff, 0xff    } /* ')' */,
		{ 0x28, SHIFT,   0x37, IGSHIFT } /* '*' */,
		{ 0x27, SHIFT,   0x4e, IGSHIFT } /* '+' */,
		{ 0x33, NOSHIFT, 0xff, 0xff    } /* ', ' */,
		{ 0x0c, NOSHIFT, 0x4a, IGSHIFT } /* '-' */,
		{ 0x34, NOSHIFT, 0x53, NUMLKON } /* '. ' */,
		{ 0x35, NOSHIFT, 0xb5, IGSHIFT } /* '/' */,
		{ 0x0b, NOSHIFT, 0x52, NUMLKON } /* '0' */,
		{ 0x02, NOSHIFT, 0x4f, NUMLKON } /* '1' */,
		{ 0x03, NOSHIFT, 0x50, NUMLKON } /* '2' */,
		{ 0x04, NOSHIFT, 0x51, NUMLKON } /* '3' */,
		{ 0x05, NOSHIFT, 0x4b, NUMLKON } /* '4' */,
		{ 0x06, NOSHIFT, 0x4c, NUMLKON } /* '5' */,
		{ 0x07, NOSHIFT, 0x4d, NUMLKON } /* '6' */,
		{ 0x08, NOSHIFT, 0x47, NUMLKON } /* '7' */,
		{ 0x09, NOSHIFT, 0x48, NUMLKON } /* '8' */,
		{ 0x0a, NOSHIFT, 0x49, NUMLKON } /* '9' */,
		{ 0x28, NOSHIFT, 0xff, 0xff    } /* ':' */,
		{ 0x27, NOSHIFT, 0xff, 0xff    } /* ';' */,
		{ 0x33, SHIFT,   0xff, 0xff    } /* '<' */,
		{ 0x0c, SHIFT,   0xff, 0xff    } /* '=' */,
		{ 0x34, SHIFT,   0xff, 0xff    } /* '>' */,
		{ 0x35, SHIFT,   0xff, 0xff    } /* '? ' */,
		{ 0x1a, NOSHIFT, 0xff, 0xff    } /* '@' */,
		{ 0x1e, CAPLKON, 0xff, 0xff    } /* 'A' */,
		{ 0x30, CAPLKON, 0xff, 0xff    } /* 'B' */,
		{ 0x2e, CAPLKON, 0xff, 0xff    } /* 'C' */,
		{ 0x20, CAPLKON, 0xff, 0xff    } /* 'D' */,
		{ 0x12, CAPLKON, 0xff, 0xff    } /* 'E' */,
		{ 0x21, CAPLKON, 0xff, 0xff    } /* 'F' */,
		{ 0x22, CAPLKON, 0xff, 0xff    } /* 'G' */,
		{ 0x23, CAPLKON, 0xff, 0xff    } /* 'H' */,
		{ 0x17, CAPLKON, 0xff, 0xff    } /* 'I' */,
		{ 0x24, CAPLKON, 0xff, 0xff    } /* 'J' */,
		{ 0x25, CAPLKON, 0xff, 0xff    } /* 'K' */,
		{ 0x26, CAPLKON, 0xff, 0xff    } /* 'L' */,
		{ 0x32, CAPLKON, 0xff, 0xff    } /* 'M' */,
		{ 0x31, CAPLKON, 0xff, 0xff    } /* 'N' */,
		{ 0x18, CAPLKON, 0xff, 0xff    } /* 'O' */,
		{ 0x19, CAPLKON, 0xff, 0xff    } /* 'P' */,
		{ 0x10, CAPLKON, 0xff, 0xff    } /* 'Q' */,
		{ 0x13, CAPLKON, 0xff, 0xff    } /* 'R' */,
		{ 0x1f, CAPLKON, 0xff, 0xff    } /* 'S' */,
		{ 0x14, CAPLKON, 0xff, 0xff    } /* 'T' */,
		{ 0x16, CAPLKON, 0xff, 0xff    } /* 'U' */,
		{ 0x2f, CAPLKON, 0xff, 0xff    } /* 'V' */,
		{ 0x11, CAPLKON, 0xff, 0xff    } /* 'W' */,
		{ 0x2d, CAPLKON, 0xff, 0xff    } /* 'X' */,
		{ 0x15, CAPLKON, 0xff, 0xff    } /* 'Y' */,
		{ 0x2c, CAPLKON, 0xff, 0xff    } /* 'Z' */,
		{ 0x1b, NOSHIFT, 0xff, 0xff    } /* '[' */,
		{ 0x7d, NOSHIFT, 0x73, NOSHIFT } /* '\' */,
		{ 0x2b, NOSHIFT, 0xff, 0xff    } /* ']' */,
		{ 0x0d, NOSHIFT, 0xff, 0xff    } /* '^' */,
		{ 0x73, SHIFT,   0xff, 0xff    } /* '_' */,
		{ 0x1a, SHIFT,   0xff, 0xff    } /* '`' */,
		{ 0x1e, CAPLKOF, 0xff, 0xff    } /* 'a' */,
		{ 0x30, CAPLKOF, 0xff, 0xff    } /* 'b' */,
		{ 0x2e, CAPLKOF, 0xff, 0xff    } /* 'c' */,
		{ 0x20, CAPLKOF, 0xff, 0xff    } /* 'd' */,
		{ 0x12, CAPLKOF, 0xff, 0xff    } /* 'e' */,
		{ 0x21, CAPLKOF, 0xff, 0xff    } /* 'f' */,
		{ 0x22, CAPLKOF, 0xff, 0xff    } /* 'g' */,
		{ 0x23, CAPLKOF, 0xff, 0xff    } /* 'h' */,
		{ 0x17, CAPLKOF, 0xff, 0xff    } /* 'i' */,
		{ 0x24, CAPLKOF, 0xff, 0xff    } /* 'j' */,
		{ 0x25, CAPLKOF, 0xff, 0xff    } /* 'k' */,
		{ 0x26, CAPLKOF, 0xff, 0xff    } /* 'l' */,
		{ 0x32, CAPLKOF, 0xff, 0xff    } /* 'm' */,
		{ 0x31, CAPLKOF, 0xff, 0xff    } /* 'n' */,
		{ 0x18, CAPLKOF, 0xff, 0xff    } /* 'o' */,
		{ 0x19, CAPLKOF, 0xff, 0xff    } /* 'p' */,
		{ 0x10, CAPLKOF, 0xff, 0xff    } /* 'q' */,
		{ 0x13, CAPLKOF, 0xff, 0xff    } /* 'r' */,
		{ 0x1f, CAPLKOF, 0xff, 0xff    } /* 's' */,
		{ 0x14, CAPLKOF, 0xff, 0xff    } /* 't' */,
		{ 0x16, CAPLKOF, 0xff, 0xff    } /* 'u' */,
		{ 0x2f, CAPLKOF, 0xff, 0xff    } /* 'v' */,
		{ 0x11, CAPLKOF, 0xff, 0xff    } /* 'w' */,
		{ 0x2d, CAPLKOF, 0xff, 0xff    } /* 'x' */,
		{ 0x15, CAPLKOF, 0xff, 0xff    } /* 'y' */,
		{ 0x2c, CAPLKOF, 0xff, 0xff    } /* 'z' */,
		{ 0x1b, SHIFT,   0xff, 0xff    } /* '{' */,
		{ 0x7d, SHIFT,   0xff, 0xff    } /* '|' */,
		{ 0x2b, SHIFT,   0xff, 0xff    } /* '}' */,
		{ 0x0d, SHIFT,   0x0b, SHIFT   } /* '~' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x7f' */,
		{ 0x01, NOSHIFT, 0xff, 0xff    } /* Esc */,
		{ 0x3b, NOSHIFT, 0xff, 0xff    } /* F1 */,
		{ 0x3c, NOSHIFT, 0xff, 0xff    } /* F2 */,
		{ 0x3d, NOSHIFT, 0xff, 0xff    } /* F3 */,
		{ 0x3e, NOSHIFT, 0xff, 0xff    } /* F4 */,
		{ 0x3f, NOSHIFT, 0xff, 0xff    } /* F5 */,
		{ 0x40, NOSHIFT, 0xff, 0xff    } /* F6 */,
		{ 0x41, NOSHIFT, 0xff, 0xff    } /* F7 */,
		{ 0x42, NOSHIFT, 0xff, 0xff    } /* F8 */,
		{ 0x43, NOSHIFT, 0xff, 0xff    } /* F9 */,
		{ 0x44, NOSHIFT, 0xff, 0xff    } /* F10 */,
		{ 0x57, NOSHIFT, 0xff, 0xff    } /* F11 */,
		{ 0x58, NOSHIFT, 0xff, 0xff    } /* F12 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F13 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F14 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F15 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F16 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F17 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F18 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F19 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F20 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x95' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x96' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x97' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x98' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x99' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9a' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9b' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9c' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9d' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9e' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9f' */,
		{ 0x1c, IGSHIFT, 0x9c, IGSHIFT } /* Enter */,
		{ 0x0e, IGSHIFT, 0xff, 0xff    } /* BackSpace */,
		{ 0x0f, NOSHIFT, 0xff, 0xff    } /* Tab */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xa3' */,
		{ 0xd2, NOSHIFT, 0x52, NUMLKOF } /* Insert */,
		{ 0xd3, NOSHIFT, 0x53, NUMLKOF } /* Delete */,
		{ 0xc7, NOSHIFT, 0x47, NUMLKOF } /* Home */,
		{ 0xcf, NOSHIFT, 0x4f, NUMLKOF } /* End */,
		{ 0xc9, NOSHIFT, 0x49, NUMLKOF } /* PageUp */,
		{ 0xd1, NOSHIFT, 0x51, NUMLKOF } /* PageDown */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xaa' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xab' */,
		{ 0xcb, NOSHIFT, 0x4b, NUMLKOF } /* Left */,
		{ 0xcd, NOSHIFT, 0x4d, NUMLKOF } /* Right */,
		{ 0xc8, NOSHIFT, 0x48, NUMLKOF } /* Up */,
		{ 0xd0, NOSHIFT, 0x50, NUMLKOF } /* Down */,
		{ 0x46, NOSHIFT, 0xff, 0xff    } /* ScrollLock */,
		{ 0x45, NOSHIFT, 0xff, 0xff    } /* NumLock */,
		{ 0x3a, SHIFT,   0xff, 0xff    } /* CapsLock */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xb3' */,
		{ 0x2a, 0xfe,    0x36, 0xfe    } /* Shift */,
		{ 0x1d, 0xfe,    0x9d, 0xfe    } /* Ctrl */,
		{ 0x38, 0xfe,    0xb8, 0xfe    } /* Alt */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xb7' */,
		{ 0xb7, NOSHIFT, 0xff, 0xff    } /* PrintScreen */,
		{ 0xff, NOSHIFT, 0xff, 0xff    } /* Pause */,
		{ 0xc6, NOSHIFT, 0xff, 0xff    } /* Break(ALT? ) */,
		{ 0x54, NOSHIFT, 0xff, 0xff    } /* SysRq(ALT? ) */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xbc' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xbd' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xbe' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xbf' */,
		{ 0xdb, NOSHIFT, 0xdc, NOSHIFT } /* Windows */,
		{ 0xdd, NOSHIFT, 0xff, 0xff    } /* Menu */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Power */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Sleep */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Wake */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc5' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc6' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc7' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc8' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc9' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xca' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xcb' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xcc' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xcd' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xce' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xcf' */,
		{ 0x29, NOSHIFT, 0xff, 0xff    } /* Zenkaku */,
		{ 0x7b, NOSHIFT, 0xff, 0xff    } /* Muhenkan */,
		{ 0x79, NOSHIFT, 0xff, 0xff    } /* Henkan */,
		{ 0x70, NOSHIFT, 0xff, 0xff    } /* Hiragana */,
		{ 0x70, SHIFT,   0xff, 0xff    } /* Katakana */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd5' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd6' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd7' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd8' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd9' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xda' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xdb' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xdc' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xdd' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xde' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xdf' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe0' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe1' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe2' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe3' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe4' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe5' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe6' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe7' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe8' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe9' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xea' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xeb' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xec' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xed' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xee' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xef' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf0' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf1' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf2' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf3' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf4' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf5' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf6' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf7' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf8' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf9' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfa' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfb' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfc' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfd' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfe' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xff' */
	#endif
	#if (defined(TOWNS))
		{ 0x35, IGSHIFT, 0xff, 0xff    } /* ' ' */,
		{ 0x02, SHIFT,   0xff, 0xff    } /* '! ' */,
		{ 0x03, SHIFT,   0x34, NOSHIFT } /* '\x22' */,
		{ 0x04, SHIFT,   0xff, 0xff    } /* '#' */,
		{ 0x05, SHIFT,   0xff, 0xff    } /* '%' */,
		{ 0x06, SHIFT,   0xff, 0xff    } /* '$' */,
		{ 0x07, SHIFT,   0xff, 0xff    } /* '&' */,
		{ 0x08, SHIFT,   0xff, 0xff    } /* '\x27' */,
		{ 0x09, SHIFT,   0xff, 0xff    } /* '(' */,
		{ 0x0a, SHIFT,   0xff, 0xff    } /* ')' */,
		{ 0x28, SHIFT,   0x36, NOSHIFT } /* '*' */,
		{ 0x27, SHIFT,   0x38, NOSHIFT } /* '+' */,
		{ 0x31, NOSHIFT, 0xff, 0xff    } /* ', ' */,
		{ 0x0c, NOSHIFT, 0x39, NOSHIFT } /* '-' */,
		{ 0x32, NOSHIFT, 0x47, NOSHIFT } /* '. ' */,
		{ 0x33, NOSHIFT, 0x37, NOSHIFT } /* '/' */,
		{ 0x0b, NOSHIFT, 0x46, NOSHIFT } /* '0' */,
		{ 0x02, NOSHIFT, 0x42, NOSHIFT } /* '1' */,
		{ 0x03, NOSHIFT, 0x43, NOSHIFT } /* '2' */,
		{ 0x04, NOSHIFT, 0x44, NOSHIFT } /* '3' */,
		{ 0x05, NOSHIFT, 0x3e, NOSHIFT } /* '4' */,
		{ 0x06, NOSHIFT, 0x3f, NOSHIFT } /* '5' */,
		{ 0x07, NOSHIFT, 0x40, NOSHIFT } /* '6' */,
		{ 0x08, NOSHIFT, 0x3a, NOSHIFT } /* '7' */,
		{ 0x09, NOSHIFT, 0x3b, NOSHIFT } /* '8' */,
		{ 0x0a, NOSHIFT, 0x3c, NOSHIFT } /* '9' */,
		{ 0x28, NOSHIFT, 0xff, 0xff    } /* ':' */,
		{ 0x27, NOSHIFT, 0xff, 0xff    } /* ';' */,
		{ 0x31, SHIFT,   0xff, 0xff    } /* '<' */,
		{ 0x0c, SHIFT,   0x3d, NOSHIFT } /* '=' */,
		{ 0x32, SHIFT,   0xff, 0xff    } /* '>' */,
		{ 0x33, SHIFT,   0xff, 0xff    } /* '? ' */,
		{ 0x1b, NOSHIFT, 0xff, 0xff    } /* '@' */,
		{ 0x1e, CAPLKON, 0xff, 0xff    } /* 'A' */,
		{ 0x2e, CAPLKON, 0xff, 0xff    } /* 'B' */,
		{ 0x2c, CAPLKON, 0xff, 0xff    } /* 'C' */,
		{ 0x20, CAPLKON, 0xff, 0xff    } /* 'D' */,
		{ 0x13, CAPLKON, 0xff, 0xff    } /* 'E' */,
		{ 0x21, CAPLKON, 0xff, 0xff    } /* 'F' */,
		{ 0x22, CAPLKON, 0xff, 0xff    } /* 'G' */,
		{ 0x23, CAPLKON, 0xff, 0xff    } /* 'H' */,
		{ 0x18, CAPLKON, 0xff, 0xff    } /* 'I' */,
		{ 0x24, CAPLKON, 0xff, 0xff    } /* 'J' */,
		{ 0x25, CAPLKON, 0xff, 0xff    } /* 'K' */,
		{ 0x26, CAPLKON, 0xff, 0xff    } /* 'L' */,
		{ 0x30, CAPLKON, 0xff, 0xff    } /* 'M' */,
		{ 0x2f, CAPLKON, 0xff, 0xff    } /* 'N' */,
		{ 0x19, CAPLKON, 0xff, 0xff    } /* 'O' */,
		{ 0x1a, CAPLKON, 0xff, 0xff    } /* 'P' */,
		{ 0x11, CAPLKON, 0xff, 0xff    } /* 'Q' */,
		{ 0x14, CAPLKON, 0xff, 0xff    } /* 'R' */,
		{ 0x1f, CAPLKON, 0xff, 0xff    } /* 'S' */,
		{ 0x15, CAPLKON, 0xff, 0xff    } /* 'T' */,
		{ 0x17, CAPLKON, 0xff, 0xff    } /* 'U' */,
		{ 0x2d, CAPLKON, 0xff, 0xff    } /* 'V' */,
		{ 0x12, CAPLKON, 0xff, 0xff    } /* 'W' */,
		{ 0x2b, CAPLKON, 0xff, 0xff    } /* 'X' */,
		{ 0x16, CAPLKON, 0xff, 0xff    } /* 'Y' */,
		{ 0x2a, CAPLKON, 0xff, 0xff    } /* 'Z' */,
		{ 0x1c, NOSHIFT, 0xff, 0xff    } /* '[' */,
		{ 0x0e, NOSHIFT, 0xff, 0xff    } /* '\' */,
		{ 0x29, NOSHIFT, 0xff, 0xff    } /* ']' */,
		{ 0x0d, NOSHIFT, 0xff, 0xff    } /* '^' */,
		{ 0x34, SHIFT,   0xff, 0xff    } /* '_' */,
		{ 0x1b, SHIFT,   0xff, 0xff    } /* '`' */,
		{ 0x1e, CAPLKOF, 0xff, 0xff    } /* 'a' */,
		{ 0x2e, CAPLKOF, 0xff, 0xff    } /* 'b' */,
		{ 0x2c, CAPLKOF, 0xff, 0xff    } /* 'c' */,
		{ 0x20, CAPLKOF, 0xff, 0xff    } /* 'd' */,
		{ 0x13, CAPLKOF, 0xff, 0xff    } /* 'e' */,
		{ 0x21, CAPLKOF, 0xff, 0xff    } /* 'f' */,
		{ 0x22, CAPLKOF, 0xff, 0xff    } /* 'g' */,
		{ 0x23, CAPLKOF, 0xff, 0xff    } /* 'h' */,
		{ 0x18, CAPLKOF, 0xff, 0xff    } /* 'i' */,
		{ 0x24, CAPLKOF, 0xff, 0xff    } /* 'j' */,
		{ 0x25, CAPLKOF, 0xff, 0xff    } /* 'k' */,
		{ 0x26, CAPLKOF, 0xff, 0xff    } /* 'l' */,
		{ 0x30, CAPLKOF, 0xff, 0xff    } /* 'm' */,
		{ 0x2f, CAPLKOF, 0xff, 0xff    } /* 'n' */,
		{ 0x19, CAPLKOF, 0xff, 0xff    } /* 'o' */,
		{ 0x1a, CAPLKOF, 0xff, 0xff    } /* 'p' */,
		{ 0x11, CAPLKOF, 0xff, 0xff    } /* 'q' */,
		{ 0x14, CAPLKOF, 0xff, 0xff    } /* 'r' */,
		{ 0x1f, CAPLKOF, 0xff, 0xff    } /* 's' */,
		{ 0x15, CAPLKOF, 0xff, 0xff    } /* 't' */,
		{ 0x17, CAPLKOF, 0xff, 0xff    } /* 'u' */,
		{ 0x2d, CAPLKOF, 0xff, 0xff    } /* 'v' */,
		{ 0x12, CAPLKOF, 0xff, 0xff    } /* 'w' */,
		{ 0x2b, CAPLKOF, 0xff, 0xff    } /* 'x' */,
		{ 0x16, CAPLKOF, 0xff, 0xff    } /* 'y' */,
		{ 0x2a, CAPLKOF, 0xff, 0xff    } /* 'z' */,
		{ 0x1c, SHIFT,   0xff, 0xff    } /* '{' */,
		{ 0x0e, SHIFT,   0xff, 0xff    } /* '|' */,
		{ 0x29, SHIFT,   0xff, 0xff    } /* '}' */,
		{ 0x0d, SHIFT,   0xff, 0xff    } /* '~' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x7f' */,
		{ 0x01, NOSHIFT, 0xff, 0xff    } /* Esc */,
		{ 0x5d, NOSHIFT, 0xff, 0xff    } /* F1 */,
		{ 0x5e, NOSHIFT, 0xff, 0xff    } /* F2 */,
		{ 0x5f, NOSHIFT, 0xff, 0xff    } /* F3 */,
		{ 0x60, NOSHIFT, 0xff, 0xff    } /* F4 */,
		{ 0x61, NOSHIFT, 0xff, 0xff    } /* F5 */,
		{ 0x62, NOSHIFT, 0xff, 0xff    } /* F6 */,
		{ 0x63, NOSHIFT, 0xff, 0xff    } /* F7 */,
		{ 0x64, NOSHIFT, 0xff, 0xff    } /* F8 */,
		{ 0x65, NOSHIFT, 0xff, 0xff    } /* F9 */,
		{ 0x66, NOSHIFT, 0xff, 0xff    } /* F10 */,
		{ 0x69, NOSHIFT, 0xff, 0xff    } /* F11 */,
		{ 0x5b, NOSHIFT, 0xff, 0xff    } /* F12 */,
		{ 0x74, NOSHIFT, 0xff, 0xff    } /* F13 */,
		{ 0x75, NOSHIFT, 0xff, 0xff    } /* F14 */,
		{ 0x76, NOSHIFT, 0xff, 0xff    } /* F15 */,
		{ 0x77, NOSHIFT, 0xff, 0xff    } /* F16 */,
		{ 0x78, NOSHIFT, 0xff, 0xff    } /* F17 */,
		{ 0x79, NOSHIFT, 0xff, 0xff    } /* F18 */,
		{ 0x7a, NOSHIFT, 0xff, 0xff    } /* F19 */,
		{ 0x7b, NOSHIFT, 0xff, 0xff    } /* F20 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x95' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x96' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x97' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x98' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x99' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9a' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9b' */,
		{ 0xf4, NOSHIFT, 0xff, 0xff    } /* EXT1(F28) */,
		{ 0xf8, NOSHIFT, 0xff, 0xff    } /* EXT2(F29) */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9e' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9f' */,
		{ 0x1d, IGSHIFT, 0x45, IGSHIFT } /* Enter */,
		{ 0x0f, IGSHIFT, 0xff, 0xff    } /* BackSpace */,
		{ 0x10, NOSHIFT, 0xff, 0xff    } /* Tab */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xa3' */,
		{ 0x48, NOSHIFT, 0xff, 0xff    } /* Insert */,
		{ 0x4b, NOSHIFT, 0xff, 0xff    } /* Delete */,
		{ 0x4e, NOSHIFT, 0xe1, NOSHIFT } /* Home */,
		{ 0xe2, NOSHIFT, 0xff, 0xff    } /* End */,
		{ 0x6e, NOSHIFT, 0xff, 0xff    } /* PageUp */,
		{ 0x70, NOSHIFT, 0xff, 0xff    } /* PageDown */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xaa' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xab' */,
		{ 0x4f, NOSHIFT, 0xff, 0xff    } /* Left */,
		{ 0x51, NOSHIFT, 0xff, 0xff    } /* Right */,
		{ 0x4d, NOSHIFT, 0xff, 0xff    } /* Up */,
		{ 0x50, NOSHIFT, 0xff, 0xff    } /* Down */,
		{ 0xe0, NOSHIFT, 0xff, 0xff    } /* ScrollLock */,
		{ 0xff, 0xff,    0xff, 0xff    } /* NumLock */,
		{ 0x55, NOSHIFT, 0xff, 0xff    } /* CapsLock */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xb3' */,
		{ 0x53, 0xfe,    0xff, 0xff    } /* Shift */,
		{ 0x52, 0xfe,    0xff, 0xff    } /* Ctrl */,
		{ 0x5c, 0xfe,    0x72, 0xfe    } /* Alt */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xb7' */,
		{ 0x7d, NOSHIFT, 0xff, 0xff    } /* COPY(PrintScreen) */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Pause */,
		{ 0x7c, NOSHIFT, 0xff, 0xff    } /* Break */,
		{ 0xdd, NOSHIFT, 0xff, 0xff    } /* SysRq */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xbc' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xbd' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xbe' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xbf' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Windows */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Menu */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Power */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Sleep */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Wake */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc5' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc6' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc7' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc8' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc9' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xca' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xcb' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xcc' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xcd' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xce' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xcf' */,
		{ 0x71, NOSHIFT, 0xff, 0xff    } /* Zenkaku */,
		{ 0x57, NOSHIFT, 0xff, 0xff    } /* Muhenkan */,
		{ 0x58, NOSHIFT, 0xff, 0xff    } /* Henkan */,
		{ 0x56, NOSHIFT, 0xff, 0xff    } /* Hiragana */,
		{ 0x5a, NOSHIFT, 0xff, 0xff    } /* Katakana */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd5' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd6' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd7' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd8' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd9' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xda' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xdb' */,
		{ 0x67, NOSHIFT, 0xff, 0xff    } /* '쉬프트왼쪽' */,
		{ 0x68, NOSHIFT, 0xff, 0xff    } /* '쉬프트 오른쪽' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xde' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xdf' */,
/* e0~f7는, 할당하는 것이 귀찮게 된 기종 고유의 마이너 키 */
		{ 0x72, NOSHIFT, 0xff, 0xff    } /* 취소 */,
		{ 0x73, NOSHIFT, 0xff, 0xff    } /* 실행 */,
		{ 0x59, NOSHIFT, 0xff, 0xff    } /* 가나한자 */,
		{ 0x4a, NOSHIFT, 0xff, 0xff    } /* 000 */,
		{ 0x6b, NOSHIFT, 0xff, 0xff    } /* 한자 사전 */,
		{ 0x6c, NOSHIFT, 0xff, 0xff    } /* 단어 말소 */,
		{ 0x6d, NOSHIFT, 0xff, 0xff    } /* 단어 등록 */,
		{ 0x6a, NOSHIFT, 0xff, 0xff    } /* 영문자 */,
		{ 0x6f, NOSHIFT, 0xff, 0xff    } /* 카타카나/영소문자 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe9' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xea' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xeb' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xec' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xed' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xee' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xef' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf0' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf1' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf2' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf3' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf4' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf5' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf6' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf7' */,
/* f8~ff는, 마이너 쉬프트 키 */
		{ 0x67, NOSHIFT, 0x68, NOSHIFT } /* 쉬프트('\xf8') */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf9' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfa' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfb' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfc' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfd' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfe' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xff' */
	#endif
	#if (defined(NEC98))
		{ 0x34, IGSHIFT, 0xff, 0xff    } /* ' ' */,
		{ 0x01, SHIFT,   0xff, 0xff    } /* '! ' */,
		{ 0x02, SHIFT,   0xff, 0xff    } /* '\x22' */,
		{ 0x03, SHIFT,   0xff, 0xff    } /* '#' */,
		{ 0x04, SHIFT,   0xff, 0xff    } /* '%' */,
		{ 0x05, SHIFT,   0xff, 0xff    } /* '$' */,
		{ 0x06, SHIFT,   0xff, 0xff    } /* '&' */,
		{ 0x07, SHIFT,   0xff, 0xff    } /* '\x27' */,
		{ 0x08, SHIFT,   0xff, 0xff    } /* '(' */,
		{ 0x09, SHIFT,   0xff, 0xff    } /* ')' */,
		{ 0x27, SHIFT,   0x45, NOSHIFT } /* '*' */,
		{ 0x26, SHIFT,   0x49, NOSHIFT } /* '+' */,
		{ 0x30, NOSHIFT, 0x4f, NOSHIFT } /* ', ' */,
		{ 0x0b, NOSHIFT, 0x40, NOSHIFT } /* '-' */,
		{ 0x31, NOSHIFT, 0x50, NOSHIFT } /* '. ' */,
		{ 0x32, NOSHIFT, 0x41, NOSHIFT } /* '/' */,
		{ 0x0a, NOSHIFT, 0x4e, NOSHIFT } /* '0' */,
		{ 0x01, NOSHIFT, 0x4a, NOSHIFT } /* '1' */,
		{ 0x02, NOSHIFT, 0x4b, NOSHIFT } /* '2' */,
		{ 0x03, NOSHIFT, 0x4c, NOSHIFT } /* '3' */,
		{ 0x04, NOSHIFT, 0x46, NOSHIFT } /* '4' */,
		{ 0x05, NOSHIFT, 0x47, NOSHIFT } /* '5' */,
		{ 0x06, NOSHIFT, 0x48, NOSHIFT } /* '6' */,
		{ 0x07, NOSHIFT, 0x42, NOSHIFT } /* '7' */,
		{ 0x08, NOSHIFT, 0x43, NOSHIFT } /* '8' */,
		{ 0x09, NOSHIFT, 0x44, NOSHIFT } /* '9' */,
		{ 0x27, NOSHIFT, 0xff, 0xff    } /* ':' */,
		{ 0x26, NOSHIFT, 0xff, 0xff    } /* ';' */,
		{ 0x30, SHIFT,   0xff, 0xff    } /* '<' */,
		{ 0x0b, SHIFT,   0x4d, NOSHIFT } /* '=' */,
		{ 0x31, SHIFT,   0xff, 0xff    } /* '>' */,
		{ 0x32, SHIFT,   0xff, 0xff    } /* '? ' */,
		{ 0x1a, NOSHIFT, 0xff, 0xff    } /* '@' */,
		{ 0x1d, CAPLKON, 0xff, 0xff    } /* 'A' */,
		{ 0x2d, CAPLKON, 0xff, 0xff    } /* 'B' */,
		{ 0x2b, CAPLKON, 0xff, 0xff    } /* 'C' */,
		{ 0x1f, CAPLKON, 0xff, 0xff    } /* 'D' */,
		{ 0x12, CAPLKON, 0xff, 0xff    } /* 'E' */,
		{ 0x20, CAPLKON, 0xff, 0xff    } /* 'F' */,
		{ 0x21, CAPLKON, 0xff, 0xff    } /* 'G' */,
		{ 0x22, CAPLKON, 0xff, 0xff    } /* 'H' */,
		{ 0x17, CAPLKON, 0xff, 0xff    } /* 'I' */,
		{ 0x23, CAPLKON, 0xff, 0xff    } /* 'J' */,
		{ 0x24, CAPLKON, 0xff, 0xff    } /* 'K' */,
		{ 0x25, CAPLKON, 0xff, 0xff    } /* 'L' */,
		{ 0x2f, CAPLKON, 0xff, 0xff    } /* 'M' */,
		{ 0x2e, CAPLKON, 0xff, 0xff    } /* 'N' */,
		{ 0x18, CAPLKON, 0xff, 0xff    } /* 'O' */,
		{ 0x19, CAPLKON, 0xff, 0xff    } /* 'P' */,
		{ 0x10, CAPLKON, 0xff, 0xff    } /* 'Q' */,
		{ 0x13, CAPLKON, 0xff, 0xff    } /* 'R' */,
		{ 0x1e, CAPLKON, 0xff, 0xff    } /* 'S' */,
		{ 0x14, CAPLKON, 0xff, 0xff    } /* 'T' */,
		{ 0x16, CAPLKON, 0xff, 0xff    } /* 'U' */,
		{ 0x2c, CAPLKON, 0xff, 0xff    } /* 'V' */,
		{ 0x11, CAPLKON, 0xff, 0xff    } /* 'W' */,
		{ 0x2a, CAPLKON, 0xff, 0xff    } /* 'X' */,
		{ 0x15, CAPLKON, 0xff, 0xff    } /* 'Y' */,
		{ 0x29, CAPLKON, 0xff, 0xff    } /* 'Z' */,
		{ 0x1b, NOSHIFT, 0xff, 0xff    } /* '[' */,
		{ 0x0d, NOSHIFT, 0xff, 0xff    } /* '\' */,
		{ 0x28, NOSHIFT, 0xff, 0xff    } /* ']' */,
		{ 0x0c, NOSHIFT, 0xff, 0xff    } /* '^' */,
		{ 0x33, SHIFT,   0xff, 0xff    } /* '_' */,
		{ 0x0c, SHIFT,   0xff, 0xff    } /* '`' */,
		{ 0x1d, CAPLKOF, 0xff, 0xff    } /* 'a' */,
		{ 0x2d, CAPLKOF, 0xff, 0xff    } /* 'b' */,
		{ 0x2b, CAPLKOF, 0xff, 0xff    } /* 'c' */,
		{ 0x1f, CAPLKOF, 0xff, 0xff    } /* 'd' */,
		{ 0x12, CAPLKOF, 0xff, 0xff    } /* 'e' */,
		{ 0x20, CAPLKOF, 0xff, 0xff    } /* 'f' */,
		{ 0x21, CAPLKOF, 0xff, 0xff    } /* 'g' */,
		{ 0x22, CAPLKOF, 0xff, 0xff    } /* 'h' */,
		{ 0x17, CAPLKOF, 0xff, 0xff    } /* 'i' */,
		{ 0x23, CAPLKOF, 0xff, 0xff    } /* 'j' */,
		{ 0x24, CAPLKOF, 0xff, 0xff    } /* 'k' */,
		{ 0x25, CAPLKOF, 0xff, 0xff    } /* 'l' */,
		{ 0x2f, CAPLKOF, 0xff, 0xff    } /* 'm' */,
		{ 0x2e, CAPLKOF, 0xff, 0xff    } /* 'n' */,
		{ 0x18, CAPLKOF, 0xff, 0xff    } /* 'o' */,
		{ 0x19, CAPLKOF, 0xff, 0xff    } /* 'p' */,
		{ 0x10, CAPLKOF, 0xff, 0xff    } /* 'q' */,
		{ 0x13, CAPLKOF, 0xff, 0xff    } /* 'r' */,
		{ 0x1e, CAPLKOF, 0xff, 0xff    } /* 's' */,
		{ 0x14, CAPLKOF, 0xff, 0xff    } /* 't' */,
		{ 0x16, CAPLKOF, 0xff, 0xff    } /* 'u' */,
		{ 0x2c, CAPLKOF, 0xff, 0xff    } /* 'v' */,
		{ 0x11, CAPLKOF, 0xff, 0xff    } /* 'w' */,
		{ 0x2a, CAPLKOF, 0xff, 0xff    } /* 'x' */,
		{ 0x15, CAPLKOF, 0xff, 0xff    } /* 'y' */,
		{ 0x29, CAPLKOF, 0xff, 0xff    } /* 'z' */,
		{ 0x1b, SHIFT,   0xff, 0xff    } /* '{' */,
		{ 0x0d, SHIFT,   0xff, 0xff    } /* '|' */,
		{ 0x28, SHIFT,   0xff, 0xff    } /* '}' */,
		{ 0x1a, SHIFT,   0xff, 0xff    } /* '~' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x7f' */,
		{ 0x00, NOSHIFT, 0xff, 0xff    } /* Esc */,
		{ 0x62, NOSHIFT, 0xff, 0xff    } /* F1 */,
		{ 0x63, NOSHIFT, 0xff, 0xff    } /* F2 */,
		{ 0x64, NOSHIFT, 0xff, 0xff    } /* F3 */,
		{ 0x65, NOSHIFT, 0xff, 0xff    } /* F4 */,
		{ 0x66, NOSHIFT, 0xff, 0xff    } /* F5 */,
		{ 0x67, NOSHIFT, 0xff, 0xff    } /* F6 */,
		{ 0x68, NOSHIFT, 0xff, 0xff    } /* F7 */,
		{ 0x69, NOSHIFT, 0xff, 0xff    } /* F8 */,
		{ 0x6a, NOSHIFT, 0xff, 0xff    } /* F9 */,
		{ 0x6b, NOSHIFT, 0xff, 0xff    } /* F10 */,
		{ 0x52, NOSHIFT, 0xff, 0xff    } /* F11 */,
		{ 0x53, NOSHIFT, 0xff, 0xff    } /* F12 */,
		{ 0x54, NOSHIFT, 0xff, 0xff    } /* F13 */,
		{ 0x55, NOSHIFT, 0xff, 0xff    } /* F14 */,
		{ 0x56, NOSHIFT, 0xff, 0xff    } /* F15 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F16 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F17 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F18 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F19 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* F20 */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x95' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x96' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x97' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x98' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x99' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9a' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9b' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* EXT1(F28) */,
		{ 0xff, 0xff,    0xff, 0xff    } /* EXT2(F29) */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9e' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\x9f' */,
		{ 0x1c, IGSHIFT, 0xff, 0xff    } /* Enter */,
		{ 0x0e, IGSHIFT, 0xff, 0xff    } /* BackSpace */,
		{ 0x0f, NOSHIFT, 0xff, 0xff    } /* Tab */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xa3' */,
		{ 0x38, NOSHIFT, 0xff, 0xff    } /* Insert */,
		{ 0x39, NOSHIFT, 0xff, 0xff    } /* Delete */,
		{ 0x3e, NOSHIFT, 0x5e, NOSHIFT } /* Home */,
		{ 0x3f, NOSHIFT, 0xff, 0xff    } /* End */,
		{ 0x37, NOSHIFT, 0xff, 0xff    } /* PageUp */,
		{ 0x36, NOSHIFT, 0xff, 0xff    } /* PageDown */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xaa' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xab' */,
		{ 0x3b, NOSHIFT, 0xff, 0xff    } /* Left */,
		{ 0x3c, NOSHIFT, 0xff, 0xff    } /* Right */,
		{ 0x3a, NOSHIFT, 0xff, 0xff    } /* Up */,
		{ 0x3d, NOSHIFT, 0xff, 0xff    } /* Down */,
		{ 0xff, 0xff,    0xff, 0xff    } /* ScrollLock */,
		{ 0xff, 0xff,    0xff, 0xff    } /* NumLock */,
		{ 0x71, NOSHIFT, 0xff, 0xff    } /* CapsLock */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xb3' */,
		{ 0x70, 0xfe,    0xff, 0xff    } /* Shift */,
		{ 0x74, 0xfe,    0xff, 0xff    } /* Ctrl */,
		{ 0x73, 0xfe,    0x72, 0xfe    } /* Alt */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xb7' */,
		{ 0x61, NOSHIFT, 0xff, 0xff    } /* COPY(PrintScreen) */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Pause */,
		{ 0x60, NOSHIFT, 0xff, 0xff    } /* STOP(Break) */,
		{ 0xff, 0xff,    0xff, 0xff    } /* SysRq */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xbc' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xbd' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xbe' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xbf' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Windows */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Menu */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Power */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Sleep */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Wake */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc5' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc6' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc7' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc8' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xc9' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xca' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xcb' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xcc' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xcd' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xce' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xcf' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Zenkaku */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Muhenkan */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Henkan */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Hiragana */,
		{ 0xff, 0xff,    0xff, 0xff    } /* Katakana */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd5' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd6' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd7' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd8' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xd9' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xda' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xdb' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xdc' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xdd' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xde' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xdf' */,
/* e0~f7는, 할당하는 것이 귀찮게 된 기종 고유의 마이너 키 */
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe0' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe1' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe2' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe3' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe4' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe5' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe6' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe7' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe8' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xe9' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xea' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xeb' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xec' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xed' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xee' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xef' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf0' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf1' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf2' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf3' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf4' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf5' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf6' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf7' */,
/* f8~ff는, 마이너 쉬프트 키 */
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf8' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xf9' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfa' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfb' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfc' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfd' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xfe' */,
		{ 0xff, 0xff,    0xff, 0xff    } /* '\xff' */
	#endif
};

#if (defined(PCAT))
	static struct {
		int cmd, length;
		int deccmd[6 * 16 + 3];
		int eoc;
	} defbindcommand = {
		0x0068, 101 * 4, {
			0x0110     /* clear */,
			0          /* opt */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x46       /* rawcode(ScrollLock) */,
			0x0000c070 /* shiftmap */,
			0x0001     /* xor bit */,
			0x00060000 /* cmd(xor) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x45       /* rawcode(NumLock) */,
			0x0000c070 /* shiftmap */,
			0x0002     /* xor bit */,
			0x00060000 /* cmd(xor) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x3a       /* rawcode(CapsLock) */,
			0x0010c070 /* shiftmap */,
			0x0004     /* xor bit */,
			0x00060000 /* cmd(xor) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x2a       /* rawcode(left-Shift) */,
			0x0000c000 /* shiftmap */,
			0x0010     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x2a       /* rawcode(left-Shift) */,
			0x4000c000 /* shiftmap */,
			~0x0010    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x36       /* rawcode(right-Shift) */,
			0x0000c000 /* shiftmap */,
			0x0010     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x36       /* rawcode(right-Shift) */,
			0x4000c000 /* shiftmap */,
			~0x0010    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x1d       /* rawcode(left-Ctrl) */,
			0x0000c000 /* shiftmap */,
			0x0020     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x1d       /* rawcode(left-Ctrl) */,
			0x4000c000 /* shiftmap */,
			~0x0020    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x9d       /* rawcode(right-Ctrl) */,
			0x0000c000 /* shiftmap */,
			0x0020     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x9d       /* rawcode(right-Ctrl) */,
			0x4000c000 /* shiftmap */,
			~0x0020    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x38       /* rawcode(left-Alt) */,
			0x0000c000 /* shiftmap */,
			0x0040     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x38       /* rawcode(left-Alt) */,
			0x4000c000 /* shiftmap */,
			~0x0040    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0xb8       /* rawcode(right-Alt) */,
			0x0000c000 /* shiftmap */,
			0x0040     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0xb8       /* rawcode(right-Alt) */,
			0x4000c000 /* shiftmap */,
			~0x0040    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0xd3       /* rawcode(Deletele) */,
			0x0060c070 /* shiftmap */,
			0          /* reserve */,
			0x00380000 /* cmd(reset) */,

			0x0000 /* EOC */
		}, 0x0000 /* EOC */
	};
#endif
#if (defined(TOWNS))
	static struct {
		int cmd, length;
		int deccmd[6 * 10 + 3];
		int eoc;
	} defbindcommand = {
		0x0068, 65 * 4, {
			0x0110     /* clear */,
			0          /* opt */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x55       /* rawcode(CapsLock) */,
			0x0000c070 /* shiftmap */,
			0x0004     /* xor bit */,
			0x00060000 /* cmd(xor) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x53       /* rawcode(Shift) */,
			0x0000c000 /* shiftmap */,
			0x0010     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x53       /* rawcode(Shift) */,
			0x4000c000 /* shiftmap */,
			~0x0010    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x52       /* rawcode(Ctrl) */,
			0x0000c000 /* shiftmap */,
			0x0020     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x52       /* rawcode(Ctrl) */,
			0x4000c000 /* shiftmap */,
			~0x0020    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x5c       /* rawcode(Alt) */,
			0x0000c000 /* shiftmap */,
			0x0040     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x5c       /* rawcode(Alt) */,
			0x4000c000 /* shiftmap */,
			~0x0040    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x72       /* rawcode(취소) */,
			0x0000c000 /* shiftmap */,
			0x0040     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x72       /* rawcode(취소) */,
			0x4000c000 /* shiftmap */,
			~0x0040    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x4b       /* rawcode(Deletele) */,
			0x0060c070 /* shiftmap */,
			0          /* reserve */,
			0x00380000 /* cmd(reset) */,

			0x0000 /* EOC */
		}, 0x0000 /* EOC */
	};
#endif
#if (defined(NEC98))
	static struct {
		int cmd, length;
		int deccmd[6 * 9 + 3];
		int eoc;
	} defbindcommand = {
		0x0068, 59 * 4, {
			0x0110     /* clear */,
			0          /* opt */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x71       /* rawcode(CapsLock) */,
			0x0000c070 /* shiftmap */,
			0x0004     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x71       /* rawcode(CapsLock) */,
			0x4000c070 /* shiftmap */,
			~0x0004    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x70       /* rawcode(Shift) */,
			0x0000c000 /* shiftmap */,
			0x0010     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x70       /* rawcode(Shift) */,
			0x4000c000 /* shiftmap */,
			~0x0010    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x74       /* rawcode(Ctrl) */,
			0x0000c000 /* shiftmap */,
			0x0020     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x74       /* rawcode(Ctrl) */,
			0x4000c000 /* shiftmap */,
			~0x0020    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x73       /* rawcode(Alt) */,
			0x0000c000 /* shiftmap */,
			0x0040     /* or bit */,
			0x00040000 /* cmd(or) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x73       /* rawcode(Alt) */,
			0x4000c000 /* shiftmap */,
			~0x0040    /* and bit */,
			0x00050000 /* cmd(and) */,

			0x010c     /* define */,
			4          /* opt(len) */,
			0x39       /* rawcode(Deletele) */,
			0x0060c070 /* shiftmap */,
			0          /* reserve */,
			0x00380000 /* cmd(reset) */,

			0x0000 /* EOC */
		}, 0x0000 /* EOC */
	};
#endif

void OsaskMain()
{
	int *signal, *signal0, i, j;
	struct WM0_WINDOW *win;
	struct STR_JOB *pjob = &job;
	struct MOSWINSIG *mws;

	#if (defined(TOWNS))
		static int TOWNS_mouseinit[] = {
			0x0064, 17 * 4, 0x0030 /* SetMouseParam */, 0x030d,
			20 /* 샘플링 rate 20밀리 세컨드 */,
			0, 0, /* TAPI의 시그널 처리 벡터(TAPI_SignalMessageTimer) */
			0x3245, 0x7f000004, 0x73756f6d, 0,
			0x000d0019 /* wait0=25, wait1=13 */, 0x0f3f0f0f /* strobe */,
			0x0f0f0f3f, 0x000030, 0x0f0f0f3f, 0x00000030,
			0 /* eoc */
		};
		static int TOWNS_mousestop[] = {
			0x0064, 4*4, 0x0030 /* SetMouseParam */, 0x0002, 0
		};
	#endif
	#if (defined(NEC98))
		static int NEC98_mouseinit[] = {
			0x0060, 4 * 4, 0x0120 /* EnableMouse */, 0, 0
		};
	#endif

	MALLOC_ADDR = pjob->readCSd10 = lib_readCSd(0x0010);
	screen.wallpaper = malloc(WALLPAPERMAXSIZE + 65536);

	lib_init(AUTO_MALLOC);
	sgg_init(AUTO_MALLOC);

	signal = signal0 = lib_opensignalbox(256 * 4, AUTO_MALLOC, 0, 4); // 1KB

	// 윈도우를 확보해, 모두미사용 윈도우로서 등록
	window = (struct WM0_WINDOW *) malloc(MAX_WINDOWS * sizeof (struct WM0_WINDOW));
	for (i = 0; i < MAX_WINDOWS; i++) {
		window[i]. sgg.handle = 0;
		chain_unuse(&window[i]);
	}

	// 사운드 트랙용 버퍼의 초기화
	sndtrk_buf = (struct SOUNDTRACK *) malloc(MAX_SOUNDTRACK * sizeof (struct SOUNDTRACK));
	for (i = 0; i < MAX_SOUNDTRACK; i++)
	//	free_sndtrk(&sndtrk_buf[i]);
		sndtrk_buf[i]. sigbox = 0;

	pjob->list = (int *) malloc(JOBLIST_SIZE * sizeof (int));
	*(pjob->rp = pjob->wp = pjob->list) = 0; /* 쌓일 일은 없다 */
	pjob->free = JOBLIST_SIZE - 1;

	defsigbuf = (struct DEFINESIGNAL *) malloc (DEFSIGBUFSIZ * sizeof (struct DEFINESIGNAL));
	for (i = 0; i < DEFSIGBUFSIZ - 1; i++)
		defsigbuf[i]. win = NULL;
	defsigbuf[DEFSIGBUFSIZ - 1]. win = -1;

	for (i = 0; i < MAXWINDEF; i++)
		windef[i]. x = 0x10000000;

	sgg_execcmd(tapisigvec);

	#if (defined(TOWNS))
		TOWNS_mouseinit[5] = tapisigvec[3];
		TOWNS_mouseinit[6] = tapisigvec[4];
		sgg_execcmd(TOWNS_mouseinit);
	#endif
	#if (defined(NEC98))
		sgg_execcmd(NEC98_mouseinit);
	#endif

	mws = moswinsig = malloc(MOSWINSIGS * sizeof(struct MOSWINSIG));
	for (i = 0; i < MOSWINSIGS - 1; i++, mws++) {
		mws->win = NULL;
		mws->flags = 0;
	}
	mws->win = (struct WM0_WINDOW *) -1;

	#if (defined(TIMEX))
		lib_opentimer(SYSTEM_TIMER);
		lib_definesignal1p0(0, 0x0010 /* timer */, SYSTEM_TIMER, 0, SIG_WRITE_TIME);
		lib_settimertime(0x0032, SYSTEM_TIMER, 0x80000000 /* 500ms */, 0, 0);
	#endif

	for (;;) {
		unsigned char siglen = 1;
		int sig4;
		win = top;
		struct SOUNDTRACK *sndtrk;
		switch (i = signal[0]) {
		case 0x0000:
		//	siglen--; /* siglen = 0; */
			if (pjob->now == 0)
				runjobnext();
			lib_waitsignal(0x0001, 0, 0);
			continue;

		case 0x0004 /* rewind */:
		//	siglen--; /* siglen = 0; */
			lib_waitsignal(0x0000, signal[1], 0);
			signal = signal0;
			continue;

		case 0x0010:
			/* 초기화 요청 */
		//	siglen = 1;
			#if (defined(PCAT))
				writejob_n(4, 0x0030 /* open VGA driver */, 0x0000,
					0x0034 /* change VGA mode */, 0x0012);
			#elif (defined(TOWNS))
				writejob_n(4, 0x0030 /* open VGA driver */, 0x0000,
					0x0034 /* change VGA mode */, 0x0000);
			#elif (defined(NEC98))
				writejob_n(4, 0x0030 /* open VGA driver */, 0x0000,
					0x0034 /* change VGA mode */, 0x0000);
			#endif
		fin_wrtjob:
			*pjob->wp = 0; /* 스톱퍼 */
		check_jobnext:
			if (pjob->now == 0)
				runjobnext();
			break;

		case 0x0018:
			/* from boot */
			siglen++; /* siglen = 2; */
			fromboot = signal[1];
			#if (defined(PCAT))
				if (fromboot == 0) {
					/* VESA의 조사를 개시 */
					writejob_n(1, 0x0044 /* VESA check */);
					goto fin_wrtjob;
				} else {
					send_signal3dw(0x4243, 0x7f000002, 0x00be /* sig_vbelist */, (int) vbelist);
				}
			#endif
			break;

		case 0x001c:
			/* 종료 요청 */
#if 0
			if (signal[1] == 4) {
				/* close all-window(포함한 pokon0) */
				signal += 2;
				lib_waitsignal(0x0000, 2, 0);
	close_all:
				allclose = 0;
				if (top == pokon0 && top->down == pokon0)
					break;
				allclose = 1;
				win = top;
				if (win == pokon0)
					win = win->down;
				sgg_wm0s_close(&win->sgg);
				closewin = win;
				break;
			}
#endif
			goto mikannsei;

		case 0x0020:
			/* 윈도우 오픈 요청(handle) */
			win = get_unuse();
			win->flags = 0;
			sgg_wm0_openwindow(&win->sgg, signal[1]);
			siglen++; /* siglen = 2; */
		//	win->ds1 = win->defsig;
			writejob_n(2, 0x0020 /* open */, (int) win);
			goto fin_wrtjob;

		case 0x0024:
			/* 윈도우 클로우즈 요청(handle) */
			win = handle2window(signal[1]);
			siglen++; /* siglen = 2; */
			if ((win->flags & 0x01) == 0) {
				win->flags |= 0x01; /* 클로우즈 처리 중 */
				writejob_n(2, 0x002c /* close */, (int) win);
				goto fin_wrtjob;
			}
			break;

		case 0x0028:
			/* 윈도우 액티브 요청(opt, handle) */
			goto mikannsei;

		case 0x002c:
			/* 윈도우 연동 디바이스 지정
			    (opt,  win-handle, reserve(signalbox),
			       default-device, default-code, len(2), 0x7f000001, signal) */

			win = handle2window(signal[2]);
			{
				struct DEFINESIGNAL *dsp;
				for (dsp = defsigbuf; dsp->win ! = NULL && dsp->win ! = -1; dsp++);
				if (dsp->win == NULL) {
					dsp->win = (int) win;
					dsp->opt = signal[1];
					dsp->dev = signal[4];
					dsp->cod = signal[5];
					dsp->len = 2;
					dsp->sig[0] = signal[7];
					dsp->sig[1] = signal[8];
					if (iactive == win) {
						int sigbox = sgg_wm0_win2sbox(&win->sgg);
						sgg_wm0_definesignal3(dsp->opt, dsp->dev,
							dsp->cod, sigbox, dsp->sig[0], dsp->sig[1]);
					}
				}
			}
			siglen = 9;
			break;

		case 0x0030:
			/* 마우스 시그널 지정
			    (win-handle, reserve(signalbox),
			       default-device, default-code, [x0, y0], len(2), 0x7f000001, signal) */
			for (mws = moswinsig; mws->win ! = NULL && mws->win ! = (struct WM0_WINDOW *) -1; mws++);
			siglen = 8;
			i = 6;
			sig4 = signal[4];
			if ((sig4 & 0xff) == 0x10 || (sig4 & 0xff) == 0x12) {
				siglen = 10;
				i += 2;
			}
			if (mws->win == NULL) {
				static int flagtable[] = { 0x00100002, 0x00700002, 0x07700002, 0x77700002 };
				mws->win = handle2window(signal[1]);
				mws->flags = (sig4 & 0xff) << 8 | flagtable[(sig4 >> 12) & 0x3];
				mws->sig[4] = signal[5];
				mws->sig[5] = signal[6];
				mws->sig[0] = signal[i];
				mws->sig[1] = signal[i + 1];
			}
			moswinsig_flagset();
			if ((sig4 & 0xff) == 0x12 && mx ! = 0x80000000)
				mousesignal(mbutton, 0, 0); /* eye등을 위해 */
			break;

		case 0x0040: /* open sound track (slot, tss, signal-base, reserve0, reserve1)
			   수리한 것을 알리기 위해서 시그널로 응답한다 */
		//	sndtrk = alloc_sndtrk();
			for (sndtrk = sndtrk_buf; sndtrk->sigbox ! = 0; sndtrk++);
			sndtrk->sigbox  = signal[2 /* tss */] + 0x0240;
			sndtrk->slot    = signal[1 /* slot */];
			sndtrk->sigbase = signal[3 /* signal-base */];
			siglen = 6;
			/* 핸들 번호의 대응 통지 */
			send_signal3dw(sndtrk->sigbox, sndtrk->sigbase + 0, sndtrk->slot, (int) sndtrk);
			if (sndtrk_active == NULL) {
				sndtrk_active = sndtrk;
				/* 액티브 시그널을 보낸다 */
	sndtrk_sendactsig:
				send_signal2dw(sndtrk->sigbox, sndtrk->sigbase + 8 /* enable */, sndtrk->slot);
			}
			break;

		case 0x0044: /* close sound track (handle) */
			sndtrk = (struct SOUNDTRACK *) signal[1];
			siglen++; /* siglen = 2; */
			/* close 완료를 알게 한다 */
			send_signal2dw(sndtrk->sigbox, sndtrk->sigbase + 4 /* close */, sndtrk->slot);
		//	free_sndtrk(sndtrk);
			sndtrk->sigbox = 0;
			if (sndtrk == sndtrk_active) {
				/* 다른 녀석을 액티브하게 한다 */
				sndtrk = NULL;
				for (i = 0; i < MAX_SOUNDTRACK; i++) {
					if (sndtrk_buf[i]. sigbox) {
						sndtrk = &sndtrk_buf[i];
						break;
					}
				}
				if (sndtrk_active = sndtrk)
					goto sndtrk_sendactsig; /* 액티브 시그널을 보낸다 */
			}
			break;

		case 0x0048: /* load external font (font-type, tss, len, sig) */
			siglen = 5;
			writejob_n(4, 0x0038 /* loadfont */, signal[1] /* type */,
				signal[2] /* tss */, signal[4] /* sig */);
			goto fin_wrtjob;

		case 0x0050:
		case 0x0051:
		case 0x0052:
		case 0x0053:
		case 0x0054: /* 0x005f까지는 리저브 */
			(*pjob->func)(i - 0x0050, 0);
		//	siglen = 1;
			goto check_jobnext;

			#if (defined(TIMEX))
		case SIG_WRITE_TIME: /* 0x0060 */
			lib_settimertime(0x0032, SYSTEM_TIMER, 0x80000000 /* 500ms */, 0, 0);
			if (mx ! = 0x80000000)
				write_time();
		//	siglen = 1;
			break;
			#endif

		case 0x0070: /* default-keybind-customize */
			siglen = 3;
			keybindtable[signal[1] - ' ']. rawcode0   =  signal[2]        & 0xff;
			keybindtable[signal[1] - ' ']. shifttype0 = (signal[2] >>  8) & 0xff;
			keybindtable[signal[1] - ' ']. rawcode1   = (signal[2] >> 16) & 0xff;
			keybindtable[signal[1] - ' ']. shifttype1 = (signal[2] >> 24) & 0xff;
			redirect_input(iactive);
			break;

		case 0x0074: /* default-specialkeybind-customize */
			siglen = 4;
			defbindcommand.deccmd[signal[1]    ] = signal[2];
			defbindcommand.deccmd[signal[1] + 1] = signal[3];
			redirect_input(iactive);
			break;

		case 0x0014: /* 화면 모드 변경 완료(result) */
		case 0x00c0: /* 갱신 정지 시그널(handle) */
		case 0x00c4: /* 그리기 완료 시그널(handle) */
		//	i = signal[0];
			j = signal[1];
			siglen++; /* siglen = 2; */
			(*pjob->func)(i, j);
			goto check_jobnext;

		case 0x00d0:
		case 0x00d1:
		case 0x00d2:
		case 0x00d3:
		case 0x00f0:
		//	i = signal[0];
		//	siglen = 1;
			job_movewin4(i);
			break;


		case 0x0200 /* active bottom window */:
		//	siglen = 1;
			if ((win->flags & 0x01) == 0) {
				writejob_n(2, 0x0024 /* active */, (int) win /* top */ ->up);
				goto fin_wrtjob;
			}
			break;

		case 0x0201 /* active second window */:
		//	siglen = 1;
			if ((win->flags & 0x01) == 0) {
				writejob_n(2, 0x0024 /* active */, (int) win /* top */ ->down);
				goto fin_wrtjob;
			}
			break;

		case 0x0202 /* move window */:
		//	siglen = 1;
			if ((win->flags & 0x01) == 0) {
				writejob_n(2, 0x0028 /* move by keyboard */, (int) win /* top */);
				goto fin_wrtjob;
			}
			break;

		case 0x0203 /* close window */:
		//	siglen = 1;
			if (win /* top */ ! = pokon0 && (win->flags & 0x01) == 0)
				sgg_wm0s_close(&win /* top */ ->sgg);
			break;

		case 0x0204 /* VGA mode 0x0012 */:
		//	siglen = 1;
			#if (defined(PCAT))
				writejob_n(2, 0x0034 /* change VGA mode */, 0x0012);
			#elif (defined(TOWNS))
				writejob_n(2, 0x0034 /* change VGA mode */, 0x0000);
			#elif (defined(NEC98))
				writejob_n(2, 0x0034 /* change VGA mode */, 0x0000);
			#endif
			goto fin_wrtjob;

		case 0x0205 /* VESA mode 0x0102 */:
		//	siglen = 1;
			#if (defined(PCAT))
				writejob_n(2, 0x0034 /* change VGA mode */, 0x0102);
			#endif
			#if (defined(TOWNS))
				writejob_n(2, 0x0034 /* change VGA mode */, 0x0001);
			#endif
			goto fin_wrtjob;

		case 0x0206:
		//	siglen = 1;
			#if (defined(PCAT))
				if (f3mode) {
					writejob_n(2, 0x0034 /* change VGA mode */, f3mode);
					goto fin_wrtjob;
				}
				break;
			#elif (defined(TOWNS) && defined(VMODE))
				writejob_n(2, 0x0034 /* change Video mode */, 0x0002);
				goto fin_wrtjob;
			#endif

		case 0x0207: /* PF4 */
			#if (defined(TOWNS))
				//	siglen = 1;
				writejob_n( 2, 0x0034, 3);
				goto fin_wrtjob;
			#endif
			#if (defined(PCAT))
			//	siglen = 1;
				if (f4mode) {
					writejob_n(2, 0x0034 /* change VGA mode */, f4mode);
					goto fin_wrtjob;
				}
				break;

		case 0x0210:
				siglen = 3;
				if (signal[1] ==  3)
					f3mode = signal[2];
				if (signal[1] == -3)
					f3mode = signal[2];
				if (signal[1] ==  4)
					f4mode = signal[2];
				if (signal[1] == -4)
					f4mode = signal[2];
				if (signal[1] >= 0) {
					writejob_n(2, 0x0034 /* change VGA mode */, signal[2]);
					goto fin_wrtjob;
				}
				break;

		case 0x0211:
				siglen = 3;
				if (signal[1] == 0)
					vbeoverride[0] = vbeoverride[1] = vbeoverride[2] = signal[2];
				else
					vbeoverride[signal[1] - 1] = signal[2];
				break;

			#endif

			#if (defined(TOWNS)) && (defined(CLGD543X))
		case 0x0220:	/* PF13이지만……0x210로는 안되는데 */
		case 0x0221:	/* PF14 */
			//	siglen = 1;
			/* HW code(5430=0x100, 5434=0x200, none=0)가 되돌아 온다 */
		  if (pf13mode < 0)
		    pf13mode = sgg_execcmd1(1 * 4 + 12, 0x00a0, 0, 0x0000);
				if (pf13mode) {
				  writejob_n(2, 0x0034 /* change VGA mode */, pf13mode + i-0x220+1);
				  goto fin_wrtjob;
				}
				break;
			#endif

		case 0x0230 /* setwindef */:
			siglen = 3;
			for (i = 0; i < MAXWINDEF - 1; i++) {
				if (windef[i]. x == 0x10000000)
					break;
			}
			windef[i]. x = signal[1];
			windef[i]. y = signal[2];
			break;

		case 0x0240 /* load JPN16$.FNT */:
		//	siglen = 1;
			writejob_n(4, 0x0038 /* loadfont */, 0x11 /* type */, 0, 0);
			goto fin_wrtjob;

#if (defined(PCAT) || defined(TOWNS))

		case 0x0244 /* capture */:
		//	siglen = 1;
			writejob_n(1, 0x003c /* capture */);
			goto fin_wrtjob;
#endif

		case 0x0248 /* load wallpaper */:
		//	siglen = 1;
			writejob_n(1, 0x0040 /* load wallpaper */);
			goto fin_wrtjob;

		case 0x024c /* set wallpaper */:
			siglen = 4;
			*(int *) &screen.wallpaper_name[0] = signal[1];
			*(int *) &screen.wallpaper_name[4] = signal[2];
			*(int *) &screen.wallpaper_name[8] = signal[3];
			writejob_n(1, 0x0048 /* set wallpaper */);
			goto fin_wrtjob;

			#if (defined(TOWNS))
		case 0x0300: /* townsmouse */
			siglen++; /* siglen = 2; */
			sgg_execcmd(TOWNS_mousestop);
			if (signal[1] & 0x0f) {
			  int par = signal[1] & 0x0f, port = 0;
			  TOWNS_mouseinit[ 3] = 0x0000030d;
			  TOWNS_mouseinit[13] = 0;
			  TOWNS_mouseinit[14] = 0x00000030;
			  TOWNS_mouseinit[15] = 0;
			  TOWNS_mouseinit[16] = 0x00000030;
			  for (; par; par >>= 2, port++)
			    if (par & 0x03) {
			      TOWNS_mouseinit[13 + port*2] = 0x0f0f0f3f;
			      if (par & 0x02) /* pad */
			        TOWNS_mouseinit[14 + port*2] = 0x0f0f0f3f;
			    }
			  sgg_execcmd(TOWNS_mouseinit);
			}
			townsmouse = signal[1];
			break;
			#endif

			#if (defined(PCAT))
		case 0x10001 /* 영어 키보드로 변경 */:
			//	siglen = 1;
				for (i = 0; i < 20; i++) {
					static struct DIFF_KEYTABLE {
						unsigned char index;
						struct KEYTABLE bind;
					} table[] = {
						'\x22',	{ 0x28, SHIFT,   0xff, 0xff    },
						'&',	{ 0x08, SHIFT,   0xff, 0xff    },
						'\x27',	{ 0x28, NOSHIFT, 0xff, 0xff    },
						'(',	{ 0x0a, SHIFT,   0xff, 0xff    },
						')',	{ 0x0b, SHIFT,   0xff, 0xff    },
						'*',	{ 0x09, SHIFT,   0xff, 0xff	   },
						'+',	{ 0x0d, SHIFT,   0x4e, IGSHIFT },
						':',	{ 0x27, SHIFT,   0xff, 0xff    },
						'=',	{ 0x0d, SHIFT,   0xff, 0xff    },
						'@',	{ 0x03, SHIFT,   0xff, 0xff    },
						'[',	{ 0x1a, NOSHIFT, 0xff, 0xff    },
						'\\',	{ 0x2b, NOSHIFT, 0xff, 0xff    },
						']',	{ 0x1b, NOSHIFT, 0xff, 0xff    },
						'^',	{ 0x07, SHIFT,   0xff, 0xff    },
						'_',	{ 0x0c, SHIFT,   0xff, 0xff    },
						'`',	{ 0x29, NOSHIFT, 0xff, 0xff    },
						'{',	{ 0x1a, SHIFT,   0xff, 0xff    },
						'|',	{ 0x2b, SHIFT,   0xff, 0xff    },
						'}',	{ 0x1b, SHIFT,   0xff, 0xff    },
						'~',	{ 0x29, SHIFT,   0xff, 0xff    }
					};
					keybindtable[table[i]. index - ' '] = table[i]. bind;
				}
				redirect_input(iactive);
				break;
			#endif

			#if (defined(NEC98))
		case 0x0999 /* mouse move to (0, 400-16) */:
				//	siglen = 1;
				if (pjob->now == 0 && mx ! = 0x80000000)
					sgg_wm0_movemouse(mx = 0, my = 400-16);
				break;
			#endif

			#if (defined(TOWNS) && defined(FMRMOUSE))
		case 0x73756f6d + 1: /* from rmouse */
			siglen++;
			if (mx ! = 0x80000000 && (townsmouse & 0x10))
				mousesignal(signal[1] & 3, (signed char)(signal[1] >> 8),
					(signed char)(signal[1] >> 16));
			break;
			#endif

		case 0x73756f6d /* from mouse */:
			#if (defined(TOWNS))
				if (mx ! = 0x80000000 && (townsmouse & 0x0f)) {
					char port, cfg = townsmouse;
					int dx = 0, dy = 0, btn = 0;
					for (port = 0; port < 2; port++, cfg >>= 2) {
						int b = signal[2+port];
						if (cfg & 3) {
							if ((cfg & 2) == 0) {
								dx -=(signed char)(0x0f &(b >> 16)| 0xf0 &(b >> 20));
								dy -=(signed char)(0x0f & b       | 0xf0 &(b >>  4));
								btn |= ((b >> 4) & 0x03) ^ 0x03;
							} else {
								static char d[] = {0, -1, 1, 0};
								b = (b & (b >> 16) & 0x0f3f) ^ 0x0f3f;
								if (cfg & 1) /* 6PAD */
									b = b | (b >> 5) & 0x40; /* C button */
								dy += d[b & 3];
								dx += d[(b >> 2) & 3];
								btn |= (b >> 4) & 7;
							}
						}
					}
					mousesignal(btn, dx, dy);
				}
				siglen = 4;
			#endif
			#if (defined(PCAT))
				if (mx ! = 0x80000000)
					mousesignal(signal[1], (signed short) (signal[2] & 0xffff), (signed short) (signal[2] >> 16));
				siglen = 3;
			#endif
			#if (defined(NEC98))
				if (mx ! = 0x80000000)
					mousesignal(signal[1] & 0xff,
						  (signed char) ((signal[1] >>  8) & 0xff),
						  (signed char) ((signal[1] >> 16) & 0xff));
				siglen = 2;
			#endif

			break;

	//	case 0x6f6b6f70 + 0 /* mousespeed */:
	//		mousescale = signal[1];
	//		siglen++; /* siglen = 2; */
	//		break;

		case 0x6f6b6f70 + 1 /* mouseaccel */:
			mouseaccel = signal[1];
			mousescale = signal[2];
			siglen+=2; /* siglen = 3; */
			break;

		default:
		mikannsei:
		//	lib_drawline(0x0020, (void *) -1, 0, 0, 0, 15, 15); /* 여기에 온 것을 알린다 */
		//	siglen = 1;
			;
		}
		if (siglen) {
			signal += siglen;
			lib_waitsignal(0x0000, siglen, 0);
		}
	}
}

void init_screen(const int x, const int y)
{
	static int old_x, old_y;
	struct STR_BGV {
		signed short col, x0, y0, x1, y1;
	};
	int x2y2[2];
	#if (defined(WIN9X))
		static struct STR_BGV linedata[] = {
			{  8,   0, -28,  -1, -28 },
			{ 15,   0, -27,  -1, -27 },
			{  8,   0, -26,  -1,  -1 },
			{ 15,   3, -24,  59, -24 },
			{ 15,   2, -24,   2,  -4 },
			{  7,   3,  -4,  59,  -4 },
			{  7,  59, -23,  59,  -5 },
			{  0,   2,  -3,  59,  -3 },
			{  0,  60, -24,  60,  -3 },
			#if (! defined(TIMEX))
				{  7, -47, -24,  -4, -24 },
				{  7, -47, -23, -47,  -4 },
				{ 15, -47,  -3,  -4,  -3 },
				{ 15,  -3, -24,  -3,  -3 },
			#else
				{  7, TIMEX - 4, -24,        -4, -24 },
				{  7, TIMEX - 4, -23, TIMEX - 4,  -4 },
				{ 15, TIMEX - 4,  -3,        -4,  -3 },
				{ 15,        -3, -24,        -3,  -3 },
			#endif
			{ -1,   0,   0,  -1, -29 }		/* for wallpaper */
		};
	#elif (defined(TMENU))
		#define TMGUIB2(c, x, y, w, h) \
			{ 15, x,         y,         x + w - 1, y         }, \
			{ 15, x,         y + 1,     x + w - 2, y + 1     }, \
			{  0, x + w - 1, y + 1,     x + w - 1, y + 1     }, \
			{ 15, x,         y + 2,     x + 1,     y + h - 2 }, \
			{  c, x + 2,     y + 2,     x + w - 3, y + h - 3 }, \
			{  0, x + w - 2, y + 2,     x + w - 1, y + h - 3 }, \
			{  0, x + 2,     y + h - 2, x + w - 1, y + h - 2 }, \
			{ 15, x,         y + h - 1, x,         y + h - 1 }, \
			{  0, x + 1,     y + h - 1, x + w - 1, y + h - 1 }
		#define TMGUIBF1(x, y, w, h) \
			{ 15, x,         y,         x + w - 1, y         }, \
			{ 15, x,         y + 1,     x,         y + h - 1 }, \
			{  0, x + w - 1, y + 1,     x + w - 1, y + h - 2 }, \
			{  0, x + 1,     y + h - 1, x + w - 1, y + h - 1 }
		#define BOXFRAME(c, x0, y0, x1, y1)	\
			{  c, x0,        y0,        x1,        y0        }, \
			{  c, x0,        y0 + 1,    x0,        y1 - 1    }, \
			{  c, x1,        y0 + 1,    x1,        y1 - 1    }, \
			{  c, x0,        y1,        x1,        y1        }
		static struct STR_BGV linedata[] = {
			TMGUIB2(0, 0, 0, 96, 20),			/* FM TOWNS button */
			{ 7, 96, 0, -1, 19 },				/* title bar */
			TMGUIB2(0, -28, 0, 20, 20),			/* exit button */
			BOXFRAME(0, 0, -28, -1, -1),
			{ 7, 1, -27, -2, -2 },				/* bar */
			TMGUIBF1( 6, -24, 90, 20),			/* "start" frame */
			{ -1, 0, 20, -1, -29 }				/* for wallpaper */
		};
		#undef	TMGUIB2
		#undef	TMGUIBF1
		#undef	BOXFRAME
	#elif (defined(CHO_OSASK))
		static struct STR_BGV linedata[] = {
			{  0,   0, -20,  -1, -20 },
			{ 15,   0, -19,  -1, -19 },
			{  7,   0, -18,  -1,  -1 },		/* System line(? ) */
			#if defined(TIMEX)
				{  0, TIMEX-2, -19, TIMEX-2,  -1 },
				{ 15, TIMEX-1, -18, TIMEX-1,  -1 },
			#else
				{  0, -82, -19, -82,  -1 },
				{ 15, -81, -18, -81,  -1 },
			#endif
			{ -1,   0,   0,  -1, -21 },		/* for wallpaper */
		};
	#elif (defined(NEWSTYLE))
		static struct STR_BGV linedata[] = {
			{ -1,   0,   0,  -1,  -1 }		/* for wallpaper */
		};
	#elif (defined(WIN31))
		static struct STR_BGV linedata[] = {
			{ -1,   0,   0,  -1,  -1 }		/* for wallpaper */
		};
	#endif

	struct STR_BGV *p;
	int x0, y0, x1, y1;

	x2y2[0] = x;
	x2y2[1] = y - (RESERVELINE0 + RESERVELINE1);
	sgg_execcmd0(0x00b0, 0, 2, 0, x2y2, 0x000c, 0x0000); /* set info */

	if (old_x ! = x || old_y ! = y) {
		screen.wallpaper_exist = 0;
		old_x = x;
		old_y = y;
	}

	for (p = linedata; ; p++) {
		if ((x0 = p->x0) < 0)
			x0 += x;
		if ((y0 = p->y0) < 0)
			y0 += y;
		if ((x1 = p->x1) < 0)
			x1 += x;
		if ((y1 = p->y1) < 0)
			y1 += y;
		if (p->col < 0) break;
		lib_drawline(0x0020, (void *) -1, p->col, x0, y0, x1, y1);
	}
	putwallpaper(x0, y0, x1 + 1, y1 + 1);

	#if (defined(TMENU))
		static int tosask[] = {
			0x00000000, 0x00000000, 0x00000000, 0x0f0f0f0f, 0x0000000f, 
			0x00000000, 0x0f0f0f0f, 0x0f0f0f0f, 0x000f0f0f, 0x00000000, 
			0x000f0000, 0x00000000, 0x00000000, 0x0f0f0f0f, 0x0f0f0f0f, 
			0x000f0f0f, 0x00000f0f, 0x00000000, 0x0f000000, 0x0e0e0e0f, 
			0x00000000, 0x00000000, 0x0f0f0000, 0x0f0f0f0f, 0x000f0f0f, 
			0x0f000000, 0x0f0f0f0f, 0x0f0f0f0f, 0x000f0f0f, 0x00000000, 
			0x0f0f0f00, 0x00000000, 0x0f000000, 0x0f0f0f0f, 0x0f0f0f0f, 
			0x000f0f0f, 0x00000f0f, 0x00000000, 0x0f0f0f00, 0x0e0e0e0f, 
			0x00000000, 0x00000000, 0x0f0f0f00, 0x00000000, 0x0f0f0f00, 
			0x0f0f0000, 0x0000000f, 0x00000000, 0x00000000, 0x00000000, 
			0x0f0f0f00, 0x00000000, 0x0f0f0000, 0x0000000f, 0x00000000, 
			0x00000000, 0x00000f0f, 0x0f000000, 0x0f0f0f0f, 0x0e0e0e00, 
			0x00000000, 0x00000000, 0x000f0f00, 0x00000000, 0x0f0f0000, 
			0x0f0f0000, 0x0000000f, 0x00000000, 0x00000000, 0x00000000, 
			0x0f000f0f, 0x0000000f, 0x0f0f0000, 0x0000000f, 0x00000000, 
			0x00000000, 0x00000f0f, 0x0f0f0f00, 0x00000f0f, 0x0e0e0e00, 
			0x00000000, 0x00000000, 0x00000f0f, 0x00000000, 0x0f000000, 
			0x0f00000f, 0x0f0f0f0f, 0x0f0f0f0f, 0x0000000f, 0x00000000, 
			0x0f000f0f, 0x0000000f, 0x0f000000, 0x0f0f0f0f, 0x0f0f0f0f, 
			0x0000000f, 0x0f0f0f0f, 0x0f0f0f0f, 0x00000000, 0x0e0e0e00, 
			0x00000000, 0x00000000, 0x00000f0f, 0x00000000, 0x0f000000, 
			0x0000000f, 0x0f0f0f0f, 0x0f0f0f0f, 0x00000f0f, 0x0f000000, 
			0x0000000f, 0x00000f0f, 0x00000000, 0x0f0f0f0f, 0x0f0f0f0f, 
			0x00000f0f, 0x0f0f0f0f, 0x000f0f0f, 0x00000000, 0x0e0e0e00, 
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0e0e0e00, 
			0x0f0f0f0f, 0x00000f0f, 0x00000f0f, 0x00000000, 0x0f000000, 
			0x0000000f, 0x00000000, 0x00000000, 0x000f0f0f, 0x0f0f0000, 
			0x0f0f0f0f, 0x000f0f0f, 0x00000000, 0x00000000, 0x00000000, 
			0x000f0f0f, 0x00000f0f, 0x0f0f0000, 0x0000000f, 0x0e0e0e00, 
			0x0f0f000f, 0x00000f00, 0x00000f0f, 0x00000000, 0x0f000000, 
			0x0000000f, 0x00000000, 0x00000000, 0x000f0f00, 0x0f0f0000, 
			0x0f0f0f0f, 0x000f0f0f, 0x00000000, 0x00000000, 0x00000000, 
			0x000f0f00, 0x00000f0f, 0x0f000000, 0x00000f0f, 0x0e0e0e00, 
			0x0f0f0000, 0x00000000, 0x000f0f00, 0x00000000, 0x0f0f0000, 
			0x00000000, 0x00000000, 0x00000000, 0x000f0f00, 0x000f0f00, 
			0x00000000, 0x0f0f0000, 0x00000000, 0x00000000, 0x00000000, 
			0x000f0f00, 0x00000f0f, 0x00000000, 0x000f0f0f, 0x0e0e0e00, 
			0x0f0f0000, 0x00000000, 0x0f0f0f00, 0x00000000, 0x0f0f0f00, 
			0x00000000, 0x00000000, 0x00000000, 0x000f0f0f, 0x000f0f00, 
			0x00000000, 0x0f0f0000, 0x00000000, 0x00000000, 0x00000000, 
			0x000f0f0f, 0x00000f0f, 0x00000000, 0x0f0f0f00, 0x0e0e0e00, 
			0x0f0f0000, 0x00000000, 0x0f0f0000, 0x0f0f0f0f, 0x000f0f0f, 
			0x0f0f0000, 0x0f0f0f0f, 0x0f0f0f0f, 0x00000f0f, 0x00000f0f, 
			0x00000000, 0x0f000000, 0x0f0f000f, 0x0f0f0f0f, 0x0f0f0f0f, 
			0x00000f0f, 0x00000f0f, 0x00000000, 0x0f0f0000, 0x0e0e0e0f, 
			0x0f0f0f00, 0x0000000f, 0x00000000, 0x0f0f0f0f, 0x0000000f, 
			0x0f0f0000, 0x0f0f0f0f, 0x0f0f0f0f, 0x0000000f, 0x00000f0f, 
			0x00000000, 0x0f000000, 0x0f0f000f, 0x0f0f0f0f, 0x0f0f0f0f, 
			0x0000000f, 0x00000f0f, 0x00000000, 0x0f000000, 0x0e0e0e0f 
		};
		static int exitdoor[] = {
			0x00000000, 0x000f0f0f, 0x00000000, 
			0x00000000, 0x000f0f0f, 0x00000000, 
			0x00000000, 0x000f0f0f, 0x00000000, 
			0x00000000, 0x000f0f0f, 0x00000000, 
			0x00000000, 0x00000000, 0x00000000, 
			0x00000000, 0x000f0f0f, 0x00000000, 
			0x0f000000, 0x0f0f0f0f, 0x00000000, 
			0x0f000000, 0x0f0f0f0f, 0x00000000, 
			0x0f0f0000, 0x0f0f0f0f, 0x0000000f, 
			0x0f0f0000, 0x0f0f0f0f, 0x0000000f, 
			0x0f0f0f00, 0x0f0f0f0f, 0x00000f0f, 
			0x0f0f0f00, 0x0f0f0f0f, 0x00000f0f, 
			0x0f0f0f0f, 0x0f0f0f0f, 0x000f0f0f, 
			0x0f0f0f0f, 0x0f0f0f0f, 0x000f0f0f, 
		};
		static int askcube[] = {
			0x07070707, 0x09000707, 0x07070700, 0x07070707,
			0x07070707, 0x09090900, 0x07000909, 0x07070707,
			0x09000707, 0x01090909, 0x09090900, 0x07070700,
			0x09090007, 0x09090101, 0x09090009, 0x07070009,

			0x01000300, 0x01000000, 0x01090909, 0x07000e00,
			0x0b0b0300, 0x09010000, 0x00000909, 0x07000e00,
			0x0b0b0b00, 0x01000b0b, 0x00000e00, 0x07000e0e,
			0x000b0b00, 0x000b0b00, 0x00000e0e, 0x0700000e,
	
			0x0b0b0000, 0x000b0003, 0x0e000e0e, 0x07000000,
			0x03000b00, 0x00000b0b, 0x0e0e0e0e, 0x07000e0e,
			0x000b0b00, 0x000b0b00, 0x00000e0e, 0x07000e0e,
			0x0b0b0007, 0x000b0b0b, 0x00000e0e, 0x0707000e,

			0x03000707, 0x000b0b0b, 0x00000e0e, 0x07070700,
			0x07070707, 0x00030300, 0x07000e0e, 0x07070707,
			0x07070707, 0x00000707, 0x07070700, 0x07070707
		};
		lib_putblock1((void *) -1,      8,      4, 77, 13, 3, &tosask);
		lib_putblock1((void *) -1, x - 24,      3, 11, 14, 1, &exitdoor);
		lib_putblock1((void *) -1,      8, y - 21, 15, 15, 1, &askcube);
	#endif

	sgg_wm0_putmouse(mx = mxx, my);
	#if (defined(TIMEX))
		write_time();
	#endif
	return;
}

struct WM0_WINDOW *handle2window(const int handle)
{
	// top중에서 찾아도 괜찮다
	int i;
	struct WM0_WINDOW *win = window;
	for (i = 0; i < MAX_WINDOWS; i++, win++) {
		if (win->sgg.handle == handle)
			return win;
	}
	return NULL;
}

void chain_unuse(struct WM0_WINDOW *win)
{
	// unuse는 맨 위
	// win는 맨 밑에 추가
	if (win->sgg.handle) {
		/* (int) [stack_sel:handle] = 0; */
		/* init.ask에, 해당의 핸들이 프리인 것을 가르치기 위해 */
		static int zero = 0;
		sgg_directwrite(0, 4, 0, win->sgg.handle, 0x01280030 /* stack_sel */, &zero, 0x000c);
		win->sgg.handle = 0; // handle2window가 잘못 검출하지 않기 위해
	}
	if (unuse) {
		struct WM0_WINDOW *bottom;
		bottom = unuse->up;
		win->down = unuse;
		win->up = bottom;
		unuse->up = win;
		bottom->down = win;
	} else {
		unuse = win;
		win->up = win;
		win->down = win;
	}
	return;
}

struct WM0_WINDOW *get_unuse()
{
	// 맨 위로부터 
	struct WM0_WINDOW *win = unuse;
	struct WM0_WINDOW *bottom = unuse->up;
	unuse = unuse->down;
	unuse->up = bottom;
	bottom->down = unuse;
	if (win == unuse)
		unuse = NULL;
	return win;
}

static struct STR_PRESS {
	enum {
		NO_PRESS = 0, CLOSE_BUTTON, TITLE_BAR
	} pos;
	struct WM0_WINDOW *win;
	int mx0, my0;
} press0 = { NO_PRESS, NULL }; 

void mousesignal(const unsigned int header, int dx, int dy)
{
	struct STR_JOB *pjob = &job;
	struct STR_PRESS *press = &press0;
	int _mx = mx, _my = my, flagmask = 0, nbutton;
	struct WM0_WINDOW *win;
	struct MOSWINSIG *mws;
	char call_flagset = 0;

	// 마우스의 버튼 상태가 변화
	// bit0:left
	// bit1:right
	// bit2:middle

/*
	dx *= mousescale;
	dy *= mousescale;
*/
	if ((header >> 28) == 0x0 /* normal mode */) {
		if ((unsigned)(dx+mouseaccel)>mouseaccel*2){
			int accel = (dx>0)? mouseaccel:-mouseaccel;
			dx = (dx - accel) * mousescale + accel;
		}
		if ((unsigned)(dy+mouseaccel)>mouseaccel*2){
			int accel = (dy>0) ?  mouseaccel : -mouseaccel;
			dy = (dy - accel) * mousescale + accel;
		}
		// 마우스 상태 변경
		int ox = _mx, oy = _my;
		_mx += dx;
		_my += dy;
		if (_mx < 0)
			_mx = 0;
		if (_mx >= screen.x2)
			_mx = screen.x2 - 1;
		if (_my < 0)
			_my = 0;
		if (_my >= screen.y2 - 15)
			_my = screen.y2 - 16;
		if (_mx ! = ox || _my ! = oy) {
			mx = _mx;
			my = _my;
			sgg_wm0_movemouse(_mx, _my);
			if (pjob->movewin4_ready) {
				if (press->pos == NO_PRESS || (press->pos == TITLE_BAR && press->win == pjob->win))
					job_movewin4m(_mx, _my);
			}
		}

		if (mbutton == (nbutton = header & 0x07)) { /* 마우스의 버튼 상태는 불변 */
			if (nbutton | mbutton) { /* 드래그 중 */
				flagmask |= 0x02;
				goto send0;
			}
			/* 통상의 마우스 이동 중 */
			if (mws_sensitivecount) {
	send0:
				win = searchwin(_mx, _my);
	send1:
				if (mws_mousewin ! = win) {
					if (mws_mousewin) {
						/* sensitive의 아웃을 송신 */
						for (mws = moswinsig; mws->win ! = (struct WM0_WINDOW *) -1; mws++) {
							if ((mws->flags & 0x0000ff00) == 0x00001100 && mws->win == mws_mousewin) {
								/* ! 부실!  len == 2만을 상정 */
								//	if ((mws->flags & 0x0f) == 2) {
									send_signal2dw(mws->win->sgg.image[WINSTR_SIGNALEBOX], mws->sig[0], mws->sig[1]);
							//	}
							}
						}
					}
					mws_mousewin = win;
					call_flagset = 1;
				}
				flagmask |= 0x04;
				if (call_flagset)
					moswinsig_flagset();
				/* 플래그에 근거해 송신 */
				flagmask <<= 16;
				call_flagset = nbutton | mbutton << 4;
				for (mws = moswinsig; mws->win ! = (struct WM0_WINDOW *) -1; mws++) {
					if (mws->flags & flagmask) {
						/* ! 부실!  len == 2만을 상정 */
						switch ((mws->flags >> 8) & 0xff) {
						case 0x10: /* 마우스 x-y */
							if (win ! = mws->win)
								break;
						case 0x12: /* 마우스 x-y(out 없음) */
							if (pjob->movewin4_ready ! = 0 && pjob->win == mws->win) {
								if (press->pos == NO_PRESS || (press->pos == TITLE_BAR && press->win == pjob->win))
									break;
							}
							send_signal4dw(mws->win->sgg.image[WINSTR_SIGNALEBOX], mws->sig[0], mws->sig[1],
								_mx - mws->win->x0 - mws->sig[4], _my - mws->win->y0 - mws->sig[5]);
							break;
						case 0x20:
							if ((call_flagset & 0x11) == 0x01)	/* ((nbutton & 0x01) ! = 0 && (mbutton & 0x01) == 0) */
								goto sendsig2dw;
							break;
						case 0x21:
							if ((call_flagset & 0x22) == 0x02)	/* ((nbutton & 0x02) ! = 0 && (mbutton & 0x02) == 0) */
								goto sendsig2dw;
							break;
						case 0x22:
							if ((call_flagset & 0x44) == 0x04)	/* ((nbutton & 0x04) ! = 0 && (mbutton & 0x04) == 0) */
								goto sendsig2dw;
							break;
						case 0x30:
							if ((call_flagset & 0x11) == 0x10)	/* ((nbutton & 0x01) == 0 && (mbutton & 0x01) ! = 0) */
								goto sendsig2dw;
							break;
						case 0x31:
							if ((call_flagset & 0x22) == 0x20)	/* ((nbutton & 0x02) == 0 && (mbutton & 0x02) ! = 0) */
								goto sendsig2dw;
							break;
						case 0x32:
							if ((call_flagset & 0x44) == 0x40) {/* ((nbutton & 0x04) == 0 && (mbutton & 0x04) ! = 0) */
				sendsig2dw:
								send_signal2dw(mws->win->sgg.image[WINSTR_SIGNALEBOX], mws->sig[0], mws->sig[1]);
							}
						//	break;
						}
					}
				}
				if (nbutton == 0) {
					press->win = NULL;
					if (mws_sensitivecount == 0)
						mws_mousewin = NULL;
					moswinsig_flagset();
				}
			}
		} else { /* 마우스의 버튼 상태가 변화 */
			if (pjob->movewin4_ready ! = 0 && press->pos == NO_PRESS && nbutton == 0x01) {
				job_movewin4(0x00f0 /* Enter */);
				goto skip;
			}
			if ((mbutton & 0x01) == 0x01 && (nbutton & 0x01) == 0x00 && press->pos ! = NO_PRESS) {
				mousesignal_sub0(_mx, _my); /* 왼쪽 버튼을 놓았다 */
				press->pos = NO_PRESS;
			}
			flagmask |= 0x03;
			if (mbutton ! = 0)
				goto send0;
			/* 눌러진 상태의 첫 시작 */
			win = searchwin(_mx, _my);
			if (top ! = win) {
				if (mbutton == 0 && win ! = NULL) {
					writejob_n(2, 0x0024 /* active */, (int) win);
					goto skip;
				}
			}
			press->win = win;
			call_flagset = 1;
			if ((nbutton & 0x01) ! = 0 && press->win ! = NULL)
				mousesignal_sub1(_mx, _my); /* 왼쪽 버튼이 밀렸다 */
			goto send1;
		}
skip:
		mbutton = nbutton;
	} else if ((header >> 28) == 0xa /* extmode byte2 */) {
		/* 마우스 리셋트 */
		mbutton = 0;
		sgg_wm0_enablemouse();
		#if (defined(DEBUG))
			lib_drawletters_ASCII(1, -1, 0xc0,  0, 0, 15, 0, "mouse reset");
		#endif
	} else {
		/* mikannsei */
	}
	/* 모인 작업이 있으면, 실행한다 */
	if (pjob->now == 0)
		runjobnext();
	return;
}

void mousesignal_sub1(int _mx, int _my)
/* 왼쪽 버튼이 밀렸을 때의 이벤트 */
{
	struct STR_JOB *pjob = &job;
	struct STR_PRESS *press = &press0;
	struct WM0_WINDOW *win = press->win;

	#if (defined(WIN9X))
		if (win->x1 - 21 <= _mx && _mx < win->x1 - 5 && win->y0 + 5 <= _my && _my < win->y0 + 19) {
			// close button를 눌렀다
			press->pos = CLOSE_BUTTON;
		} else if (win->x0 + 3 <= _mx && _mx < win->x1 - 4 && win->y0 + 3 <= _my && _my < win->y0 + 21) {
			/* title-bar를 눌렀다 */
			if (pjob->free >= 2) {
				/* 빈 곳이 충분히 있다 */
				press->pos = TITLE_BAR;
				press->mx0 = _mx;
				press->my0 = _my;
				writejob_n(2, 0x0028 /* move */, (int) win);
			}
		}
	#elif (defined(TMENU) || defined(WIN31))
		if (win->x0 + 2 <= _mx && _mx < win->x0 + 20 && win->y0 + 3 <= _my && _my < win->y0 + 21) {
			// close button를 눌렀다
			press->pos = CLOSE_BUTTON;
		} else if (win->x0 + 1 <= _mx && _mx < win->x1 - 1 && win->y0 + 1 <= _my && _my < win->y0 + 21) {
			/* title-bar를 눌렀다 */
			if (pjob->free >= 2) {
				/* 빈 곳이 충분히 있다 */
				press->pos = TITLE_BAR;
				press->mx0 = _mx;
				press->my0 = _my;
				writejob_n(2, 0x0028 /* move */, (int) win);
			}
		}
	#elif (defined(CHO_OSASK))
		if (win->x0 + 4 <= _mx && _mx < win->x0 + 20 && win->y0 + 4 <= _my && _my < win->y0 + 20) {
			// close button를 눌렀다
			press->pos = CLOSE_BUTTON;
		} else if (win->x0 + 1 <= _mx && _mx < win->x1 - 1 && win->y0 + 1 <= _my && _my < win->y0 + 21) {
			/* title-bar를 눌렀다 */
			if (pjob->free >= 2) {
				/* 빈 곳이 충분히 있다 */
				press->pos = TITLE_BAR;
				press->mx0 = _mx;
				press->my0 = _my;
				writejob_n(2, 0x0028 /* move */, (int) win);
			}
		}
	#elif (defined(NEWSTYLE)) /* 클로우즈 버튼은 없습니다 */
		if (win->x0 + 1 <= _mx && _mx < win->x1 - 1 && win->y0 + 1 <= _my && _my < win->y0 + 21) {
			/* title-bar를 눌렀다 */
			if (pjob->free >= 2) {
				/* 빈 곳이 충분히 있다 */
				press->pos = TITLE_BAR;
				press->mx0 = _mx;
				press->my0 = _my;
				writejob_n(2, 0x0028 /* move */, (int) win);
			}
		}
	#endif
	return;
}

void mousesignal_sub0(int _mx, int _my)
/* 왼쪽 버튼이 놓았을 때의 이벤트 */
{
	struct STR_JOB *pjob = &job;
	struct STR_PRESS *press = &press0;
	struct WM0_WINDOW *win = press->win;

	switch (press->pos) {
	case CLOSE_BUTTON:
		#if (defined(WIN9X))
			if (press->win->x1 - 21 <= _mx && _mx < press->win->x1 - 5 &&
				press->win->y0 + 5 <= _my && _my < press->win->y0 + 19) {
				if (press->win ! = pokon0)
					sgg_wm0s_close(&press->win->sgg);
			}
		#elif (defined(TMENU) || defined(WIN31))
			if (press->win->x0 + 2 <= _mx && _mx < press->win->x0 + 20 &&
				press->win->y0 + 3 <= _my && _my < press->win->y0 + 21) {
				if (press->win ! = pokon0)
					sgg_wm0s_close(&press->win->sgg);
			}
		#elif (defined(CHO_OSASK))
			if (press->win->x0 + 4 <= _mx && _mx < press->win->x0 + 20 &&
				press->win->y0 + 4 <= _my && _my < press->win->y0 + 20) {
				if (press->win ! = pokon0)
					sgg_wm0s_close(&press->win->sgg);
			}
		#endif
		break;

	case TITLE_BAR:
		if (press->win == pjob->win && pjob->movewin4_ready ! = 0) {
			/* 이동처 확정 시그널을 보낸다 */
			job_movewin4(0x00f0 /* Enter */);
		}
	//	break;
	}
	return;
}

int writejob_n(int n, int p0,...)
{
	struct STR_JOB *pjob = &job;
	if (pjob->free >= n) {
		int *p = &p0;
		do {
			*pjob->wp++ = *p++;
			pjob->free--;
			if (pjob->wp == pjob->list + JOBLIST_SIZE)
				pjob->wp = pjob->list;
		} while (--n);
		*pjob->wp = 0;
		return 1;
	}
	return 0;
}

const int readjob()
{
	struct STR_JOB *pjob = &job;
	int i = *pjob->rp++;
	pjob->free++;
	if (pjob->rp == pjob->list + JOBLIST_SIZE)
		pjob->rp = pjob->list;
	return i;
}

void runjobnext()
{
	struct STR_JOB *pjob = &job;
	int i, j;
	void (*func1)(int);

	do {
		if ((pjob->now = *pjob->rp) == 0)
			return;

		readjob(); // 읽기
		func1 = NULL;
		switch (pjob->now) {
		case 0x0020 /* open new window */:
		//	job_openwin0((struct WM0_WINDOW *) readjob());
			func1 = (void (*)(int)) job_openwin0;
			break;

		case 0x0024 /* active window */:
		//	job_activewin0((struct WM0_WINDOW *) readjob());
			func1 = (void (*)(int)) job_activewin0;
			break;

		case 0x0028 /* move window by keyboard */:
		//	job_movewin0((struct WM0_WINDOW *) readjob());
			func1 = (void (*)(int)) job_movewin0;
			break;

		case 0x002c /* close window */:
		//	job_closewin0((struct WM0_WINDOW *) readjob());
			func1 = (void (*)(int)) job_closewin0;
			break;

		case 0x0030 /* open VGA driver */:
		//	job_openvgadriver(readjob());
			func1 = job_openvgadriver;
			break;

		case 0x0034 /* change VGA mode */:
		//	job_setvgamode0(readjob());
			func1 = job_setvgamode0;
			break;

		case 0x0038 /* load external font */:
			i = readjob();
			j = readjob();
			job_loadfont0(i, j, readjob());
			break;

		#if (defined(PCAT) || defined(TOWNS))
			case 0x003c /* capture */:
		  	  job_savevram0();
		  	  break;
		#endif

		case 0x0040 /* load wallpaper */:
			job_openwallpaper();
			break;

		#if (defined(PCAT))
			case 0x0044 /* VESA check */:
				job_vesacheck0();
				break;
		#endif

		case 0x0048 /* set wallpaper */:
			screen.wallpaper_exist = 0;
			job_openwallpaper();
			break;
		}
		if (func1)
			(*func1)(readjob());
	} while (pjob->now == 0);
	return;
}

const int overrapwin(const struct WM0_WINDOW *win0, const struct WM0_WINDOW *win1)
{
	return win0->x0 < win1->x1 && win1->x0 < win0->x1 && win0->y0 < win1->y1 && win1->y0 < win0->y1;
}

void job_openwin0(struct WM0_WINDOW *win)
{
	int xsize = sgg_wm0_winsizex(&win->sgg);
	int ysize = sgg_wm0_winsizey(&win->sgg);
	int i;

	// 우선, 좌표를 결정한다
	if (top == NULL) {
		// for pokon0
		win->x0 = 16;
		win->y0 = 32;
		pokon0 = win;
	} else {
		win->x0 = windef[0]. x;
		win->y0 = windef[0]. y;
		if (win->x0 + xsize > screen.x2 || win->y0 < RESERVELINE0 || win->y0 + ysize + RESERVELINE1 > screen.y2) {
			win->x0 = (screen.x2 - xsize) / 2;
			win->y0 = RESERVELINE0 + (screen.y2 - (RESERVELINE0 + RESERVELINE1) - ysize) / 2;
		}
		win->x0 &= ~(screen.moveunit - 1); // 오픈 위치를 8의 배수가 되도록 조정
		for (i = 0; i < MAXWINDEF - 1; i++) {
			windef[i]. x = windef[i + 1]. x;
			windef[i]. y = windef[i + 1]. y;
		}
		windef[MAXWINDEF - 1]. x = 0x10000000;
		if (win->x0 < 0 || win->x0 > screen.x2 || win->y0 < RESERVELINE0 || win->y0 + ysize + RESERVELINE1 > screen.y2) {
			win->up = win->down = win; /* 고립 */
			sgg_wm0s_close(&win->sgg);
			win->job_flag0 = WINFLG_NOWINDOW;
			win->condition = 0;
			job.now = 0;
			#if (defined(TIMEX))
				winmanerr("window too large. ");
			#endif
			return;
		}
	}

	// 각종 파라미터(parameter)의 초기화
	win->x1 = win->x0 + xsize;
	win->y1 = win->y0 + ysize;
	win->job_flag0 = WINFLG_MUSTREDRAW | WINFLG_OVERRIDEDISABLED; // override-disable & must-redraw

	// 입력 액티브를 변경
	redirect_input(win); // 이 함수는 윈도우 컨트롤은 하지 않는다
//	iactive = win;

	// 접속
	if (top == NULL)
		win->up = win->down = top = win;
	else {
		struct WM0_WINDOW *bottom;
		win->up = bottom = top->up;
		win->down = top;
		top->up = win;
		bottom->down = win;
		top = win;
	}

	sgg_wm0s_movewindow(&win->sgg, win->x0, win->y0);
	sgg_wm0s_setstatus(&win->sgg, win->condition = 0x03 /* bit0:표시, bit1:입력 */);
	job_general1();
	return;
}

void redirect_input(struct WM0_WINDOW *win)
{
	// 키 입력 시그널의 대응 초기화
//	sgg_wm0_definesignal0(255, 0x0100, 0);
//	sgg_wm0_definesignal3com();
	sgg_execcmd(&defbindcommand);

	// winman0의 키 조작을 등록(F9~F12)
	sgg_wm0_definesignal3(3, 0x0100, 0x00701089 /* F9~F12 */,
		0x3240 /* winman0 signalbox */, 0x7f000001, 0x0200);
	// for WIN9X & WIN31 by koyanagi
	#if (defined(WIN9X) || defined(WIN31))
	sgg_wm0_definesignal3(0, 0x0100, 0x407010a2 /* ALT+TAB */,
	      0x3240 /* winman0 signalbox */, 0x7f000001, 0x0200);/* switch */
	sgg_wm0_definesignal3(0, 0x0100, 0x40701084 /* ALT+F4 */,
	      0x3240 /* winman0 signalbox */, 0x7f000001, 0x0203);/* close */
	#endif

	sgg_wm0_definesignal3(0, 0x0100, 0x00701085 /* F5 */,
		0x3240 /* winman0 signalbox */, 0x7f000001, 0x0240); /* JPN16$.FNT의 즉시 로드 */
	sgg_wm0_definesignal3(0, 0x0100, 0x00701087 /* F7 */,
		0x3240 /* winman0 signalbox */, 0x7f000001, 0x0248); /* load wallpaper */

	#if (defined(PCAT) | defined(TOWNS))
		sgg_wm0_definesignal3(3, 0x0100, 0x00701081 /* F1 */,
			0x3240 /* winman0 signalbox */, 0x7f000001, 0x0204);
		sgg_wm0_definesignal3(0, 0x0100, 0x10701087 /* Shift+F7 */,
			0x3240 /* winman0 signalbox */, 0x7f000001, 0x0244); /* CAPTURE */
	#endif
	#if (defined(TOWNS) & defined(CLGD543X))
		sgg_wm0_definesignal3(1, 0x0100, 0x0070108d /* PF13 */,
			0x3240 /* winman0 signalbox */, 0x7f000001, 0x0220);
	#endif
	#if (defined(NEC98))
		sgg_wm0_definesignal3(1, 0x0100, 0x20701089 /* Ctrl+F9~F10 */,
			0x3240 /* winman0 signalbox */, 0x7f000001, 0x0202);
		sgg_wm0_definesignal3(0, 0x0100, 0x20701081 /* Ctrl+F1 */,
			0x3240 /* winman0 signalbox */, 0x7f000001, 0x0999);
	#endif

	if (win) {
		struct DEFINESIGNAL *dsp;
		int sigbox = sgg_wm0_win2sbox(&win->sgg);
		for (dsp = defsigbuf; dsp->win ! = -1; dsp++) {
			if (dsp->win == (int) win) {
				if (dsp->len == 2) {
					sgg_wm0_definesignal3(dsp->opt, dsp->dev,
						dsp->cod, sigbox, dsp->sig[0], dsp->sig[1]);
				}
			}
		}
	}

	iactive = win;
	moswinsig_flagset();

	return;
}

void job_activewin0(struct WM0_WINDOW *win)
{
	int x0, y0, x1, y1;
	struct WM0_WINDOW *win_up, *win_down;

	if (top == win) {
		/* top와 iactive는 항상 동일하다 */
		job.now = 0;
		return;
	}

	/* win를 리스트로부터 한 번 떼어낸다 */
	win_up = win->up;
	win_down = win->down;
	win_up->down = win_down;
	win_down->up = win_up;

	/* 재접속 */
	win->up = win_down = top->up;
	win->down = top;
	top->up = win;
	win_down->down = win;
	top = win;

	/* 입력 액티브를 변경 */
	redirect_input(win); /* 이 함수는 윈도우를 컨트롤하지 않는다 */
//	iactive = win;
//	win = top;
//	do {	/* deleted by I.Tak.  (Jenny1. 2) */
//		win->job_flag0 = 0;
//	} while ((win = win->down) ! = top);
	job_general1();
	return;
}

void job_movewin0(struct WM0_WINDOW *win)
{
	// win는 반드시 top로, 한편 iactive인 것
	struct STR_JOB *pjob = &job;

	pjob->win = top /* win */;

	/* 어차피 다시 그리니까 0으로 해 버린다.
	   반드시 0으로 해 두지 않으면 윈도우를 지우고 있는 한중간에
	   그리거나 하여 귀찮다. 지우기 직전에 그리기 금지
	   하는 방법도 있지만. */
	win->condition = 0;
	pjob->count = 1;
	pjob->func = job_movewin1;
	sgg_wm0s_accessdisable(&win->sgg);
	do {
		win->job_flag0 = 0;
	} while ((win = win->down) ! = top);

	// 키 입력 시그널의 대응 초기화
//	sgg_wm0_definesignal0(255, 0x0100, 0);
//	sgg_wm0_definesignal3com();
	sgg_execcmd(&defbindcommand);

#if 0	/* deleted by I.Tak.  (Jenny1. 2) */
	// 우선, 모든 윈도우의 화면 갱신권을 일시적으로 박탈한다
	pjob->count = 0;
	win = top;
	pjob->func = job_movewin1;
	do {
		win->job_flag0 = 0x01;
		if (win->condition & 0x01) {
			pjob->count++; // disable
			sgg_wm0s_accessdisable(&win->sgg);
		}
		win = win->down;
	} while (win ! = top);

//	이하는 성립하지 않는다(최저1개는 출력 액티브한 윈도우가 존재하기 때문에)
//	if (pjob->count == 0) {
//		job_movewin2(); // 곧바로 윈도우범위 표시에
//	}
#else	/* added by I.Tak.  (Jenny1. 2) */
	pjob->movewin_x0 = pjob->movewin_x = win->x0;
	pjob->movewin_y0 = pjob->movewin_y = win->y0;
#endif

	return;
}

void job_movewin1(const int cmd, const int handle)
{
	if (cmd ! = 0x00c0) {
		#if (defined(DEBUG))
			unsigned char s[12];
			s[8] = '\0';
			debug_bin2hex(cmd, s);    lib_drawletters_ASCII(1, -1, 0xc0,  0, 0, 13, 0, s);
			debug_bin2hex(handle, s); lib_drawletters_ASCII(1, -1, 0xc0, 80, 0, 13, 0, s);
		#endif
		return;
	}
	// 0x00c0밖에 오지 않는다
	if (--job.count == 0)
		job_movewin2();
	return;
}

void job_movewin2()
{
	struct STR_JOB *pjob = &job;
	struct STR_PRESS *press = &press0;
	struct WM0_WINDOW *win = pjob->win;
	// 시그널을 선언(0 x00d0~0x00d3, 0x00f0)
	sgg_wm0_definesignal3(3, 0x0100, 0x00ac /* left */,
		0x3240 /* winman0 signalbox */, 0x7f000001, 0x00d0);
	sgg_wm0_definesignal3(0, 0x0100, 0x00a0 /* Enter */,
		0x3240 /* winman0 signalbox */, 0x7f000001, 0x00f0);

	// 테두리를 그린다
//	pjob->movewin_x0 = pjob->movewin_x = win->x0;	/* deleted by I.Tak.  (Jenny1. 2) */
//	pjob->movewin_y0 = pjob->movewin_y = win->y0;
	job_movewin3();
	pjob->movewin4_ready = 1;
	if (press->pos == NO_PRESS) {
		if (pjob->movewin_x > mx || mx >= win->x1 || pjob->movewin_y > my || my >= win->y1) {
			mx = (win->x0 + win->x1) / 2;
			my = win->y0 + 12;
			sgg_wm0_movemouse(mx, my);
		}
		press->mx0 = mx;
		press->my0 = my;
	}
	return;
}

void job_movewin3()
{
	struct STR_JOB *pjob = &job;
	int x0, y0, x1, y1;
	struct WM0_WINDOW *win = pjob->win;

	x0 = pjob->movewin_x;
	y0 = pjob->movewin_y;
	x1 = x0 + win->x1 - win->x0 - 1;
	y1 = y0 + win->y1 - win->y0 - 1;

	lib_drawline(0x00e0, (void *) -1, 9, x0,     y0,     x1 - 3, y0 + 2);
	lib_drawline(0x00e0, (void *) -1, 9, x0 + 3, y1 - 2, x1,     y1    );
	lib_drawline(0x00e0, (void *) -1, 9, x0,     y0 + 3, x0 + 2, y1    );
	lib_drawline(0x00e0, (void *) -1, 9, x1 - 2, y0,     x1,     y1 - 3);
	return;
}

void job_movewin4(int sig)
{
	struct STR_JOB *pjob = &job;
	struct WM0_WINDOW *win0 = pjob->win;
	struct STR_PRESS *press = &press0;

	int x0 = pjob->movewin_x;
	int y0 = pjob->movewin_y;
	int xsize = win0->x1 - win0->x0;
	int ysize = win0->y1 - win0->y0;

	if (sig == 0x00d0 && x0 >= screen.moveunit)
		x0 -= screen.moveunit;
	if (sig == 0x00d1 && x0 + xsize <= screen.x2 - 4)
		x0 += screen.moveunit;
	if (sig == 0x00d2 && y0 >= screen.moveunit + RESERVELINE0)
		y0 -= screen.moveunit;
	if (sig == 0x00d3 && y0 + ysize <= screen.y2 - (RESERVELINE1 + screen.moveunit))
		y0 += screen.moveunit;

	if ((x0 - pjob->movewin_x) | (y0 - pjob->movewin_y)) {
		if (press->pos == NO_PRESS || press->pos == TITLE_BAR) {
			mx += x0 - pjob->movewin_x;
			my += y0 - pjob->movewin_y;
			sgg_wm0_movemouse(mx, my);
		}
		#if 1	/* added by I.Tak.  (Jenny1. 2) */
		if (pjob->count == 0){
			job_movewin3();
			pjob->movewin_x = x0;
			pjob->movewin_y = y0;
			if (job_movewin5())
				return;
			job_movewin3();
		}
		#else
		job_movewin3();
		pjob->movewin_x = x0;
		pjob->movewin_y = y0;
		job_movewin3();
		#endif
	}

	if (sig == 0x00f0) {
		struct WM0_WINDOW *win1;
		pjob->movewin4_ready = 0;
		press->mx0 = -1;
	//	job_movewin3();	/* deleted by I.Tak.  (Jenny1. 2) */
		win0->job_flag0 |= WINFLG_MUSTREDRAW;	/* changed by I.Tak.  (Jenny1. 2) */
	//	win0->job_flag0 = WINFLG_MUSTREDRAW | WINFLG_OVERRIDEDISABLED; // override-disabled & must-redraw
		win1 = win0 /* top */->down;
		do {
			/* deleted by I.Tak.  (Jenny1. 2) */
		//	int flag0 = WINFLG_OVERRIDEDISABLED; // override-disabled
		//	if (overrapwin(win0, win1))
		//		flag0 = WINFLG_MUSTREDRAW | WINFLG_OVERRIDEDISABLED; // override-disabled & must-redraw
		//	win1->job_flag0 = flag0;
			/* added by I.Tak.  (Jenny1. 2) */
			/* 최초부터 그리기 금지로 작업 종료후도 그리기 금지
			   의 윈도우는 job_general에서는 다시 그리게 되지 않는다 */
			if ((win1->condition & 0x01) == 0 && overrapwin(win0, win1))
				win1->job_flag0 |= WINFLG_MUSTREDRAW;
		} while ((win1 = win1->down) ! = win0);

		// 윈도우를 지운다
		putwallpaper(win0->x0, win0->y0, win0->x1, win0->y1);

		win0->x0 = x0;
		win0->y0 = y0;
		win0->x1 = x0 + xsize;
		win0->y1 = y0 + ysize;
		sgg_wm0s_movewindow(&win0->sgg, win0->x0, win0->y0);
		redirect_input(win0);
		job_general1();
	}
	return;
}

void job_movewin4m(int x, int y)
{
	struct STR_JOB *pjob = &job;
	struct WM0_WINDOW *win0 = pjob->win;

	int x0 = pjob->movewin_x0 + (x - press0.mx0) & ~(screen.moveunit - 1);
	int y0 = pjob->movewin_y0 + y - press0.my0;
	int xsize = win0->x1 - win0->x0;
	int ysize = win0->y1 - win0->y0;

	if (x0 < 0)
		x0 = 0;
	if (x0 > screen.x2 - xsize)
		x0 = (screen.x2 - xsize) & ~(screen.moveunit - 1);
	if (y0 < RESERVELINE0)
		y0 = RESERVELINE0;
	if (y0 > screen.y2 - ysize - RESERVELINE1)
		y0 = screen.y2 - ysize - RESERVELINE1;
	/* deleted by I.Tak.  (Jenny1. 2) */
//	if ((x0 - pjob->movewin_x) | (y0 - pjob->movewin_y)) {
//		job_movewin3();
//		pjob->movewin_x = x0;
//		pjob->movewin_y = y0;
//		job_movewin3();
//	}
	/* added by I.Tak.  (Jenny1. 2) */
	if (pjob->count == 0 && ((x0 - pjob->movewin_x) | (y0 - pjob->movewin_y)) ! = 0) {
		job_movewin3();
		pjob->movewin_x = x0;
		pjob->movewin_y = y0;
		if (job_movewin5() == 0)
			job_movewin3();
	}
	return;
}

/* 윈도우범위를 그리기 전에 해당되어 판정을 해, 그리기 금지나 차이분 그리기를 발행한다.
   시그널 발행수를 되돌려주므로 리턴값이 영이 아니라면 시그널을 기다릴 것.  */
int job_movewin5()
{
	struct STR_JOB *pjob = &job;
	struct WM0_WINDOW *win = pjob->win;
	int x0 = pjob->movewin_x;
	int y0 = pjob->movewin_y;
	int x1 = win->x1 - win->x0 + x0;
	int y1 = win->y1 - win->y0 + y0;
	int count = 0;

	win = top;
	do {
		if (win->condition & 0x01) {
			char flag = (win->job_flag0 & WINFLG_OVERRIDEDISABLED) == 0;
			if (win->x0 < x1 && win->x1 > x0 && win->y0 < y1 && win->y1 > y0) {
				if (flag) {
					count++;
					win->job_flag0 |= WINFLG_OVERRIDEDISABLED;
					sgg_wm0s_accessdisable(&win->sgg);
				}
			} else if (flag == 0) {
				count++;
				win->job_flag0 &= ~WINFLG_OVERRIDEDISABLED;
				sgg_wm0s_accessenable(&win->sgg);
				sgg_wm0s_redrawdif(&win->sgg);
			}
		}
		win = win->down;
	} while (win ! = top);

	if ((pjob->count = count) ! = 0)
		pjob->func = job_movewin6;
	return count;
}

/* 그리기 금지 등이 모두 수신되면 테두리를 그린다 */
void job_movewin6(const int cmd, const int handle)
{
	if ((cmd & 0x00c3) ! = 0x00c0)
		return;
	// 0x00c0나 0x00c4밖에 오지 않는다
	if (--job.count == 0)
		job_movewin3();
	return;
}

void job_closewin0(struct WM0_WINDOW *win0)
// 이 윈도우는 이미 accessdisable가 되어 있는 것이 전제
{
	struct STR_JOB *pjob = &job;
	struct WM0_WINDOW *win1, *win_up, *win_down;
	struct DEFINESIGNAL *dsp;
	struct MOSWINSIG *mws;

	/* win0를 리스트로부터 떼어낸다 */
	win_up = win0->up;
	win_down = win0->down;
	win_up->down = win_down;
	win_down->up = win_up;

	sgg_wm0s_windowclosed(&win0->sgg);

	for (dsp = defsigbuf; dsp->win ! = -1; dsp++) {
		if (dsp->win == (int) win0)
			dsp->win = NULL;
	}
	for (mws = moswinsig; mws->win ! = (struct WM0_WINDOW *) -1; mws++) {
		if (mws->win == win0) {
			mws->win = NULL;
			mws->flags = 0;
			if (mws->flags & 0x88880000)
				mws_sensitivecount--;
		}
	}

	if (win0->job_flag0 & WINFLG_NOWINDOW) {
		pjob->now = 0;
		return;
	}

//	pjob->count = 0;	/* deleted by I.Tak.  (Jenny1. 2) */
//	pjob->func = job_closewin1;

#if 0
	if (allclose) {
		lib_drawline(0x0020, (void *) -1, 6, win0->x0, win0->y0, win0->x1 - 1, win0->y1 - 1);
		chain_unuse(win0);
		allclose--;
		return;
	}
#endif

	if (win0 == top) {
		top = win_down;
		if (win_up == win0) {
			top = NULL;
			redirect_input(0);
		//	iactive = NULL;
			goto no_window;
		}
		redirect_input(top);
	//	iactive = top;
	}

	/* 최초부터 그리기 금지로, 작업 종료 후도 그리기 금지
	   의 윈도우는 job_general로 다시 그리지 않는다 (Jenny1. 2) */
	/* 상기 조건에 해당하는 윈도우는 있을 수 없는 생각이 들므로,
		I.Tak. 씨의 그 판정 부분을 죽여본다.  by K, Twitchell1 */
	win1 = top;
	do {
		/* deleted by I.Tak.  (Jenny1. 2) */
	//	int flag0 = 0;
	//	if (overrapwin(win0, win1)) {
	//		flag0 = WINFLG_MUSTREDRAW | WINFLG_OVERRIDEDISABLED;
	//		if (win1->condition & 0x01) {
	//			sgg_wm0s_accessdisable(&win1->sgg);
	//			pjob->count++;
	//			flag0 = WINFLG_MUSTREDRAW | WINFLG_OVERRIDEDISABLED | WINFLG_WAITDISABLE;
	//		}
	//	}
	//	win1->job_flag0 = flag0;
		/* added by I.Tak.  (Jenny1. 2) */
		if (/* (win1->condition & 1) == 0 && */ overrapwin(win0, win1)) /* by K, Twitchell1 */
			win1->job_flag0 |= WINFLG_MUSTREDRAW;
	} while ((win1 = win1->down) ! = top);

no_window:

	/* 윈도우를 지운다 */
	putwallpaper(win0->x0, win0->y0, win0->x1, win0->y1);
	chain_unuse(win0);
//	if (pjob->count == 0)	/* deleted by I.Tak.  (Jenny1. 2) */
//		job_general1();
	job_general1();	/* added by I.Tak.  (Jenny1. 2) */
	return;
}

#if 0	/* deleted by I.Tak.  (Jenny1. 2) */

void job_closewin1(const int cmd, const int handle)
{
	struct WM0_WINDOW *win = handle2window(handle);

	if (cmd ! = 0x00c0 || (win->job_flag0 & WINFLG_WAITDISABLE) == 0) {
		#if (defined(DEBUG))
			unsigned char s[12];
			s[8] = '\0';
			debug_bin2hex(cmd, s);    lib_drawletters_ASCII(1, -1, 0xc0,  0, 0, 14, 0, s);
			debug_bin2hex(handle, s); lib_drawletters_ASCII(1, -1, 0xc0, 80, 0, 14, 0, s);
		#endif
		return;
	}
	win->job_flag0 &= ~WINFLG_WAITDISABLE;
	if (--job.count == 0)
		job_general1();
	return;
}

#endif

// 윈도우 소거의 경우, 지우고 싶은 윈도우와 겹치고 있는 것을 모두 "must-redraw"로 한 뒤
// 윈도우를 지워, 나머지는 자동에 맡긴다

// 윈도우 오픈의 경우 추가하고, 나중에는 자동에 맡긴다

// 윈도우 이동의 경우, 이동원으로 겹치고 있는 것을 모두"must-redraw"로 한 뒤
// 원래 있던 장소를 지우고 나머지는 자동에 맡긴다


void job_general1()
/* condition.bit 0 ... 0:accessdisable 1:accessenable
   condition.bit 1 ... 0:inputdisable(not active) 1:inputenable(active)

   job_flag0.bit 0 ... new condition.bit 0(auto-set)
   job_flag0.bit 1 ... new condition.bit 1(auto-set)
   job_flag0.bit 8 ... disable-accept waiting
   job_flag0.bit 9 ... redraw finish waiting
   job_flag0.bit24 ... 0:normal 1:override-accessdisabled
   job_flag0.bit30 ... 0:normal 1:must-redraw-difference
   job_flag0.bit31 ... 0:normal 1:must-redraw

   condition는 job에 들어가기 전 상태
   job_flag0의 대응하는 비트는 job 종료후 상태가 되도록 다시 계산한다)
   job_flag0의 OVERRIDEDISABLED는, 어쨌든 현재의 그리기 금지 상태 */
{
	struct STR_JOB *pjob = &job;
	struct WM0_WINDOW *win0, *win1, *bottom, *top_ = top /* 고속화, 컴팩트화이기 때문에 */;
	int flag0;

	if (top_ == NULL) {
		pjob->now = 0;
	//	pjob->func = NULL;
		return;
	}

	/* accessenable & not input active */
	win0 = top_;
	do {
		/* 입력 없음, 대기 없음, 먼저 그리기 허가 */
		/* not-input-active & no-waiting(~0x0302), accessenable(1) */
		flag0 = (win0->job_flag0 & ~0x0302) | 1;
		/* 그리기 금지였던 녀석에게는 override-disabled */
		if ((win0->condition & 0x01) == 0)
			flag0 |= WINFLG_OVERRIDEDISABLED;
		win0->job_flag0 = flag0;
	} while ((win0 = win0->down) ! = top_);

	/* 위에서 봤을 때 겹쳐 있는 것은 accessdisable */
	win0 = top_;
	do {
		for (win1 = win0->down; win1 ! = top_; win1 = win1->down) {
			if (win1->job_flag0 & 0x01) { /* 이 if문이 없어도 실행 결과는 변함없지만, 고속화를 위해 */
				if (overrapwin(win0, win1))
					win1->job_flag0 &= ~0x01; /* accessdisable */
			}
		}
	} while ((win0 = win0->down) ! = top_);

	/* top는 job_flag0.bit 1 = 1; */
	top_->job_flag0 |= 0x02; /* input-active */

	/* 아래에서 봤을 때 condition이 변화할 예정이라면 (차이분) 다시 그리기
	   그 때 자신과 겹쳐지고 있는 위의 것도 모두 다시 그리기 */
	/* 그것까지 그리기 허가가 되었다면, 차이분 그리기로 끝마친다 */
	pjob->count = 0;	/* added by I.Tak.  (Jenny1. 2) */
	pjob->func = job_general2;	/* added by I.Tak.  (Jenny1. 2) */
	win0 = bottom = top_->up; /* 맨 위 위는 맨 밑 */
	do {
		flag0 = win0->job_flag0;
		if ((flag0 & (WINFLG_OVERRIDEDISABLED | 0x01)) == (WINFLG_OVERRIDEDISABLED | 0x01)
			|| (win0->condition & 0x03) ! = (flag0 & 0x03)) {
			/* added by I.Tak.  (Jenny1. 2) (if-block) */
			if (win0->condition & 1)
				flag0 |= WINFLG_MUSTREDRAWDIF;
			else
				flag0 |= WINFLG_MUSTREDRAW;
				/* 다시 그리기와 차이분 그리기가 동시에 하는 일도 있는데
				   다시 그리기를 우선하게 되어 있다 */
			win0->job_flag0 = flag0;
		}
		if (flag0 & WINFLG_MUSTREDRAW) {
			for (win1 = win0->up; win1 ! = bottom; win1 = win1->up) {
				if ((win1->job_flag0 & WINFLG_MUSTREDRAW) == 0) { /* 이 if문이 없어도 실행 결과는 변함없지만, 고속화를 위해 */
					if (overrapwin(win0, win1))
						win1->job_flag0 |= WINFLG_MUSTREDRAW; /* must-redraw */
				}
			}
		}
	} while ((win0 = win0->down) ! = bottom);

#if 0	/* deleted by I.Tak.  (Jenny1. 2) */
	/* redraw를 예정하고 있는 윈도우로, 다른 redraw 예정 윈도우와 오버랩 하고 있는 것은,
		모두 override-accessdisabled로 한다 */
	pjob->count = 0;
	pjob->func = job_general2;
	win0 = top_;
	do {
		if (win0->job_flag0 & WINFLG_MUSTREDRAW) {
			for (win1 = win0->down; win1 ! = win0; win1 = win1->down) {
				if (win1->job_flag0 & WINFLG_MUSTREDRAW) {
					if (overrapwin(win0, win1)) {
						if ((win0->job_flag0 & WINFLG_OVERRIDEDISABLED) == 0) {
							sgg_wm0s_accessdisable(&win0->sgg);
							win0->job_flag0 |= WINFLG_OVERRIDEDISABLED | WINFLG_WAITDISABLE;
							pjob->count++;
						}
						if ((win1->job_flag0 & (WINFLG_MUSTREDRAW | WINFLG_OVERRIDEDISABLED)) == WINFLG_MUSTREDRAW) {
							sgg_wm0s_accessdisable(&win1->sgg);
							win1->job_flag0 |= WINFLG_OVERRIDEDISABLED | WINFLG_WAITDISABLE;
							pjob->count++;
						}
					}
				}
			}
		}
	} while ((win0 = win0->down) ! = top_);
#endif
	pjob->win = NULL;
	if (pjob->count == 0)
		job_general2(0, 0);
	return;
}

void job_general2(const int cmd, const int handle)
{
	struct STR_JOB *pjob = &job;
	struct WM0_WINDOW *win;
	int flag0;

	if (cmd ! = 0 || handle ! = 0) {
		win = handle2window(handle);
		if (cmd == 0x00c0 /* 갱신 정지 수리 시그널 */) {
			if (win->job_flag0 & WINFLG_WAITDISABLE)
				win->job_flag0 &= ~WINFLG_WAITDISABLE;
			else {
				#if (defined(DEBUG))
					unsigned char s[12];
				#endif
error:
				#if (defined(DEBUG))
					s[8] = '\0';
					debug_bin2hex(cmd, s);    lib_drawletters_ASCII(1, -1, 0xc0,  0, 0, 15, 0, s);
					debug_bin2hex(handle, s); lib_drawletters_ASCII(1, -1, 0xc0, 80, 0, 15, 0, s);
				#endif
				return;
			}
		} else if (cmd == 0x00c4 /* 그리기 완료 시그널 */) {
			if (win->job_flag0 & WINFLG_WAITREDRAW)
				win->job_flag0 &= ~WINFLG_WAITREDRAW;
			else
				goto error;
		}
		if (--pjob->count)
			return;
	}

	win = pjob->win;

	if (win == top && win->job_flag0 == 0)
		goto fin;
		/* 1주목으로 win->job_flag0가 0이 되는 일도 있을 수 있다 */
		// 모든 작업이 완료
	if (win == NULL)
		win = top;

	do {
		win = win->up;
		flag0 = win->job_flag0;
		win->job_flag0 = 0;
		if ((flag0 & (WINFLG_OVERRIDEDISABLED | 0x01)) == (WINFLG_OVERRIDEDISABLED | 0x01) && screen.x2 ! = 0)
			sgg_wm0s_accessenable(&win->sgg);
	//	if (flag0 & WINFLG_MUSTREDRAW) {	/* deleted by I.Tak.  (jenny1. 2) */
		if (flag0 & (WINFLG_MUSTREDRAW | WINFLG_MUSTREDRAWDIF)) {	/* added by I.Tak.  (jenny1. 2) */
			pjob->win = win;
			if ((flag0 & (WINFLG_OVERRIDEDISABLED | 0x01)) == 0) {
				sgg_wm0s_accessdisable(&win->sgg);
				win->job_flag0 |= WINFLG_WAITDISABLE;
				pjob->count = 1;
			}
			if ((flag0 & 0x03) ! = win->condition)
				sgg_wm0s_setstatus(&win->sgg, win->condition = (flag0 & 0x03));
		//	sgg_wm0s_redraw(&win->sgg);	/* deleted by I.Tak.  (jenny1. 2) */
			/* added by I.Tak.  (jenny1. 2) (if-block) */
			if (flag0 & WINFLG_MUSTREDRAW)
				sgg_wm0s_redraw(&win->sgg);
			else
				sgg_wm0s_redrawdif(&win->sgg);
			win->job_flag0 |= WINFLG_WAITREDRAW;
			pjob->count++;
			return;
		}
	} while (win ! = top);
fin:
	pjob->now = 0;
//	pjob->func = NULL;
	return;
}

#if 0

void free_sndtrk(struct SOUNDTRACK *sndtrk)
{
	sndtrk->sigbox = 0;
	return;
}

struct SOUNDTRACK *alloc_sndtrk()
{
	struct SOUNDTRACK *sndtrk = sndtrk_buf;
	int i;
	for (i = 0; i < MAX_SOUNDTRACK; i++, sndtrk++) {
		if (sndtrk->sigbox == 0)
			return sndtrk;
	}
	return NULL;
}

#endif

void send_signal2dw(const int sigbox, const int data0, const int data1)
{
	sgg_execcmd0(0x0020, 0x80000000 + 3, sigbox | 2, data0, data1, 0x000);
	return;
}

void send_signal3dw(const int sigbox, const int data0, const int data1, const int data2)
{
	sgg_execcmd0(0x0020, 0x80000000 + 4, sigbox | 3, data0, data1, data2, 0x000);
	return;
}

void send_signal4dw(const int sigbox, const int data0, const int data1, const int data2, const int data3)
{
	sgg_execcmd0(0x0020, 0x80000000 + 5, sigbox | 4, data0, data1, data2, data3, 0x000);
	return;
}

void gapi_loadankfont();

void gapi_driverinit(int drv)
{
	#define	S	((((((((0
	#define	O	* 2 + 1)
	#define	_	* 2 + 0)
	#define	T	, ((((((((0

	static union {
		struct {
			unsigned char p0[32], p1[32];
		};
		int align;
	} mcursor = {
		#if (defined(WIN9X))
			S O O _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S O _ O _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S O _ _ O _ _ _ _ T _ _ _ _ _ _ _ _,
			S O _ _ _ O _ _ _ T _ _ _ _ _ _ _ _,
			S O _ _ _ _ O _ _ T _ _ _ _ _ _ _ _,
			S O _ _ _ _ _ O _ T _ _ _ _ _ _ _ _,
			S O _ _ _ _ _ _ O T _ _ _ _ _ _ _ _,
			S O _ _ _ _ _ _ _ T O _ _ _ _ _ _ _,
			S O _ _ _ _ _ O O T _ _ _ _ _ _ _ _,
			S O _ _ _ _ O _ _ T _ _ _ _ _ _ _ _,
			S O _ O _ _ _ O _ T _ _ _ _ _ _ _ _,
			S O O _ O _ _ O _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ O _ _ O T _ _ _ _ _ _ _ _,
			S _ _ _ _ O _ _ O T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ O _ _ T O _ _ _ _ _ _ _,
			S _ _ _ _ _ _ O O T _ _ _ _ _ _ _ _,

			S _ _ _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O O _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O O O _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O O O O _ _ _ T _ _ _ _ _ _ _ _,
			S _ O O O O O _ _ T _ _ _ _ _ _ _ _,
			S _ O O O O O O _ T _ _ _ _ _ _ _ _,
			S _ O O O O O O O T _ _ _ _ _ _ _ _,
			S _ O O O O O _ _ T _ _ _ _ _ _ _ _,
			S _ O O O O _ _ _ T _ _ _ _ _ _ _ _,
			S _ O _ O O O _ _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ O O _ _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ O O _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ O O _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ _ O O T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ _ _ _ T _ _ _ _ _ _ _ _
		#elif (defined(TMENU))
			/* 오리지날 마우스 커서 패턴(16x16, mono) by I.Tak.  */
			/* TOWNS 의 내장에 들어오고 있는 것을 닮아 있습니다만 풀 스크래치입니다. */
			S O _ _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S O O _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S O _ O _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S O _ _ O _ _ _ _ T _ _ _ _ _ _ _ _,
			S O O _ _ O _ _ _ T _ _ _ _ _ _ _ _,
			S O _ O _ _ O _ _ T _ _ _ _ _ _ _ _,
			S O O _ _ _ _ O _ T _ _ _ _ _ _ _ _,
			S O _ O _ _ _ _ O T _ _ _ _ _ _ _ _,
			S O O _ O _ _ _ _ T O _ _ _ _ _ _ _,
			S O _ O _ _ _ O O T O O _ _ _ _ _ _,
			S O O _ O _ O O O T _ _ _ _ _ _ _ _,
			S O _ O O _ O O _ T _ _ _ _ _ _ _ _,
			S O O O O O _ O _ T _ _ _ _ _ _ _ _,
			S O O _ _ O _ O _ T _ _ _ _ _ _ _ _,
			S O _ _ _ O O O _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ O O _ T _ _ _ _ _ _ _ _,

			S _ _ _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O O _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ _ O O _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O _ O O _ _ _ T _ _ _ _ _ _ _ _,
			S _ _ O O O O _ _ T _ _ _ _ _ _ _ _,
			S _ O _ O O O O _ T _ _ _ _ _ _ _ _,
			S _ _ O _ O O O O T _ _ _ _ _ _ _ _,
			S _ O _ O O O _ _ T _ _ _ _ _ _ _ _,
			S _ _ O _ O _ _ _ T _ _ _ _ _ _ _ _,
			S _ O _ _ O _ _ _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ O _ _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ O _ _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ _ _ _ T _ _ _ _ _ _ _ _
		#elif (defined(CHO_OSASK))
			/* CHO Kanji like cursor made by I.Tak.  */
			0xe0, 0x00, 0x90, 0x00, 0x90, 0x00, 0x48, 0x00,
			0x48, 0x00, 0x25, 0xe0, 0x26, 0x18, 0x12, 0x08,
			0x30, 0x04, 0x28, 0x04, 0x28, 0x04, 0x20, 0x0e,
			0x10, 0x3e, 0x0c, 0xf8, 0x03, 0xe0, 0x03, 0x80,

			0x00, 0x00, 0x60, 0x00, 0x60, 0x00, 0x30, 0x00,
			0x30, 0x00, 0x18, 0x00, 0x19, 0xe0, 0x0d, 0xf0,
			0x0f, 0xf8, 0x17, 0xf8, 0x17, 0xf8, 0x1f, 0xf0,
			0x0f, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00
#if 0
			S O O O _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S O _ _ O _ _ _ _ T _ _ _ _ _ _ _ _,
			S O _ _ O _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O _ _ O _ _ _ T _ _ _ _ _ _ _ _,
			S _ O _ _ O _ _ _ T _ _ _ _ _ _ _ _,
			S _ _ O _ _ O _ O T O O O _ _ _ _ _,
			S _ _ O _ _ O O _ T _ _ _ O O _ _ _,
			S _ _ _ O _ _ O _ T _ _ _ _ O _ _ _,
			S _ _ O O _ _ _ _ T _ _ _ _ _ O _ _,
			S _ _ O _ O _ _ _ T _ _ _ _ _ O _ _,
			S _ _ O _ O _ _ _ T _ _ _ _ _ O _ _,
			S _ _ O _ _ _ _ _ T _ _ _ _ O O O _,
			S _ _ _ O _ _ _ _ T _ _ O O O O O _,
			S _ _ _ _ O O _ _ T O O O O O _ _ _,
			S _ _ _ _ _ _ O O T O O O _ _ _ _ _,
			S _ _ _ _ _ _ O O T O _ _ _ _ _ _ _
#endif
		#elif (defined(NEWSTYLE))
			/* NEWSTYLE Cursor made by I.Tak.  */
			S O O _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S O _ O O _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O _ _ O O O _ T _ _ _ _ _ _ _ _,
			S _ O _ _ _ _ O _ T _ _ _ _ _ _ _ _,
			S _ _ O _ _ O _ _ T _ _ _ _ _ _ _ _,
			S _ _ O _ O _ O _ T _ _ _ _ _ _ _ _,
			S _ _ O O _ O _ O T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ _ O _ T O _ _ _ _ _ _ _,
			S _ _ _ _ _ _ _ O T _ O _ _ _ _ _ _,
			S _ _ _ _ _ _ _ _ T O _ O _ _ _ _ _,
			S _ _ _ _ _ _ _ _ T _ O _ O _ _ _ _,
			S _ _ _ _ _ _ _ _ T _ _ O _ O _ _ _,
			S _ _ _ _ _ _ _ _ T _ _ _ O _ O _ _,
			S _ _ _ _ _ _ _ _ T _ _ _ _ O _ O _,
			S _ _ _ _ _ _ _ _ T _ _ _ _ _ O _ O,
			S _ _ _ _ _ _ _ _ T _ _ _ _ _ _ O _,

			S _ _ _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ _ O O _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ _ O O O O _ _ T _ _ _ _ _ _ _ _,
			S _ _ _ O O _ _ _ T _ _ _ _ _ _ _ _,
			S _ _ _ O _ O _ _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ _ O _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ _ _ O T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ _ _ _ T O _ _ _ _ _ _ _,
			S _ _ _ _ _ _ _ _ T _ O _ _ _ _ _ _,
			S _ _ _ _ _ _ _ _ T _ _ O _ _ _ _ _,
			S _ _ _ _ _ _ _ _ T _ _ _ O _ _ _ _,
			S _ _ _ _ _ _ _ _ T _ _ _ _ O _ _ _,
			S _ _ _ _ _ _ _ _ T _ _ _ _ _ O _ _,
			S _ _ _ _ _ _ _ _ T _ _ _ _ _ _ O _,
			S _ _ _ _ _ _ _ _ T _ _ _ _ _ _ _ _
		#elif (defined(WIN31))
			/* 개조가 끝난 마우스 커서 패턴(16x16, mono) by 성인 */
			S O O _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S O _ O _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S O _ _ O _ _ _ _ T _ _ _ _ _ _ _ _,
			S O _ _ _ O _ _ _ T _ _ _ _ _ _ _ _,
			S O _ _ _ _ O _ _ T _ _ _ _ _ _ _ _,
			S O _ _ _ _ _ O _ T _ _ _ _ _ _ _ _,
			S O _ _ _ _ _ _ O T _ _ _ _ _ _ _ _,
			S O _ _ _ _ _ _ _ T O _ _ _ _ _ _ _,
			S O _ _ _ _ _ _ _ T _ O _ _ _ _ _ _,
			S O _ _ _ _ _ O O T O O _ _ _ _ _ _,
			S O _ _ O _ _ O _ T _ _ _ _ _ _ _ _,
			S O _ O _ O _ _ O T _ _ _ _ _ _ _ _,
			S O O _ _ O _ _ O T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ O _ _ T O _ _ _ _ _ _ _,
			S _ _ _ _ _ O _ _ T O _ _ _ _ _ _ _,
			S _ _ _ _ _ _ O O T _ _ _ _ _ _ _ _,

			S _ _ _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O O _ _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O O O _ _ _ _ T _ _ _ _ _ _ _ _,
			S _ O O O O _ _ _ T _ _ _ _ _ _ _ _,
			S _ O O O O O _ _ T _ _ _ _ _ _ _ _,
			S _ O O O O O O _ T _ _ _ _ _ _ _ _,
			S _ O O O O O O O T _ _ _ _ _ _ _ _,
			S _ O O O O O O O T O _ _ _ _ _ _ _,
			S _ O O O O O _ _ T _ _ _ _ _ _ _ _,
			S _ O O _ O O _ _ T _ _ _ _ _ _ _ _,
			S _ O _ _ _ O O _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ O O _ T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ _ O O T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ _ O O T _ _ _ _ _ _ _ _,
			S _ _ _ _ _ _ _ _ T _ _ _ _ _ _ _ _,
		#endif
	};

	#undef	S
	#undef	O
	#undef	_
	#undef	T

	// close 할 때에 모든 윈도우는 disable되어 있을 것이므로,
	// 여기에서는 disable에는 하지 않는다

	#if (defined(PCAT))
		/* sgg로 드라이버 변환을 지정 */
		sgg_execcmd0(0x0088, 0x0000, drv, 0x0000);
		if (drv == 3)
			drv++;
		if (screen.vbecoldep ! = drv)
			screen.wallpaper_exist = 0;
		screen.vbecoldep = drv;
	#elif (defined(TOWNS))
		/* sgg로 드라이버 변환을 지정 */
		sgg_execcmd0(0x0088, 0x0000, drv, 0x0000);
		if (screen.vbecoldep ! = drv)
			screen.wallpaper_exist = 0;
		screen.vbecoldep = drv;	/* 상위에 들어가 있는 팁 정보는 */
		screen.driver = drv >> 8; /* 여기에 보존 */
	#endif

	sgg_wm0_gapicmd_0010_0000();
	gapi_loadankfont();

	/* 마우스 패턴 전송 */
	sgg_execcmd0(0x0050, 7 * 4, 0x0180, 0, 0x0001, &mcursor, 0x000c, 0x0000);
	return;
}

void job_openvgadriver(const int drv)
{
	gapi_driverinit(drv);
	job.now = 0;
	return;
}

void job_setvgamode0(const int mode)
{
	struct STR_JOB *pjob = &job;
	struct WM0_WINDOW *win;
	pjob->int0 = mode;

	if (mx ! = 0x80000000) {
		mxx = mx;
		sgg_wm0_removemouse();
		mx = 0x80000000;
	}

	// 우선, 모든 윈도우의 화면 갱신권을 일시적으로 박탈한다
	pjob->count = 0;
	pjob->func = job_setvgamode1;
	if (win = top) {
		do {
			int x;
			win->job_flag0 = (WINFLG_MUSTREDRAW | WINFLG_OVERRIDEDISABLED); // override-accessdisabled & must-redraw
			if ((win->condition & 0x01) ! = 0 && screen.x2 ! = 0) {
				pjob->count++; // disable
				sgg_wm0s_accessdisable(&win->sgg);
			}
		} while ((win = win->down) ! = top);
	}

	if (pjob->count == 0)
		job_setvgamode2(); // 곧바로 디스플레이 모드 변경으로

	return;
}

void job_setvgamode1(const int cmd, const int handle)
{
	// 0x00c0 밖에 오지 않는다
	if (--job.count == 0)
		job_setvgamode2();
	return;
}

#if (defined(PCAT))

struct STR_VBEMODE *vbe_modeinfo(int mode)
{
	struct STR_VBEMODE *p;
	for (p = vbelist; p->mode; p++) {
		if (mode == p->mode)
			return p;
	}
	return NULL;
}

#endif

void job_setvgamode2()
{
	struct WM0_WINDOW *win;
	#if (defined(TOWNS))
		int width[]
		  = { TWVSW, TWVSW, TWVSW, 512, 1024, 800 };
		int height[]
		  = { 1024*512/TWVSW, 1024*512/TWVSW, 1024*512/TWVSW,
		      480, 768, 600 };
		char bpp[] = {1,1,1,2,1,2};
		int newmode, newdrv;

		/* drvcode: TOWNS15 = 0x002, VESA16 = 0x102/0x202로 했다.
		 * job.int0는 화면 모드 번호 0,1,2,3, 0x101, 0x102의 어떤 것인가.
		 * gapi_drvinit (sgg(0x0088))에는 drvcode를 준다.
		 * 1:8bpp, 2:16bpp | 0x000=FM, 0x100=CL5430, 0x200=CL5434
		 */
		newmode = job.int0;
		if ((newdrv = job.int0 & 0xff00))
		  newmode = (newmode & 0xff) + 3;
		if (newmode == 4)
		  newdrv &= 0xff; /* CL8bpp는 TOWNS 네이티브 공용 드라이버 */
		newdrv |= bpp[newmode];

		if ((width[newmode] < screen.x2 || height[newmode] < screen.y2) && (win = top)) {
			/* 전 윈도우의 좌표 범위를 확인(초과하면 화면을 바꾸지 않는다) */
			do {
				if (win->x1 > width[newmode] || win->y1 > height[newmode] - RESERVELINE1) {
					job_general1();
					sgg_wm0_putmouse(mx = mxx, my);
					return;
				}
			} while ((win = win->down) ! = top);
		}

		/* 화면 모드 0 설정(640x480x8bpp) */
		/* 화면 모드 1 설정(768x512x8bpp) */
		/* 화면 모드 2 설정(640 x480 비디오) */
		/* 화면 모드 3 설정(512x480x16bpp) */
		/* 화면 모드 4 설정(1024x768x8bpp CLGD543x) */
		/* 화면 모드 5 설정(800x600x16bpp CLGD543x) */
		screen.x2 = width[newmode];
		screen.y2 = height[newmode];
		if (mx > screen.x2)
			mx = screen.x2 - 1;
		if (my > screen.y2 - 16)
			my = screen.y2 - 16;
		screen.backcolor = screen.backcolors[newdrv & 0xff];
		screen.moveunit = screen.moveunits[newdrv & 0xff];
		#if (defined(VMODE))
			if (job.int0 == 2)
			  screen.backcolor = 0;
		#endif

		/* GAPI 교환 vesa16 때는 화면 모드 변경도 동시에 한다 */
		gapi_driverinit(newdrv);
		if ((newdrv & 0xff) == 2) /* towns15/vesa16에 x2, y2를 알린다 */
		  sgg_execcmd0(0x0050, 8 * 4, 0x001c, 0, 0x0020, screen.x2, screen.y2, 0x0000, 0x0000);
		else
		  sgg_execcmd0(0x0050, 7 * 4, 0x001c, 0, 0x0020, job.int0 /* mode */, 0x0000, 0x0000); /* 화면 모드 변경 */
 		if (job.int0 < 3)
 			sgg_wm0_gapicmd_001c_0004(); /* 하드웨어 초기화(팔레트 설정) */
		#if (defined(CLGD543X))
			/* 불필요한 때도 많지만…… */
			sgg_execcmd0(0x0088, 1, job.int0, 0x0000); /* VRAM&CRT 출력 변환 */
		#endif
		init_screen(screen.x2, screen.y2);
		job_general1();
		return;
	#endif

	#if (defined(PCAT))
		struct STR_VBEMODE *p;
		int drv, vram;
		int x2_old = screen.x2, y2_old = screen.y2;
		int mode = job.int0 & 0x3fff;

		#if (defined(BOCHS))
			screen.x2 = 640; /* Bochs는 가상 화면을 사용할 수 없다 */
			screen.y2 = 480;
		#else
			screen.x2 = 800;
			screen.y2 = 600;
		#endif

		if (fromboot & 0x0001) {
			/* 보통 방법을 사용할 수 없다 */
			/* (가상 86 모드에서의 VGA 모드 변경이 잘 되지 않는다) */
		//	x2 = 640;
		//	y2 = 480;
		//	sgg_wm0_gapicmd_001c_0020(); // 화면 모드 설정(640x480)
			sgg_execcmd0(0x0050, 7 * 4, 0x001c, 0, 0x0020, 0x0012, 0x0000, 0x0000);
			sgg_wm0_gapicmd_001c_0004(); // 하드웨어 초기화
			init_screen(screen.x2, screen.y2);
			job_general1();
			return;
		}
		if (lock_v86())
			goto skip; /* V86 태스크가 사용중이라면 전환 실패 */
		if (mode ! = 0x0012 && mode ! = 0x0102) {
			struct STR_VBEMODE *p = vbe_modeinfo(mode);
			int drv, vram;
			if (p == NULL)
				goto skip;
			drv = p->flag;
			vram = p->linear;
			if (drv >= 4)
				goto skip;
			if (drv ! = 0 && vbeoverride[drv - 1] ! = 0)
				vram = vbeoverride[drv - 1];
			if (vram == 0)
				goto skip;
			screen.x2 = p->x_res;
			screen.y2 = p->y_res;
		}
		if (win = top) {
			do {
				if (win->x1 > screen.x2 || win->y1 > screen.y2 - RESERVELINE1) {
skip:
					screen.x2 = x2_old;
					screen.y2 = y2_old;
					job_general1();
					sgg_wm0_putmouse(mx = mxx, my);
					return;
				}
			} while ((win = win->down) ! = top);
		}
		job.func = &job_setvgamode3;
		sgg_wm0_setvideomode(job.int0 /* mode */, 0x0014);
		return;
	#endif

	#if (defined(NEC98))
		screen.x2 = 640;
		screen.y2 = 400;
		sgg_execcmd0(0x0050, 7 * 4, 0x001c, 0, 0x0020, job.int0 /* mode */, 0x0000, 0x0000);
		sgg_wm0_gapicmd_001c_0004(); /* 하드웨어 초기화 */
		init_screen(screen.x2, screen.y2);
		job_general1();
		return;
	#endif
}

#if (defined(PCAT))

void job_setvgamode3(const int sig, const int result)
{
	static int oldmode = 0x0012; /* VGA */
	int x2y2[2];
	int mode = job.int0 & 0x3fff;
	// 0x0014 밖에 오지 않는다
	if (result == 0) {
		/* 화면 모드 변환 성공 */
		int drv = 0 /* 4bit */, vram;
		struct STR_VBEMODE *p;
		struct WM0_WINDOW *win;
		if (mode ! = 0x0012 && mode ! = 0x0102) {
			p = vbe_modeinfo(mode);
			if (p == NULL)
				goto skip;
			drv = p->flag;
			vram = p->linear;
			if (drv >= 4)
				goto skip;
			if (drv ! = 0 && vbeoverride[drv - 1] ! = 0)
				vram = vbeoverride[drv - 1];
			if (vram == 0)
				goto skip;
			screen.x2 = p->x_res;
			screen.y2 = p->y_res;
			/* linear를 전달 */
			sgg_execcmd0(0x0088, 0x0001, vram, 0x0000);
		}
		if (mxx > screen.x2 - 1 || my > screen.y2 - 16) {
			mxx = my = 1;
		}
		x2y2[0] = screen.x2;
		x2y2[1] = screen.y2 - (RESERVELINE0 + RESERVELINE1);
		sgg_execcmd0(0x00b0, 0, 2, 0, x2y2, 0x000c, 0x0000); /* set info */
		if (drv == 0) {
			/* linear = 0을 전달 */
			sgg_execcmd0(0x0088, 0x0001, 0, 0x0000);
		}
		screen.backcolor = screen.backcolors[drv];
		screen.moveunit = screen.moveunits[drv];
		gapi_driverinit(drv);
		if (drv == 0) {
			sgg_execcmd0(0x0050, 7 * 4, 0x001c, 0, 0x0020, mode | 0x01000000, 0x0000, 0x0000);
				/* 스크롤 파라미터 초기화 */
		}
		if (drv ! = 0)
			sgg_execcmd0(0x0050, 8 * 4, 0x001c, 0, 0x0020, screen.x2, screen.y2, 0x0000, 0x0000);
		sgg_wm0_gapicmd_001c_0004(); // 하드웨어 초기화
		if (win = top){
			int mask = screen.moveunit - 1;
			int d;
			/* 몰래 위치를 수정 */
			do {
				if (d = win->x0 & mask){
					win->x0 -= d;
					win->x1 -= d;
					/* deleted by I.Tak.  (Jenny1. 2) */
				//	win->job_flag0 = WINFLG_MUSTREDRAW | WINFLG_OVERRIDEDISABLED; // override-disabled & must-redraw
					sgg_wm0s_movewindow(&win->sgg, win->x0, win->y0);
				}
			} while ((win = win->down) ! = top);
		}
		unlock_v86();
		oldmode = job.int0;
		init_screen(screen.x2, screen.y2);
		job_general1();
		return;
	}

	// VESA의 비지원 등에 의해 화면 모드 변경 실패
skip:
//	job.func = job_setvgamode3;
	#if (defined(BOCHS))
		screen.x2 = 640; /* Bochs는 가상 화면을 사용할 수 없다 */
		screen.y2 = 480;
	#else
		screen.x2 = 800;
		screen.y2 = 600;
	#endif
	sgg_wm0_setvideomode(job.int0 = oldmode, 0x0014);
	return;
}

#endif

void job_loadfont0(int fonttype, int tss, int sig)
{
	struct STR_JOB *pjob = &job;
	pjob->sig = sig;
	pjob->fonttss = tss;
	if (pjob->fontflag & 0x01) {
		job_loadfont1(0, 0);
		return;
	}
	/* 로드 코드 */
	lib_initmodulehandle0(0x000c, 0x0200); /* machine-dir에 초기화 */
	pjob->func = job_loadfont1;
	lib_steppath0(0, 0x0200, "JPN16V00.FNT", 0x0050 /* sig */);
	return;
}

void job_loadfont1(int flag, int dmy)
{
	struct STR_JOB *pjob = &job;
	if ((pjob->fontflag & 0x01) == 0) {
		char *fp = (char *) pjob->readCSd10 /* malloc 영역의 마지막 == mapping 영역의 시작 */;
		if (flag == 0 /* 로드 성공 */) {
			int siz = lib_readmodulesize(0x0200);
			lib_mapmodule(0x0000, 0x0200, 0x5, 320 * 1024, fp, 0);

			/* 여기서 slot:0 x0210에 gapidata를 배치 */
			/* 로드가 끝나고 나서 배치하는 것이 중요하고, 그렇지 않으면 gapidata의 확장이 끝나지 않을지도 모르니까 */
			sgg_execcmd0(0x007c, 0, 0x0210, 0x0000);
			lib_mapmodule(0x0000, 0x0210, 0x7 /* R/W */, 304 * 1024, fp + 320 * 1024, (8 + 16) * 1024);
			if (siz ! = 304 * 1024)
				lib_decodetek0(304 * 1024, (int) fp, 0x000c, (int) fp + 320 * 1024, 0x000c);
			else {
				for (siz = 0; siz < 304 * 1024; siz += 8) {
					*(int *) (fp + siz +  320 * 1024)      = *(int *) (fp + siz);
					*(int *) (fp + siz + (320 * 1024 + 4)) = *(int *) (fp + siz + 4);
				}
			}
			lib_unmapmodule(0, 768 * 1024, fp);
			pjob->fontflag |= 0x01;
			lib_initmodulehandle0(0x000c, 0x0200); /* machine-dir에 초기화 */

		}
		#if (defined(TOWNS) && defined(KROM))
			else {
				/* fontfile 로드에 실패했으므로 ROM를 의지한다 */
				sgg_execcmd0(0x007c, 0, 0x0210, 0x0000);
				lib_mapmodule(0x0000, 0x0210, 0x7 /* R/W */, 304 * 1024, fp, (8 + 16) * 1024);
				sgg_execcmd0(0x0100, fp, 0x000c, 0x0000); /* ROM to GAPI data */
				lib_unmapmodule(0, 304 * 1024, fp);
				pjob->fontflag |= 0x01;
			}
		#endif
	}

	job_loadfont2();
	return;
}

void job_loadfont2()
{
	if (job.fontflag & 0x02) {
		job_loadfont3(0, 0);
		return;
	}
	/* 로드 코드 */
	lib_initmodulehandle0(0x000c, 0x0200); /* machine-dir에 초기화 */
	job.func = job_loadfont3;
	lib_steppath0(0, 0x0200, "KOR16V00.FNT", 0x0050 /* sig */);
	return;
}

void job_loadfont3(int flag, int dmy)
{
	struct STR_JOB *pjob = &job;
	if ((pjob->fontflag & 0x02) == 0 && flag == 0 /* 로드 성공 */) {
		char *fp = (char *) pjob->readCSd10 /* malloc 영역의 마지막 == mapping 영역의 시작 */;
		int siz = lib_readmodulesize(0x0200);
		lib_mapmodule(0x0000, 0x0200, 0x5, 320 * 1024, fp, 0);

		/* 여기서 slot:0 x0210에 gapidata를 배치 */
		/* 로드가 끝나고 나서 배치하는 것이 중요하고, 그렇지 않으면 gapidata의 확장이 끝나지 않을지도 모르니까 */
		sgg_execcmd0(0x007c, 0, 0x0210, 0x0000);

		lib_mapmodule(0x0000, 0x0210, 0x7 /* R/W */, 288 * 1024, fp + 320 * 1024, (8 + 320) * 1024);
		if (siz ! = 288 * 1024)
			lib_decodetek0(288 * 1024, (int) fp, 0x000c, (int) fp + 320 * 1024, 0x000c);
		else {
			for (siz = 0; siz < 288 * 1024; siz += 8) {
				*(int *) (fp + siz +  288 * 1024)      = *(int *) (fp + siz);
				*(int *) (fp + siz + (288 * 1024 + 4)) = *(int *) (fp + siz + 4);
			}
		}
		lib_unmapmodule(0, 768 * 1024, fp);
		pjob->fontflag |= 0x02;
		lib_initmodulehandle0(0x000c, 0x0200); /* machine-dir에 초기화 */
	}
	if (pjob->fonttss) {
		send_signal2dw(pjob->fonttss | 0x240, 0x7f000001, pjob->sig);
		send_signal2dw(pjob->fonttss | 0x240, 0x000000cc, 0); /* to pioneer0 */
	}
	pjob->now = 0;
//	pjob->func = NULL;
	return;
}

#if (defined(DEBUG))

void lib_drawletters_ASCII(const int opt, const int win, const int charset, const int x0, const int y0,
	const int color, const int backcolor, const char *str)
{
	struct COMMAND {
		int cmd; /* 0x0048 */
		int opt /* 반드시 0x0001로 한다 */;
		int window, charset /* 반드시 0x00c0로 한다 */;
		int x0, y0 /* dot 단위 */;
		int color, backcolor;
		int length;
		int letters[80];
		int eoc;
    } command;

	const unsigned char *s;
	int *t;
	int length;
	
	// str의 길이를 조사한다
	for (s = (const unsigned char *) str; *s++; );

	if (length = s - (const unsigned char *) str - 1) {
		command.cmd = 0x0048;
		command.opt = opt;
		command.window = win;
		command.charset = charset;
		command.x0 = x0;
		command.y0 = y0;
		command.color = color;
		command.backcolor = backcolor;
		command.length = length;

		s = (const unsigned char *) str;
		t = command.letters;
		while (*t++ = *s++);

		lib_execcmd(command);
	}
	return;
}

void debug_bin2hex(unsigned int i, unsigned char *s)
{
	int j;
	unsigned char c;
	s += 7;
	for (j = 0; j < 8; j++) {
		c = (i & 0x0f) | '0';
		i >>= 4;
		if (c > '9')
			c += 'A' - '0' - 10;
		*s = c;
		s--;
	}
	return;
}

#endif

static int defsig_signal[4] /* , defsig_opt */; /* 사이즈 축소를 위해 */

void sgg_wm0_definesignal3(const int opt, const int device, int keycode,
	const int sig0, const int sig1, const int sig2)
{
	int i = opt & 0x0fff;
	int phase = (keycode >> 14) & 0x3;
//	defsig_opt = opt;
	defsig_signal[0] = sig0 | 0x02;
	defsig_signal[1] = sig1;
	defsig_signal[2] = sig2;
	do {
		sgg_wm0_definesignal3sub(keycode);
		if (opt & 0x00008000)
			defsig_signal[2] += 3;
		defsig_signal[2]++;
		if (opt & 0x00ff0000) {
			do {
				phase++;
				if ((phase &= 0x3) == 0) {
					keycode &= 0xffff3fff;
					keycode++;
				} else
					keycode += 0x00004000;
			} while (((opt >> phase) & 0x00010000) == 0);
		} else
			keycode++;
	} while (i--);
	return;
/*

·keysignal 정의에 대해
  opt	bit 0-11 : 연속 정의 시그널수
		bit15	 : interval(+1인가+4인가)
		bit16-19 : make/break/remake/overbreak
				   이것은, 확장 모드에 이행 하기 위해서 필요
				   모든 것을 0으로 하면, make|remake라고 하는 식으로 취급해진다
				   물론, 인크리먼트(increment)의 방침이라고 해도 이용된다
				   스타트는, keycode내의 bit30-31에 쓰여져 있다
*/

}

void sgg_wm0_definesignal3sub(const int keycode)
/* 연속 지정을 서포트하지 않는다 */
/* 디코드만(make/break/remake의 판정은 하지 않는다) */
{
	static struct {
		int type0, type1;
	} shifttype[] = {
		{ 0x0000c070, 0 },
		{ 0x0010c070, 0 },
		{ 0x0000c060, 0 },
		{ 0x0004c074, 0x0010c074 },
		{ 0x0000c074, 0x0014c074 },
		{ 0x0002c072, 0x0010c072 },
		{ 0x0000c072, 0x0012c072 },
		{ 0x0040c070, 0 }
	};
	if ((keycode & 0xfffff000) == 0) {
		/* 종래대로의 지정 */
		if (' ' <= keycode && keycode <= 0xff) {
			struct KEYTABLE *kt = &keybindtable[keycode - ' '];
			int rawcode, st, shiftmap;
			if ((st = kt->shifttype0) < 0xf0) {
				rawcode = kt->rawcode0;
				shiftmap = shifttype[st]. type0;
				sgg_wm0_definesignal3sub3(rawcode, shiftmap);
				sgg_wm0_definesignal3sub3(rawcode, shiftmap | 0x80000000);
				if (shiftmap = shifttype[st]. type1) {
					sgg_wm0_definesignal3sub3(rawcode, shiftmap);
					sgg_wm0_definesignal3sub3(rawcode, shiftmap | 0x80000000);
				}
				if ((st = kt->shifttype1) < 0xf0) {
					rawcode = kt->rawcode1;
					shiftmap = shifttype[st]. type0;
					sgg_wm0_definesignal3sub3(rawcode, shiftmap);
					sgg_wm0_definesignal3sub3(rawcode, shiftmap | 0x80000000);
					if (shiftmap = shifttype[st]. type1) {
						sgg_wm0_definesignal3sub3(rawcode, shiftmap);
						sgg_wm0_definesignal3sub3(rawcode, shiftmap | 0x80000000);
					}
				}
			}
		}
	} else {
		int shiftmap = ((keycode >> 16) & 0x000000ff) | ((keycode >> 8) & 0x00ff0000)
					  | ((keycode << 16) & 0xc0000000) | 0x0000c000;
		int rawcode = keycode & 0x00003fff;
		if (' ' + 0x1000 <= rawcode && rawcode <= 0x10ff) {
			struct KEYTABLE *kt = &keybindtable[rawcode - (' ' + 0x1000)];
			if (kt->shifttype0 ! = 0xff)
				sgg_wm0_definesignal3sub3(kt->rawcode0, shiftmap);
		} else if (' ' + 0x2000 <= rawcode && rawcode <= 0x20ff) {
			struct KEYTABLE *kt = &keybindtable[rawcode - (' ' + 0x2000)];
			if (kt->shifttype1 ! = 0xff)
				sgg_wm0_definesignal3sub3(kt->rawcode1, shiftmap);
		}
	}
	return;
}

/*
void sgg_wm0_definesignal3sub2(const int rawcode, const int shiftmap)
{
	if ((defsig_opt & 0x00ff0000) == 0)
		sgg_wm0_definesignal3sub3(rawcode, shiftmap | 0x80000000);
	sgg_wm0_definesignal3sub3(rawcode, shiftmap);
	return;
}
*/

void sgg_wm0_definesignal3sub3(int rawcode, const int shiftmap)
{
	if (rawcode == 0xff)
		rawcode = 0x100;
	sgg_execcmd0(0x0068, 12 * 4,
			0x010c   /* define */,
			7        /* opt(len) */,
			rawcode  /* rawcode */,
			shiftmap /* shiftmap */,
			tapisigvec[3] /* vector(ofs) */,
			0x03030000 | tapisigvec[4] /* vector(sel), cmd, len */,
			defsig_signal[0] /* signal[0] */,
			defsig_signal[1] /* signal[1] */,
			defsig_signal[2] /* signal[2] */,
			0x0000 /* EOC */,
		0);

#if 0
	/* cmd, opt(len), raw 코드, shift-lock-bitmap(mask, equal), subcmd,... */
	static struct {
		int cmd, length;
		int deccmd[10];
		int eoc;
	} command = {
		0x0068, 12 * 4, {
			0x010c /* define */,
			7      /* opt(len) */,
			0      /* rawcode */,
			0      /* shiftmap */,
			0      /* vector(ofs) */,
			0      /* vector(sel), cmd, len */,
			0      /* signal[0] */,
			0      /* signal[1] */,
			0      /* signal[2] */,
			0x0000 /* EOC */
		}, 0 /* EOC */
	};
	command.deccmd[2] = rawcode;
	command.deccmd[3] = shiftmap;
	command.deccmd[4] = tapisigvec[3] /* ofs */;
	command.deccmd[5] = 0x03030000 | tapisigvec[4] /* sel */;
	command.deccmd[6] = defsig_signal[0];
	command.deccmd[7] = defsig_signal[1];
	command.deccmd[8] = defsig_signal[2];
	sgg_execcmd(&command);
#endif
	return;
}

#if 0
void sgg_wm0_definesignal3com()
{
	sgg_execcmd(&defbindcommand);
	return;
}
#endif

/*
·keysignal 정의에 대해
  opt	bit 0-11 : 연속 정의 시그널수
		bit15	 : interval(+1인가+4인가)
		bit16-17 : make/break/remake/overbreak
		bit20-23 : 인크리먼트(increment)시의 make/break/remake/overbreak
				   전부 0 또한 인크리먼트(increment)는 허락하지 않는다
					bit16-23이 0의 경우 종래대로 취급한다

  keycode bit12-14 : 000:키코드 지정
					 001:코드는 아니고 키 지정(제1키)
					 010:코드는 아니고 키 지정(제2키)
					 100:코드는 아니고 키 지정(제3키)
		  bit15    : bit16-31은 유효(bit12-14가 제로가 아니라면 유효하게 정해져 있지 않을까)
		  bit16-23 : shift 마스크
		  bit24-31 : shift 비교
*/

void gapi_loadankfont()
{
	static char flag = 1;
	static struct {
		int cmd, length;
		int gapicmd[15];
		int eoc;
	} command = {
		0x0050, 17 * 4, {
			0x0104 /* loadfont */,
			0x0000 /* opt */,
			0x0001 /* type */,
			0x0100 /* len */,
			0x1000 /* to */,
			0x0000 /* from(ofs) */,
			7 * 8  /* from(sel) */,
			0x0104 /* loadfont */,
			0x0000 /* opt */,
			0x0001 /* type */,
			0x0100 /* len */,
			0x2000 /* to */,
			0x0000 /* from(ofs) */,
			7 * 8  /* from(sel) */,
			0x0000 /* EOC */
		}, 0 /* EOC */
	};
	if (flag) { /* 한 번 전송 하면 그것으로 좋으니까 */
		sgg_execcmd(&command);
		flag = 0;
	}
	return;
}

#if (defined(PCAT) || defined(TOWNS))

void job_savevram0()
{
#if (defined(PCAT))
  if (screen.vbecoldep == 0 || screen.x2 * screen.y2 * screen.vbecoldep > SCRNSHOTMAXSIZ - 2048) {
    job.now = 0;
    return;
  }
#endif
    /* 보존처 준비 */
    lib_initmodulehandle0(0x0008, 0x0200); /* user-dir에 초기화 */
    job.func = &job_savevram1;
    lib_steppath0(1, 0x0200, "SCRNSHOT.BMP", 0x0050 /* sig */);
    return;
    /* SCRNSHOT.TIF는 514 KB이상이 됩니다 */
    /* 사전에 모듈을 만들어 두지 않아도 괜찮습니다 */
}

static int paletteSizeTable[] = {256, 3, 0, 3}; /* 256,64K/32K, , 16M */

void job_savevram1(int flag, int dmy)
{
  if (flag == 0) { /* open 성공 */
    lib_resizemodule(0, 0x0200, 0x38 + paletteSizeTable[screen.vbecoldep-1] * 4
		     + screen.x2 * screen.y2 * screen.vbecoldep, 0x0050 /* sig */);
    job.func = &job_savevram2;
  } else {
    job.now = 0;
  }
  return;
}

#if 0
/* BM의 뒤에 패딩이 들어갈 가능성!  어떻게 막는 건가?  */
struct bmphead {
  char BM[2];
  int fileSize, reserved, imageOffset, infoheaderSize;
  int width, height;
  short plane, bit;
  int compression, imageSize;
  int XPPM, YPPM, colorUsed, colorImportant;
};
#endif

static int systemPalette[] = {  // OSASK system palette
 0x000000,
 0x840000,
 0x008400,
 0x848400,
 0x000084,
 0x840084,
 0x008484,
 0x848484,
 0xc6c6c6,
 0xff0000,
 0x00ff00,
 0xffff00,
 0x0000ff,
 0xff00ff,
 0x00ffff,
 0xffffff
};

void job_savevram2(int flag, int dmy)
{
  int paletteSize = paletteSizeTable[screen.vbecoldep-1];
  int imageOffset = 0x38 + paletteSize * 4;
  int imageSize = screen.x2 * screen.y2 * screen.vbecoldep;
  int fileSize = imageOffset + imageSize;

  if (flag == 0 && 
      lib_readmodulesize(0x0200) >= fileSize) {
    char *fp = (char*) job.readCSd10;
    int *fi = (int *)(fp + 2), i;

    lib_mapmodule(0x0000, 0x0200, 0x7, SCRNSHOTMAXSIZ, fp, 0);
    fp[0] = 'B';
    fp[1] = 'M';
    fi[0] = fileSize;
    fi[1] = 0;
    fi[2] = imageOffset;
    fi[3] = 0x28;
    fi[4] = screen.x2;
    fi[5] = -screen.y2;		/* 탑 다운 저장 */
    fi[6] = (screen.vbecoldep*8 << 16) + 1;
    fi[7] = (screen.vbecoldep==1)? 0:3;
    fi[8] = imageSize;
    fi[9] = fi[10] = fi[11] = fi[12] = 0;
    fi = &fi[13];		/* palette/bitMask */
    if (screen.vbecoldep == 1) {
      for (i = 0; i < 16; i++)
	fi[i] = systemPalette[i];
      for (i = 0; i < 64; i++)
	fi[i+192]=((i & 0x03)<<16|(i & 0x0c) << 6|(i & 0x30) >> 4) * 0x55;
    } else if (screen.vbecoldep == 2) {
#if (defined(PCAT))
      fi[0] = 0xf800;		/* R */
      fi[1] = 0x07e0;		/* G */
      fi[2] = 0x001f;		/* B */
#elif (defined(TOWNS))
#if (defined(CLGD543X))
      if (screen.x2 == 800 && screen.y2 == 600) { /* CLGD……부실 판정 */
	fi[0] = 0xf800;		/* R */
	fi[1] = 0x07e0;		/* G */
	fi[2] = 0x001f;		/* B */
      } else
#endif
      {
	fi[0] = 0x03e0;		/* R */
	fi[1] = 0x7c00;		/* G */
	fi[2] = 0x001f;		/* B */
      }
#endif
    } else { 			/* vbecoldep == 4 */
      fi[0] = 0x00ff0000;
      fi[1] = 0x0000ff00;
      fi[2] = 0x000000ff;
    }
    /* 사실은 0x36이지만 align 한다 */
    sgg_debug00(0, screen.x2 * screen.y2 * screen.vbecoldep, 0, 0xe0000000, 0x01280008,
		(int) &fp[0x38 + paletteSize*4] , 0x000c);
    lib_unmapmodule(0, SCRNSHOTMAXSIZ, fp);
    lib_initmodulehandle0(0x0008, 0x0200); /* user-dir에 초기화 */
    // send_signal3dw(0x4000 /* pokon0 */ | 0x240, 0x7f000002,
    //   0x008c /* SIGNAL_FREE_FILES */, 0x3000 /* winman0 */);
  }
  lib_initmodulehandle0(0x0008, 0x0200); /* user-dir에 초기화 */
  job.now = 0;
  return;
}
#endif

/* 배경화면 로드 루틴, DLL 버젼 */

static char picture0_mapped = 0;

void job_mappicture0(int flag, int dmy)
{
  char *fp;
  if (flag) {
    job_loadwallpaper(1,0);
    return;
  }
  /* PICTURE0.BIN의 슬롯은 0x210 */
  fp = (char*)(lib_readCSd(0x10) + WALLPAPERMAXSIZE);
  lib_mapmodule(0x0000, 0x220, 0x5, 65536, fp, 0);
  lib_execcmd0(0x00f0, 0, 0x0200, 0x40fa, 65535, fp, 0x0008 , 0);
  picture0_mapped = 1;
  job_openwallpaper();
}

void job_openwallpaper()
/* 2002.05. 27 카와이 : 배경화면 표시가 토글이 되도록 변경 */
{
	int bytepp = screen.vbecoldep;
	if (bytepp == 0)
	  bytepp++;
	if (screen.wallpaper_exist) {
	  job_loadwallpaper(-1, 0); /* 해제 */
	  return;
	}
	if (screen.x2 * screen.y2 * bytepp >= WALLPAPERMAXSIZE) {
	  job_loadwallpaper(1,0);
	} else if (picture0_mapped) {
	  job.func = &job_loadwallpaper;
	  lib_initmodulehandle0(0x0008, 0x0200);
	  lib_steppath0(0, 0x0200, screen.wallpaper_name, 0x0050 /* sig */);
	} else {
	  job.func = &job_mappicture0;
	  lib_initmodulehandle0(0x0008, 0x0210);
	  lib_steppath0(0, 0x0220, "PICTURE0.BIN", 0x0050 /* sig */);
	}
}

typedef struct {
  int type, flag, width, height;
  int typedepend[4];
} PIC0_INFO;

void wallpaperdecode(int *env, char *wallpaper, int buftype, int c, int x2, int y2, PIC0_INFO *info, int ms, char *fp)
{
  int bw, bh;			/* blank width/height */
  int xsz = x2, ysz = y2, skip = 0, yskip = 0, x0 = 0, y0 = 0;

  bh = y2 - info->height;
  if (bh >= 0) {
    ysz = info->height;
    yskip = bh / 2;
  } else
    y0 = -bh / 2;
  bw = x2 - info->width;
  if (bw >= 0) {
    xsz = info->width;
    skip = bw;
  } else
    x0 = -bw / 2;
  if ((bw & bh) >= 0) {
    int i;
    if (buftype & 2)
      c *= 0x00010001;
    if (buftype & 1)
      c *= 0x01010101;
    for (i = 0; i < x2 * y2 / (buftype & 7); i++)
      ((int*) wallpaper)[i] = c;	/* 부실 */
  }
  call_dll0207_48i(env, 5,0, info->type, xsz, ysz, x0, y0, ms, fp, buftype, wallpaper + (x2 * yskip + skip / 2) * (buftype & 7), skip * (buftype & 7), 0x0000);
}

void job_loadwallpaper(int flag, int dmy)
/* 2002.05. 27 카와이 : 조금 개조 */
/* flag! =0이라면 배경화면 없음, 0이라면 open 성공 */
{
	struct WM0_WINDOW *win;
	screen.wallpaper_exist = 0;
	unsigned char *const fp = (unsigned char *) lib_readCSd(0x0010);
	if (flag == 0) {	/* opening succeeded.  */
		int ms = lib_readmodulesize(0x200);
		int *env = (int*)(screen.wallpaper + WALLPAPERMAXSIZE);
		static PIC0_INFO info;
		static int cmd0[] = { 1,0, 2,0, (int) &info, 0,0,0, 0 };

		if (ms>WALLPAPERMAXSIZE)
			ms = WALLPAPERMAXSIZE;
		lib_mapmodule(0x0000, 0x0200, 0x5, ms, fp, 0);
		cmd0[5] = ms;
		cmd0[6] = (int) fp;
		call_dll0207_48(env, cmd0);
		if (info.type) {
		  static const int buftype[5] = { 0x0201, 0x0801, 2, 3, 4 };
		  wallpaperdecode(env, screen.wallpaper, buftype[screen.vbecoldep],
				  screen.backcolor, screen.x2, screen.y2, &info, ms, fp);
		  if (env[0] == 0)
		    screen.wallpaper_exist = 1;
		}
		lib_unmapmodule(0, WALLPAPERMAXSIZE, fp);
	}
	lib_initmodulehandle0(0x0008, 0x0200);

	/* 전 윈도우 다시 그리기 */
	if (mx ! = 0x80000000) {
		mxx = mx;
		sgg_wm0_removemouse();
		mx = 0x80000000;
	}

	init_screen(screen.x2, screen.y2);
	if (win = top) {
		do {
			win->job_flag0 |= WINFLG_MUSTREDRAW;
		} while ((win = win->down) ! = top);
	}
	job_general1();		// job.now = 0; /* ←를 포함한다 */
	return;
}
#define	lib_putblock(mode, win, x, y, sx, sy, skip, p) \
	lib_execcmd0(0x004c, mode, (void *) (win), (int) (x), (int) (y), (int) (sx), \
	(int) (sy), (int) (skip), (void *) (p), 0x000c, 0x0000)

void putwallpaper(int x0, int y0, int x1, int y1)
/* 2002.05. 27 카와이 : x1, y1의 값의 취급 사양 변경 */
{
	int bytepp = screen.vbecoldep;
	if (bytepp)
		bytepp--;
	bytepp++;
	if (screen.wallpaper_exist)
		lib_putblock(bytepp, (void *) -1, x0, y0, x1 - x0, y1 - y0,
			bytepp*(screen.x2 - (x1 - x0)), &screen.wallpaper[(screen.x2 * y0 + x0) *bytepp]);
	else {
		lib_drawline(0x0020, (void *) -1, screen.backcolors[0], x0, y0, x1 - 1, y1 - 1);
	}
	return;
}

void moswinsig_flagset()
{
	struct MOSWINSIG *mws;
	mws_sensitivecount = 0;
	for (mws = moswinsig; mws->win ! = (struct WM0_WINDOW *) -1; mws++) {
		if (mws->win ! = NULL) {
			int i = 12;
			if (mws_mousewin == mws->win)
				i = 8;
			if (press0.win == mws->win)
				i = 4;
			mws->flags = (mws->flags & ~0x000f0000) | ((mws->flags >> i) & 0x000f0000);
			if (mws->flags & 0x44000000)
				mws_sensitivecount++;
		}
	}
	return;
}

struct WM0_WINDOW *searchwin(int x, int y)
// 어느 window를 클릭했는지를 검출
{
	struct WM0_WINDOW *win;

	if (win = top) {
		do {
			if (win->x0 <= x && x < win->x1 && win->y0 <= y && y < win->y1)
				goto ret;
			win = win->down;
		} while (win ! = top);
		win = NULL;
	}
ret:
	return win;
}

#if (defined(PCAT))

void job_vesacheck0()
{
	if (lock_v86())
		job.now = 0;
	else {
		job.func = &job_vesacheck1;
		sgg_execcmd0(0x0090, 0, 0x4f00, 0, 0x3240 + 3, 0x7f000002, 0x0014, 0x0000);
	}
	return;
}

void job_vesacheck1(int sig, int result)
{
	int buf[256 / 4];
	unsigned short *sp;
	struct STR_VBEMODE *plist = vbelist;
	if (flag_vbe2 == 0) {
		vbelist[127]. flag = 31;
		if (result) {
			flag_vbe2 = 0; /* VESA 없음 */
			job_vesacheck2();
			return;
		}
		sgg_execcmd0(0x0094, 1, 256, buf, 0x000c, 0x0000);
	//	if (buf[0] == ('V' | 'E' << 8 | 'S' << 16 | 'A' << 24)) {
	//		flag_vbe2 = 1;
	//	}
	//	if ((buf[0] & 0x00ffffff) == ('V' | 'B' << 8 | 'E' << 16)) {
	//		flag_vbe2 = 2;
	//	}
		flag_vbe2 = (buf[1] >> 8) & 0xff;
		vbelist[127]. dummy = flag_vbe2;
		for (sp = (unsigned short *) &buf[2]; *sp ! = 0xffff; sp++, plist++) {
			plist->mode = *sp;
			plist->flag = 0x7f;
		}
		plist->mode = 0;
		if (vbelist[0]. mode == 0) {
			job_vesacheck2();
			return;
		}
		sgg_execcmd0(0x0090, 0, 0x4f01, (int) (vbelist[0]. mode), 0x3240 + 3, 0x7f000002, 0x0014, 0x0000);
		return;
	}
	while (plist->flag ! = 0x7f)
		plist++;
	sgg_execcmd0(0x0094, 2, 256, buf, 0x000c, 0x0000);
	plist->x_res = buf[2];
	plist->y_res = buf[3];
	plist->linebytes = buf[1];
	plist->linear = buf[8];
	if (flag_vbe2 <= 1 || (buf[0] & 0x80) == 0)
		plist->linear = 0;
	plist->flag = 0;
	if ((buf[0] & 0x19) == 0x19) {
		if (buf[4] == 8)
			plist->flag = 16; /* unknowned direct-color 8bit */
		if (buf[4] == 16)
			plist->flag = 17; /* unknowned direct-color 16bit */
		if (buf[4] == 24)
			plist->flag = 18; /* unknowned direct-color 24bit */
		if (buf[4] == 32)
			plist->flag = 19; /* unknowned direct-color 32bit */
		if (buf[2] >= 640 && buf[3] >= 480) { /* 너무 작은 화면은 음울하기 때문에 배제 */
			if (buf[5] == 4 && buf[4] == 8)
				plist->flag = 1; /* 256 color */
			if (buf[5] == 6) {
				if (buf[4] == 16 && buf[6] == 0x05060b05 && buf[7] == 0x00000005)
					plist->flag = 2; /* 16bit color */ 
				if (buf[4] == 32 && buf[6] == 0x08081008 && buf[7] == 0x18080008)
					plist->flag = 3; /* 24bit color */
				if (buf[4] == 32 && buf[6] == 0x08081008 && buf[7] == 0x00000008)
					plist->flag = 3; /* 24bit color */
			}
		}
	}
	plist->dummy = 0; /* Window */
	if (plist->linear)
		plist->dummy = 1; /* Linear */
	plist++;
	if (plist->flag == 0x7f) {
		sgg_execcmd0(0x0090, 0, 0x4f01, (int) (plist->mode), 0x3240 + 3, 0x7f000002, 0x0014, 0x0000);
		return;
	}
	job_vesacheck2();
	return;
}

void job_vesacheck2()
{
	send_signal3dw(0x4243, 0x7f000002, 0x00be /* sig_vbelist */, (int) vbelist);
	job.now = 0;
	unlock_v86();
	return;
}

int lock_v86()
{
	static int cmd[4] = { 0x0058, 0x0001, 0x0000, 0x0000 };
	sgg_execcmd(cmd);
	return cmd[2];
}

void unlock_v86()
{
	sgg_execcmd0(0x0058, 0x0000, 0x0000);
	return;
}

#endif

#if (defined(TIMEX))

void write_time()
{
	/* 2003.01. 06 KIYOTO, nikq, Kawai */
	static int cmd[] = { 0x008c, 0, 0, 0, 0x0000 };
	static int msg[] = {
		0x0048, 0x0001, -1, 0x00c0, TIMEX, TIMEY, TIMEC, TIMEBC, 23,
		'0', '0', '0', '0', '/', '0', '0', '/', '0', '0', '(', '0', '0', '0', ')',
		'0', '0', ':', '0', '0', ':', '0', '0',
		/* "0000/00/00(000)00:00:00" */
		0x0000
	};
	int year, month, day;
	char *week;
	sgg_execcmd(cmd); /* 일자 취득 */
	msg[ 9] = (((unsigned char *) cmd)[14] >> 4) + '0';
	msg[10] = (((unsigned char *) cmd)[14] & 15) + '0';
	msg[11] = (((unsigned char *) cmd)[13] >> 4) + '0';
	msg[12] = (((unsigned char *) cmd)[13] & 15) + '0';
	msg[14] = (((unsigned char *) cmd)[12] >> 4) + '0';
	msg[15] = (((unsigned char *) cmd)[12] & 15) + '0';
	msg[17] = (((unsigned char *) cmd)[11] >> 4) + '0';
	msg[18] = (((unsigned char *) cmd)[11] & 15) + '0';
	msg[24] = (((unsigned char *) cmd)[10] >> 4) + '0';
	msg[25] = (((unsigned char *) cmd)[10] & 15) + '0';
	msg[27] = (((unsigned char *) cmd)[ 9] >> 4) + '0';
	msg[28] = (((unsigned char *) cmd)[ 9] & 15) + '0';
	msg[30] = (((unsigned char *) cmd)[ 8] >> 4) + '0';
	msg[31] = (((unsigned char *) cmd)[ 8] & 15) + '0';
	year = (((unsigned char *) cmd)[13] & 15)
	     + (((unsigned char *) cmd)[13] >> 4) * 10
	     + (((unsigned char *) cmd)[14] & 15) * 100
	     + (((unsigned char *) cmd)[14] >> 4) * 1000;
	month = (((unsigned char *) cmd)[12] & 15)
	      + (((unsigned char *) cmd)[12] >> 4) * 10;
	day = (((unsigned char *) cmd)[11] & 15)
	    + (((unsigned char *) cmd)[11] >> 4) * 10;
	if (month <= 2) {
		year--;
		month += 12;
	}
	week = &"SunMonTueWedThuFriSat"
		[((year + (year / 4) - (year / 100) + (year / 400) + (13 * month + 8) / 5 + day) % 7) * 3];
	msg[20] = week[0];
	msg[21] = week[1];
	msg[22] = week[2];
	#if (TIMEX < 0)
		msg[4] = screen.x2 + TIMEX;
	#endif
	#if (TIMEY < 0)
		msg[5] = screen.y2 + TIMEY;
	#endif
	lib_execcmd(msg);
	if (winmanerr_time > 0) {
		if (--winmanerr_time == 0)
			winmanerr_clr();
	}
	return;
}

void winmanerr(const unsigned char *s)
{
	int msg[50], len;
	winmanerr_time = 10 * 2; /* 10sec */
	winmanerr_clr();
	msg[0] = 0x0048;
	msg[1] = 0x0001;
	msg[2] = -1;
	msg[3] = 0x00c0;
	msg[4] = ERRMSGX;
	msg[5] = ERRMSGY;
	msg[6] = ERRMSGC;
	msg[7] = ERRMSGBC;
	len = 0;
	while ((msg[9 + len] = s[len]) ! = 0)
		len++;
	msg[8] = len;
	if (ERRMSGX < 0)
		msg[4] += screen.x2; 
	if (ERRMSGY < 0)
		msg[5] += screen.y2; 
	lib_execcmd(msg);
	return;
}

void winmanerr_clr()
{
	int x0, y0;
	x0 = ERRMSGX;
	y0 = ERRMSGY;
	if (ERRMSGX < 0)
		x0 += screen.x2;
	if (ERRMSGY < 0)
		y0 += screen.y2;
	lib_drawline(0x0020, (void *) -1, ERRMSGCC, x0, y0, x0 + (8 * 40 - 1), y0 + 15);
	return;
}

#endif
