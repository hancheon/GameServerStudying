#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "myList.h"
#include "Protocol.h"
#pragma comment(lib, "ws2_32.lib")

#define PACKET_SIZE 16
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 23
#define SERVER_PORT 3000
#define MAX_S 63

// 전역 변수
SOCKET listen_sock;
int retval;
int retval_wsa;
int retval_sock;
int retval_opt;
int retval_bind;
int retval_listen;
int retval_trans;
int retval_accept;
int retval_send;
int retval_recv;

// Player 구조체
struct PLAYER
{
    SOCKET sock;
    wchar_t ipAddr[16];
    int port;
    int id;
    int xpos;
    int ypos;
    bool isAlive;
};
myList<PLAYER*> players;

bool Networking();
bool AcceptProc();
bool RecvProc(PLAYER*);
bool SendUnicast(PLAYER*, char*);
bool SendBroadcast(PLAYER*, char*);
bool Disconnect(PLAYER*);
bool MoveStar();


int main()
{
    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        retval_wsa = GetLastError();
        return -1;
    }

    // 리슨 소켓 생성
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET)
    {
        retval_sock = GetLastError();
        closesocket(listen_sock);
        return -1;
    }

    linger closeOpt;
    closeOpt.l_onoff = 1;
    closeOpt.l_linger = 0;
    retval = setsockopt(listen_sock, SOL_SOCKET, SO_LINGER, (char*)&closeOpt, sizeof(closeOpt));
    if (retval == SOCKET_ERROR)
    {
        retval_opt = GetLastError();
        closesocket(listen_sock);
        return -1;
    }

    // 리슨 소켓 바인딩
    SOCKADDR_IN servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVER_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    retval = bind(listen_sock, (SOCKADDR*)&servAddr, sizeof(servAddr));
    if (retval == SOCKET_ERROR)
    {
        retval_bind = GetLastError();
        closesocket(listen_sock);
        return -1;
    }

    // 리슨
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR)
    {
        retval_listen = GetLastError();
        closesocket(listen_sock);
        return -1;
    }

    // 논블로킹 소켓으로 전환
    u_long nonblock = 1;
    retval = ioctlsocket(listen_sock, FIONBIO, &nonblock);
    if (retval == SOCKET_ERROR)
    {
        retval_trans = GetLastError();
        closesocket(listen_sock);
        return -1;
    }

    // 서버 로직
    while (1)
    {
        // 네트워크()
        if (!Networking())
            return -1;

        // 로직()
        if (!MoveStar())
            return -1;

        // 랜더링() → 좌표 동기화 확인용(전용 클라로 만드는 경우도 있음) 해도 되고 안해도 되고 → 나 : 윈도우로 창 만들어보자 ?

    }

    closesocket(listen_sock);
    WSACleanup();
    
    return 0;
}

bool Networking()
{
    FD_SET readSet;
    FD_ZERO(&readSet);
    FD_SET(listen_sock, &readSet);

    // 타임아웃: 요청한 기능을 못할 때 최대 대기시간
    // timeval = NULL로 설정하는 이유: 어차피 서버 로직 없으니까 항상 기다리고 있어도 괜찮음
    retval = select(0, &readSet, nullptr, nullptr, NULL);
    if (retval == SOCKET_ERROR)
    {

    }

    // 리슨 소켓에 들어온 클라이언트를 유저로 승격
    /*myList<PLAYER>::iterator iter;
    for (int i = 0; i < players.size(); i++)
    {
        FD_SET(players[], &readSet);
    }*/
   
    if (FD_ISSET(listen_sock, &readSet))
    {
        AcceptProc();
    }

    myList<PLAYER*>::iterator iter;
    for (iter = players.begin(); iter != players.end(); iter++) // ret값이랑 recv 된 수랑 비교해서 반복 횟수 줄이기 가능
    {
        if (FD_ISSET((*iter)->sock, &readSet))
        {
            if (!RecvProc(*iter))
                return false;
        }
    }

    return true;
}

