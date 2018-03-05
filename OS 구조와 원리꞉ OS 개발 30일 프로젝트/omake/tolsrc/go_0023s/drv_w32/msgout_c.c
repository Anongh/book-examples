/* for w32 */
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
	HANDLE h;
	int bytes, len = b1 - b0;
	h = CreateFileA((char *) name, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE)
		errout_s_NL("can't open file: ", name);
	if (ReadFile(h, b0, len, &bytes, NULL))
	CloseHandle(h);
	if (len == bytes)
		errout("input filebuf over! " NL);
	return b0 + bytes;
}
