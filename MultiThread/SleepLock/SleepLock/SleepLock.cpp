// Sleep.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

class SleepLock
{
    //---------------------------------------------------------------
    // lock을 획득하지 못하면 일정 시간을 대기하는 스립락
    //---------------------------------------------------------------
public:
    //-----------------------
    // lock 획득
    //-----------------------
    void lock()
    {
        // CAS (Compare-And-Swap)
        bool expected = false;
        bool desired = true;
        while (isLocked_.compare_exchange_strong(expected, desired) == false) {
            expected = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(0));
            // std::this_thread::yield();
        }
    }

    //-----------------------
    // lock 반환
    //-----------------------
    void unlock()
    {
        isLocked_.store(false);
    }

private:
    std::atomic<bool> isLocked_ = false;
};

__int32 sum = 0;
std::mutex m;
SleepLock sleeplock;

void Add()
{
    for (__int32 i = 0; i < 100'000; i++) {
        //std::lock_guard<std::mutex> guard(m);
        std::lock_guard<SleepLock> guard(sleeplock);
        sum++;
    }
}

void Sub()
{
    for (__int32 i = 0; i < 100'000; i++) {
        //std::lock_guard<std::mutex> guard(m);
        std::lock_guard<SleepLock> guard(sleeplock);
        sum--;
    }
}

int main()
{
    std::thread t1(Add);
    std::thread t2(Sub);

    t1.join();
    t2.join();

    std::cout << sum << std::endl;
}