#pragma once

class RingBuffer
{
public:
	RingBuffer();
	RingBuffer(int bufferSize);
	~RingBuffer();

	// 버퍼 크기 변경
	void Resize(int size);

	// 현재 사용중인 크기 얻기
	int GetUseSize();

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
private:
	// 버퍼 크기 얻기
	int GetBufferSize();

	// 버퍼 포인터로 외부에서 한방에 읽고, 쓸 수 있는 길이 (끊어지지 않고 이어진 길이)
	// 원형 큐의 구조상 끝단에 있는 데이터는 끝 -> 처음으로 돌아가서 2번에 데이터를 얻거나 넣을 수 있음
	int DirectEnqueueSize();
	int DirectDequeueSize();

	// 읽기 위치 이동, 쓰기 위치 이동?
	int MoveRear(int size);
	int MoveFront(int size);

	// 버퍼의 frontPos 포인터 얻기
	char* GetFrontBufferPtr();

	// 버퍼의 rearPos 포인터 얻기
	char* GetRearBufferPtr();
private:
	char* _front;
	char* _rear;
	int _bufferSize; // 버퍼 최대 크기
	int _usedSize; // 사용 중인 크기
};