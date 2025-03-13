#pragma once

#define DEFAULT_BUFSIZE 1000

class RingBuffer
{
private:
	char* m_bufferPtr;

	int m_bufferSize;
	int m_usedSize;

	int m_front;
	int m_rear;

public:
	RingBuffer();
	RingBuffer(int);
	~RingBuffer();

	// 일반적인 데이터 삽입/추출
	int Enqueue(const char*, int);
	int Dequeue(char*, int);

	// 데이터 추출하지 않고 복사
	int Peek(char*, int);

	// 직접적인 포인터 접근으로 데이터 삽입/추출
	int DirectEnqueue(char*, int); // 현재 rear 위치에 직접 삽입
	int DirectDequeue(char*, int); // 현재 front 위치에서 직접 추출

	// 버퍼 데이터 삭제
	void ClearBuffer(); // 데이터 흔적 남기기 위해 front와 rear 위치로 초기화

	// 크기 얻는 함수
	int GetBufferSize() const;
	int GetFreeSize() const;
	int GetUsedSize() const;
	int GetDirectEnqueueSize() const; // 현재 rear 위치에서 연속적으로 삽입할 수 있는 데이터 크기
	int GetDirectDequeueSize() const; // 현재 front 위치에서 연속적으로 추출할 수 있는 데이터 크기
	bool IsEmpty() const;
	bool IsFull() const;

	// 위치 얻는 함수
	char* GetRearPtr() const; // 직접 데이터를 넣기 위해 필요한 것으로 실제론 데이터를 넣을 다음 위치 반환 (rear + 1)
	char* GetFrontPtr() const; // 직접 데이터를 뽑기 위해 필요한 것으로 실제론 데이터가 들어있는 시작 위치 반환 (front + 1)

	// front와 rear 위치 이동
	int MoveRear(int);
	int MoveFront(int);
};