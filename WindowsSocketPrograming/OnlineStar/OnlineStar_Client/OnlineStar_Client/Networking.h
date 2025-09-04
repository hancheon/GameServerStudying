#pragma once

#define SERVER_PORT 3000

bool setClientSocket(SOCKET);
bool connectServer(SOCKET, wchar_t*);
bool transBlockingOpt(SOCKET);
bool networking();