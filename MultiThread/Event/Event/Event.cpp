// Event.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <queue>

HANDLE hEvent;
std::mutex m;
std::queue<__int32> q;

using namespace std::chrono_literals;

void Producer()
{
	while (true) {
		{
			std::unique_lock<std::mutex> lock(m);
			q.push(100);
		}

		/* 이벤트 객체 Signaled로 변경 */
		SetEvent(hEvent);

		std::this_thread::sleep_for(1000ms);
	}
}

void Consumer()
{
	while (true) {
		WaitForSingleObject(hEvent, INFINITE);
		/* 이벤트 자동 리셋: Signaled -> Non-Signaled */

		std::unique_lock<std::mutex> lock(m);
		if (q.empty() == false) {
			__int32 data = q.front();
			q.pop();
			std::cout << data << std::endl;
		}
	}
}

int main()
{
	/* 자동 리셋 이벤트 객체(커널 오브젝트) 생성 */
	hEvent = CreateEvent(NULL, false, false, NULL);

	std::thread producer(Producer);
	std::thread consumer(Consumer);

	producer.join();
	consumer.join();

	CloseHandle(hEvent);
}