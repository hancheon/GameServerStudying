#include <iostream>
#include <Windows.h>
#include <cmath>
#include "Session.h"
#include "Packet.h"
#include "Network.h"
#include "myList.h"

extern myList<SESSION*> users;
extern myList<SESSION*> disconnects;

bool packetProc(SESSION* p, BYTE packet_type, char* payload)
{
    if (p == nullptr)
        return false;

    switch (packet_type)
    {
    case dfPACKET_CS_MOVE_START:
        return packetProc_MOVE_START(p, payload);
    case dfPACKET_CS_MOVE_STOP:
        return packetProc_MOVE_STOP(p, payload);
    case dfPACKET_CS_ATTACK1:
        return packetProc_ATTACK1(p, payload);
    case dfPACKET_CS_ATTACK2:
        return packetProc_ATTACK2(p, payload);
    case dfPACKET_CS_ATTACK3:
        return packetProc_ATTACK3(p, payload);
    }

    return false;
}

bool packetProc_MOVE_START(SESSION* p, char* payload)
{
    CS_MOVE_START* msg = (CS_MOVE_START*)payload;
    if (abs(p->xPos - msg->xPos) > dfERROR_RANGE || abs(p->yPos - msg->yPos) > dfERROR_RANGE)
    {
        printf("허용 위치 벗어남\n");
        disconnect(p);
        return false;
    }
    p->action = msg->direction;

    switch (msg->direction) // 시야 변경
    {
    case dfPACKET_MOVE_DIR_LL:
    case dfPACKET_MOVE_DIR_LU:
    case dfPACKET_MOVE_DIR_LD:
        p->direction = dfPACKET_MOVE_DIR_LL;
        break;
    case dfPACKET_MOVE_DIR_RR:
    case dfPACKET_MOVE_DIR_RU:
    case dfPACKET_MOVE_DIR_RD:
        p->direction = dfPACKET_MOVE_DIR_RR;
        break;
    }

    p->xPos = msg->xPos;
    p->yPos = msg->yPos;
    printf("[MOVE_START] SessionID: %d | direction: %d | xPos: %d | yPos: %d\n", p->sessionID, p->action, p->xPos, p->yPos);

    HEADER header;
    SC_MOVE_START newPayload;
    if (!createPacket_MOVE_START(&header, (char*)&payload, p->sessionID, p->direction, p->xPos, p->yPos))
    {
        printf("[MOVE_START] 패킷 생성 오류\n");
        return false;
    }
    broadcast(p, &header, (char*)&payload);

    return true;
}

bool packetProc_MOVE_STOP(SESSION* p, char* payload)
{
    CS_MOVE_STOP* msg = (CS_MOVE_STOP*)payload;
    if (abs(p->xPos - msg->xPos) > dfERROR_RANGE || abs(p->yPos - msg->yPos) > dfERROR_RANGE)
    {
        printf("허용 위치 벗어남\n");
        disconnect(p);
        return false;
    }
    p->direction = msg->direction;
    p->xPos = msg->xPos;
    p->yPos = msg->yPos;
    printf("[MOVE_STOP]  SessionID: %d | direction: %d | xPos: %d | yPos: %d\n", p->sessionID, p->action, p->xPos, p->yPos);

    switch (msg->direction) // 시야 변경
    {
    case dfPACKET_MOVE_DIR_LL:
    case dfPACKET_MOVE_DIR_LU:
    case dfPACKET_MOVE_DIR_LD:
        p->direction = dfPACKET_MOVE_DIR_LL;
        break;
    case dfPACKET_MOVE_DIR_RR:
    case dfPACKET_MOVE_DIR_RU:
    case dfPACKET_MOVE_DIR_RD:
        p->direction = dfPACKET_MOVE_DIR_RR;
        break;
    }

    HEADER header;
    SC_MOVE_STOP newPayload;
    if (!createPacket_MOVE_STOP(&header, (char*)&payload, p->sessionID, p->direction, p->xPos, p->yPos))
    {
        printf("[MOVE_STOP]  패킷 생성 오류\n");
        return false;
    }
    broadcast(p, &header, (char*)&payload);
    
    return true;
}

// 현재 게임에는 공격 쿨타임x
bool packetProc_ATTACK1(SESSION* p, char* payload)
{
    CS_ATTACK1* msg = (CS_ATTACK1*)payload;
    
    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        if ((*iter) != p)
        {
            if (((*iter)->xPos == msg->xPos) && ((*iter)->yPos == msg->yPos))
            {
                (*iter)->hp -= 5;
                printf("[ATTACK1] Attacker: %d, Demaged: %d, HP: %d\n", p->sessionID, (*iter)->sessionID, (*iter)->hp);
            }
        }
    }

    HEADER header;
    SC_ATTACK1 newPayload;
    if (!createPacket_ATTACK1(&header, (char*)&payload, p->sessionID, p->direction, p->xPos, p->yPos))
    {
        printf("[ATTACK1] 패킷 생성 오류\n");
        return false;
    }
    broadcast(p, &header, (char*)&payload);

    return true;
}

bool packetProc_ATTACK2(SESSION* p, char* payload)
{
    CS_ATTACK2* msg = (CS_ATTACK2*)payload;

    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        if ((*iter) != p)
        {
            if (((*iter)->xPos == msg->xPos) && ((*iter)->yPos == msg->yPos))
            {
                (*iter)->hp -= 5;
                printf("[ATTACK2] Attacker: %d, Demaged: %d, HP: %d\n", p->sessionID, (*iter)->sessionID, (*iter)->hp);
            }
        }
    }

    HEADER header;
    SC_ATTACK2 newPayload;
    if (!createPacket_ATTACK2(&header, (char*)&payload, p->sessionID, p->direction, p->xPos, p->yPos))
    {
        printf("[ATTACK2] 패킷 생성 오류\n");
        return false;
    }
    broadcast(p, &header, (char*)&payload);

    return true;
}

bool packetProc_ATTACK3(SESSION* p, char* payload)
{
    CS_ATTACK3* msg = (CS_ATTACK3*)payload;

    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        if ((*iter) != p)
        {
            if (((*iter)->xPos == msg->xPos) && ((*iter)->yPos == msg->yPos))
            {
                (*iter)->hp -= 10;
                printf("[ATTACK3] Attacker: %d, Demaged: %d, HP: %d\n", p->sessionID, (*iter)->sessionID, (*iter)->hp);
            }
        }
    }

    HEADER header;
    SC_ATTACK3 newPayload;
    if (!createPacket_ATTACK3(&header, (char*)&payload, p->sessionID, p->direction, p->xPos, p->yPos))
    {
        printf("[ATTACK3] 패킷 생성 오류\n");
        return false;
    }
    broadcast(p, &header, (char*)&payload);

    return true;
}

bool createPacket_MOVE_START(HEADER* header, char* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
    return true;
}

bool createPacket_MOVE_STOP(HEADER* header, char* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
    return true;
}

bool createPacket_ATTACK1(HEADER* header, char* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
    return true;
}

bool createPacket_ATTACK2(HEADER* header, char* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
    return true;
}

bool createPacket_ATTACK3(HEADER* header, char* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
    return true;
}
