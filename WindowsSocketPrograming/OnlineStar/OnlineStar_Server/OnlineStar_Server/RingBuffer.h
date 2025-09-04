#pragma once

#define DEFAULT_RINGBUFSIZE 1024

class RingBuffer
{
public:
	RingBuffer();
	RingBuffer(int bufferSize);
	~RingBuffer();

	// ���� ũ�� ���
	int GetBufferSize();

	// ���� ������� ũ�� ���
	int GetUsedSize();

	// ���� ���ۿ� ���� ũ�� ���  
	int GetFreeSize();

	// writePos�� ������ �ֱ�
	int Enqueue(const char* data, int size);

	// readPos���� ������ ��������
	int Dequeue(char* data, int size);

	// readPos���� ������ ���������� readPos ����x
	int Peek(char* data, int size);

	// ���� ��� ������ ����
	void ClearBuffer();

	// ���� �����ͷ� �ܺο��� �ѹ濡 �а�, �� �� �ִ� ���� (�������� �ʰ� �̾��� ����)
	// ���� ť�� ������ ���ܿ� �ִ� �����ʹ� �� -> ó������ ���ư��� 2���� �����͸� ��ų� ���� �� ����
	int GetDirectEnqueueSize();
	int GetDirectDequeueSize();

	// ������ front/rear ������ ���
	char* GetFrontBufferPtr();
	char* GetRearBufferPtr();

	// ���� Front/Rear ��ġ �̵�
	int MoveFront(int size);
	int MoveRear(int size);

private:
	char* m_buffer; // ������ �޸� ��ġ ������

	int m_front;
	int m_rear;

	int m_bufferSize; // ���� �ִ� ũ��
	int m_usedSize; // ���� �� ���� ��� ���� ũ��
};