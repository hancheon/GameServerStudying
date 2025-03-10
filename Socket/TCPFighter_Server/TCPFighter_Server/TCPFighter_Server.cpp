#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <locale.h>
#include "Protocol.h"
#include "Session.h"
#include "Network.h"
#include "Packet.h"
#include "Contents.h"
#include "myList.h"
#pragma comment(lib, "Winmm")
#pragma comment(lib, "ws2_32")

// 전역변수
SOCKET listen_sock;
int oldTick;

// 전방선언
bool initServer();

int wmain()
{
    timeBeginPeriod(1);
    setlocale(LC_ALL, "Korean");

    // 서버 세팅
    if (!initServer())
    {
        closesocket(listen_sock);
        return -1;
    }

    oldTick = timeGetTime();
    // 서버 로직
    while (1)
    {
        if (!networkProc())
            break;
        
        if (!update())
            break;
    }

    // 서버 종료
    closesocket(listen_sock);
    WSACleanup();
    timeEndPeriod(1);

    return 0;
}

bool initServer()
{
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return false;

    // 소켓 생성
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET)
    {
        printf("%d\n", GetLastError());
        return false;
    }

    LINGER lin;
    lin.l_linger = 0;
    lin.l_onoff = 1;
    retval = setsockopt(listen_sock, SOL_SOCKET, SO_LINGER, (char*)&lin, sizeof(lin));
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", GetLastError());
        return false;
    }

    // 바인딩
    SOCKADDR_IN servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVER_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    retval = bind(listen_sock, (SOCKADDR*)&servAddr, sizeof(servAddr));
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", GetLastError());
        return false;
    }

    // 리슨
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", GetLastError());
        return false;
    }

    // 논블로킹 소켓으로 전환
    u_long nonblock = 1;
    retval = ioctlsocket(listen_sock, FIONBIO, &nonblock);
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", GetLastError());
        return false;
    }

    return true;
}