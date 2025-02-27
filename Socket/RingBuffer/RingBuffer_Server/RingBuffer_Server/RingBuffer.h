#pragma once

class RingBuffer
{
public:
	RingBuffer();
	RingBuffer(int bufferSize);
	~RingBuffer();

	// ���� ũ�� ����
	void Resize();

	// ���� ũ�� ���
	int GetBufferSize();

	// ���� ������� �뷮 ���
	int GetUseSize();

	// ���� ���ۿ� ���� �뷮 ���  
	int GetFreeSize();

	// writePos�� ������ �ֱ�
	int Enqueue(const char* data, int size);

	// readPos���� ������ ��������
	int Dequeue(char* data, int size);

	// readPos���� ������ ���������� readPos ����x
	int Peek(char* data, int size);

	// ���� ��� ������ ����
	void ClearBuffer();
private:
	// ���� �����ͷ� �ܺο��� �ѹ濡 �а�, �� �� �ִ� ���� (�������� �ʰ� �̾��� ����)
	// ���� ť�� ������ ���ܿ� �ִ� �����ʹ� �� -> ó������ ���ư��� 2���� �����͸� ��ų� ���� �� ����
	int DirectEnqueueSize();
	int DirectDequeueSize();

	// �б� ��ġ �̵�, ���� ��ġ �̵�?
	int MoveRear(int size);
	int MoveFront(int size);

	// ������ frontPos ������ ���
	char* GetFrontBufferPtr();

	// ������ rearPos ������ ���
	char* GetRearBufferPtr();
};