// Future.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <future>
#include <string>
using namespace std::chrono_literals;

__int64 sum = 0;

__int64 Calculate()
{
	for (__int32 i = 0; i < 100'000; i++)
		sum += i;

	return sum;
}

void PromiseWorker(std::promise<std::string>&& promise)
{
	promise.set_value("Secret Message");
}

void TaskWorker(std::packaged_task<__int64(void)>&& task)
{
	task(); // 여기서 실행된 함수의 결과 값이 future에 저장됨
}

int main()
{
	// 한 번 발생하는 이벤트를 처리하는 경우에 효과적

	/* std::future */
	{
		// 일단 future 객체 만들고 계속 코드 실행 (결과 상관 없이) -> 미래에 실행됨
		// future 옵션
		// 1. deferred: lazy evaluation 지연 실행 (get때 실행하겠다, 실행 시점을 늦추겠다)
		// 2. async: 별도의 스레드를 생성해서 실행 (자동으로 스레드 생성)
		// 3. deferred | async: 둘 중 알아서 선택해서 실행
		std::future<__int64> future = std::async(std::launch::async, Calculate); // 등록 함수 전용 스레드 생성

		// TODO
		// future.wait();

		/* 결과물 필요한 시점 -> future에 저장된 결과 받아오기 */
		__int64 sum = future.get();
	}

	/* std::promise */
	{
		// 미래에 결과물을 반환할 것을 약속
		std::promise<std::string> promise;
		std::future<std::string> future = promise.get_future(); // future에 등록

		std::thread t(PromiseWorker, std::move(promise));

		std::string message = future.get();
		std::cout << message << std::endl;

		t.join();
	}

	/* std::packaged_task */
	{
		std::packaged_task<__int64(void)> task(Calculate);
		std::future<__int64> future = task.get_future(); // future에 등록

		std::thread t(TaskWorker, std::move(task));

		__int64 sum = future.get();

		t.join();
	}
}
