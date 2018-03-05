/* for stdc */

//	void GOLD_exit(int s);		/* 종료한다 */
int GOLD_getsize(const UCHAR *name); /* 파일 사이즈 취득 */
int GOLD_read(const UCHAR *name, int len, UCHAR *b0);
	/* 파일 read, 바이너리 모드,
		사이즈를 호출하는 측에서 직전에 파일을 체크하고 있고,
		딱 맞는 파일 사이즈를 요구해 온다 */

#if (defined(USE_SYS_STAT_H))

#include <sys/stat.h>

int GOLD_getsize(const UCHAR *name)
/* 파일 사이즈 취득 */
{
	FILE *fp;
	struct stat st;
	int i;
	fp = fopen(name, "rb");
	if (fp == NULL)
		goto err;
	i = fstat(fileno(fp), &st);
	fclose(fp);
	if (i == -1)
		goto err;
	return st.st_size;
err:
	return -1;
}

#else

int GOLD_getsize(const UCHAR *name)
{
	FILE *fp;
	int len;
	fp = fopen(name, "rb");
	if (fp == NULL)
		goto err;
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fclose(fp);
	return len;
err:
	return -1;
}

#endif

int GOLD_read(const UCHAR *name, int len, UCHAR *b0)
{
	FILE *fp;
	int i;
	fp = fopen(name, "rb");
	if (fp == NULL)
		goto err;
	i = fread(b0, 1, len, fp);
	fclose(fp);
	if (len ! = i)
		goto err;
	return 0;
err:
	return 1;
}
