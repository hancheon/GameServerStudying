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
    InetPtonW(AF_INET, L"127.0.0.1", &servAddr.sin_addr); // loopback
    servAddr.sin_port = htons(9001);
    retval = connect(sock, (SOCKADDR*)&servAddr, sizeof(servAddr));
    if (retval == SOCKET_ERROR)
    {
        wprintf(L"%d\n", GetLastError());
        return -1;
    }

    wchar_t buf[BUFSIZE + 1];
    int len;

    while (1)
    {
        // 데이터 입력
        wprintf(L"보낼 데이터: ");
        if (fgetws(buf, BUFSIZE, stdin) == NULL)
            break;

        // '\n' 제거
        len = wcslen(buf);
        if (buf[len - 1] == L'\n')
        {
            buf[len - 1] = L'\0';
        }

        // 데이터 보내기
        retval = send(sock, (char*)buf, len * 2, 0);
        if (retval == SOCKET_ERROR)
        {
            wprintf(L"%d\n", GetLastError());
            break;
        }
        wprintf(L"%d 바이트를 보냈습니다.\n", retval);

        // 데이터 받기
        retval = recv(sock, (char*)buf, BUFSIZE, 0);
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

        buf[retval / 2] = L'\0';
        wprintf(L"%d 바이트를 받았습니다.\n", retval);
        wprintf(L"받은 데이터: %s\n", buf);
    }

    closesocket(sock);

    WSACleanup();
    return 0;
}