#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <locale.h>
#include "Protocol.h"
#include "myList.h"
#include "RingBuffer.h"
#pragma comment(lib, "Winmm")
#pragma comment(lib, "ws2_32")

#define SERVER_PORT 5000
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MAX_HP 100
#define MAX_BUFSIZE 1024

#define PACKET_CODE 0x89

#define dfRANGE_MOVE_TOP	50
#define dfRANGE_MOVE_LEFT	10
#define dfRANGE_MOVE_RIGHT	630
#define dfRANGE_MOVE_BOTTOM	470

struct PLAYER
{
    SOCKET sock;
    int port;
    wchar_t ip[16];
    int id;
    short xPos;
    short yPos;
    char direction;
    char hp;
    RingBuffer readQ;
    RingBuffer writeQ;
};

// 전역변수
SOCKET listen_sock;
myList<PLAYER*> players;
myList<PLAYER*> disconnects;
int uniqueId = 0;

// 전방선언
bool initWinSock();
bool initServer();
bool networking();
bool acceptProc();
bool readProc(PLAYER*);
bool writeProc(PLAYER*);
bool unicast(PLAYER*, HEADER*, char*);
bool broadcast(PLAYER*, HEADER*, char*);
bool disconnect(PLAYER*);
bool deleteUser(PLAYER*);

int main()
{
    timeBeginPeriod(1);
    setlocale(LC_ALL, "Korean");

    // 윈속 초기화
    if (!initWinSock())
        return -1;

    // 서버 세팅
    if (!initServer())
    {
        closesocket(listen_sock);
        return -1;
    }

    int oldTick = timeGetTime();
    // 서버 로직
    while (1)
    {
        if (!networking())
        {
            closesocket(listen_sock);
            return -1;
        }

        int workTime = timeGetTime() - oldTick;
        if (workTime < 20)
            Sleep(20 - workTime);
        oldTick += 20;
    }

    // 서버 종료
    closesocket(listen_sock);
    WSACleanup();
    timeEndPeriod(1);

    return 0;
}

bool initWinSock()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return false;
    return true;
}

bool initServer()
{
    int retval;

    // 소켓 생성
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET)
    {
        printf("%d\n", GetLastError());
        return false;
    }

    LINGER lin;
    lin.l_linger = 0;
    lin.l_onoff = 1;
    retval = setsockopt(listen_sock, SOL_SOCKET, SO_LINGER, (char*)&lin, sizeof(lin));
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", GetLastError());
        return false;
    }

    // 바인딩
    SOCKADDR_IN servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVER_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    retval = bind(listen_sock, (SOCKADDR*)&servAddr, sizeof(servAddr));
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", GetLastError());
        return false;
    }

    // 리슨
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", GetLastError());
        return false;
    }

    // 논블로킹 소켓으로 전환
    u_long nonblock = 1;
    retval = ioctlsocket(listen_sock, FIONBIO, &nonblock);
    if (retval == SOCKET_ERROR)
    {
        printf("%d\n", GetLastError());
        return false;
    }

    return true;
}

bool networking()
{
    int retval;

    FD_SET rSet;
    FD_SET wSet;

    FD_ZERO(&rSet);
    FD_ZERO(&wSet);

    // set 설정
    FD_SET(listen_sock, &rSet);

    myList<PLAYER*>::iterator iter;
    for (iter = players.begin(); iter != players.end(); iter++)
    {
        FD_SET((*iter)->sock, &rSet);
        if ((*iter)->writeQ.GetUsedSize() > 0)
            FD_SET((*iter)->sock, &wSet);
    }

    retval = select(0, &rSet, &wSet, nullptr, nullptr);
    if (retval == SOCKET_ERROR)
    {
        if (GetLastError() != WSAEWOULDBLOCK)
        {
            printf("%d\n", GetLastError());
            return false;
        }
    }

    // accept()
    if (FD_ISSET(listen_sock, &rSet))
    {
        if (!acceptProc())
            return false;
    }

    for (iter = players.begin(); iter != players.end(); iter++)
    {
        if (FD_ISSET((*iter)->sock, &rSet))
        {
            if (!readProc(*iter))
                return false;
        }
        if (FD_ISSET((*iter)->sock, &wSet))
        {
            if (!writeProc(*iter))
                return false;
        }
    }

    return true;
}

