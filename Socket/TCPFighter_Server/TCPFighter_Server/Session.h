#pragma once

#include <Windows.h>
#include "RingBuffer.h"

struct SESSION
{
    SOCKET sock;
    DWORD sessionID;
    RingBuffer readQ;
    RingBuffer writeQ;

    DWORD action; // ĳ���� ���� (�̵�)
    BYTE direction; // ĳ���� �þ� ����

    short xPos;
    short yPos;
    char HP;
};