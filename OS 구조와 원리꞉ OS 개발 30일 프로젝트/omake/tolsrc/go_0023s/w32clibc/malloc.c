#include "windows.h"

/* 컴팩트한 것은 아니지만, 느리다 */

void *malloc(unsigned int bytes)
{
	return HeapAlloc(GetProcessHeap(), 0, bytes);
}