bool acceptProc()
{
    SOCKET client_sock;
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(clientAddr);

    client_sock = accept(listen_sock, (SOCKADDR*)&clientAddr, &addrLen);
    if (client_sock == INVALID_SOCKET)
    {
        if (GetLastError() != WSAEWOULDBLOCK)
        {
            printf("%d\n", GetLastError());
            return false;
        }
    }

    // 새로운 플레이어 데이터 초기화
    PLAYER* newPlayer = new PLAYER;
    newPlayer->sock = client_sock;
    InetNtop(AF_INET, &clientAddr.sin_addr.s_addr, newPlayer->ip, 16);
    newPlayer->port = ntohs(clientAddr.sin_port);
    newPlayer->id = uniqueId;
    newPlayer->direction = dfPACKET_MOVE_DIR_RR;
    newPlayer->xPos = dfRANGE_MOVE_RIGHT / 2;
    newPlayer->yPos = dfRANGE_MOVE_BOTTOM / 2;
    newPlayer->hp = MAX_HP;

    // [서버 -> 새로운 플레이어] ID 할당 Unicast
    HEADER header;
    header.p_code = PACKET_CODE;
    header.p_size = sizeof(SC_CREATE_MY_CHARACTER);
    header.p_type = dfPACKET_SC_CREATE_MY_CHARACTER;

    /// TODO: 구조체를 매개변수로 받기 어려움 (형태가 다 다르니까)
    /// 방법1: 구조체를 크기에 맞는 배열에 저장하는 함수 -> 인큐 함수에 전달
    /// 방법2: 
    SC_CREATE_MY_CHARACTER sc_my_character;
    sc_my_character.id = newPlayer->id;
    sc_my_character.direction = newPlayer->direction;
    sc_my_character.xPos = newPlayer->xPos;
    sc_my_character.yPos = newPlayer->yPos;
    sc_my_character.hp = newPlayer->hp;
    if (!unicast(newPlayer, &header, (char*)&sc_my_character))
        return false;

    // [서버 -> 모든 플레이어] 새로운 플레이어 생성 Broadcast
    header.p_size = sizeof(SC_CREATE_OTHER_CHARACTER);
    header.p_type = dfPACKET_SC_CREATE_OTHER_CHARACTER;

    SC_CREATE_OTHER_CHARACTER sc_other_character;
    sc_other_character.id = newPlayer->id;
    sc_other_character.direction = newPlayer->direction;
    sc_other_character.xPos = newPlayer->xPos;
    sc_other_character.yPos = newPlayer->yPos;
    sc_other_character.hp = newPlayer->hp;
    if (!broadcast(nullptr, &header, (char*)&sc_other_character))
        return false;

    // [서버 -> 새로운 플레이어] 기존 플레이어 생성 Unicast
    myList<PLAYER*>::iterator iter;
    for (iter = players.begin(); iter != players.end(); iter++)
    {
        sc_other_character.id = (*iter)->id;
        sc_other_character.direction = (*iter)->direction;
        sc_other_character.xPos = (*iter)->xPos;
        sc_other_character.yPos = (*iter)->yPos;
        sc_other_character.hp = (*iter)->hp;
        if (!unicast(newPlayer, &header, (char*)&sc_other_character))
            return false;
    }

    // 플레이어 리스트에 추가
    players.push_back(newPlayer);
    uniqueId++;

    wprintf(L"[새로운 유저 접속] IP: %s, port: %d\n", newPlayer->ip, newPlayer->port);

    return true;
}

bool unicast(PLAYER* p, HEADER* header, char* payload)
{
    int retval;

    if (p == nullptr)
    {
        printf("Unicast 대상이 존재하지 않음\n");
        return false;
    }
    else if (p->writeQ.GetFreeSize() == 0) // 처리 불가능한 데이터 존재
    {
        printf("Unicast 링버퍼 가득참\n");
        disconnect(p);
        return true;
    }

    retval = p->writeQ.Enqueue((char*)header, sizeof(HEADER));
    if (retval != sizeof(HEADER))
    {
        printf("[Unicast 헤더 인큐 에러] 요청: %llu 성공: %d\n", sizeof(HEADER), retval);
        disconnect(p);
        return true;
    }

    retval = p->writeQ.Enqueue(payload, header->p_size);
    if (retval != header->p_size)
    {
        printf("[Unicast 페이로드 인큐 에러] 요청: %d 성공: %d\n", header->p_size, retval);
        disconnect(p);
        return true;
    }

    return true;
}

