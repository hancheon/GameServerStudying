#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <locale.h>
#pragma comment(lib, "ws2_32")

#define SERVER_PORT 9000
#define BUFSIZE 100

int wmain()
{
    setlocale(LC_ALL, "Korean");

    int retval;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return -1;

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
    {
        printf("%d\n", GetLastError());
        return -1;
    }

    BOOL bEnable = TRUE;
    retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable));
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", GetLastError());
        return -1;
    }

    SOCKADDR_IN servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVER_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    
    SOCKADDR_IN echoAddr;
    int addrLen = sizeof(echoAddr);
    wchar_t buffer[BUFSIZE];
    int len;

    while (1)
    {
        printf("보낼 데이터: ");
        if (fgetws(buffer, BUFSIZE, stdin) == NULL)
            break;

        len = wcslen(buffer);
        if (buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';
        if (wcslen(buffer) == NULL)
            break;

        retval = sendto(sock, (char*)buffer, len * 2, 0, (SOCKADDR*)&servAddr, sizeof(servAddr));
        if (retval == SOCKET_ERROR)
        {
            printf("%d", GetLastError());
            break;
        }

        retval = recvfrom(sock, (char*)buffer, BUFSIZE, 0, (SOCKADDR*)&echoAddr, &addrLen);
        if (retval == SOCKET_ERROR)
        {
            printf("%d", GetLastError());
            break;
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
