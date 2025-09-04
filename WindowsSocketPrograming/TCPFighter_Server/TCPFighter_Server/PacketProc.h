#pragma once
#include "Protocol.h"
#include "Packet.h"

#define dfERROR_RANGE 50
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20

#define DAMAGE1 3
#define DAMAGE2 5

// recv한 패킷 처리 함수
bool packetProc(SESSION*, BYTE, Packet*);
bool packetProc_MOVE_START(SESSION*, Packet*);
bool packetProc_MOVE_STOP(SESSION*, Packet*);
bool packetProc_ATTACK1(SESSION*, Packet*);
bool packetProc_ATTACK2(SESSION*, Packet*);
bool packetProc_ATTACK3(SESSION*, Packet*);

// 패킷 생성 함수
void createPacket_CREATE_MY_CHARACTER(Packet*, DWORD, BYTE, short, short, char);
void createPacket_CREATE_OTHER_CHARACTER(Packet*, DWORD, BYTE, short, short, char);
void createPacket_MOVE_START(Packet*, DWORD, BYTE, short, short);
void createPacket_MOVE_STOP(Packet*, DWORD, BYTE, short, short);
void createPacket_ATTACK1(Packet*, DWORD, BYTE, short, short);
void createPacket_ATTACK2(Packet*, DWORD, BYTE, short, short);
void createPacket_ATTACK3(Packet*, DWORD, BYTE, short, short);
void createPacket_DAMAGE(Packet*, DWORD, DWORD, char);
void createPacket_DELETE(Packet*, DWORD);