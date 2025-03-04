#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include "myList.h"
#include "Protocol.h"
#include "Console.h"
#include "RingBuffer.h"
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "ws2_32.lib")

#define PACKET_SIZE 16
#define SERVER_PORT 3000
#define MAX_SIZE 16

// 전역 변수
SOCKET listen_sock;
int retval;
int retval_wsa;
int retval_sock;
int retval_opt;
int retval_bind;
int retval_listen;
int retval_trans;
int retval_accept;
int retval_send;
int retval_recv;
int retval_select;
int uniqueId = 0;
char backBuffer[SCREEN_HEIGHT][SCREEN_WIDTH + 1];

// Player 구조체
struct PLAYER
{
	SOCKET sock;
	wchar_t ipAddr[16];
	int port;
	int id;
	int xpos;
	int ypos;
	RingBuffer recvQ; // recvRingBuffer
	RingBuffer sendQ; // sendRingBuffer
};
myList<PLAYER*> players;
myList<PLAYER*> disconnects;

bool Networking();
void AcceptProc();
void RecvProc(PLAYER*);
void SendProc(PLAYER*);
void SendUnicast(PLAYER*, char*);
void SendBroadcast(PLAYER*, char*);
void Disconnect(PLAYER*);
void DeleteUser();

void Rendering();
void flip();
void spriteDraw(int, int, char);
void clear();

int main()
{
	timeBeginPeriod(1);

	consoleInit();

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		retval_wsa = GetLastError();
		return -1;
	}

	// 리슨 소켓 생성
	listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET)
	{
		retval_sock = GetLastError();
		closesocket(listen_sock);
		return -1;
	}


	// 링거 옵션 설정
	linger closeOpt;
	closeOpt.l_onoff = 1;
	closeOpt.l_linger = 0;
	retval = setsockopt(listen_sock, SOL_SOCKET, SO_LINGER, (char*)&closeOpt, sizeof(closeOpt));
	if (retval == SOCKET_ERROR)
	{
		retval_opt = GetLastError();
		closesocket(listen_sock);
		return -1;
	}

	// 리슨 소켓 바인딩
	SOCKADDR_IN servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVER_PORT);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR*)&servAddr, sizeof(servAddr));
	if (retval == SOCKET_ERROR)
	{
		retval_bind = GetLastError();
		closesocket(listen_sock);
		return -1;
	}

	// 리슨
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		retval_listen = GetLastError();
		closesocket(listen_sock);
		return -1;
	}

	// 논블로킹 소켓으로 전환
	u_long nonblock = 1;
	retval = ioctlsocket(listen_sock, FIONBIO, &nonblock);
	if (retval == SOCKET_ERROR)
	{
		retval_trans = GetLastError();
		closesocket(listen_sock);
		return -1;
	}

	// 서버 로직
	while (1)
	{
		// 네트워크()
		if (!Networking())
			break;

		// 지연 삭제()
		DeleteUser();

		Rendering();

		Sleep(20);
	}

	players.clear();
	disconnects.clear();

	closesocket(listen_sock);
	WSACleanup();

	timeEndPeriod(1);

	return 0;
}

bool Networking()
{
	FD_SET readSet, writeSet;
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	FD_SET(listen_sock, &readSet);

	// 플레이어 리스트에 있는 유저 검사 대상으로 설정
	myList<PLAYER*>::iterator iter;
	for (iter = players.begin(); iter != players.end(); iter++)
	{
		FD_SET((*iter)->sock, &readSet);
		if ((*iter)->sendQ.GetUsedSize() > 0) // 보낼 데이터가 있으면 검사 대상으로 지정
			FD_SET((*iter)->sock, &writeSet);
	}

	// 타임아웃: 요청한 기능을 못할 때 최대 대기시간
	// timeval = NULL로 설정하는 이유: 어차피 서버 로직 없으니까 항상 기다리고 있어도 괜찮음
	retval = select(0, &readSet, nullptr, nullptr, NULL); // 무한 대기
	if (retval == SOCKET_ERROR)
	{
		return false;
	}

	// 리슨 소켓에 들어온 클라이언트를 유저로 승격
	if (FD_ISSET(listen_sock, &readSet))
	{
		AcceptProc();
	}

	// 플레이어 데이터 처리
	for (iter = players.begin(); iter != players.end(); iter++)
	{
		if (FD_ISSET((*iter)->sock, &readSet))
		{
			RecvProc(*iter);
		}

		if (FD_ISSET((*iter)->sock, &writeSet))
		{
			SendProc(*iter);
		}
	}

	return true;
}

