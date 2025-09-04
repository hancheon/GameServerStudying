#include <iostream>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <locale.h>
#pragma comment(lib, "ws2_32")

#define SERVERPORT 9001
#define BUFSIZE 512

int wmain(int argc, wchar_t** argv)
{
    // 유니코드 한글 설정
    _wsetlocale(LC_ALL, L"korean");

    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket
    SOCKET listen_sock;
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET)
    {
        wprintf(L"%d\n", GetLastError());
        closesocket(listen_sock);
        return -1;
    }

    // bind
    SOCKADDR_IN servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVERPORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    retval = bind(listen_sock, (SOCKADDR*)&servAddr, sizeof(servAddr));
    if (retval == SOCKET_ERROR)
    {
        wprintf(L"%d\n", GetLastError());
        closesocket(listen_sock);
        return -1;
    }

    // listen
    retval = listen(listen_sock, SOMAXCONN_HINT(65535));
    if (retval == SOCKET_ERROR)
    {
        wprintf(L"%d\n", GetLastError());
        closesocket(listen_sock);
        return -1;
    }
    wprintf(L"클라이언트 연결 대기중\n");


    while (1)
    {
        // accept를 하지 않아 연결 요청이 Backlog Queue에 쌓이도록 함
    }

    closesocket(listen_sock);

    WSACleanup();
    return 0;
}