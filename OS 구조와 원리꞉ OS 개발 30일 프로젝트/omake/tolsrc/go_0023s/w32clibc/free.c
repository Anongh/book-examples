#include "windows.h"

/* 컴팩트한 것은 아니지만 느리다 */

void free(void *p)
{
	HeapFree(GetProcessHeap(), 0, p);
	return;
}