void AcceptProc()
{
	// accept()
	SOCKET client_sock;
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	client_sock = accept(listen_sock, (SOCKADDR*)&clientAddr, &addrLen);
	if (client_sock == INVALID_SOCKET)
	{
		retval_accept = GetLastError();
		closesocket(client_sock);
		return;
	}

	// Player 생성 세팅 ID 부여
	PLAYER* newPlayer = new PLAYER;
	newPlayer->sock = client_sock;
	InetNtop(AF_INET, &clientAddr.sin_addr, newPlayer->ipAddr, 16);
	newPlayer->port = ntohs(clientAddr.sin_port);
	newPlayer->id = uniqueId;
	newPlayer->xpos = SCREEN_WIDTH / 2;
	newPlayer->ypos = SCREEN_HEIGHT / 2;

	// ID 할당 메세지 Unicast send
	ID_ALLOC allocMsg;
	allocMsg.type = (int)MSG_TYPE::ID_ALLOC;
	allocMsg.id = newPlayer->id;
	SendUnicast(newPlayer, (char*)&allocMsg);

	// 기존 접속자 별 생성 메세지 Unicast send
	STAR_CREATE createMsg;
	myList<PLAYER*>::iterator iter;
	for (iter = players.begin(); iter != players.end(); iter++)
	{
		createMsg.type = (int)MSG_TYPE::STAR_CREATE;
		createMsg.id = (*iter)->id;
		createMsg.xpos = (*iter)->xpos;
		createMsg.ypos = (*iter)->ypos;
		SendUnicast(newPlayer, (char*)&createMsg);
	}

	// Player 리스트에 추가
	players.push_back(newPlayer);

	// 모두에게 새로운 접속자 별 생성 메세지 Broadcast send
	createMsg.type = (int)MSG_TYPE::STAR_CREATE;
	createMsg.id = newPlayer->id;
	createMsg.xpos = newPlayer->xpos;
	createMsg.ypos = newPlayer->ypos;
	SendBroadcast(nullptr, (char*)&createMsg);

	uniqueId++;
}

void SendUnicast(PLAYER* p, char* buffer)
{
	if (p->sendQ.GetFreeSize() > PACKET_SIZE)
	{
		retval = p->sendQ.Enqueue(buffer, PACKET_SIZE);
		if (retval != PACKET_SIZE)
		{
			// 로그 남기기
		}
	}
}

void SendBroadcast(PLAYER* p, char* buffer)
{
	myList<PLAYER*>::iterator iter;
	for (iter = players.begin(); iter != players.end(); iter++)
	{
		if (p != (*iter))
		{
			if (p->sendQ.GetFreeSize() > PACKET_SIZE)
			{
				retval = p->sendQ.Enqueue(buffer, PACKET_SIZE);
				if (retval != PACKET_SIZE)
				{
					// if (ret != size / ret = false) 면 어떻게 할지 생각해보기 -> ret != size == 송신버퍼가 이미 가득 참
					// 저장하고 남은 데이터를 저장? 아니면 다음 프레임에 다시 저장?
					// peek?
				}
			}
		}
	}
}


