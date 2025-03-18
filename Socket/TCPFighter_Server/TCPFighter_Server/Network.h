#pragma once
#include "Packet.h"
#include "Protocol.h"
#include "Session.h"

#define SERVER_PORT 5000
#define PACKET_CODE 0x89
#define MAX_BUFSIZE 1024

void networkProc();
void acceptProc();
void readProc(SESSION*);
void writeProc(SESSION*);
void unicast(SESSION*, Packet*);
void broadcast(SESSION*, Packet*);
void disconnect(SESSION*);
void deleteUser();