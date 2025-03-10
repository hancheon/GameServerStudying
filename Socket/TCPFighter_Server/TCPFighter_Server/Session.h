#pragma once

#include <Windows.h>
#include "RingBuffer.h"

struct SESSION
{
    SOCKET sock;
    DWORD sessionID;
    RingBuffer readQ;
    RingBuffer writeQ;

    DWORD action; // 실제 방향
    BYTE direction; // 캐릭터 시야 방향

    short xPos;
    short yPos;
    char hp;
};