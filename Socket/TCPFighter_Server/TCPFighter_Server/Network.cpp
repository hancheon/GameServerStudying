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
    HEADER header;
    SC_CREATE_MY_CHARACTER sc_my_character;
    createPacket_CREATE_MY_CHARACTER(&header, (char*)&sc_my_character, newPlayer->sessionID, newPlayer->direction, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
    unicast(newPlayer, &header, (char*)&sc_my_character);

    // [���� -> ��� �÷��̾�] ���ο� �÷��̾� ���� Broadcast
    SC_CREATE_OTHER_CHARACTER sc_other_character;
    createPacket_CREATE_OTHER_CHARACTER(&header, (char*)&sc_other_character, newPlayer->sessionID, newPlayer->direction, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
    broadcast(nullptr, &header, (char*)&sc_other_character);

    // [���� -> ���ο� �÷��̾�] ���� �÷��̾� ���� Unicast
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

    // �÷��̾� ����Ʈ�� �߰�
    users.push_back(newPlayer);

    uniqueID++;

    printf("# [New User] SessionID: %lu | direction: RR | xPos: %d | yPos: %d | HP: %d\n", newPlayer->sessionID, newPlayer->xPos, newPlayer->yPos, newPlayer->HP);
}

void unicast(SESSION* p, HEADER* header, char* payload)
{
    int retval;

    if (p == nullptr)
    {
        printf("Unicast ����� �������� ����\n");
        serverShut = true;
        return;
    }
    else if (p->writeQ.GetFreeSize() == 0) // ó�� �Ұ����� �����ͷ� ������ ������
    {
        printf("Unicast ������ ������\n");
        disconnect(p);
        return;
    }

    retval = p->writeQ.Enqueue((char*)header, sizeof(HEADER));
    if (retval != sizeof(HEADER))
    {
        printf("[Unicast ��� ��ť ����] ��û: %llu ����: %d\n", sizeof(HEADER), retval);
        disconnect(p);
        serverShut = true;
        return;
    }

    retval = p->writeQ.Enqueue(payload, header->p_size);
    if (retval != header->p_size)
    {
        printf("[Unicast ���̷ε� ��ť ����] ��û: %d ����: %d\n", header->p_size, retval);
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
            if ((*iter)->writeQ.GetFreeSize() == 0) // ó�� �Ұ����� ������ ����
            {
                printf("Broadcast ������ ������");
                disconnect(*iter);
                continue;
            }

            retval = (*iter)->writeQ.Enqueue((char*)header, sizeof(HEADER));
            if (retval != sizeof(HEADER))
            {
                printf("[Broadcast ��� ��ť ����] ��û: %llu ����: %d\n", sizeof(HEADER), retval);
                disconnect(*iter);
                continue;
            }

            retval = (*iter)->writeQ.Enqueue(payload, header->p_size);
            if (retval != header->p_size)
            {
                printf("[Broadcast ���̷ε� ��ť ����] ��û: %d ����: %d\n", header->p_size, retval);
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

    if (p->readQ.GetFreeSize() == 0) // ó�� �Ұ����� �����ͷ� ������ ���� ��
    {
        printf("���� ������ ������");
        disconnect(p);
        return;
    }

    // tcp ���Ź��� -> ���� ����
    retval = recv(p->sock, buffer, min(MAX_BUFSIZE, p->readQ.GetFreeSize()), 0); // �����ۿ� ���� �� �ִ� �ִ�� ������
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

    // ���� ���� -> ������
    int retval_enq = p->readQ.Enqueue(buffer, retval);
    if (retval_enq != retval)
    {
        printf("[Read ��ť ����] ��û: %d ����: %d\n", retval, retval_enq);
        serverShut = true;
        return;
    }

    while (1)
    {
        // ��� ���� Ȯ��
        if (p->readQ.GetUsedSize() < sizeof(HEADER))
            break;

        // ��� Ȯ��
        HEADER header;
        retval = p->readQ.Peek((char*)&header, sizeof(HEADER));
        if (retval != sizeof(HEADER))
        {
            printf("[Read ��� Peek ����] ��û: %llu ����: %d\n", sizeof(HEADER), retval);
            break;
        }
        
        // ��� �ڵ� Ȯ��
        if (header.p_code != PACKET_CODE)
        {
            printf("��Ŷ �ڵ� �̻� GetUsedSize(): %d\n", p->readQ.GetUsedSize());
            break;
        }

        // ��� + ���̷ε� Ȯ��
        if (p->readQ.GetUsedSize() < sizeof(HEADER) + header.p_size)
            break;

        // �����ŭ ����Ʈ �̵�
        p->readQ.MoveFront(retval);

        char payload[100];
        retval = p->readQ.Dequeue(payload, header.p_size);
        if (retval != header.p_size)
        {
            printf("[Read ���̷ε� ��ť ����] ��û: %lu ����: %d\n", header.p_size, retval);
            break;
        }

        if (!packetProc(p, header.p_type, payload))
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
    // [Server -> ��� �÷��̾�] �ش� ���� ���� ��û Broadcast
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
    // ���� ���� ���� ���� ����
    myList<SESSION*>::iterator iter;
    for (iter = disconnects.begin(); iter != disconnects.end(); iter++)
    {
        closesocket((*iter)->sock);
        users.remove(*iter);
    }
    disconnects.clear();
}