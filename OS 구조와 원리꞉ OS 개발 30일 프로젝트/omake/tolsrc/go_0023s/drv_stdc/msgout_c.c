/* for stdc */
/* 이것을 이식하는 것이 귀찮은 경우는,"others.c"+"msgout.c"에 옮겨 놓으면 된다 */
/* 이쪽이 컴팩트하다 */

void msgout(UCHAR *s)
{
	GOLD_write_t(NULL, GO_strlen(s), s);
	return;
}

void errout(UCHAR *s)
{
	msgout(s);
	GOLD_exit(1);
}

void errout_s_NL(UCHAR *s, UCHAR *t)
{
	msgout(s);
	msgout(t);
	msgout(NL);
	GOLD_exit(1);
}

UCHAR *readfile(UCHAR *name, UCHAR *b0, UCHAR *b1)
{
	FILE *fp;
	int bytes, len = b1 - b0;
	fp = fopen(name, "rb");
	if (fp == NULL)
		errout_s_NL("can't open file: ", name);
	bytes = fread(b0, 1, len, fp);
	fclose(fp);
	if (len == bytes)
		errout("input filebuf over! " NL);
	return b0 + bytes;
}
