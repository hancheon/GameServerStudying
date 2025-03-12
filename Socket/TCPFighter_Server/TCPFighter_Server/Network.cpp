#include <iostream>
//#include "Protocol.h"
#include "Network.h"
//#include "Session.h"
#include "Packet.h"
#include "myList.h"
#include "Contents.h"

extern SOCKET listen_sock;
int uniqueID = 0;
myList<SESSION*> users;
myList<SESSION*> disconnects;

bool networkProc()
{
    int retval;

    FD_SET rSet;
    FD_SET wSet;

    FD_ZERO(&rSet);
    FD_ZERO(&wSet);

    // set 설정
    FD_SET(listen_sock, &rSet);

    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        FD_SET((*iter)->sock, &rSet);
        if ((*iter)->writeQ.GetUsedSize() > 0)
            FD_SET((*iter)->sock, &wSet);
    }

    timeval time;
    time.tv_sec = 0;
    time.tv_usec = 0;

    retval = select(0, &rSet, &wSet, nullptr, &time);
    if (retval == SOCKET_ERROR)
    {
        if (GetLastError() != WSAEWOULDBLOCK)
        {
            printf("%d\n", GetLastError());
            return false;
        }
    }
    else if (retval == 0)
    {
        return true;
    }

    // accept()
    if (FD_ISSET(listen_sock, &rSet))
    {
        if (!acceptProc())
            return false;
    }

    for (iter = users.begin(); iter != users.end(); iter++)
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

    deleteUser();

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
    SESSION* newPlayer = new SESSION;
    newPlayer->sock = client_sock;
    newPlayer->sessionID = uniqueID;
    newPlayer->direction = dfPACKET_MOVE_DIR_RR;
    newPlayer->action = NOT_MOVE;
    newPlayer->xPos = (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) / 2;
    newPlayer->yPos = (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) / 2;
    newPlayer->HP = MAX_HP;

    // [서버 -> 새로운 플레이어] ID 할당 Unicast
    HEADER header;
    SC_CREATE_MY_CHARACTER sc_my_character;
    createPacket_CREATE_MY_CHARACTER(&header, (char*)&sc_my_character, newPlayer->sessionID, newPlayer->direction, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
    if (!unicast(newPlayer, &header, (char*)&sc_my_character))
        return false;

    // [서버 -> 모든 플레이어] 새로운 플레이어 생성 Broadcast
    SC_CREATE_OTHER_CHARACTER sc_other_character;
    createPacket_CREATE_OTHER_CHARACTER(&header, (char*)&sc_other_character, newPlayer->sessionID, newPlayer->direction, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
    if (!broadcast(nullptr, &header, (char*)&sc_other_character))
        return false;

    // [서버 -> 새로운 플레이어] 기존 플레이어 생성 Unicast
    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        createPacket_CREATE_OTHER_CHARACTER(&header, (char*)&sc_other_character, (*iter)->sessionID, (*iter)->direction, (*iter)->xPos, (*iter)->yPos, (*iter)->HP);
        printf("[sc_create_other] id: %lu, direction: %d, xPos: %d, yPos: %d, HP: %d\n", (*iter)->sessionID, (*iter)->direction, (*iter)->xPos, (*iter)->yPos, (*iter)->HP);
        if (!unicast(newPlayer, &header, (char*)&sc_other_character))
            return false;
    }

    // 플레이어 리스트에 추가
    users.push_back(newPlayer);
    uniqueID++;

    wprintf(L"# [New User] SessionID: %lu | direction: %d | xPos: %d | yPos: %d | HP: %d\n", newPlayer->sessionID, newPlayer->direction, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);

    return true;
}

bool unicast(SESSION* p, HEADER* header, char* payload)
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

bool broadcast(SESSION* p, HEADER* header, char* payload)
{
    int retval;

    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
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
                return false;
            }

            retval = (*iter)->writeQ.Enqueue(payload, header->p_size);
            if (retval != header->p_size)
            {
                printf("[Broadcast 페이로드 인큐 에러] 요청: %d 성공: %d\n", header->p_size, retval);
                return false;
            }
        }
    }

    return true;
}

bool readProc(SESSION* p)
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

    while (1)
    {
        // 헤더 유무 확인
        if (p->readQ.GetUsedSize() < sizeof(HEADER))
            break;

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
            break;

        // 헤더만큼 프론트 이동
        p->readQ.MoveFront(retval);

        char payload[30];
        retval = p->readQ.Dequeue(payload, header.p_size);
        if (retval != header.p_size)
        {
            printf("[Read 페이로드 디큐 에러] 요청: %llu 성공: %d\n", sizeof(HEADER), retval);
            return false;
        }

        if (!packetProc(p, header.p_type, payload))
        {
            wprintf(L"[packetProc() 에러]\n");
            return false;
        }
    }

    return true;
}

bool writeProc(SESSION* p)
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

bool disconnect(SESSION* p)
{
    // [Server -> 모든 플레이어] 해당 유저 삭제 요청 Broadcast
    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        if ((*iter) != p)
        {
            HEADER header;
            SC_DELETE_CHARACTER payload;
            createPacket_DELETE(&header, (char*)&payload, p->sessionID);
            if (!broadcast(p, &header, (char*)&payload))
                return false;
        }
    }

    disconnects.push_back(p);

    printf("# [Delete User] SessionID: %lu\n", p->sessionID);

    return true;
}

void deleteUser()
{
    // 연결 끊긴 세션 지연 삭제
    myList<SESSION*>::iterator iter;
    for (iter = disconnects.begin(); iter != disconnects.end(); iter++)
    {
        closesocket((*iter)->sock);
        users.remove(*iter);
    }
    disconnects.clear();
}