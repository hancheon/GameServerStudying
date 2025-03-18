#include <iostream>
#include <Windows.h>
#include <cmath>
#include "Session.h"
#include "PacketProc.h"
#include "Network.h"
#include "myList.h"
#include "Packet.h"

extern myList<SESSION*> users;
extern myList<SESSION*> disconnects;

//====================================================================//
// 받은 패킷 분류 함수
//====================================================================//
bool packetProc(SESSION* p, BYTE packet_type, Packet* payload)
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
bool packetProc_MOVE_START(SESSION* p, Packet* payload)
{
	BYTE action;
	short xPos;
	short yPos;

	*payload >> action >> xPos >> yPos;

	if (abs(p->xPos - xPos) > dfERROR_RANGE || abs(p->yPos - yPos) > dfERROR_RANGE)
	{
		printf("Error: MOVE_START 좌표 허용 오차 초과\n");
		printf("요청 xpos: %d ypos: %d | 서버 xpos: %d ypos: %d\n", xPos, yPos, p->xPos, p->yPos);
		disconnect(p);
		return false;
	}

	p->action = action;
	p->xPos = xPos;
	p->yPos = yPos;

	switch (p->action) // 시야 변경
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

	Packet moveStart;
	createPacket_MOVE_START(&moveStart, p->sessionID, p->action, p->xPos, p->yPos);
	broadcast(p, &moveStart);

	switch (p->action)
	{
	case dfPACKET_MOVE_DIR_LL:
		printf("# [MOVE] SessionID: %d | direction: LL | xPos: %d | yPos: %d\n", p->sessionID, p->xPos, p->yPos);
		break;
	case dfPACKET_MOVE_DIR_LU:
		printf("# [MOVE] SessionID: %d | direction: LU | xPos: %d | yPos: %d\n", p->sessionID, p->xPos, p->yPos);
		break;
	case dfPACKET_MOVE_DIR_UU:
		printf("# [MOVE] SessionID: %d | direction: UU | xPos: %d | yPos: %d\n", p->sessionID, p->xPos, p->yPos);
		break;
	case dfPACKET_MOVE_DIR_RU:
		printf("# [MOVE] SessionID: %d | direction: RU | xPos: %d | yPos: %d\n", p->sessionID, p->xPos, p->yPos);
		break;
	case dfPACKET_MOVE_DIR_RR:
		printf("# [MOVE] SessionID: %d | direction: RR | xPos: %d | yPos: %d\n", p->sessionID, p->xPos, p->yPos);
		break;
	case dfPACKET_MOVE_DIR_RD:
		printf("# [MOVE] SessionID: %d | direction: RD | xPos: %d | yPos: %d\n", p->sessionID, p->xPos, p->yPos);
		break;
	case dfPACKET_MOVE_DIR_DD:
		printf("# [MOVE] SessionID: %d | direction: DD | xPos: %d | yPos: %d\n", p->sessionID, p->xPos, p->yPos);
		break;
	case dfPACKET_MOVE_DIR_LD:
		printf("# [MOVE] SessionID: %d | direction: LD | xPos: %d | yPos: %d\n", p->sessionID, p->xPos, p->yPos);
		break;
	}

	return true;
}

bool packetProc_MOVE_STOP(SESSION* p, Packet* payload)
{
	BYTE direction;
	short xPos;
	short yPos;

	*payload >> direction >> xPos >> yPos;

	if (abs(p->xPos - xPos) > dfERROR_RANGE || abs(p->yPos - yPos) > dfERROR_RANGE)
	{
		printf("Error: MOVE_STOP 좌표 허용 오차 초과\n");
		printf("요청 xpos: %d ypos: %d | 서버 xpos: %d ypos: %d\n", xPos, yPos, p->xPos, p->yPos);
		disconnect(p);
		return false;
	}

	p->direction = direction;
	p->xPos = xPos;
	p->yPos = yPos;
	p->action = dfPACKET_MOVE_STOP;

	Packet moveStop;
	createPacket_MOVE_STOP(&moveStop, p->sessionID, p->direction, p->xPos, p->yPos);
	broadcast(p, &moveStop);

	if (p->direction == dfPACKET_MOVE_DIR_LL)
	{
		printf("# [MOVE_STOP]  SessionID: %d | direction: LL | xPos: %d | yPos: %d\n", p->sessionID, p->xPos, p->yPos);
	}
	else
	{
		printf("# [MOVE_STOP]  SessionID: %d | direction: RR | xPos: %d | yPos: %d\n", p->sessionID, p->xPos, p->yPos);
	}

	return true;
}

