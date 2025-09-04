#include <iostream>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include "Protocol.h"
#include "Console.h"
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "ws2_32")

#define SERVER_PORT 3000
#define MAX_CLIENTS 30

struct STAR
{
	int id;
	int xpos;
	int ypos;
	bool isAlive;
};

// 전역 변수
SOCKET sock;
wchar_t addrIP[16];

STAR stars[MAX_CLIENTS];
STAR* myStar;

int playerNum = 0;
char backBuffer[SCREEN_HEIGHT][SCREEN_WIDTH + 1];

// 에러 저장 전역 변수
int retval;
int retval_WSA;
int retval_sock;
int retval_conn;
int retval_trans;
int retval_select;
int retval_send;
int retval_recv;

bool keyInput();
bool setClientSocket();
bool connectServer();
bool transBlockingOpt();
bool selecting();
void flip();
void spriteDraw(int, int, char);
void clear();

int main()
{
	timeBeginPeriod(1);

	// 콘솔 초기화
	consoleInit();

	// 접속할 서버 입력
	printf("접속할 IP를 입력하세요: ");
	wscanf_s(L"%s", addrIP, 16);
	addrIP[wcslen(addrIP)] = '\0';

	// 소켓 생성
	if (!setClientSocket())
		return -1;

	// 서버에 연결
	if (!connectServer())
		return -1;

	// 논블로킹 소켓으로 전환
	if (!transBlockingOpt())
		return -1;

	int oldTick = timeGetTime();
	bool isSkip = false;

	// 로직
	while (1)
	{
		// 키입력
		if (!keyInput())
			return -1;

		// 네트워크 처리
		if (!selecting())
			return -1;

		clear();
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (stars[i].isAlive == true)
			{
				spriteDraw(stars[i].xpos, stars[i].ypos, '*');
			}
		}
		flip();

		Sleep(30);
	}

	closesocket(sock);
	WSACleanup();

	timeEndPeriod(1);

	return 0;
}

// 소켓 생성
bool setClientSocket()
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		retval_WSA = GetLastError();
		return false;
	}

	// socket()
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		retval_sock = GetLastError();
		closesocket(sock);
		return false;
	}

	return true;
}

// 서버에 접속
bool connectServer()
{
	// connect()
	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	InetPton(AF_INET, addrIP, &addr.sin_addr);
	retval = connect(sock, (SOCKADDR*)&addr, sizeof(addr));
	if (retval == SOCKET_ERROR)
	{
		retval_conn = GetLastError();
		closesocket(sock);
		return false;
	}

	return true;
}

// 논블로킹 소켓으로 전환
bool transBlockingOpt()
{
	// ioctlsocket()
	u_long blockingMode = 1;
	retval = ioctlsocket(sock, FIONBIO, &blockingMode);
	if (retval == SOCKET_ERROR)
	{
		retval_trans = GetLastError();
		closesocket(sock);
		return false;
	}

	return true;
}

// 키입력
bool keyInput()
{
	// 내 별 있는지 확인
	if (myStar == nullptr)
	{
		return true;
	}

	// 키입력 확인
	int nextX;
	int nextY;
	BOOL isMove = false;

	if (GetAsyncKeyState(VK_UP) & 0x8001)
	{
		nextY = myStar->ypos - 1;
		if (nextY >= 0 && nextY < SCREEN_HEIGHT)
		{
			myStar->ypos--;
			isMove = true;
		}
	}

	if (GetAsyncKeyState(VK_DOWN) & 0x8001)
	{
		nextY = myStar->ypos + 1;
		if (nextY >= 0 && nextY < SCREEN_HEIGHT)
		{
			myStar->ypos++;
			isMove = true;
		}
	}

	if (GetAsyncKeyState(VK_RIGHT) & 0x8001)
	{
		nextX = myStar->xpos + 1;
		if (nextX >= 0 && nextX < SCREEN_WIDTH)
		{
			myStar->xpos++;
			isMove = true;
		}
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8001)
	{
		nextX = myStar->xpos - 1;
		if (nextX >= 0 && nextX < SCREEN_WIDTH)
		{
			myStar->xpos--;
			isMove = true;
		}
	}

	// 키입력 있으면 변경된 위치 send()
	if (isMove)
	{
		STAR_MOVE msg;
		msg.type = static_cast<int>(MSG_TYPE::STAR_MOVE);
		msg.id = myStar->id;
		msg.xpos = myStar->xpos;
		msg.ypos = myStar->ypos;
		retval = send(sock, (const char*)&msg, 16, 0);
		if (retval == SOCKET_ERROR)
		{
			if (GetLastError() != WSAEWOULDBLOCK)
			{
				retval_send = GetLastError();
				closesocket(sock);
				return false;
			}
		}
	}

	return true;
}

