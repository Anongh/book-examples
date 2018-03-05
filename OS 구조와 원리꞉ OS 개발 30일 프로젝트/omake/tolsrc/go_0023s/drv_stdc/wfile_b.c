/* for stdc */

int GOLD_write_b(const UCHAR *name, int len, const UCHAR *p0)
/* 바이너리 모드로 파일에 출력 */
{
	int ll = 0;
	FILE *fp;
	fp = fopen(name, "wb");
	if (fp == NULL)
		goto err;
	if (len)
		ll = fwrite(p0, 1, len, fp);
	fclose(fp);
	if (ll ! = len)
		goto err;
	return 0;
err:
	return 1;
}

