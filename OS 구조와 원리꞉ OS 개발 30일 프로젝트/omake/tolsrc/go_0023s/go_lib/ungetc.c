/* copyright(C) 2002 H.Kawai (under KL-01).  */

#include <stdio.h>
#include <stdlib.h>

/* 읽은 문자 밖에 read, 되돌릴 수 없는 버젼 */
/* 본래의 ungetc와는 사양이 다르다 */

int GO_ungetc(int c, GO_FILE *stream)
{
	if (stream->p0 < stream->p && stream->p[-1] == c) {
		stream->p--;
		return c;
	}
	fputs("GO_ungetc:error! \n", stderr);
	abort();
}
