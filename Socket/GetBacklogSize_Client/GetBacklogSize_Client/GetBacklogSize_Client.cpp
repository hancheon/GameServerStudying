#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <locale.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 512

int wmain(int argc, wchar_t** argv)
{
    // 유니코드 한글 설정
    _wsetlocale(LC_ALL, L"korean");

    int retval;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    int backlog_queue = 0;
    while (1)
    {
        // socket
        SOCKET sock;
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
        {
            wprintf(L"%d\n", GetLastError());
            return -1;
        }

        // connect
        SOCKADDR_IN servAddr;
        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        //InetPtonW(AF_INET, argv[0], &servAddr.sin_addr); // serverIP
        InetPtonW(AF_INET, L"192.168.30.34", &servAddr.sin_addr); // serverIP
        servAddr.sin_port = htons(9001);
        retval = connect(sock, (SOCKADDR*)&servAddr, sizeof(servAddr));
        if (retval == SOCKET_ERROR)
        {
            wprintf(L"%d\n", GetLastError());
            break;
        }

        // 실제 연결이 목적이 아니기 때문에 연결 요청 후 소켓 닫기
        closesocket(sock);
        ++backlog_queue;
    }

    printf("%d\n", backlog_queue);

    WSACleanup();
    return 0;
}