bool broadcast(PLAYER* p, HEADER* header, char* payload)
{
    int retval;

    myList<PLAYER*>::iterator iter;
    for (iter = players.begin(); iter != players.end(); iter++)
    {
        if ((*iter) != p)
        {
            if ((*iter)->writeQ.GetFreeSize() == 0) // 처리 불가능한 데이터 존재
            {
                printf("Broadcast 링버퍼 가득참");
                disconnect(*iter);
                continue;
            }

            retval = (*iter)->writeQ.Enqueue((char*)header, sizeof(HEADER));
            if (retval != sizeof(HEADER))
            {
                printf("[Broadcast 헤더 인큐 에러] 요청: %llu 성공: %d\n", sizeof(HEADER), retval);
                disconnect(*iter);
                return true;
            }

            retval = (*iter)->writeQ.Enqueue(payload, header->p_size);
            if (retval != sizeof(payload))
            {
                printf("[Broadcast 페이로드 인큐 에러] 요청: %d 성공: %d\n", header->p_size, retval);
                disconnect(*iter);
                return true;
            }
        }
    }

    return true;
}

bool readProc(PLAYER* p)
{
    if (p == nullptr)
        return false;

    int retval;
    char buffer[MAX_BUFSIZE];

    if (p->readQ.GetFreeSize() == 0)
    {
        printf("수신 링버퍼 가득참");
        disconnect(p);
        return true;
    }

    // tcp 수신버퍼 -> 지역 버퍼
    retval = recv(p->sock, buffer, min(MAX_BUFSIZE, p->readQ.GetFreeSize()), 0);
    if (retval == SOCKET_ERROR)
    {
        if (GetLastError() != WSAEWOULDBLOCK)
        {
            printf("%d", GetLastError());
            return false;
        }
    }
    else if (retval == 0)
    {
        disconnect(p);
        return true;
    }

    // 지역 버퍼 -> 링버퍼
    int retval_enq = p->readQ.Enqueue(buffer, retval);
    if (retval_enq != retval)
    {
        printf("[Read 인큐 에러] 요청: %d 성공: %d\n", retval, retval_enq);
        return false;
    }

    // 헤더 유무 확인
    if (p->readQ.GetUsedSize() < sizeof(HEADER))
        return true;

    // 헤더 확인
    HEADER header;
    retval = p->readQ.Peek((char*)&header, sizeof(HEADER));
    if (retval != sizeof(HEADER))
    {
        printf("[Read 헤더 Peek 에러] 요청: %llu 성공: %d\n", sizeof(HEADER), retval);
        return false;
    }

    // 헤더 + 페이로드 확인
    if (p->readQ.GetUsedSize() < sizeof(HEADER) + header.p_size)
        return true;

    char payload[30];
    retval = p->readQ.Dequeue((char*)&header, sizeof(HEADER));
    if (retval != sizeof(HEADER))
    {
        printf("[Read 헤더 디큐 에러] 요청: %llu 성공: %d\n", sizeof(HEADER), retval);
        return false;
    }
    retval = p->readQ.Dequeue(payload, header.p_size);
    if (retval != header.p_size)
    {
        printf("[Read 페이로드 디큐 에러] 요청: %llu 성공: %d\n", sizeof(HEADER), retval);
        return false;
    }

    switch (header.p_type)
    {
    case dfPACKET_CS_MOVE_START:
    {
        CS_MOVE_START* msg = (CS_MOVE_START*)payload;
        p->direction = msg->direction;
        p->xPos = msg->xPos;
        p->yPos = msg->xPos;
        wprintf(L"[MOVE_START] Session: %s | Direction: %d | X: %d | Y: %d\n", p->ip, p->direction, p->xPos, p->yPos);
        break;
    }
    case dfPACKET_CS_MOVE_STOP:
    {
        CS_MOVE_STOP* msg = (CS_MOVE_STOP*)payload;
        p->direction = msg->direction;
        p->xPos = msg->xPos;
        p->yPos = msg->xPos;
        wprintf(L"[MOVE_STOP] Session: %s | Direction: %d | X: %d | Y: %d\n", p->ip, p->direction, p->xPos, p->yPos);
        break;
    }
    case dfPACKET_CS_ATTACK1:

        break;
    case dfPACKET_CS_ATTACK2:

        break;
    case dfPACKET_CS_ATTACK3:

        break;
    }

    return true;
}

bool writeProc(PLAYER* p)
{
    if (p == nullptr)
        return false;

    int retval;
    char buffer[MAX_BUFSIZE];

    retval = p->writeQ.Dequeue(buffer, min(MAX_BUFSIZE, p->writeQ.GetUsedSize()));
    retval = send(p->sock, buffer, retval, 0);
    if (retval == SOCKET_ERROR)
    {
        if (GetLastError() != WSAEWOULDBLOCK)
        {
            printf("%d", GetLastError());
            return false;
        }
    }

    return true;
}

bool disconnect(PLAYER*)
{
    return false;
}

bool deleteUser(PLAYER*)
{
    return false;
}