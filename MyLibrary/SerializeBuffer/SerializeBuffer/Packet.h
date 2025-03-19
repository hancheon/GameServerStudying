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

	// 버퍼 시작 주소 포인터 반환
	char* GetBufferPtr();

	// 데이터 직접 삽입 및 추출
	int GetData(char*, int);
	int PutData(char*, int);

	// 이동한 사이즈 반환
	int MoveWritePos(unsigned int);
	int MoveReadPos(unsigned int);

	// 넣기 연산자 오버로딩
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

	// 빼기 연산자 오버로딩
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