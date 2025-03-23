#include <Windows.h>
#include "Packet.h"

createPacket_CREATE_MY_CHARACTER(DWORD sessionID, BYTE, short x, short y, char HP);
createPacket_CREATE_OTHER_CHARACTER(DWORD sessionID, BYTE, short x, short y, char HP);
createPacket_MOVE_START(DWORD sessionID, BYTE, short x, short y);
createPacket_MOVE_STOP(DWORD sessionID, BYTE, short x, short y);
createPacket_ATTACK1(DWORD sessionID, BYTE, short x, short y);
createPacket_ATTACK2(DWORD sessionID, BYTE, short x, short y);
createPacket_ATTACK3(DWORD sessionID, BYTE, short x, short y);
createPacket_DAMAGE(DWORD attackerID, DWORD victimID, char victimHP);
createPacket_DELETE(DWORD sessionID);