// select()
bool selecting()
{
	FD_SET readSet;
	char buffer[16];

	// 셋 초기화
	FD_ZERO(&readSet);
	FD_SET(sock, &readSet);

	// 대기시간 설정
	timeval time;
	time.tv_sec = 0;
	time.tv_usec = 0;

	// select()
	retval = select(0, &readSet, nullptr, nullptr, &time);
	if (retval == SOCKET_ERROR)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			retval_select = GetLastError();
			closesocket(sock);
			return false;
		}
	}

	// readset 검사
	if (FD_ISSET(sock, &readSet) != 0)
	{
		while (1)
		{
			retval = recv(sock, buffer, 16, 0);
			if (retval == SOCKET_ERROR)
			{
				if (GetLastError() == WSAEWOULDBLOCK)
				{
					break;
				}
				retval_recv = GetLastError();
				closesocket(sock);
				return false;
			}
			else if (retval != 16) // retval == 0인 경우까지 처리
			{
				retval_recv = retval;
				closesocket(sock);
				return false;
			}

			int type = *(int*)buffer;
			switch (static_cast<MSG_TYPE>(type))
			{
			case MSG_TYPE::ID_ALLOC:
			{
				ID_ALLOC* idAlloc = (ID_ALLOC*)buffer;
				if (stars[0].isAlive == false)
				{
					myStar = &stars[0];
					myStar->id = idAlloc->id;
				}
				break;
			}
			case MSG_TYPE::STAR_CREATE:
			{
				STAR_CREATE* starCreate = (STAR_CREATE*)buffer;
				if ((myStar != nullptr) && (starCreate->id == myStar->id))
				{
					myStar->xpos = starCreate->xpos;
					myStar->ypos = starCreate->ypos;
					myStar->isAlive = true;
					break;
				}
				for (int i = 1; i < MAX_CLIENTS; i++)
				{
					if (stars[i].isAlive == false)
					{
						stars[i].id = starCreate->id;
						stars[i].xpos = starCreate->xpos;
						stars[i].ypos = starCreate->ypos;
						stars[i].isAlive = true;
						break;
					}
				}
				break;
			}
			case MSG_TYPE::STAR_DELETE:
			{
				STAR_DELETE* starDelete = (STAR_DELETE*)buffer;
				for (int i = 1; i < MAX_CLIENTS; i++)
				{
					if (stars[i].id == starDelete->id)
					{
						stars[i].isAlive = false;
						break;
					}
				}
				break;
			}
			case MSG_TYPE::STAR_MOVE:
			{
				STAR_MOVE* starMove = (STAR_MOVE*)buffer;
				if (starMove->xpos < 0 || starMove->xpos >= SCREEN_WIDTH || starMove->ypos < 0 || starMove->ypos >= SCREEN_HEIGHT)
					return true;
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (stars[i].id == starMove->id)
					{
						stars[i].xpos = starMove->xpos;
						stars[i].ypos = starMove->ypos;
						break;
					}
				}
				break;
			}
			default:
				// 에러처리
				closesocket(sock);
				return false;
			}
		}
	}

	return true;
}

void flip()
{
	for (int yCnt = 0; yCnt < SCREEN_HEIGHT; yCnt++)
	{
		// 커서를 각 줄의 첫 번째 위치로 이동
		moveCursor(0, yCnt);

		// 처음부터 문자열 한 줄씩 화면에 출력
		printf("%s", backBuffer[yCnt]);
	}
}

void spriteDraw(int x, int y, char sprite)
{
	// 요청받은 위치가 유효하지 않은 좌표인 경우
	if (x < 0 || y < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
		return;

	// 유효한 좌표일 경우 sprite 저장
	backBuffer[y][x] = sprite;
}

void clear()
{
	// 백 버퍼를 공백으로 채워 비움
	memset(backBuffer, ' ', SCREEN_HEIGHT * (SCREEN_WIDTH + 1));

	// 비운 버퍼의 각 줄 끝에 NULL 삽입
	for (int yCnt = 0; yCnt < SCREEN_HEIGHT; yCnt++)
	{
		backBuffer[yCnt][SCREEN_WIDTH] = '\0';
	}
}