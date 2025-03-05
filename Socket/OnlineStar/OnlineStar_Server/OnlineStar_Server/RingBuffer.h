#pragma once

#define DEFAULT_RINGBUFSIZE 1024

class RingBuffer
{
public:
	RingBuffer();
	RingBuffer(int bufferSize);
	~RingBuffer();

	// 버퍼 크기 얻기
	int GetBufferSize();

	// 현재 사용중인 크기 얻기
	int GetUsedSize();

	// 현재 버퍼에 남은 크기 얻기  
	int GetFreeSize();

	// writePos에 데이터 넣기
	int Enqueue(const char* data, int size);

	// readPos에서 데이터 가져오기
	int Dequeue(char* data, int size);

	// readPos에서 데이터 가져오지만 readPos 변경x
	int Peek(char* data, int size);

	// 버퍼 모든 데이터 삭제
	void ClearBuffer();

	// 버퍼 포인터로 외부에서 한방에 읽고, 쓸 수 있는 길이 (끊어지지 않고 이어진 길이)
	// 원형 큐의 구조상 끝단에 있는 데이터는 끝 -> 처음으로 돌아가서 2번에 데이터를 얻거나 넣을 수 있음
	int GetDirectEnqueueSize();
	int GetDirectDequeueSize();

	// 버퍼의 front/rear 포인터 얻기
	char* GetFrontBufferPtr();
	char* GetRearBufferPtr();

	// 실제 Front/Rear 위치 이동
	int MoveFront(int size);
	int MoveRear(int size);

private:
	char* m_buffer; // 링버퍼 메모리 위치 포인터

	int m_front;
	int m_rear;

	int m_bufferSize; // 버퍼 최대 크기
	int m_usedSize; // 버퍼 내 현재 사용 중인 크기
};