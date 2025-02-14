#include <iostream>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <locale.h>
#pragma comment(lib, "ws2_32.lib")

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
        return -1;
    }

    // listen
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR)
    {
        wprintf(L"%d\n", GetLastError());
        return -1;
    }
    wprintf(L"클라이언트 연결 대기중\n");

    SOCKET client_sock;
    SOCKADDR_IN clientAddr;
    int clientAddrLen;
    wchar_t buf[BUFSIZE];


    // accept
    while (1)
    {
        clientAddrLen = sizeof(clientAddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientAddr, &clientAddrLen);
        if (client_sock == INVALID_SOCKET)
        {
            wprintf(L"%d\n", GetLastError());
            break;
        }

        wchar_t IPAddressBuf[30] = { 0 };
        InetNtop(AF_INET, &clientAddr.sin_addr, IPAddressBuf, 30);
        wprintf(L"[TCP] 클라이언트 접속\nIP 주소: %s, 포트번호: %d\n", IPAddressBuf, ntohs(clientAddr.sin_port));

        while (1)
        {
            // 데이터 받기
            retval = recv(client_sock, (char*)buf, BUFSIZE, 0);
            if (retval == SOCKET_ERROR)
            {
                wprintf(L"%d\n", GetLastError());
                break;
            }
            else if (retval == 0)
            {
                wprintf(L"연결이 종료되었습니다.\n");
                break;
            }

            // 받은 데이터 출력
            buf[retval / 2] = L'\0';
            wprintf(L"[%s:%d] %s\n", IPAddressBuf, htons(clientAddr.sin_port), buf);

            // 데이터 보내기
            retval = send(client_sock, (char*)buf, retval, 0);
            if (retval == SOCKET_ERROR)
            {
                wprintf(L"%d\n", GetLastError());
                break;
            }
        }
        closesocket(client_sock);
    }

    closesocket(listen_sock);
    WSACleanup();
    return 0;
}