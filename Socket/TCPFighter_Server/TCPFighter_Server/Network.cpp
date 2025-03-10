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

    // set ����
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

    // ���ο� �÷��̾� ������ �ʱ�ȭ
    SESSION* newPlayer = new SESSION;
    newPlayer->sock = client_sock;
    newPlayer->sessionID = uniqueID;
    newPlayer->direction = dfPACKET_MOVE_DIR_RR;
    newPlayer->xPos = (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) / 2;
    newPlayer->yPos = (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) / 2;
    newPlayer->hp = MAX_HP;

    // [���� -> ���ο� �÷��̾�] ID �Ҵ� Unicast
    HEADER header;
    header.p_code = PACKET_CODE;
    header.p_size = sizeof(SC_CREATE_MY_CHARACTER);
    header.p_type = dfPACKET_SC_CREATE_MY_CHARACTER;

    SC_CREATE_MY_CHARACTER sc_my_character;
    sc_my_character.ID = newPlayer->sessionID;
    sc_my_character.direction = newPlayer->direction;
    sc_my_character.xPos = newPlayer->xPos;
    sc_my_character.yPos = newPlayer->yPos;
    sc_my_character.hp = newPlayer->hp;
    if (!unicast(newPlayer, &header, (char*)&sc_my_character))
        return false;

    // [���� -> ��� �÷��̾�] ���ο� �÷��̾� ���� Broadcast
    header.p_size = sizeof(SC_CREATE_OTHER_CHARACTER);
    header.p_type = dfPACKET_SC_CREATE_OTHER_CHARACTER;

    SC_CREATE_OTHER_CHARACTER sc_other_character;
    sc_other_character.ID = newPlayer->sessionID;
    sc_other_character.direction = newPlayer->direction;
    sc_other_character.xPos = newPlayer->xPos;
    sc_other_character.yPos = newPlayer->yPos;
    sc_other_character.hp = newPlayer->hp;
    if (!broadcast(nullptr, &header, (char*)&sc_other_character))
        return false;

    // [���� -> ���ο� �÷��̾�] ���� �÷��̾� ���� Unicast
    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        sc_other_character.ID = (*iter)->sessionID;
        sc_other_character.direction = (*iter)->direction;
        sc_other_character.xPos = (*iter)->xPos;
        sc_other_character.yPos = (*iter)->yPos;
        sc_other_character.hp = (*iter)->hp;
        if (!unicast(newPlayer, &header, (char*)&sc_other_character))
            return false;
    }

    // �÷��̾� ����Ʈ�� �߰�
    users.push_back(newPlayer);
    uniqueID++;

    wprintf(L"[���ο� ���� ����]\n");

    return true;
}

bool unicast(SESSION* p, HEADER* header, char* payload)
{
    int retval;

    if (p == nullptr)
    {
        printf("Unicast ����� �������� ����\n");
        return false;
    }
    else if (p->writeQ.GetFreeSize() == 0) // ó�� �Ұ����� ������ ����
    {
        printf("Unicast ������ ������\n");
        disconnect(p);
        return true;
    }

    retval = p->writeQ.Enqueue((char*)header, sizeof(HEADER));
    if (retval != sizeof(HEADER))
    {
        printf("[Unicast ��� ��ť ����] ��û: %llu ����: %d\n", sizeof(HEADER), retval);
        disconnect(p);
        return true;
    }

    retval = p->writeQ.Enqueue(payload, header->p_size);
    if (retval != header->p_size)
    {
        printf("[Unicast ���̷ε� ��ť ����] ��û: %d ����: %d\n", header->p_size, retval);
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
                return true;
            }

            retval = (*iter)->writeQ.Enqueue(payload, header->p_size);
            if (retval != sizeof(payload))
            {
                printf("[Broadcast ���̷ε� ��ť ����] ��û: %d ����: %d\n", header->p_size, retval);
                disconnect(*iter);
                return true;
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
        printf("���� ������ ������");
        disconnect(p);
        return true;
    }

    // tcp ���Ź��� -> ���� ����
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

    // ���� ���� -> ������
    int retval_enq = p->readQ.Enqueue(buffer, retval);
    if (retval_enq != retval)
    {
        printf("[Read ��ť ����] ��û: %d ����: %d\n", retval, retval_enq);
        return false;
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
            return false;
        }

        // ��� + ���̷ε� Ȯ��
        if (p->readQ.GetUsedSize() < sizeof(HEADER) + header.p_size)
            break;

        // �����ŭ ����Ʈ �̵�
        p->readQ.MoveFront(retval);

        char payload[30];
        retval = p->readQ.Dequeue(payload, header.p_size);
        if (retval != header.p_size)
        {
            printf("[Read ���̷ε� ��ť ����] ��û: %llu ����: %d\n", sizeof(HEADER), retval);
            return false;
        }

        if (!packetProc(p, header.p_type, payload))
        {
            wprintf(L"[packetProc() ����]\n");
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
    // [Server -> ��� �÷��̾�] �ش� ���� ���� ��û Broadcast
    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        if ((*iter) != p)
        {

        }
    }

    disconnects.push_back(p);

    return true;
}

bool deleteUser(SESSION*)
{
    // ���� ���� ���� ���� ����
    myList<SESSION*>::iterator iter;
    for (iter = disconnects.begin(); iter != disconnects.end(); iter++)
    {

    }

    return true;
}