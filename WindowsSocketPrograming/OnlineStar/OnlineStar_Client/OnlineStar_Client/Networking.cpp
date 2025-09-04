#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Networking.h"

extern int retval_sock;
extern int retval_conn;
extern int retval_trans;

// 소켓 생성
bool setClientSocket(SOCKET sock)
{
    // 윈속 초기화
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

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
bool connectServer(SOCKET sock, wchar_t* addrIP)
{
    // connect()
    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    InetPton(AF_INET, addrIP, &addr.sin_addr);
    int retval = connect(sock, (SOCKADDR*)&addr, sizeof(addr));
    if (retval == SOCKET_ERROR)
    {
        retval_conn = GetLastError();
        closesocket(sock);
        return false;
    }

    return true;
}

// 논블로킹 소켓으로 전환
bool transBlockingOpt(SOCKET sock)
{
    // ioctlsocket()
    u_long blockingMode = 1;
    int retval = ioctlsocket(sock, FIONBIO, &blockingMode);
    if (retval == SOCKET_ERROR)
    {
        retval_trans = GetLastError();
        closesocket(sock);
        return false;
    }

    return true;
}

bool networking()
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