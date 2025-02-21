#include <iostream>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include "OnlineStar_Client.h"
#pragma comment(lib, "ws2_32")

#define SERVER_PORT 3000

// 전역 변수
SOCKET sock;
PLAYER* players[20];
PLAYER* p;
int playerNum = 0;
int fps;

void keyInput();
int networking();
void rendering();

int main()
{
    timeBeginPeriod(1);

    // 에러 저장 변수
    int retval;
    int retval_sock;
    int retval_conn;
    int retval_select;
    
    // 윈속 초기화
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // socket()
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        retval_sock = GetLastError();
        closesocket(sock);
        return -1;
    }

    // connect()
    SOCKADDR_IN addr;
    wchar_t addrIP[30];
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    printf("접속할 IP를 입력하세요: ");
    fgetws(addrIP, 30, stdin);
    InetPton(AF_INET, addrIP, &addr.sin_addr);
    retval = connect(sock, (SOCKADDR*)&addr, sizeof(addr));
    if (retval == SOCKET_ERROR)
    {
        retval_conn = GetLastError();
        closesocket(sock);
        return -1;
    }

    // 논블로킹 소켓으로 전환
    u_long blockingMode = 1;
    retval = ioctlsocket(sock, FIONBIO, &blockingMode);

    // 로직
    while (1)
    {
        // 키입력
        keyInput();
        
        // 네트워크 처리
        retval = networking();
        if (retval != 0)
        {
            closesocket(sock);
            return -1;
        }

        // 랜더링

        // 프레임 처리 (약 100프레임?)
        Sleep();
    }

    closesocket(sock);
    WSACleanup();
    timeEndPeriod(1);

    return 0;
}

void keyInput()
{
    // 내 별 있는지 확인
    if (p == nullptr)
    {
        return;
    }

    if (GetAsyncKeyState(VK_UP))
    {
        p->ypos++;
    }

    if (GetAsyncKeyState(VK_DOWN))
    {
        p->ypos--;
    }

    if (GetAsyncKeyState(VK_RIGHT))
    {
        p->xpos++;
    }

    if (GetAsyncKeyState(VK_LEFT))
    {
        p->ypos--;
    }
}

int networking()
{
    // 셋 초기화
    FD_SET readSet;
    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);

    // 대기시간 X
    timeval time;
    time.tv_sec = 0;
    time.tv_usec = 0;

    // select()
    int retval = select(0, &readSet, nullptr, nullptr, &time);
    if ((retval == SOCKET_ERROR) && (GetLastError() != WSAEWOULDBLOCK))
    {
        return GetLastError();
    }

    char buffer[16];

    while (1)
    {
        retval = recv(sock, buffer, 16, 0);
        if (retval == SOCKET_ERROR)
        {

        }
        else if (retval == 0)
        {

        }
    }
}

void rendering()
{
    printf("Connect Client : %d / FPS : %d\n", playerNum, fps);
}