#include <iostream>
#include "Network.h"
#include "PacketProc.h"
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
    Packet payload;
    createPacket_CREATE_MY_CHARACTER(&payload, newPlayer->sessionID, newPlayer->direction, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
    unicast(newPlayer, &payload);

    // [서버 -> 모든 플레이어] 새로운 플레이어 생성 Broadcast
    payload.Clear();
    createPacket_CREATE_OTHER_CHARACTER(&payload, newPlayer->sessionID, newPlayer->direction, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
    broadcast(nullptr, &payload);

    // [서버 -> 새로운 플레이어] 기존 플레이어 생성 Unicast
    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        payload.Clear();
        createPacket_CREATE_OTHER_CHARACTER(&payload, (*iter)->sessionID, (*iter)->direction, (*iter)->xPos, (*iter)->yPos, (*iter)->HP);
        unicast(newPlayer, &payload);

        if ((*iter)->action != dfPACKET_MOVE_STOP)
        {
            payload.Clear();
            createPacket_MOVE_START(&payload, (*iter)->sessionID, (*iter)->action, (*iter)->xPos, (*iter)->yPos);
            unicast(newPlayer, &payload);
        }
    }

    // 플레이어 리스트에 추가
    users.push_back(newPlayer);

    uniqueID++;

    printf("# [New User] SessionID: %lu | direction: RR | xPos: %d | yPos: %d | HP: %d\n", newPlayer->sessionID, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
}

void unicast(SESSION* p, Packet* payload)
{
    int retval;

    if (p == nullptr)
    {
        printf("Unicast 대상이 존재하지 않음\n");
        serverShut = true;
        return;
    }
    else if (p->writeQ.IsFull()) // 처리 불가능한 데이터로 링버터 가득참
    {
        printf("Unicast 링버퍼 가득참\n");
        disconnect(p);
        return;
    }

    retval = p->writeQ.Enqueue(payload->GetBufferPtr(), payload->GetDataSize());
    if (retval != payload->GetDataSize())
    {
        printf("[Unicast 페이로드 인큐 에러] 요청: %d 성공: %d\n", payload->GetDataSize(), retval);
        disconnect(p);
        serverShut = true;
        return;
    }
    payload->MoveReadPos(retval);
}

void broadcast(SESSION* p, Packet* payload)
{
    int retval;

    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        if ((*iter) != p)
        {
            if ((*iter)->writeQ.IsFull()) // 처리 불가능한 데이터 존재
            {
                printf("Broadcast 링버퍼 가득참");
                disconnect(*iter);
                continue;
            }

            retval = (*iter)->writeQ.Enqueue(payload->GetBufferPtr(), payload->GetDataSize());
            if (retval != payload->GetDataSize())
            {
                printf("[Broadcast 페이로드 인큐 에러] 요청: %d 성공: %d\n", payload->GetDataSize(), retval);
                disconnect(*iter);
                continue;
            }
        }
    }
    payload->MoveReadPos(retval);
}

void readProc(SESSION* p)
{
    if (p == nullptr)
        return;

    int retval;

    if (p->readQ.IsFull()) // 처리 불가능한 데이터로 링버퍼 가득 참
    {
        printf("수신 링버퍼 가득참");
        disconnect(p);
        return;
    }

    // tcp 수신버퍼 -> 링버퍼
    retval = recv(p->sock, p->readQ.GetRearPtr(), p->readQ.GetDirectEnqueueSize(), 0);
    if (retval == SOCKET_ERROR)
    {
        if (GetLastError() != WSAEWOULDBLOCK)
        {
            if (GetLastError() == WSAECONNRESET) // 클라이언트의 강제종료
            {
                disconnect(p);
                return;
            }
            printf("%d\n", GetLastError());
            serverShut = true;
            return;
        }
    }
    else if (retval == 0)
    {
        disconnect(p);
        return;
    }

    p->readQ.MoveRear(retval);

    while (1)
    {
        // 헤더 유무 확인
        if (p->readQ.GetUsedSize() < sizeof(Header))
            break;

        // 헤더 확인
        Header header;
        retval = p->readQ.Peek((char*)&header, sizeof(Header));
        if (retval != sizeof(Header))
        {
            printf("[Read 헤더 Peek 에러] 요청: %llu 성공: %d\n", sizeof(Header), retval);
            break;
        }
        
        // 헤더 코드 확인
        if (header.p_code != PACKET_CODE)
        {
            printf("패킷 코드 이상\n");
            break;
        }

        // 헤더 + 페이로드 확인
        if (p->readQ.GetUsedSize() < sizeof(Header) + header.p_size)
            break;

        // 헤더만큼 프론트 이동
        p->readQ.MoveFront(retval);

        Packet payload;
        retval = p->readQ.Dequeue(payload.GetBufferPtr(), header.p_size);
        if (retval != header.p_size)
        {
            printf("[Read 페이로드 디큐 에러] 요청: %lu 성공: %d\n", header.p_size, retval);
            break;
        }
        payload.MoveWritePos(retval);

        if (!packetProc(p, header.p_type, &payload))
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

    retval = send(p->sock, p->writeQ.GetFrontPtr(), p->writeQ.GetDirectDequeueSize(), 0);
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
    p->writeQ.MoveFront(retval);
}

void disconnect(SESSION* p)
{
    // [Server -> 모든 플레이어] 해당 유저 삭제 요청 Broadcast
    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        if ((*iter) != p)
        {
            Packet payload;
            createPacket_DELETE(&payload, p->sessionID);
            broadcast(p, &payload);
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