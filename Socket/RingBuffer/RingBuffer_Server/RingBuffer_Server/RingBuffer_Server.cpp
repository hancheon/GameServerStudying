#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <list>
#include "Console.h"
#pragma comment(lib, "ws2_32.lib")

#define MAX_RECV 400
#define MAX_SEND 400
#define SERVER_PORT 3000

struct SESSION
{
    int id;
    SOCKET client_sock;
    char ipAddr[16];
    int port;
    char recvQ[MAX_RECV];
};

// 전역 변수
SOCKET listen_sock;
std::list<SESSION*> sessions;
std::list<SESSION*> disconnects;

// 전방 선언
void ErrorHandling(int); // ErrorCode
bool InitServerSocket();
bool Listen();
bool SelectProc();
void Rendering();

int main()
{
    // 서버 리슨 소켓 세팅
    if (!InitServerSocket())
        return -1;

    // 리슨 및 논블로킹 소켓으로 전환
    if (!Listen())
        return -1;

    // 로직
    while (1)
    {
        if (!SelectProc())
            return -1;

        Rendering();

        Sleep(20);
    }

}

bool InitServerSocket()
{
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        ErrorHandling(GetLastError());
        return false;
    }

    // 리슨 소켓 생성
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET)
    {
        ErrorHandling(GetLastError());
        return false;
    }

    // 리슨 소켓 바인딩
    SOCKADDR_IN servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = ntohs(SERVER_PORT);
    servAddr.sin_addr.s_addr = ntohl(INADDR_ANY);
    retval = bind(listen_sock, (SOCKADDR*)&servAddr, sizeof(servAddr));
    if (retval == SOCKET_ERROR)
    {
        ErrorHandling(GetLastError());
        return false;
    }

    return true;
}

bool SelectProc()
{
    FD_SET readSet, writeSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);

    FD_SET(listen_sock, &readSet);
    std::list<SESSION*>::iterator iter;
    for (iter = sessions.begin(); iter != sessions.end(); iter++)
    {
        FD_SET((*iter)->client_sock, &readSet);
        if ((*iter)->recvQ)
    }
}

void Rendering()
{
    // 윈도우 프로그램

}

bool Listen()
{
    int retval;

    // 리슨
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR)
    {
        ErrorHandling(GetLastError());
        return false;
    }

    // 논블로킹 소켓으로 전환
    u_long blockingOpt = 1;
    retval = ioctlsocket(listen_sock, FIONBIO, &blockingOpt);
    if (retval == SOCKET_ERROR)
    {
        ErrorHandling(GetLastError());
        return false;
    }

    return true;
}

void ErrorHandling(int errorCode)
{
    char fileName[30] = "ErrorLog_";
    
    tm today;
    time_t timer;

    timer = time(NULL);
    localtime_s(&today, &timer);
    sprintf_s(fileName, "%d%d%d_%d%d%d.txt", today.tm_year + 1900, today.tm_mon + 1, today.tm_mday, today.tm_hour, today.tm_min, today.tm_sec);
    
    FILE* fd;
    if (fopen_s(&fd, fileName, "wb"))
        return;

    fprintf_s(fd, "%d\n", errorCode);

    fclose(fd);
}
