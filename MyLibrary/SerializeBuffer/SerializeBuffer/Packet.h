#ifndef __PACKET__
#define __PACKET__

#include <Windows.h>
#define DEFUALT_BUFSIZE 1000

class Packet
{
public:
	Packet();
	Packet(int);
	~Packet();

	void Clear();

	int GetBufferSize();
	int GetDataSize();

	// ���� ���� �ּ� ������ ��ȯ
	char* GetBufferPtr();

	// ������ ���� ���� �� ����
	int GetData(char*, int);
	int PutData(char*, int);

	// �̵��� ������ ��ȯ
	int MoveWritePos(unsigned int);
	int MoveReadPos(unsigned int);

	// �ֱ� ������ �����ε�
	Packet& operator=(Packet&);

	Packet& operator<<(unsigned char);
	Packet& operator<<(char);

	Packet& operator<<(unsigned short);
	Packet& operator<<(short);

	Packet& operator<<(DWORD);
	Packet& operator<<(int);
	Packet& operator<<(float);

	Packet& operator<<(__int64);
	Packet& operator<<(double);

	// ���� ������ �����ε�
	Packet& operator>>(BYTE&);
	Packet& operator>>(char&);

	Packet& operator>>(WORD&);
	Packet& operator>>(short&);

	Packet& operator>>(DWORD&);
	Packet& operator>>(int&);
	Packet& operator>>(float&);

	Packet& operator>>(__int64&);
	Packet& operator>>(double&);

private:
	char* m_bufferPtr;
	int m_bufferSize;
	int m_dataSize;

	int m_front;
	int m_rear;
};

#endif