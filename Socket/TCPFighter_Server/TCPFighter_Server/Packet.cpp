#include <iostream>
#include "Packet.h"

Packet::Packet() : m_bufferSize(DEFUALT_BUFSIZE), m_dataSize(0), m_front(-1), m_rear(-1)
{
	m_bufferPtr = new char[m_bufferSize];
}

Packet::Packet(int size) : m_bufferSize(size), m_dataSize(0), m_front(-1), m_rear(-1)
{
	m_bufferPtr = new char[m_bufferSize];
}

Packet::~Packet()
{
	delete[] m_bufferPtr;
}

void Packet::Clear()
{
	m_front = -1;
	m_rear = -1;
	m_dataSize = 0;
}

int Packet::GetBufferSize()
{
	return m_bufferSize;
}

int Packet::GetDataSize()
{
	return m_dataSize;
}

char* Packet::GetBufferPtr()
{
	return m_bufferPtr;
}

int Packet::PutData(char* srcData, int size)
{
	memcpy(m_bufferPtr + m_rear + 1, srcData, size);
	m_rear = (m_rear + size) % m_bufferSize;
	m_dataSize += size;
	return size;
}

int Packet::GetData(char* desData, int size)
{
	memcpy(desData, m_bufferPtr + m_front + 1, size);
	m_front = (m_front + size) % m_bufferSize;
	m_dataSize -= size;
	return size;
}

int Packet::MoveWritePos(unsigned int size)
{
	m_rear = (m_rear + size) % m_bufferSize;
	m_dataSize += size;
	return size;
}

int Packet::MoveReadPos(unsigned int size)
{
	m_front = (m_front + size) % m_bufferSize;
	m_dataSize -= size;
	return size;
}

Packet& Packet::operator=(Packet& srcPacket)
{
	memcpy(m_bufferPtr, &srcPacket, srcPacket.GetDataSize());
	return *this;
}

Packet& Packet::operator<<(unsigned char value)
{
	memcpy(m_bufferPtr + m_rear + 1, &value, sizeof(unsigned char));
	m_rear = (m_rear + sizeof(unsigned char)) % m_bufferSize;
	m_dataSize += sizeof(unsigned char);
	return *this;
}

Packet& Packet::operator<<(char value)
{
	memcpy(m_bufferPtr + m_rear + 1, &value, sizeof(char));
	m_rear = (m_rear + sizeof(char)) % m_bufferSize;
	m_dataSize += sizeof(char);
	return *this;
}

Packet& Packet::operator<<(unsigned short value)
{
	memcpy(m_bufferPtr + m_rear + 1, &value, sizeof(unsigned short));
	m_rear = (m_rear + sizeof(unsigned short)) % m_bufferSize;
	m_dataSize += sizeof(unsigned short);
	return *this;
}

Packet& Packet::operator<<(short value)
{
	memcpy(m_bufferPtr + m_rear + 1, &value, sizeof(short));
	m_rear = (m_rear + sizeof(short)) % m_bufferSize;
	m_dataSize += sizeof(short);
	return *this;
}

Packet& Packet::operator<<(DWORD value)
{
	memcpy(m_bufferPtr + m_rear + 1, &value, sizeof(long));
	m_rear = (m_rear + sizeof(long)) % m_bufferSize;
	m_dataSize += sizeof(long);
	return *this;
}

Packet& Packet::operator<<(int value)
{
	memcpy(m_bufferPtr + m_rear + 1, &value, sizeof(int));
	m_rear = (m_rear + sizeof(int)) % m_bufferSize;
	m_dataSize += sizeof(int);
	return *this;
}

Packet& Packet::operator<<(float value)
{
	memcpy(m_bufferPtr + m_rear + 1, &value, sizeof(float));
	m_rear = (m_rear + sizeof(float)) % m_bufferSize;
	m_dataSize += sizeof(float);
	return *this;
}

Packet& Packet::operator<<(__int64 value)
{
	memcpy(m_bufferPtr + m_rear + 1, &value, sizeof(__int64));
	m_rear = (m_rear + sizeof(__int64)) % m_bufferSize;
	m_dataSize += sizeof(__int64);
	return *this;
}

Packet& Packet::operator<<(double value)
{
	memcpy(m_bufferPtr + m_rear + 1, &value, sizeof(double));
	m_rear = (m_rear + sizeof(double)) % m_bufferSize;
	m_dataSize += sizeof(double);
	return *this;
}

Packet& Packet::operator>>(BYTE& value)
{
	memcpy(&value, m_bufferPtr + m_front + 1, sizeof(BYTE));
	m_front = (m_front + sizeof(BYTE)) % m_bufferSize;
	m_dataSize -= sizeof(BYTE);
	return *this;
}

Packet& Packet::operator>>(char& value)
{
	memcpy(&value, m_bufferPtr + m_front + 1, sizeof(char));
	m_front = (m_front + sizeof(char)) % m_bufferSize;
	m_dataSize -= sizeof(char);
	return *this;
}

Packet& Packet::operator>>(WORD& value)
{
	memcpy(&value, m_bufferPtr + m_front + 1, sizeof(WORD));
	m_front = (m_front + sizeof(WORD)) % m_bufferSize;
	m_dataSize -= sizeof(WORD);
	return *this;
}

Packet& Packet::operator>>(short& value)
{
	memcpy(&value, m_bufferPtr + m_front + 1, sizeof(short));
	m_front = (m_front + sizeof(short)) % m_bufferSize;
	m_dataSize -= sizeof(short);
	return *this;
}

Packet& Packet::operator>>(DWORD& value)
{
	memcpy(&value, m_bufferPtr + m_front + 1, sizeof(DWORD));
	m_front = (m_front + sizeof(DWORD)) % m_bufferSize;
	m_dataSize -= sizeof(DWORD);
	return *this;
}

Packet& Packet::operator>>(int& value)
{
	memcpy(&value, m_bufferPtr + m_front + 1, sizeof(int));
	m_front = (m_front + sizeof(int)) % m_bufferSize;
	m_dataSize -= sizeof(int);
	return *this;
}

Packet& Packet::operator>>(float& value)
{
	memcpy(&value, m_bufferPtr + m_front + 1, sizeof(float));
	m_front = (m_front + sizeof(float)) % m_bufferSize;
	m_dataSize -= sizeof(float);
	return *this;
}

Packet& Packet::operator>>(__int64& value)
{
	memcpy(&value, m_bufferPtr + m_front + 1, sizeof(__int64));
	m_front = (m_front + sizeof(__int64)) % m_bufferSize;
	m_dataSize -= sizeof(__int64);
	return *this;
}

Packet& Packet::operator>>(double& value)
{
	memcpy(&value, m_bufferPtr + m_front + 1, sizeof(double));
	m_front = (m_front + sizeof(double)) % m_bufferSize;
	m_dataSize -= sizeof(double);
	return *this;
}
