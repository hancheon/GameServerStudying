#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 512

struct SOCKETINFO
{
    SOCKET sock;
    wchar_t buf[BUFSIZE];
    int recvBytes;
    int sendBytes;
};

int nTotalSockets = 0;
SOCKETINFO* SocketInfoArray[FD_SETSIZE - 1];

BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int nIndex);

int main()
{
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("%d\n", WSAGetLastError());
        return -1;
    }

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET)
    {
        printf("%d\n", WSAGetLastError());
        closesocket(listen_sock);
        return -1;
    }

    // bind()
    SOCKADDR_IN servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = 9001;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    retval = bind(listen_sock, (SOCKADDR*)&servAddr, sizeof(servAddr));
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", WSAGetLastError());
        closesocket(listen_sock);
        return -1;
    }

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", WSAGetLastError());
        closesocket(listen_sock);
        return -1;
    }

    // 넌블로킹 소켓으로 변경
    u_long blockingMode = 1;
    retval = ioctlsocket(listen_sock, FIONBIO, &blockingMode);
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", WSAGetLastError());
        closesocket(listen_sock);
        return -1;
    }
    
    // Select 모델 변수
    FD_SET rSet, wSet; // 일반적으로 예외 셋은 제외

    // 클라이언트 정보 변수
    SOCKET client_sock;
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(clientAddr);
    wchar_t ipAddr[30] = { 0 };

    printf("클라이언트 접속 대기 중...\n");

    // accept()
    while (1)
    {
        FD_ZERO(&rSet);
        FD_ZERO(&wSet);
        FD_SET(listen_sock, &rSet);
        for (int i = 0; i < nTotalSockets; i++)
        {
            if (SocketInfoArray[i]->recvBytes > SocketInfoArray[i]->sendBytes)
                FD_SET(SocketInfoArray[i]->sock, &wSet);
            else
                FD_SET(SocketInfoArray[i]->sock, &rSet);
        }

        // select()
        retval = select(0, &rSet, &wSet, nullptr, nullptr);
        if (retval == SOCKET_ERROR)
        {
            printf("%d\n", WSAGetLastError());
            break;
        }

        // 소켓 셋 검사1: 클라이언트 접속 수용
        if (FD_ISSET(listen_sock, &rSet))
        {
            client_sock = accept(listen_sock, (SOCKADDR*)&clientAddr, &addrLen);
            if (client_sock == INVALID_SOCKET)
            {
                printf("%d\n", WSAGetLastError());
                closesocket(client_sock);
                break;
            }
            InetNtop(AF_INET, &clientAddr.sin_addr, ipAddr, 30);
            wprintf(L"[TCP 서버] 클라이언트 접속\nIP: %s, 포트 번호: %d\n", ipAddr, htons(clientAddr.sin_port));
            AddSocketInfo(client_sock);
        }

        // 소켓 셋 검사2: 데이터 통신
        for (int i = 0; i < nTotalSockets; i++)
        {
            SOCKETINFO* ptr = SocketInfoArray[i];

            // 데이터 받기
            if (FD_ISSET(ptr->sock, &rSet))
            {
                retval = recv(ptr->sock, (char*)ptr->buf, BUFSIZE, 0);
                if (retval == SOCKET_ERROR)
                {
                    printf("%d\n", WSAGetLastError());
                    RemoveSocketInfo(i);
                    continue;
                }
                else if (retval == 0)
                {
                    RemoveSocketInfo(i);
                    continue;
                }
                ptr->recvBytes = retval;
                
                // 받은 데이터 출력
                getpeername(ptr->sock, (SOCKADDR*)&clientAddr, &addrLen);
                ptr->buf[retval / 2] = '\0';
                wprintf(L"[TCP %s:%d] %s\n", ipAddr, ntohs(clientAddr.sin_port), ptr->buf);
            }

            // 데이터 보내기
            if (FD_ISSET(ptr->sock, &wSet))
            {
                retval = send(ptr->sock, (char*)ptr->buf + ptr->sendBytes, ptr->recvBytes - ptr->recvBytes, 0);
                if (retval == SOCKET_ERROR)
                {
                    printf("%d\n", WSAGetLastError());
                    RemoveSocketInfo(i);
                    continue;
                }

                ptr->sendBytes += retval;

                if (ptr->recvBytes == ptr->sendBytes)
                {
                    ptr->recvBytes = 0;
                    ptr->sendBytes = 0;
                }
            }
        }
    }

    closesocket(listen_sock);
    WSACleanup();
    return 0;
}

BOOL AddSocketInfo(SOCKET sock)
{
    if (nTotalSockets >= FD_SETSIZE - 1)
    {
        printf("Error: 셋 가득 참\n");
        return FALSE;
    }

    SOCKETINFO* ptr = new SOCKETINFO;
    if (ptr == NULL)
    {
        printf("Error: 메모리 부족\n");
        return FALSE;
    }

    ptr->sock = sock;
    ptr->recvBytes = 0;
    ptr->sendBytes = 0;
    SocketInfoArray[nTotalSockets++] = ptr;

    return TRUE;
}

void RemoveSocketInfo(int nIndex)
{
    SOCKETINFO* ptr = SocketInfoArray[nIndex];

    // 클라이언트 정보 얻기
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(clientAddr);
    wchar_t ipAddr[100] = { 0 };
    getpeername(ptr->sock, (SOCKADDR*)&clientAddr, &addrLen);
    InetNtop(AF_INET, &clientAddr.sin_addr, ipAddr, 30);
    wprintf(L"[TCP 서버] 클라이언트 종료\nIP 주소: %s, 포트 번호: %d\n", ipAddr, ntohs(clientAddr.sin_port));

    closesocket(ptr->sock);
    delete ptr;

    if (nIndex != (nTotalSockets - 1))
        SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets - 1];

    --nTotalSockets;
}