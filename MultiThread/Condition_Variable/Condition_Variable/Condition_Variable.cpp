// Condition_Variable.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <queue>
using namespace std::chrono_literals;

std::mutex m;
std::queue<__int32> q;

/* condition_variable(User-Mode 오브젝트)를 이용하는 이벤트 방식 */
std::condition_variable cv;

void Producer()
{
	while (true) {
		// 1. Lock 잡기
		// 2. 공유 변수 사용
		// 3. Lock 해제
		// 4. 조건 변수를 통해 다른 스레드에게 통지

		{
			std::unique_lock<std::mutex> lock(m);
			q.push(100);
		}

		/* 다른 스레드에게 통지 */
		cv.notify_one(); // 대기 중인 스레드 중 단 한 개만 깨움
		// cv.notify_all(); // 대기 중인 모든 스레드 깨움

		// 왜 lock을 잡은 채로 notify하지 않는 걸까
		// lock을 아직 놓치 않은 채로 notify를 하면
		// lock을 잡은 스레드가 lock을 놓기 전에 먼저 consumer 스레드가 일어날 수 있음
		// 그럼 결국 조건을 확인하는 부분에서 락을 바로 획득 못할 수도 있음
		// 락을 획득하지 못한 스레드는 unique_lock에 의해 다시 대기 상태로 진입하면서 문맥교환 발생

		std::this_thread::sleep_for(1000ms);
	}
}

void Consumer()
{
	while (true) {
		std::unique_lock<std::mutex> lock(m);
		cv.wait(lock, []() {return q.empty() == false; });
		// 1. Lock 잡기
		// 2. 조건 확인
		// --> true: 그대로 진행
		// --> false: lock 해제 후 대기 상태로 전환

		// 어차피 notify가 실행되어서 깨어났는데 왜 조건을 다시 확인할까?
		// Spurious Wakeup (가짜 기상)
		// Event 방식의 동기화 문제와 동일함
		// Event로 해당 스레드가 깨어났지만 깨어남과 lock이 동시에 일어나지 않으니까
		// 그 사이에 다른 스레드가 공유 자원을 먼저 점유했을 수 있다의 문제가 발생했었는데
		// 똑같이 notify가 실행되어서 깨어났지만 다른 스레드가 먼저 공유 자원을 점유해서 공유 자원의 상태가 변경됬을 수 있음

		__int32 data = q.front();
		q.pop();
		std::cout << data << std::endl;
	}
}

int main()
{
	std::thread producer(Producer);
	std::thread consumer(Consumer);

	producer.join();
	consumer.join();
}