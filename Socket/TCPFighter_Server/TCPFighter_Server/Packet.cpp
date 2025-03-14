#include <iostream>
#include <Windows.h>
#include <cmath>
#include "Session.h"
#include "Packet.h"
#include "Network.h"
#include "myList.h"

extern myList<SESSION*> users;
extern myList<SESSION*> disconnects;

//====================================================================//
// 받은 패킷 분류 함수
//====================================================================//
bool packetProc(SESSION* p, BYTE packet_type, const char* payload)
{
	if (p == nullptr || payload == nullptr)
	{
		printf("유효하지 않은 유저 또는 페이로드 주소 이상\n");
		return false;
	}

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

//====================================================================//
// 분류된 패킷 처리 함수
//====================================================================//
bool packetProc_MOVE_START(SESSION* p, const char* payload)
{
	CS_MOVE_START* msg = (CS_MOVE_START*)payload;
	if (abs(p->xPos - msg->xPos) > dfERROR_RANGE || abs(p->yPos - msg->yPos) > dfERROR_RANGE)
	{
		printf("Error: MOVE_START 좌표 허용 오차 초과\n");
		printf("요청 xpos: %d ypos: %d | 서버 xpos: %d ypos: %d\n", msg->xPos, msg->yPos, p->xPos, p->yPos);
		disconnect(p);
		return false;
	}
	p->xPos = msg->xPos;
	p->yPos = msg->yPos;
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

	HEADER header;
	SC_MOVE_START newPayload;
	createPacket_MOVE_START(&header, (char*)&newPayload, p->sessionID, p->action, p->xPos, p->yPos);
	broadcast(p, &header, (char*)&newPayload);

	printf("# [MOVE_START] SessionID: %d | direction: %d | xPos: %d | yPos: %d\n", p->sessionID, p->action, p->xPos, p->yPos);

	return true;
}

bool packetProc_MOVE_STOP(SESSION* p, const char* payload)
{
	CS_MOVE_STOP* msg = (CS_MOVE_STOP*)payload;
	if (abs(p->xPos - msg->xPos) > dfERROR_RANGE || abs(p->yPos - msg->yPos) > dfERROR_RANGE)
	{
		printf("Error: MOVE_STOP 좌표 허용 오차 초과\n");
		printf("요청 xpos: %d ypos: %d | 서버 xpos: %d ypos: %d\n", msg->xPos, msg->yPos, p->xPos, p->yPos);
		disconnect(p);
		return false;
	}
	p->direction = msg->direction;
	p->xPos = msg->xPos;
	p->yPos = msg->yPos;
	p->action = NOT_MOVE;

	HEADER header;
	SC_MOVE_STOP newPayload;
	createPacket_MOVE_STOP(&header, (char*)&newPayload, p->sessionID, p->direction, p->xPos, p->yPos);
	broadcast(p, &header, (char*)&newPayload);

	printf("# [MOVE_STOP]  SessionID: %d | direction: %d | xPos: %d | yPos: %d\n", p->sessionID, p->direction, p->xPos, p->yPos);

	return true;
}

bool packetProc_ATTACK1(SESSION* p, const char* payload)
{
	CS_ATTACK1* msg = (CS_ATTACK1*)payload;

	// 공격 모션 지시
	HEADER header;
	SC_ATTACK1 sc_attack_payload;
	createPacket_ATTACK1(&header, (char*)&sc_attack_payload, p->sessionID, p->direction, p->xPos, p->yPos);
	broadcast(p, &header, (char*)&sc_attack_payload);

	// 공격 판정 후 결과 통보
	myList<SESSION*>::iterator iter;
	for (iter = users.begin(); iter != users.end(); iter++)
	{
		if ((*iter) != p)
		{
			if (msg->direction == dfPACKET_MOVE_DIR_LL)
			{
				if ((msg->xPos - (*iter)->xPos > dfATTACK1_RANGE_X) || (msg->xPos - (*iter)->xPos < 0) || abs((*iter)->yPos - msg->yPos) > dfATTACK1_RANGE_Y)
					continue;
			}
			else
			{
				if (((*iter)->xPos - msg->xPos > dfATTACK1_RANGE_X) || ((*iter)->xPos - msg->xPos < 0) || abs((*iter)->yPos - msg->yPos) > dfATTACK1_RANGE_Y)
					continue;
			}

			if ((*iter)->HP > 0)
			{
				(*iter)->HP -= DAMAGE1;
			}
			else
			{
				disconnect((*iter));
			}

			HEADER header;
			SC_DAMAGE newPayload;
			createPacket_DAMAGE(&header, (char*)&newPayload, p->sessionID, (*iter)->sessionID, (*iter)->HP);
			broadcast(nullptr, &header, (char*)&newPayload);
			printf("# [ATTACK1] AttackerID: %d | DemagedID: %d | HP: %d\n", p->sessionID, (*iter)->sessionID, (*iter)->HP);

		}
	}

	return true;
}

bool packetProc_ATTACK2(SESSION* p, const char* payload)
{
	CS_ATTACK2* msg = (CS_ATTACK2*)payload;

	// 공격 모션 지시
	HEADER header;
	SC_ATTACK2 sc_attack_payload;
	createPacket_ATTACK2(&header, (char*)&sc_attack_payload, p->sessionID, p->direction, p->xPos, p->yPos);
	broadcast(p, &header, (char*)&sc_attack_payload);

	myList<SESSION*>::iterator iter;
	for (iter = users.begin(); iter != users.end(); iter++)
	{
		if ((*iter) != p)
		{
			if (msg->direction == dfPACKET_MOVE_DIR_LL)
			{
				if ((msg->xPos - (*iter)->xPos > dfATTACK2_RANGE_X) || (msg->xPos - (*iter)->xPos < 0) || abs((*iter)->yPos - msg->yPos) > dfATTACK2_RANGE_Y)
					continue;
			}
			else
			{
				if (((*iter)->xPos - msg->xPos > dfATTACK2_RANGE_X) || ((*iter)->xPos - msg->xPos < 0) || abs((*iter)->yPos - msg->yPos) > dfATTACK2_RANGE_Y)
					continue;
			}

			if ((*iter)->HP > 0)
			{
				(*iter)->HP -= DAMAGE1;
			}
			else
			{
				disconnect((*iter));
			}

			HEADER header;
			SC_DAMAGE newPayload;
			createPacket_DAMAGE(&header, (char*)&newPayload, p->sessionID, (*iter)->sessionID, (*iter)->HP);
			broadcast(nullptr, &header, (char*)&newPayload);
			printf("# [ATTACK2] AttackerID: %d | DemagedID: %d | HP: %d\n", p->sessionID, (*iter)->sessionID, (*iter)->HP);
		}
	}

	return true;
}

bool packetProc_ATTACK3(SESSION* p, const char* payload)
{
	CS_ATTACK3* msg = (CS_ATTACK3*)payload;

	// 공격 모션 지시
	HEADER header;
	SC_ATTACK3 sc_attack_payload;
	createPacket_ATTACK3(&header, (char*)&sc_attack_payload, p->sessionID, p->direction, p->xPos, p->yPos);
	broadcast(p, &header, (char*)&sc_attack_payload);

	// 공격 판정 후 결과 통보
	myList<SESSION*>::iterator iter;
	for (iter = users.begin(); iter != users.end(); iter++)
	{
		if ((*iter) != p)
		{
			if (msg->direction == dfPACKET_MOVE_DIR_LL)
			{
				if ((msg->xPos - (*iter)->xPos > dfATTACK3_RANGE_X) || (msg->xPos - (*iter)->xPos < 0) || abs((*iter)->yPos - msg->yPos) > dfATTACK3_RANGE_Y)
					continue;
			}
			else
			{
				if (((*iter)->xPos - msg->xPos > dfATTACK3_RANGE_X) || ((*iter)->xPos - msg->xPos < 0) || abs((*iter)->yPos - msg->yPos) > dfATTACK3_RANGE_Y)
					continue;
			}

			if ((*iter)->HP > 0)
			{
				(*iter)->HP -= DAMAGE2;
			}
			else
			{
				disconnect((*iter));
			}

			HEADER header;
			SC_DAMAGE newPayload;
			createPacket_DAMAGE(&header, (char*)&newPayload, p->sessionID, (*iter)->sessionID, (*iter)->HP);
			broadcast(nullptr, &header, (char*)&newPayload);
			printf("# [ATTACK3] AttackerID: %d | DemagedID: %d | HP: %d\n", p->sessionID, (*iter)->sessionID, (*iter)->HP);

		}
	}

	return true;
}

//====================================================================//
// 패킷 생성 함수
//====================================================================//
void createPacket_CREATE_MY_CHARACTER(HEADER* header, char* payload, DWORD ID, BYTE direction, short xPos, short yPos, char HP)
{
	header->p_code = PACKET_CODE;
	header->p_size = sizeof(SC_CREATE_MY_CHARACTER);
	header->p_type = dfPACKET_SC_CREATE_MY_CHARACTER;

	SC_CREATE_MY_CHARACTER* msg = (SC_CREATE_MY_CHARACTER*)payload;
	msg->ID = ID;
	msg->direction = direction;
	msg->xPos = xPos;
	msg->yPos = yPos;
	msg->HP = HP;
}

void createPacket_CREATE_OTHER_CHARACTER(HEADER* header, char* payload, DWORD ID, BYTE direction, short xPos, short yPos, char HP)
{
	header->p_code = PACKET_CODE;
	header->p_size = sizeof(SC_CREATE_OTHER_CHARACTER);
	header->p_type = dfPACKET_SC_CREATE_OTHER_CHARACTER;

	SC_CREATE_OTHER_CHARACTER* msg = (SC_CREATE_OTHER_CHARACTER*)payload;
	msg->ID = ID;
	msg->direction = direction;
	msg->xPos = xPos;
	msg->yPos = yPos;
	msg->HP = HP;
}

void createPacket_MOVE_START(HEADER* header, char* payload, DWORD ID, BYTE action, short xPos, short yPos)
{
	header->p_code = PACKET_CODE;
	header->p_size = sizeof(SC_MOVE_START);
	header->p_type = dfPACKET_SC_MOVE_START;

	SC_MOVE_START* msg = (SC_MOVE_START*)payload;
	msg->ID = ID;
	msg->direction = action;
	msg->xPos = xPos;
	msg->yPos = yPos;
}

void createPacket_MOVE_STOP(HEADER* header, char* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
	header->p_code = PACKET_CODE;
	header->p_size = sizeof(SC_MOVE_STOP);
	header->p_type = dfPACKET_SC_MOVE_STOP;

	SC_MOVE_STOP* msg = (SC_MOVE_STOP*)payload;
	msg->ID = ID;
	msg->direction = direction;
	msg->xPos = xPos;
	msg->yPos = yPos;
}

void createPacket_ATTACK1(HEADER* header, char* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
	header->p_code = PACKET_CODE;
	header->p_size = sizeof(SC_ATTACK1);
	header->p_type = dfPACKET_SC_ATTACK1;

	SC_ATTACK1* msg = (SC_ATTACK1*)payload;
	msg->ID = ID;
	msg->direction = direction;
	msg->xPos = xPos;
	msg->yPos = yPos;
}

void createPacket_ATTACK2(HEADER* header, char* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
	header->p_code = PACKET_CODE;
	header->p_size = sizeof(SC_ATTACK2);
	header->p_type = dfPACKET_SC_ATTACK2;

	SC_ATTACK2* msg = (SC_ATTACK2*)payload;
	msg->ID = ID;
	msg->direction = direction;
	msg->xPos = xPos;
	msg->yPos = yPos;
}

void createPacket_ATTACK3(HEADER* header, char* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
	header->p_code = PACKET_CODE;
	header->p_size = sizeof(SC_ATTACK3);
	header->p_type = dfPACKET_SC_ATTACK3;

	SC_ATTACK3* msg = (SC_ATTACK3*)payload;
	msg->ID = ID;
	msg->direction = direction;
	msg->xPos = xPos;
	msg->yPos = yPos;
}

void createPacket_DAMAGE(HEADER* header, char* payload, DWORD attackerID, DWORD damagedID, char HP)
{
	header->p_code = PACKET_CODE;
	header->p_size = sizeof(SC_DAMAGE);
	header->p_type = dfPACKET_SC_DAMAGE;

	SC_DAMAGE* msg = (SC_DAMAGE*)payload;
	msg->attackerID = attackerID;
	msg->damagedID = damagedID;
	msg->remainHP = HP;
}

void createPacket_DELETE(HEADER* header, char* payload, DWORD ID)
{
	header->p_code = PACKET_CODE;
	header->p_size = sizeof(SC_DELETE_CHARACTER);
	header->p_type = dfPACKET_SC_DELETE_CHARACTER;

	SC_DELETE_CHARACTER* msg = (SC_DELETE_CHARACTER*)payload;
	msg->ID = ID;
}