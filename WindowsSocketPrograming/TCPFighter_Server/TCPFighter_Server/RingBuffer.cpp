#include "RingBuffer.h"
#include <iostream>

RingBuffer::RingBuffer() : m_bufferSize(DEFAULT_BUFSIZE), m_front(0), m_rear(0)
{
	m_bufferPtr = new char[DEFAULT_BUFSIZE + 1];
}

RingBuffer::RingBuffer(int size) : m_bufferSize(size), m_front(0), m_rear(0)
{
	m_bufferPtr = new char[size + 1];
}

RingBuffer::~RingBuffer()
{
	delete[] m_bufferPtr;
}

int RingBuffer::Enqueue(const char* data, int requestSize)
{
	if (data == nullptr || requestSize <= 0 || IsFull())
		return 0;

	// 링버퍼의 남은 공간보다 많은 데이터 인큐 요청 -> 가능한 크기만큼은 넣어주자
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
		memcpy(m_bufferPtr + m_rear + 1, data, size);
	}
	else
	{
		int splite1 = GetDirectEnqueueSize();
		int splite2 = size - splite1;

		memcpy(m_bufferPtr + m_rear + 1, data, splite1);
		memcpy(m_bufferPtr, data + splite1, splite2);
	}

	m_rear = (m_rear + size) % (m_bufferSize + 1);

	return size;
}

int RingBuffer::Dequeue(char* buffer, int requestSize)
{
	// 일단은 요청받은 크기만큼 데이터가 존재하지 않을 때는 데이터 추출x
	if (buffer == nullptr || requestSize <= 0 || IsEmpty() || GetUsedSize() < requestSize)
		return 0;

	if (GetDirectDequeueSize() >= requestSize)
	{
		memcpy(buffer, m_bufferPtr + m_front + 1, requestSize);
	}
	else
	{
		int splite1 = GetDirectDequeueSize();
		int splite2 = requestSize - splite1;

		memcpy(buffer, m_bufferPtr + m_front + 1, splite1);
		memcpy(buffer + splite1, m_bufferPtr, splite2);
	}

	m_front = (m_front + requestSize) % (m_bufferSize + 1);

	return requestSize;
}

int RingBuffer::Peek(char* buffer, int requestSize)
{
	if (buffer == nullptr || requestSize <= 0 || IsEmpty() || GetUsedSize() < requestSize)
		return 0;

	if (GetDirectDequeueSize() >= requestSize)
	{
		memcpy(buffer, m_bufferPtr + m_front + 1, requestSize);
	}
	else
	{
		int splite1 = GetDirectDequeueSize();
		int splite2 = requestSize - splite1;

		memcpy(buffer, m_bufferPtr + m_front + 1, splite1);
		memcpy(buffer + splite1, m_bufferPtr, splite2);
	}

	return requestSize;
}

void RingBuffer::ClearBuffer()
{
	m_front = m_rear;
}

int RingBuffer::GetBufferSize() const
{
	return m_bufferSize;
}

int RingBuffer::GetFreeSize() const
{
	return (m_rear >= m_front) ? (m_bufferSize - m_rear + m_front) : (m_front - m_rear - 1);
}

int RingBuffer::GetUsedSize() const
{
	return (m_rear >= m_front) ? (m_rear - m_front) : (m_bufferSize - m_front + m_rear + 1);
}

int RingBuffer::GetDirectEnqueueSize() const
{
	return (m_front <= m_rear) ? (m_bufferSize - m_rear) : (m_front - m_rear - 1);
}

int RingBuffer::GetDirectDequeueSize() const
{
	return (m_front <= m_rear) ? (m_rear - m_front) : m_bufferSize - m_front;
}

bool RingBuffer::IsEmpty() const
{
	return m_front == m_rear;
}

bool RingBuffer::IsFull() const
{
	return ((m_rear + 1) % (m_bufferSize + 1)) == m_front;
}


char* RingBuffer::GetBufferPtr() const
{
	return m_bufferPtr;
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

	m_rear = (m_rear + size) % (m_bufferSize + 1);

	return size;
}

int RingBuffer::MoveFront(int size)
{
	if (GetUsedSize() < size)
		return 0;

	m_front = (m_front + size) % (m_bufferSize + 1);

	return size;
}