void RecvProc(PLAYER* p) // -> 플레이어 리스트 순환 중
{
	// 플레이어 포인터 확인
	if (p == nullptr)
	{
		return;
	}

	// 플레이어 recvQ에 남은 공간이 없다 -> dequeue 불가능한 데이터로 가득 찼다 -> 비정상 상태 연결 종료
	if (p->recvQ.GetFreeSize() == 0)
	{
		Disconnect(p);
		return;
	}

	char buffer[PACKET_SIZE];

	int enqueueSize;
	if (p->recvQ.GetFreeSize() < PACKET_SIZE) // 링버퍼에 남은 공간이 메세지 크기보다 작을 때
	{
		enqueueSize = p->recvQ.GetFreeSize();
	}
	else
	{
		enqueueSize = PACKET_SIZE;
	}

	retval = recv(p->sock, buffer, enqueueSize, 0);
	if (retval == SOCKET_ERROR)
	{
		if (GetLastError() == WSAEWOULDBLOCK) // 더이상 읽을 데이터x면 복귀
			return;
		retval_recv = GetLastError();
		Disconnect(p);
		return;
	}
	else if (retval != 16) // retval == 0이거나 이상한 길이면 연결 종료
	{
		Disconnect(p);
		return;
	}

	retval = p->recvQ.Enqueue(buffer, enqueueSize);
	if (retval);

	while (1)
	{
		if (p->recvQ.GetUsedSize() < PACKET_SIZE)
		{
			break;
		}

		char message[PACKET_SIZE];

		retval = p->recvQ.Dequeue(message, PACKET_SIZE);
		if (retval != 16)
		{
			// 덤프 뜨기
			return;
		}

		int type = *(int*)message;
		switch (static_cast<MSG_TYPE>(type))
		{
		case MSG_TYPE::STAR_MOVE:
		{
			// 이동처리
			STAR_MOVE* moveMsg = (STAR_MOVE*)message;
			if (p->id != moveMsg->id) // 이동 통보 대상과 해당 대상의 id가 다른 경우 벤
			{
				Disconnect(p);
				break;
			}

			// 좌표 유효성 확인 한번더?

			p->xpos = moveMsg->xpos;
			p->ypos = moveMsg->ypos;

			// 이동된 좌표 브로드캐스팅
			SendBroadcast(p, (char*)moveMsg);
			break;
		}
		default:
			Disconnect(p);
			break;
		}
	}
}

void SendProc(PLAYER* p)
{
	if (p == nullptr)
	{
		return;
	}

	char buffer[PACKET_SIZE];

	while (1)
	{
		// 메세지 크기보다 적은 데이터가 링버퍼에 있는 경우 -> 아직 덜 저장됨
		if (p->sendQ.GetUsedSize() < PACKET_SIZE)
		{
			return;
		}

		p->sendQ.Dequeue(buffer, PACKET_SIZE);

		retval = send(p->sock, buffer, PACKET_SIZE, 0);
		if (retval == SOCKET_ERROR)
		{
			// 에러처리
		}

	}
	// ret = sendQ peek(buffer, MAX_SIZE) -> 이건 할 수 있는 만큼 가져오는 코드
	// s = send(buffer, ret)
	// if (s != ret)
	// sendQ.moveFront(s);
}

void Disconnect(PLAYER* p)
{
	// 플레이어 리스트에서 해당 플레이어 제거 -> 문제: RecvProc에서 리스트 이터레이터 순환 중 -> 댕글링 포인터 발생
	// 여기서 제거를 안하고 나중에 한번에 삭제 -> 지연 삭제
	disconnects.push_back(p);

	// 해당 플레이어 별 삭제 메세지 SendBroadcast -> 이 함수 안에서도 Disconnect가 일어날 수 있음 -> 재귀 발생
	STAR_DELETE deleteMsg;
	deleteMsg.type = (int)MSG_TYPE::STAR_DELETE;
	deleteMsg.id = p->id;
	SendBroadcast(p, (char*)&deleteMsg); // 해당 플레이어는 이미 연결 해제라 제외
}

void DeleteUser()
{
	myList<PLAYER*>::iterator iter;
	for (iter = disconnects.begin(); iter != disconnects.end(); ++iter)
	{
		closesocket((*iter)->sock);
		players.remove((*iter));
	}
	disconnects.clear();
}

void Rendering()
{
	clear();
	char clientNum[30] = "Connected User: \0";
	myList<PLAYER*>::iterator iter;
	for (iter = players.begin(); iter != players.end(); ++iter)
	{
		spriteDraw((*iter)->xpos, (*iter)->ypos, '*');
	}
	snprintf(&clientNum[(int)strlen(clientNum)], strlen(clientNum), "%d", players.size());
	memcpy(&backBuffer[0][0], clientNum, strlen(clientNum));
	flip();
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

// 릴리즈말고 디버그로 왜? 댕글링 포인터 에러가 안나서 디버그는 할당 해제한 메모리를 0xdd 다 밀어주니까