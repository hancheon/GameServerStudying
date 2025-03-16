#include <iostream>
#include "Network.h"
#include "Packet.h"
#include "myList.h"
#include "Contents.h"

extern SOCKET listen_sock;
DWORD uniqueID = 0;
myList<SESSION*> users;
myList<SESSION*> disconnects;
extern bool serverShut;

void networkProc()
{
    int retval;

    FD_SET rSet;
    FD_SET wSet;

    FD_ZERO(&rSet);
    FD_ZERO(&wSet);

    // read셋에 리슨 소켓 등록
    FD_SET(listen_sock, &rSet);

    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        FD_SET((*iter)->sock, &rSet); // 언제 올지 모르니 모두 등록
        if ((*iter)->writeQ.GetUsedSize() > 0) // 쓸 게 있는 소켓만 등록
            FD_SET((*iter)->sock, &wSet);
    }

    timeval timeout; // 대기X 어차피 while문으로 계속 반복중
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    retval = select(0, &rSet, &wSet, nullptr, &timeout);
    if (retval > 0)
    {
        // accept()
        if (FD_ISSET(listen_sock, &rSet)) // 리슨 소켓 연결 요청 확인
        {
            acceptProc();
        }

        for (iter = users.begin(); iter != users.end(); iter++)
        {
            if (FD_ISSET((*iter)->sock, &rSet)) // readSet 조사
            {
                readProc(*iter);
            }
            if (FD_ISSET((*iter)->sock, &wSet)) // writeSet 조사
            {
                writeProc(*iter);
            }
        }

        deleteUser();
    }
    else if (retval == SOCKET_ERROR)
    {
        if (GetLastError() != WSAEWOULDBLOCK)
        {
            printf("%d\n", GetLastError());
            serverShut = true;
        }
    }
}

void acceptProc()
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
            serverShut = true;
        }
    }

    // 새로운 플레이어 데이터 초기화
    SESSION* newPlayer = new SESSION;
    newPlayer->sock = client_sock;
    newPlayer->sessionID = uniqueID;
    newPlayer->direction = dfPACKET_MOVE_DIR_RR;
    newPlayer->action = dfPACKET_MOVE_STOP; // 캐릭터 행동 상태 확인용 변수
    newPlayer->xPos = (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) / 2;
    newPlayer->yPos = (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) / 2;
    newPlayer->HP = MAX_HP;

    // [서버 -> 새로운 플레이어] ID 할당 Unicast
    HEADER header;
    SC_CREATE_MY_CHARACTER sc_my_character;
    createPacket_CREATE_MY_CHARACTER(&header, (char*)&sc_my_character, newPlayer->sessionID, newPlayer->direction, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
    unicast(newPlayer, &header, (char*)&sc_my_character);

    // [서버 -> 모든 플레이어] 새로운 플레이어 생성 Broadcast
    SC_CREATE_OTHER_CHARACTER sc_other_character;
    createPacket_CREATE_OTHER_CHARACTER(&header, (char*)&sc_other_character, newPlayer->sessionID, newPlayer->direction, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
    broadcast(nullptr, &header, (char*)&sc_other_character);

    // [서버 -> 새로운 플레이어] 기존 플레이어 생성 Unicast
    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        createPacket_CREATE_OTHER_CHARACTER(&header, (char*)&sc_other_character, (*iter)->sessionID, (*iter)->direction, (*iter)->xPos, (*iter)->yPos, (*iter)->HP);
        unicast(newPlayer, &header, (char*)&sc_other_character);

        if ((*iter)->action != dfPACKET_MOVE_STOP)
        {
            SC_MOVE_START sc_move_start;
            createPacket_MOVE_START(&header, (char*)&sc_move_start, (*iter)->sessionID, (*iter)->action, (*iter)->xPos, (*iter)->yPos);
            unicast(newPlayer, &header, (char*)&sc_move_start);
        }
    }

    // 플레이어 리스트에 추가
    users.push_back(newPlayer);

    uniqueID++;

    printf("# [New User] SessionID: %lu | direction: RR | xPos: %d | yPos: %d | HP: %d\n", newPlayer->sessionID, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
}

void unicast(SESSION* p, HEADER* header, char* payload)
{
    int retval;

    if (p == nullptr)
    {
        printf("Unicast 대상이 존재하지 않음\n");
        serverShut = true;
        return;
    }
    else if (p->writeQ.GetFreeSize() == 0) // 처리 불가능한 데이터로 링버터 가득참
    {
        printf("Unicast 링버퍼 가득참\n");
        disconnect(p);
        return;
    }

    retval = p->writeQ.Enqueue((char*)header, sizeof(HEADER));
    if (retval != sizeof(HEADER))
    {
        printf("[Unicast 헤더 인큐 에러] 요청: %llu 성공: %d\n", sizeof(HEADER), retval);
        disconnect(p);
        serverShut = true;
        return;
    }

    retval = p->writeQ.Enqueue(payload, header->p_size);
    if (retval != header->p_size)
    {
        printf("[Unicast 페이로드 인큐 에러] 요청: %d 성공: %d\n", header->p_size, retval);
        disconnect(p);
        serverShut = true;
        return;
    }
}

void broadcast(SESSION* p, HEADER* header, char* payload)
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
                disconnect(*iter);
                continue;
            }

            retval = (*iter)->writeQ.Enqueue(payload, header->p_size);
            if (retval != header->p_size)
            {
                printf("[Broadcast 페이로드 인큐 에러] 요청: %d 성공: %d\n", header->p_size, retval);
                disconnect(*iter);
                continue;
            }
        }
    }
}

