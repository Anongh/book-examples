#include "windows.h"

#include <stdio.h>

/* 텍스트 모드의 경우, 0x0d를 다만 지운다고 하는 단순한 사양 */

unsigned int fread(void *ptr, unsigned int size, unsigned int nobj, FILE *stream)
{
	unsigned int l = 0, ll, bytes = size * nobj;
	unsigned char *p, *p1, *q;
	if (bytes == 0)
		return 0;
	if (stream->flags & 0x08)
		return 0;
	if (stream->flags & 0x10) {
		*(char *) ptr = stream->ungetc;
		ptr = ((char *) ptr) + 1;
		l++;
		if (--bytes == 0)
			goto done;
		stream->flags &= ~0x10;
	}
reread:
	ReadFile(stream->handle, ptr, bytes, &ll, NULL);
	if ((stream->flags & 0x01) == 0 && ll ! = 0) {
		p = (char *) ptr;
		p1 = p + ll;
		do {
			if (*p == '\r')
				goto find_cr;
			p++;
		} while (p < p1);
	}
done:
	l = (l + ll) / size;
	if (l ! = nobj)
		stream->flags |= 0x08; /* EOF */
	return l; /* size가  1이외의 때, 이것이라면 사실은 안된다 */

find_cr:
	q = p;
	p++;
	if (p < p1) {
		do {
			if (*p ! = '\r')
				*q++ = *p;
			p++;
		} while (p < p1);
	}
	bytes = p - q; /* 지운 바이트수 */
	ptr = q;
	l += ll - bytes; /* 모인 문자수 */
	goto reread;
}
