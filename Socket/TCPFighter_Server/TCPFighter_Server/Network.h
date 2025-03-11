#pragma once
#include "Protocol.h"
#include "Session.h"

#define SERVER_PORT 5000
#define PACKET_CODE 0x89
#define MAX_BUFSIZE 1024

bool networkProc();
bool acceptProc();
bool readProc(SESSION*);
bool writeProc(SESSION*);
bool unicast(SESSION*, HEADER*, char*);
bool broadcast(SESSION*, HEADER*, char*);
bool disconnect(SESSION*);
void deleteUser();