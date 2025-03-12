#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <locale.h>
#pragma comment(lib, "ws2_32")

#define SERVER_PORT 9000
#define BUFSIZE 100

int main()
{
    setlocale(LC_ALL, "Korean");

    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return -1;

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
    {
        printf("%d\n", GetLastError());
        return -1;
    }

    // 바인딩
    SOCKADDR_IN servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(SERVER_PORT);
    retval = bind(sock, (SOCKADDR*)&servAddr, sizeof(servAddr));
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", GetLastError());
        return -1;
    }

    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(clientAddr);
    wchar_t buffer[BUFSIZE + 1];
    wchar_t ip[16];

    while (1)
    {
        retval = recvfrom(sock, (char*)buffer, BUFSIZE, 0, (SOCKADDR*)&clientAddr, &addrLen);
        if (retval == SOCKET_ERROR)
        {
            printf("%d\n", GetLastError());
            break;
        }
        
        buffer[retval/2] = '\0';
        InetNtop(AF_INET, &clientAddr.sin_addr, ip, 16);
        wprintf(L"[UDP | %s:%u] %s\n", ip, ntohs(clientAddr.sin_port), buffer);

        retval = sendto(sock, (char*)buffer, retval, 0, (SOCKADDR*)&clientAddr, addrLen);
        if (retval == SOCKET_ERROR)
        {
            printf("%d\n", GetLastError());
            break;
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
