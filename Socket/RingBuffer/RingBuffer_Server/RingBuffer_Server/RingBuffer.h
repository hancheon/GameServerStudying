#pragma once

class RingBuffer
{
public:
	RingBuffer();
	RingBuffer(int bufferSize);
	~RingBuffer();

	// ���� ũ�� ����
	void Resize(int size);

	// ���� ������� ũ�� ���
	int GetUseSize();

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
private:
	// ���� ũ�� ���
	int GetBufferSize();

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
private:
	char* _front;
	char* _rear;
	int _bufferSize; // ���� �ִ� ũ��
	int _usedSize; // ��� ���� ũ��
};