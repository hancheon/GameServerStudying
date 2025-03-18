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

    // read�¿� ���� ���� ���
    FD_SET(listen_sock, &rSet);

    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        FD_SET((*iter)->sock, &rSet); // ���� ���� �𸣴� ��� ���
        if ((*iter)->writeQ.GetUsedSize() > 0) // �� �� �ִ� ���ϸ� ���
            FD_SET((*iter)->sock, &wSet);
    }

    timeval timeout; // ���X ������ while������ ��� �ݺ���
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    retval = select(0, &rSet, &wSet, nullptr, &timeout);
    if (retval > 0)
    {
        // accept()
        if (FD_ISSET(listen_sock, &rSet)) // ���� ���� ���� ��û Ȯ��
        {
            acceptProc();
        }

        for (iter = users.begin(); iter != users.end(); iter++)
        {
            if (FD_ISSET((*iter)->sock, &rSet)) // readSet ����
            {
                readProc(*iter);
            }
            if (FD_ISSET((*iter)->sock, &wSet)) // writeSet ����
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

    // ���ο� �÷��̾� ������ �ʱ�ȭ
    SESSION* newPlayer = new SESSION;
    newPlayer->sock = client_sock;
    newPlayer->sessionID = uniqueID;
    newPlayer->direction = dfPACKET_MOVE_DIR_RR;
    newPlayer->action = dfPACKET_MOVE_STOP; // ĳ���� �ൿ ���� Ȯ�ο� ����
    newPlayer->xPos = (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) / 2;
    newPlayer->yPos = (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) / 2;
    newPlayer->HP = MAX_HP;

    // [���� -> ���ο� �÷��̾�] ID �Ҵ� Unicast
    Packet payload;
    createPacket_CREATE_MY_CHARACTER(&payload, newPlayer->sessionID, newPlayer->direction, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
    unicast(newPlayer, &payload);

    // [���� -> ��� �÷��̾�] ���ο� �÷��̾� ���� Broadcast
    payload.Clear();
    createPacket_CREATE_OTHER_CHARACTER(&payload, newPlayer->sessionID, newPlayer->direction, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
    broadcast(nullptr, &payload);

    // [���� -> ���ο� �÷��̾�] ���� �÷��̾� ���� Unicast
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

    // �÷��̾� ����Ʈ�� �߰�
    users.push_back(newPlayer);

    uniqueID++;

    printf("# [New User] SessionID: %lu | direction: RR | xPos: %d | yPos: %d | HP: %d\n", newPlayer->sessionID, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
}

void unicast(SESSION* p, Packet* payload)
{
    int retval;

    if (p == nullptr)
    {
        printf("Unicast ����� �������� ����\n");
        serverShut = true;
        return;
    }
    else if (p->writeQ.IsFull()) // ó�� �Ұ����� �����ͷ� ������ ������
    {
        printf("Unicast ������ ������\n");
        disconnect(p);
        return;
    }

    retval = p->writeQ.Enqueue(payload->GetBufferPtr(), payload->GetDataSize());
    if (retval != payload->GetDataSize())
    {
        printf("[Unicast ���̷ε� ��ť ����] ��û: %d ����: %d\n", payload->GetDataSize(), retval);
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
            if ((*iter)->writeQ.IsFull()) // ó�� �Ұ����� ������ ����
            {
                printf("Broadcast ������ ������");
                disconnect(*iter);
                continue;
            }

            retval = (*iter)->writeQ.Enqueue(payload->GetBufferPtr(), payload->GetDataSize());
            if (retval != payload->GetDataSize())
            {
                printf("[Broadcast ���̷ε� ��ť ����] ��û: %d ����: %d\n", payload->GetDataSize(), retval);
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

    if (p->readQ.IsFull()) // ó�� �Ұ����� �����ͷ� ������ ���� ��
    {
        printf("���� ������ ������");
        disconnect(p);
        return;
    }

    // tcp ���Ź��� -> ������
    retval = recv(p->sock, p->readQ.GetRearPtr(), p->readQ.GetDirectEnqueueSize(), 0);
    if (retval == SOCKET_ERROR)
    {
        if (GetLastError() != WSAEWOULDBLOCK)
        {
            if (GetLastError() == WSAECONNRESET) // Ŭ���̾�Ʈ�� ��������
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
        // ��� ���� Ȯ��
        if (p->readQ.GetUsedSize() < sizeof(Header))
            break;

        // ��� Ȯ��
        Header header;
        retval = p->readQ.Peek((char*)&header, sizeof(Header));
        if (retval != sizeof(Header))
        {
            printf("[Read ��� Peek ����] ��û: %llu ����: %d\n", sizeof(Header), retval);
            break;
        }
        
        // ��� �ڵ� Ȯ��
        if (header.p_code != PACKET_CODE)
        {
            printf("��Ŷ �ڵ� �̻�\n");
            break;
        }

        // ��� + ���̷ε� Ȯ��
        if (p->readQ.GetUsedSize() < sizeof(Header) + header.p_size)
            break;

        // �����ŭ ����Ʈ �̵�
        p->readQ.MoveFront(retval);

        Packet payload;
        retval = p->readQ.Dequeue(payload.GetBufferPtr(), header.p_size);
        if (retval != header.p_size)
        {
            printf("[Read ���̷ε� ��ť ����] ��û: %lu ����: %d\n", header.p_size, retval);
            break;
        }
        payload.MoveWritePos(retval);

        if (!packetProc(p, header.p_type, &payload))
        {
            printf("[packetProc() ����]\n");
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
    // [Server -> ��� �÷��̾�] �ش� ���� ���� ��û Broadcast
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
    // ���� ���� ���� ���� ����
    myList<SESSION*>::iterator iter;
    for (iter = disconnects.begin(); iter != disconnects.end(); iter++)
    {
        closesocket((*iter)->sock);
        users.remove(*iter);
    }
    disconnects.clear();
}