/* for w32 */

//	void GOLD_exit(int s);		/* 종료한다 */
int GOLD_getsize(const UCHAR *name); /* 파일 사이즈 취득 */
int GOLD_read(const UCHAR *name, int len, UCHAR *b0);
	/* 파일 read, 바이너리 모드,
		사이즈를 호출한 측에서 직전의 파일을 체크하고 있고,
		딱 맞는 파일 사이즈를 요구해 온다 */

int GOLD_getsize(const UCHAR *name)
{
	HANDLE h;
	int len = -1;
	h = CreateFileA((char *) name, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE)
		goto err;
	len = GetFileSize(h, NULL);
	CloseHandle(h);
err:
	return len;
}

int GOLD_read(const UCHAR *name, int len, UCHAR *b0)
{
	HANDLE h;
	int i;
	h = CreateFileA((char *) name, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE)
		goto err;
	ReadFile(h, b0, len, &i, NULL);
	CloseHandle(h);
	if (len ! = i)
		goto err;
	return 0;
err:
	return 1;
}
