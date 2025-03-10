#pragma once
#include "Protocol.h"

#define dfERROR_RANGE 50

// recv�� ��Ŷ ó�� �Լ�
bool packetProc(SESSION*, BYTE, char*);
bool packetProc_MOVE_START(SESSION*, char*);
bool packetProc_MOVE_STOP(SESSION*, char*);
bool packetProc_ATTACK1(SESSION*, char*);
bool packetProc_ATTACK2(SESSION*, char*);
bool packetProc_ATTACK3(SESSION*, char*);

// ��Ŷ ���� �Լ�
bool createPacket_MOVE_START(HEADER*, char*, DWORD, BYTE, short, short);
bool createPacket_MOVE_STOP(HEADER*, char*, DWORD, BYTE, short, short);
bool createPacket_ATTACK1(HEADER*, char*, DWORD, BYTE, short, short);
bool createPacket_ATTACK2(HEADER*, char*, DWORD, BYTE, short, short);
bool createPacket_ATTACK3(HEADER*, char*, DWORD, BYTE, short, short);
