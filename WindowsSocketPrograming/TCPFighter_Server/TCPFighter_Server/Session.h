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

    BYTE action; // ĳ���� ���� (�̵�)
    BYTE direction; // ĳ���� �þ� ���� LL | RR

    short xPos;
    short yPos;
    char HP;
};