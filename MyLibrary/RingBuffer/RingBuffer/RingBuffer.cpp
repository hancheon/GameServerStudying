#include "RingBuffer.h"
#include <iostream>

RingBuffer::RingBuffer() : m_bufferSize(DEFAULT_BUFSIZE), m_usedSize(0), m_front(0), m_rear(0)
{
	m_bufferPtr = new char[DEFAULT_BUFSIZE + 1];
}

RingBuffer::RingBuffer(int size) : m_bufferSize(size), m_usedSize(0), m_front(0), m_rear(0)
{
	m_bufferPtr = new char[size + 1];
}

RingBuffer::~RingBuffer()
{
	delete[] m_bufferPtr;
}

int RingBuffer::Enqueue(const char* data, int requestSize)
{
	if (data == nullptr || requestSize <= 0 || GetFreeSize() == 0)
		return 0;

	// �������� ���� �������� ���� ������ ��ť ��û -> ������ ũ�⸸ŭ�� �־�����
	int size;
	if (GetFreeSize() >= requestSize)
	{
		size = requestSize;
	}
	else
	{
		size = GetFreeSize();
	}

	if (GetDirectEnqueueSize() >= size)
	{
		memcpy(GetRearPtr(), data, size);
	}
	else
	{
		int splite1 = GetDirectEnqueueSize();
		int splite2 = size - splite1;

		memcpy(GetRearPtr(), data, splite1);
		memcpy(m_bufferPtr, data + splite1, splite2);
	}

	m_usedSize += size;
	MoveRear(size);

	return size;
}

int RingBuffer::Dequeue(char* buffer, int requestSize)
{
	// �ϴ��� ��û���� ũ�⸸ŭ �����Ͱ� �������� ���� ���� ������ ����x
	if (buffer == nullptr || requestSize <= 0 || m_usedSize == 0 || m_usedSize < requestSize)
		return 0;

	if (GetDirectDequeueSize() >= requestSize)
	{
		memcpy(buffer, GetFrontPtr(), requestSize);
	}
	else
	{
		int splite1 = GetDirectDequeueSize();
		int splite2 = requestSize - splite1;

		memcpy(buffer, GetFrontPtr(), splite1);
		memcpy(buffer + splite1, m_bufferPtr, splite2);
	}

	m_usedSize -= requestSize;
	MoveFront(requestSize);

	return requestSize;
}

int RingBuffer::Peek(char* buffer, int requestSize)
{
	// �ϴ��� ��û���� ũ�⸸ŭ �����Ͱ� �������� ���� ���� ������ ����x
	if (buffer == nullptr || requestSize <= 0 || m_usedSize == 0 || m_usedSize < requestSize)
		return 0;

	if (GetDirectDequeueSize() >= requestSize)
	{
		memcpy(buffer, GetFrontPtr(), requestSize);
	}
	else
	{
		int splite1 = GetDirectDequeueSize();
		int splite2 = requestSize - splite1;

		memcpy(buffer, GetFrontPtr(), splite1);
		memcpy(buffer + splite1, m_bufferPtr, splite2);
	}

	return requestSize;
}

int RingBuffer::DirectEnqueue(char* data, int size)
{
	return 0;
}

int RingBuffer::DirectDequeue(char* buffer, int size)
{
	return 0;
}

void RingBuffer::ClearBuffer()
{
	m_front = m_rear;
	m_usedSize = 0;
}

int RingBuffer::GetBufferSize() const
{
	return m_bufferSize;
}

int RingBuffer::GetFreeSize() const
{
	return m_bufferSize - m_usedSize;
}

int RingBuffer::GetUsedSize() const
{
	return m_usedSize;
}

int RingBuffer::GetDirectEnqueueSize() const
{
	if (m_front <= m_rear)
	{
		return m_bufferSize - m_rear;
	}
	else
	{
		return m_front - m_rear - 1;
	}
}

int RingBuffer::GetDirectDequeueSize() const
{
	if (m_front <= m_rear)
	{
		return m_rear - m_front;
	}
	else
	{
		return m_bufferSize - m_front;
	}
}

bool RingBuffer::IsEmpty() const
{
	return m_front == m_rear;
}

bool RingBuffer::IsFull() const
{
	return m_usedSize == m_bufferSize;
}

char* RingBuffer::GetRearPtr() const
{
	return (m_rear == m_bufferSize) ? m_bufferPtr : (m_bufferPtr + m_rear + 1);
}

char* RingBuffer::GetFrontPtr() const
{
	return (m_front == m_bufferSize) ? m_bufferPtr : (m_bufferPtr + m_front + 1);
}

int RingBuffer::MoveRear(int size)
{
	if (GetFreeSize() < size)
		return 0;

	if (GetDirectEnqueueSize() >= size)
	{
		m_rear += size;
	}
	else
	{
		m_rear = size - GetDirectEnqueueSize() - 1;
	}

	return size;
}

int RingBuffer::MoveFront(int size)
{
	if (GetUsedSize() < size)
		return 0;

	if (GetDirectDequeueSize() >= size)
	{
		m_front += size;
	}
	else
	{
		m_front = size - GetDirectDequeueSize() - 1;
	}

	return size;
}