bool AcceptProc()
{
    // 메세지 버퍼
    char buffer[PACKET_SIZE];

    // accept()
    SOCKET client_sock;
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(clientAddr);
    client_sock = accept(listen_sock, (SOCKADDR*)&clientAddr, &addrLen);
    if (client_sock == INVALID_SOCKET)
    {
        retval_accept = GetLastError();
        closesocket(client_sock);
        return false;
    }

    // Player 생성 세팅 ID 부여
    PLAYER* newPlayer = new PLAYER;
    newPlayer->sock = client_sock;
    InetNtop(AF_INET, &clientAddr.sin_addr, newPlayer->ipAddr, sizeof(newPlayer->ipAddr));
    newPlayer->port = ntohs(clientAddr.sin_port);
    newPlayer->id = NULL;
    newPlayer->xpos = SCREEN_WIDTH / 2;
    newPlayer->ypos = SCREEN_HEIGHT / 2;

    // ID 할당 메세지 Unicast send
    buffer[PACKET_SIZE] = { 0 };
    ID_ALLOC* allocMsg = (ID_ALLOC*)buffer;
    allocMsg->type = 0;
    allocMsg->id = NULL;
    SendUnicast(newPlayer, buffer);
    
    // 기존 접속자 별 생성 메세지 Unicast send
    buffer[PACKET_SIZE] = { 0 };
    STAR_CREATE* createMsg = (STAR_CREATE*)buffer;
    myList<PLAYER*>::iterator iter;
    for (iter = players.begin(); iter != players.end(); iter++)
    {
        createMsg->type = 1;
        createMsg->id = (*iter)->id;
        createMsg->xpos = (*iter)->xpos;
        createMsg->ypos = (*iter)->ypos;
        SendUnicast(newPlayer, buffer);
    }

    // 모두에게 새로운 접속자 별 생성 메세지 Broadcast send
    buffer[PACKET_SIZE] = { 0 };
    STAR_CREATE* createMsg = (STAR_CREATE*)buffer;
    createMsg->type = 1;
    createMsg->id = newPlayer->id;
    createMsg->xpos = newPlayer->xpos;
    createMsg->ypos = newPlayer->ypos;
    SendBroadcast(nullptr, buffer);

    // Player 리스트에 추가
    players.push_back(newPlayer);
}

bool SendUnicast(PLAYER* p, char* buffer)
{
    retval = send(p->sock, buffer, PACKET_SIZE, 0);
    if (retval == SOCKET_ERROR)
    {
        retval_send = GetLastError();
        return false;
    }
    return true;
}

bool SendBroadcast(PLAYER* p, char* buffer)
{
    myList<PLAYER*>::iterator iter;
    for (iter = players.begin(); iter != players.end(); iter++)
    {
        if (p != *iter)
        {
            retval = send((*iter)->sock, buffer, PACKET_SIZE, 0);
            if (retval == SOCKET_ERROR)
            {
                retval_send = GetLastError();
                return false;
            }
        }
    }
    return true;
}


bool RecvProc(PLAYER* p) // -> 플레이어 리스트 순환 중
{
    char buffer[PACKET_SIZE] = { 0 };
    retval = recv(p->sock, buffer, PACKET_SIZE, 0);
    if (retval == SOCKET_ERROR)
    {
        retval_recv = GetLastError();
        return false;
    }
    if (retval == 0)
    {
        if (!Disconnect(p))
            return false;
    }
    
    STAR_MOVE* moveMsg = (STAR_MOVE*)buffer;

    if (moveMsg->type != (int)MSG_TYPE::STAR_MOVE) // 타입이 올바르지 않을 때
    {
        return false;
    }

    myList<PLAYER*>::iterator iter;
    for (iter = players.begin(); iter != players.end(); iter++)
    {
        if ((*iter)->id == moveMsg->id)
        {
            (*iter)->xpos = moveMsg->xpos;
            (*iter)->ypos = moveMsg->ypos;
        }
    }

    return true;
}

bool Disconnect(PLAYER* p)
{
    // 메세지 버퍼
    char buffer[PACKET_SIZE] = { 0 };

    // 플레이어 리스트에서 해당 플레이어 제거 -> 문제: RecvProc에서 리스트 이터레이터 순환 중 -> 댕글링 포인터 발생
    // 여기서 제거를 안하고 나중에 한번에 삭제 -> 지연 삭제
    p->isAlive = false;

    // 해당 플레이어 별 삭제 메세지 SendBroadcast -> 이 함수 안에서도 Disconnect가 일어날 수 있음 -> 재귀 발생
    STAR_DELETE* deleteMsg = (STAR_DELETE*)buffer;
    deleteMsg->type = 2;
    deleteMsg->id = p->id;
    if (!SendBroadcast(p, buffer))
        return false;

    return true;
}

bool MoveStar()
{

}

// 릴리즈말고 디버그로 왜? 댕글링 포인터 에러가 안나서 디버그는 할당 해제한 메모리를 0xdd 다 밀어주니까