#pragma once

#include <Windows.h>
#include "RingBuffer.h"

#define MAX_HP 100

struct SESSION
{
    SOCKET sock;
    DWORD sessionID;
    RingBuffer readQ;
    RingBuffer writeQ;

    BYTE action; // 캐릭터 상태 (이동)
    BYTE direction; // 캐릭터 시야 방향 LL | RR

    short xPos;
    short yPos;
    char HP;
};