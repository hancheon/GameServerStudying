#pragma once
#include "Protocol.h"

#define dfERROR_RANGE 50
#define DAMAGE 10
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20

#define NOT_MOVE 111

// recv한 패킷 처리 함수
bool packetProc(SESSION*, BYTE, const char*);
bool packetProc_MOVE_START(SESSION*, const char*);
bool packetProc_MOVE_STOP(SESSION*, const char*);
bool packetProc_ATTACK1(SESSION*, const char*);
bool packetProc_ATTACK2(SESSION*, const char*);
bool packetProc_ATTACK3(SESSION*, const char*);

// 패킷 생성 함수
void createPacket_CREATE_MY_CHARACTER(HEADER*, char*, DWORD, BYTE, short, short, char);
void createPacket_CREATE_OTHER_CHARACTER(HEADER*, char*, DWORD, BYTE, short, short, char);
void createPacket_MOVE_START(HEADER*, char*, DWORD, BYTE, short, short);
void createPacket_MOVE_STOP(HEADER*, char*, DWORD, BYTE, short, short);
void createPacket_ATTACK1(HEADER*, char*, DWORD, BYTE, short, short);
void createPacket_ATTACK2(HEADER*, char*, DWORD, BYTE, short, short);
void createPacket_ATTACK3(HEADER*, char*, DWORD, BYTE, short, short);
void createPacket_DAMAGE(HEADER*, char*, DWORD, DWORD, char);
void createPacket_DELETE(HEADER*, char*, DWORD);