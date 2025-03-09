#pragma once

struct AllocInfo
{
	void* ptr;
	int size;
	char filename[128];
	int  line;
	bool array;
};

extern char logFileName[24];

void setFileName();

#undef new
	void* operator new(size_t size, const char* File, int Line);
	void* operator new[](size_t size, const char* File, int Line);
#define new new(__FILE__, __LINE__)

#undef delete
	void operator delete(void* ptr, char* File, int Line);
	void operator delete[](void* ptr, char* File, int Line);
#define delete delete(__FILE__, __LINE__)
