0 : createPacket_CREATE_MY_CHARACTER(DWORD sessionID, BYTE, short x, short y, char HP);
1 : createPacket_CREATE_OTHER_CHARACTER(DWORD sessionID, BYTE, short x, short y, char HP);
2 : createPacket_DELETE(DWORD sessionID);
11 : createPacket_MOVE_START(DWORD sessionID, BYTE, short x, short y);
13 : createPacket_MOVE_STOP(DWORD sessionID, BYTE, short x, short y);
21 : createPacket_ATTACK1(DWORD sessionID, BYTE, short x, short y);
23 : createPacket_ATTACK2(DWORD sessionID, BYTE, short x, short y);
25 : createPacket_ATTACK3(DWORD sessionID, BYTE, short x, short y);
30 : createPacket_DAMAGE(DWORD attackerID, DWORD victimID, char victimHP);