bool packetProc_ATTACK1(SESSION* p, Packet* payload)
{
	BYTE direction;
	short xPos;
	short yPos;

	*payload >> direction >> xPos >> yPos;

	// 공격 모션 지시
	Packet attack;
	createPacket_ATTACK1(&attack, p->sessionID, p->direction, p->xPos, p->yPos);
	broadcast(p, &attack);

	// 공격 판정 후 결과 통보
	myList<SESSION*>::iterator iter;
	for (iter = users.begin(); iter != users.end(); iter++)
	{
		if ((*iter) != p)
		{
			if (direction == dfPACKET_MOVE_DIR_LL)
			{
				if ((xPos - (*iter)->xPos > dfATTACK1_RANGE_X) || (xPos - (*iter)->xPos < 0) || abs((*iter)->yPos - yPos) > dfATTACK1_RANGE_Y)
					continue;
			}
			else
			{
				if (((*iter)->xPos - xPos > dfATTACK1_RANGE_X) || ((*iter)->xPos - xPos < 0) || abs((*iter)->yPos - yPos) > dfATTACK1_RANGE_Y)
					continue;
			}

			(*iter)->HP = max((*iter)->HP - DAMAGE1, 0);

			Packet damage;
			createPacket_DAMAGE(&damage, p->sessionID, (*iter)->sessionID, (*iter)->HP);
			broadcast(nullptr, &damage);
			printf("# [ATTACK1] AttackerID: %d | DemagedID: %d | HP: %d\n", p->sessionID, (*iter)->sessionID, (*iter)->HP);

			if ((*iter)->HP <= 0)
			{
				disconnect((*iter));
			}
		}
	}

	return true;
}

bool packetProc_ATTACK2(SESSION* p, Packet* payload)
{
	BYTE direction;
	short xPos;
	short yPos;

	*payload >> direction >> xPos >> yPos;

	// 공격 모션 지시
	Packet attack;
	createPacket_ATTACK2(&attack, p->sessionID, p->direction, p->xPos, p->yPos);
	broadcast(p, &attack);

	myList<SESSION*>::iterator iter;
	for (iter = users.begin(); iter != users.end(); iter++)
	{
		if ((*iter) != p)
		{
			if (direction == dfPACKET_MOVE_DIR_LL)
			{
				if ((xPos - (*iter)->xPos > dfATTACK2_RANGE_X) || (xPos - (*iter)->xPos < 0) || abs((*iter)->yPos - yPos) > dfATTACK2_RANGE_Y)
					continue;
			}
			else
			{
				if (((*iter)->xPos - xPos > dfATTACK2_RANGE_X) || ((*iter)->xPos - xPos < 0) || abs((*iter)->yPos - yPos) > dfATTACK2_RANGE_Y)
					continue;
			}

			(*iter)->HP = max((*iter)->HP - DAMAGE1, 0);

			Packet damage;
			createPacket_DAMAGE(&damage, p->sessionID, (*iter)->sessionID, (*iter)->HP);
			broadcast(nullptr, &damage);
			printf("# [ATTACK2] AttackerID: %d | DemagedID: %d | HP: %d\n", p->sessionID, (*iter)->sessionID, (*iter)->HP);
			
			if ((*iter)->HP <= 0)
			{
				disconnect((*iter));
			}
		}
	}

	return true;
}

bool packetProc_ATTACK3(SESSION* p, Packet* payload)
{
	BYTE direction;
	short xPos;
	short yPos;

	*payload >> direction >> xPos >> yPos;

	// 공격 모션 지시
	Packet attack;
	createPacket_ATTACK3(&attack, p->sessionID, p->direction, p->xPos, p->yPos);
	broadcast(p, &attack);

	// 공격 판정 후 결과 통보
	myList<SESSION*>::iterator iter;
	for (iter = users.begin(); iter != users.end(); iter++)
	{
		if ((*iter) != p)
		{
			if (direction == dfPACKET_MOVE_DIR_LL)
			{
				if ((xPos - (*iter)->xPos > dfATTACK3_RANGE_X) || (xPos - (*iter)->xPos < 0) || abs((*iter)->yPos - yPos) > dfATTACK3_RANGE_Y)
					continue;
			}
			else
			{
				if (((*iter)->xPos - xPos > dfATTACK3_RANGE_X) || ((*iter)->xPos - xPos < 0) || abs((*iter)->yPos - yPos) > dfATTACK3_RANGE_Y)
					continue;
			}

			if ((*iter)->HP > 0)
			{
				(*iter)->HP = max((*iter)->HP - DAMAGE2, 0);
			}

			Packet damage;
			createPacket_DAMAGE(&damage, p->sessionID, (*iter)->sessionID, (*iter)->HP);
			broadcast(nullptr, &damage);
			printf("# [ATTACK3] AttackerID: %d | DemagedID: %d | HP: %d\n", p->sessionID, (*iter)->sessionID, (*iter)->HP);

			if ((*iter)->HP <= 0)
			{
				disconnect((*iter));
			}
		}
	}

	return true;
}

