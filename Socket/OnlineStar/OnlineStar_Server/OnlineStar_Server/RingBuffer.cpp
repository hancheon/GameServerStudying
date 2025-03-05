#include <iostream>
#include "RingBuffer.h"

RingBuffer::RingBuffer() : m_front(0), m_rear(0), m_bufferSize(DEFAULT_RINGBUFSIZE), m_usedSize(0)
{
	m_buffer = new char[m_bufferSize + 1];
}

RingBuffer::RingBuffer(int bufferSize) : m_front(0), m_rear(0), m_bufferSize(bufferSize), m_usedSize(0)
{
	m_buffer = new char[bufferSize + 1];
} 

RingBuffer::~RingBuffer()
{
	delete[] m_buffer;
}

int RingBuffer::GetBufferSize()
{
	return m_bufferSize;
}

int RingBuffer::GetUsedSize()
{
	return m_usedSize;
}

int RingBuffer::GetFreeSize()
{
	return m_bufferSize - m_usedSize;
}

int RingBuffer::Enqueue(const char* data, int size)
{
	int freeSize = GetFreeSize();

	// ���� ������ ���ų�, data�� nullptr�� ���, size�� 0���� ���� ��쿡 ����ó��
	if (freeSize == 0 || data == nullptr || size <= 0)
	{
		return 0;
	}

	// ��ť ũ�� ����
	int enqueueSize;
	if (freeSize < size)
	{
		enqueueSize = freeSize;
	}
	else
	{
		enqueueSize = size;
	}

	int directSize = GetDirectEnqueueSize();
	if (directSize < enqueueSize) // �������� rear�� ������ �ε����� �ʰ��ϴ� ���
	{
		memcpy(m_buffer + m_rear + 1, data, directSize);
		memcpy(m_buffer, data + directSize, enqueueSize - directSize);
	}
	else
	{
		memcpy(m_buffer + m_rear + 1, data, enqueueSize);
	}
	m_usedSize += enqueueSize;

	// rear �̵�
	MoveRear(enqueueSize);

	return enqueueSize;
}

int RingBuffer::Dequeue(char* data, int size)
{
	if (m_usedSize == 0 || m_usedSize < size || data == nullptr || size <= 0) // ����ó��
	{
		return 0;
	}

	// ��ť
	int directSize = GetDirectDequeueSize();
	if (directSize < size) // �������� front�� ������ �ε����� �ʰ��ϴ� ���
	{
		memcpy(data, m_buffer + m_front + 1, directSize);
		memcpy(data + directSize, m_buffer, size - directSize);
	}
	else
	{
		memcpy(data, m_buffer + m_front + 1, size);
	}
	m_usedSize -= size;

	MoveFront(size);

	return size;
}

int RingBuffer::Peek(char* data, int size)
{
	if (m_usedSize == 0 || m_usedSize < size || data == nullptr || size <= 0) // ����ó��
	{
		return 0;
	}

	int directSize = GetDirectDequeueSize();
	if (directSize < size) // �������� front�� ������ �ε����� �ʰ��ϴ� ���
	{
		memcpy_s(data, directSize, m_buffer + m_front + 1, directSize);
		memcpy_s(data + directSize, size - directSize, m_buffer, size - directSize);
	}
	else
	{
		memcpy_s(data, size, m_buffer + m_front + 1, size);
	}

	return size;
}

void RingBuffer::ClearBuffer()
{
	m_front = 0;
	m_rear = 0;
	m_usedSize = 0;
}

int RingBuffer::GetDirectEnqueueSize()
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

int RingBuffer::GetDirectDequeueSize()
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

int RingBuffer::MoveRear(int size)
{
	int directSize = GetDirectEnqueueSize();
	if (size > directSize) // �������� rear�� ������ �ε����� �ʰ��ϴ� ���
	{
		m_rear = size - directSize - 1;
	}
	else
	{
		m_rear += size;
	}

	return size;
}

int RingBuffer::MoveFront(int size)
{
	int directSize = GetDirectDequeueSize();
	if (size > directSize) // �������� front�� ������ �ε����� �ʰ��ϴ� ���
	{
		m_front = size - directSize - 1;
	}
	else
	{
		m_front += size;
	}

	return size;
}

char* RingBuffer::GetFrontBufferPtr()
{
	return m_buffer + m_front;
}

char* RingBuffer::GetRearBufferPtr()
{
	return m_buffer + m_rear;
}
