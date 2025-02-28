#include "RingBuffer.h"

RingBuffer::RingBuffer() : _front(nullptr), _rear(nullptr), _bufferSize(0), _usedSize(0)
{
	
}

RingBuffer::RingBuffer(int bufferSize)
{

}

RingBuffer::~RingBuffer()
{

}

void RingBuffer::Resize(int size)
{

}

int RingBuffer::GetBufferSize()
{
	return _bufferSize;
}

int RingBuffer::GetUseSize()
{
	return _usedSize;
}

int RingBuffer::GetFreeSize()
{
	return (_bufferSize - _usedSize);
}

int RingBuffer::Enqueue(const char* data, int size)
{
	return 0;
}

int RingBuffer::Dequeue(char* data, int size)
{
	if (_usedSize < size)
	{
		return 0;
	}

	return size;
}

int RingBuffer::Peek(char* data, int size)
{
	return 0;
}

void RingBuffer::ClearBuffer()
{
}

int RingBuffer::DirectEnqueueSize()
{
	return 0;
}

int RingBuffer::DirectDequeueSize()
{
	return 0;
}

int RingBuffer::MoveRear(int size)
{
	return 0;
}

int RingBuffer::MoveFront(int size)
{
	return 0;
}

char* RingBuffer::GetFrontBufferPtr()
{
	return _front;
}

char* RingBuffer::GetRearBufferPtr()
{
	return _rear;
}
