#include <iostream>
#include <cstdlib>
#include <ctime>
#include <Windows.h>
#include "RingBuffer.h"

int main()
{
	srand((unsigned int)time(NULL));

	while (1)
	{
		RingBuffer ringBuffer(1000);
		char* checkBuffer1 = new char[1001];
		char* checkBuffer2 = new char[1001];
		const char* data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

		int cnt = 0;
		int oldCnt = 0;
		int oldReadCnt = 0;
		while (ringBuffer.GetFreeSize() != 0)
		{
			int size = rand() % 52;
			int newCnt = ringBuffer.Enqueue(data, size);
			memcpy(checkBuffer1 + oldCnt, data, newCnt);
			oldCnt += newCnt;
		}
		while (ringBuffer.GetUsedSize() != 0)
		{
			int size = rand() % 52;
			int readCnt = ringBuffer.Dequeue(checkBuffer2 + oldReadCnt, size);
			oldReadCnt += readCnt;
		}
		checkBuffer1[1000] = '\0';
		checkBuffer2[1000] = '\0';
		printf("%s\n", checkBuffer1);
		printf("%s\n", checkBuffer2);

		if (strncmp(checkBuffer1, checkBuffer2, 1001))
		{
			printf("링버퍼 오류\n");
			break;
		}
		else
		{
			printf("같은 문자열\n");
		}


		delete[] checkBuffer1;
		delete[] checkBuffer2;

		Sleep(1000);
	}

	return 0;
}