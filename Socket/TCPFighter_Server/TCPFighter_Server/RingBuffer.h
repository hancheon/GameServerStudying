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

	// �Ϲ����� ������ ����/����
	int Enqueue(const char*, int);
	int Dequeue(char*, int);

	// ������ �������� �ʰ� ����
	int Peek(char*, int);

	// �������� ������ �������� ������ ����/����
	int DirectEnqueue(char*, int); // ���� rear ��ġ�� ���� ����
	int DirectDequeue(char*, int); // ���� front ��ġ���� ���� ����

	// ���� ������ ����
	void ClearBuffer(); // ������ ���� ����� ���� front�� rear ��ġ�� �ʱ�ȭ

	// ũ�� ��� �Լ�
	int GetBufferSize() const;
	int GetFreeSize() const;
	int GetUsedSize() const;
	int GetDirectEnqueueSize() const; // ���� rear ��ġ���� ���������� ������ �� �ִ� ������ ũ��
	int GetDirectDequeueSize() const; // ���� front ��ġ���� ���������� ������ �� �ִ� ������ ũ��
	bool IsEmpty() const;
	bool IsFull() const;

	// ��ġ ��� �Լ�
	char* GetRearPtr() const; // ���� �����͸� �ֱ� ���� �ʿ��� ������ ������ �����͸� ���� ���� ��ġ ��ȯ (rear + 1)
	char* GetFrontPtr() const; // ���� �����͸� �̱� ���� �ʿ��� ������ ������ �����Ͱ� ����ִ� ���� ��ġ ��ȯ (front + 1)

	// front�� rear ��ġ �̵�
	int MoveRear(int);
	int MoveFront(int);
};