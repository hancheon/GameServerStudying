#include "юс╫ц.h"
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

int RingBuffer::Enqueue(const char* data, int size)
{
	if (data == nullptr || size <= 0 || GetFreeSize() < size)
		return 0;

	if (GetDirectEnqueueSize() > size)
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

int RingBuffer::Dequeue(char* buffer, int size)
{
	return 0;
}

int RingBuffer::Peek(char* buffer, int size)
{
	return 0;
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


	return 0;
}

int RingBuffer::MoveFront(int size)
{
	return 0;
}