void readProc(SESSION* p)
{
    if (p == nullptr)
        return;

    int retval;
    char buffer[MAX_BUFSIZE];

    if (p->readQ.GetFreeSize() == 0) // 처리 불가능한 데이터로 링버퍼 가득 참
    {
        printf("수신 링버퍼 가득참");
        disconnect(p);
        return;
    }

    // tcp 수신버퍼 -> 지역 버퍼
    retval = recv(p->sock, buffer, min(MAX_BUFSIZE, p->readQ.GetFreeSize()), 0); // 링버퍼에 넣을 수 있는 최대로 가져옴
    if (retval == SOCKET_ERROR)
    {
        if (GetLastError() != WSAEWOULDBLOCK)
        {
            if (GetLastError() == WSAECONNRESET)
            {
                disconnect(p);
                return;
            }
            printf("%d", GetLastError());
            serverShut = true;
            return;
        }
    }
    else if (retval == 0)
    {
        disconnect(p);
        return;
    }

    // 지역 버퍼 -> 링버퍼
    int retval_enq = p->readQ.Enqueue(buffer, retval);
    if (retval_enq != retval)
    {
        printf("[Read 인큐 에러] 요청: %d 성공: %d\n", retval, retval_enq);
        serverShut = true;
        return;
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
            break;
        }
        
        // 헤더 코드 확인
        if (header.p_code != PACKET_CODE)
        {
            printf("패킷 코드 이상 GetUsedSize(): %d\n", p->readQ.GetUsedSize());
            break;
        }

        // 헤더 + 페이로드 확인
        if (p->readQ.GetUsedSize() < sizeof(HEADER) + header.p_size)
            break;

        // 헤더만큼 프론트 이동
        p->readQ.MoveFront(retval);

        char payload[100];
        retval = p->readQ.Dequeue(payload, header.p_size);
        if (retval != header.p_size)
        {
            printf("[Read 페이로드 디큐 에러] 요청: %lu 성공: %d\n", header.p_size, retval);
            break;
        }

        if (!packetProc(p, header.p_type, payload))
        {
            printf("[packetProc() 에러]\n");
            break;
        }
    }
}

void writeProc(SESSION* p)
{
    if (p == nullptr)
        return;

    int retval;
    char buffer[MAX_BUFSIZE];

    retval = p->writeQ.Dequeue(buffer, min(MAX_BUFSIZE, p->writeQ.GetUsedSize()));
    retval = send(p->sock, buffer, retval, 0);
    if (retval == SOCKET_ERROR)
    {
        if (GetLastError() != WSAEWOULDBLOCK)
        {
            printf("%d", GetLastError());
            serverShut = true;
            return;
        }
    }
}

void disconnect(SESSION* p)
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
            broadcast(p, &header, (char*)&payload);
        }
    }

    disconnects.push_back(p);

    printf("# [Delete User] SessionID: %lu\n", p->sessionID);
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