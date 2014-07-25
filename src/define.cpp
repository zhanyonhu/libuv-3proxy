#include "define.h"

void * operator new(size_t size)
{
	return malloc(size);
}

void operator delete(void *p)
{
	free(p);
}

void * operator new[](size_t size)
{
	return malloc(size);
}

void operator delete[](void * ptr)
{
	free(ptr);
}

#ifdef _DEBUG
void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return malloc(size);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return malloc(size);
}
#endif