//====================================================================//
// 패킷 생성 함수
//====================================================================//
void createPacket_CREATE_MY_CHARACTER(Packet* payload, DWORD ID, BYTE direction, short xPos, short yPos, char HP)
{
	Header header;
	header.p_code = PACKET_CODE;
	header.p_size = sizeof(SC_CREATE_MY_CHARACTER);
	header.p_type = dfPACKET_SC_CREATE_MY_CHARACTER;

	payload->PutData((char*)&header, sizeof(Header));
	*payload << ID << direction << xPos << yPos << HP;
}

void createPacket_CREATE_OTHER_CHARACTER(Packet* payload, DWORD ID, BYTE direction, short xPos, short yPos, char HP)
{
	Header header;
	header.p_code = PACKET_CODE;
	header.p_size = sizeof(SC_CREATE_OTHER_CHARACTER);
	header.p_type = dfPACKET_SC_CREATE_OTHER_CHARACTER;

	payload->PutData((char*)&header, sizeof(Header));

	*payload << ID << direction << xPos << yPos << HP;

}

void createPacket_MOVE_START(Packet* payload, DWORD ID, BYTE action, short xPos, short yPos)
{
	Header header;
	header.p_code = PACKET_CODE;
	header.p_size = sizeof(SC_MOVE_START);
	header.p_type = dfPACKET_SC_MOVE_START;

	payload->PutData((char*)&header, sizeof(Header));

	*payload << ID << action << xPos << yPos;
}

void createPacket_MOVE_STOP(Packet* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
	Header header;
	header.p_code = PACKET_CODE;
	header.p_size = sizeof(SC_MOVE_STOP);
	header.p_type = dfPACKET_SC_MOVE_STOP;

	payload->PutData((char*)&header, sizeof(Header));

	*payload << ID << direction << xPos << yPos;
}

void createPacket_ATTACK1(Packet* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
	Header header;
	header.p_code = PACKET_CODE;
	header.p_size = sizeof(SC_ATTACK1);
	header.p_type = dfPACKET_SC_ATTACK1;

	payload->PutData((char*)&header, sizeof(Header));

	*payload << ID << direction << xPos << yPos;
}

void createPacket_ATTACK2(Packet* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
	Header header;
	header.p_code = PACKET_CODE;
	header.p_size = sizeof(SC_ATTACK2);
	header.p_type = dfPACKET_SC_ATTACK2;

	payload->PutData((char*)&header, sizeof(Header));

	*payload << ID << direction << xPos << yPos;
}

void createPacket_ATTACK3(Packet* payload, DWORD ID, BYTE direction, short xPos, short yPos)
{
	Header header;
	header.p_code = PACKET_CODE;
	header.p_size = sizeof(SC_ATTACK3);
	header.p_type = dfPACKET_SC_ATTACK3;

	payload->PutData((char*)&header, sizeof(Header));

	*payload << ID << direction << xPos << yPos;
}

void createPacket_DAMAGE(Packet* payload, DWORD attackerID, DWORD damagedID, char HP)
{
	Header header;
	header.p_code = PACKET_CODE;
	header.p_size = sizeof(SC_DAMAGE);
	header.p_type = dfPACKET_SC_DAMAGE;

	payload->PutData((char*)&header, sizeof(Header));

	*payload << attackerID << damagedID << HP;
}

void createPacket_DELETE(Packet* payload, DWORD ID)
{
	Header header;
	header.p_code = PACKET_CODE;
	header.p_size = sizeof(SC_DELETE_CHARACTER);
	header.p_type = dfPACKET_SC_DELETE_CHARACTER;

	payload->PutData((char*)&header, sizeof(Header));

	*payload << ID;
}