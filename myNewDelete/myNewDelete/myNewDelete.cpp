#include <iostream>
#include <time.h>
#include <string>
#include "myNewDelete.h"
using namespace std;

int index = 0;
AllocInfo allocArray[100];
char logFileName[24] = "Alloc_";

#undef new
void* operator new(size_t size, const char* File, int Line)
{
	printf("overloading operator new\n");
	void* ptr = malloc(size);

	allocArray[index].ptr = ptr;
	allocArray[index].size = size;
	strcpy_s(allocArray[index].filename, sizeof(allocArray[index].filename), File);
	allocArray[index].line = Line;
	allocArray[index].array = false;
	index++;

	return ptr;
}

void* operator new[](size_t size, const char* File, int Line)
{
	printf("overloading operator new\n");
	void* ptr = malloc(size);

	allocArray[index].ptr = ptr;
	allocArray[index].size = size;
	strcpy_s(allocArray[index].filename, sizeof(allocArray[index].filename), File);
	allocArray[index].line = Line;
	allocArray[index].array = true;
	index++;

	return ptr;
}
#define new new(__FILE__, __LINE__)

#undef delete
void operator delete(void* ptr, char* File, int Line)
{
	for (int i = 0; i < index; i++)
	{
		if (ptr == allocArray[i].ptr)
		{
			if (allocArray[i].array == false)
			{
				// 할당 내역 삭제
				allocArray[i] = { 0 };
				free(ptr);
			}
			else
			{
				// error: 배열을 배열이 아닌 것으로 delete 시도
			}
		}
		else
		{
			// error: 잘못된 포인터 delete 시도
		}
	}
}

void operator delete[](void* ptr, char* File, int Line)
{
	for (int i = 0; i < index; i++)
	{
		if (ptr == allocArray[i].ptr)
		{
			// 할당 내역 삭제
			allocArray[i] = { 0 };
		}
		else
		{
			// 해제 문제 로그 저장
		}
	}
}
#define delete delete(__FILE__, __LINE__)

void setFileName()
{
	tm currentTime = { 0 };
	time_t now = time(NULL);

	localtime_s(&currentTime, &now);

	strcat(logFileName, ((itoa(currentTime.tm_year + 1900) - '0'));
	if (currentTime.tm_mon + 1 >= 10)
	{
		logFileName += to_string(currentTime.tm_mon + 1);
	}
	else
	{
		logFileName += "0";
		logFileName += to_string(currentTime.tm_mon + 1);
	}
	if (currentTime.tm_mday >= 10)
	{
		logFileName += to_string(currentTime.tm_mday);
	}
	else
	{
		logFileName += "0";
		logFileName += to_string(currentTime.tm_mday);
	}
	logFileName += "_";
	if (currentTime.tm_hour >= 10)
	{
		logFileName += to_string(currentTime.tm_hour);
	}
	else
	{
		logFileName += "0";
		logFileName += to_string(currentTime.tm_hour);
	}
	if (currentTime.tm_min >= 10)
	{
		logFileName += to_string(currentTime.tm_min);
	}
	else
	{
		logFileName += "0";
		logFileName += to_string(currentTime.tm_min);
	}
	if (currentTime.tm_sec >= 10)
	{
		logFileName += to_string(currentTime.tm_sec);
	}
	else
	{
		logFileName += "0";
		logFileName += to_string(currentTime.tm_sec);
	}
	logFileName += ".txt";

	cout << logFileName << endl;
}