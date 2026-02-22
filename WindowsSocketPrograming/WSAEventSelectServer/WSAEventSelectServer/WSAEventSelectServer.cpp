// WSAEventSelectServer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
// EchoServer

#include <iostream>
#include <list>
#include <format>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define BUFSIZE 516
#define PRINT_ERROR(X) PrintErrorLog(WSAGetLastError(), X, __LINE__)

void AddSession(SOCKET);
void RemoveSession(int);
void PrintErrorLog(int error, std::string funcName, int line);

struct Session
{
	SOCKET socket;
	char buf[BUFSIZE + 1];
	int recvBytes;
	int sendBytes;
};

int clientCnt = 0;
Session* sessions[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS];

int main()
{
	int retval;
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		PRINT_ERROR("WSAStartup");
		return 0;
	}

	SOCKET servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (servSock == INVALID_SOCKET) {
		PRINT_ERROR("socket");
		WSACleanup();
		return 0;
	}

	SOCKADDR_IN servAddr;
	ZeroMemory(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	retval = bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr));
	if (retval == SOCKET_ERROR) {
		PRINT_ERROR("bind");
		closesocket(servSock);
		WSACleanup();
		return 0;
	}

	retval = listen(servSock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		PRINT_ERROR("listen");
		closesocket(servSock);
		WSACleanup();
		return 0;
	}

	Session* listenSock = new Session;
	listenSock->socket = servSock;
	sessions[clientCnt] = listenSock;

	WSAEVENT hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT) {
		PRINT_ERROR("WSACreateEvent");
		closesocket(servSock);
		WSACleanup();
		return 0;
	}
	events[clientCnt] = hEvent;
	clientCnt++;

	retval = WSAEventSelect(servSock, hEvent, FD_ACCEPT | FD_CLOSE);
	if (retval == SOCKET_ERROR) {
		PRINT_ERROR("WSAEventSelect");
		closesocket(servSock);
		WSACleanup();
		return 0;
	}

	WSANETWORKEVENTS networkEvents;
	SOCKET clientSock;
	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);

	while (1) {
		// 이벤트 대기
		DWORD startIdx = WSAWaitForMultipleEvents(clientCnt, events, FALSE, WSA_INFINITE, FALSE);
		if (startIdx == WSA_WAIT_FAILED) continue;
		startIdx -= WSA_WAIT_EVENT_0;

		for (int index = startIdx; index < clientCnt; index++) {
			Session* target = sessions[index];

			// 이벤트 종류 확인
			retval = WSAEnumNetworkEvents(target->socket, events[index], &networkEvents);
			if (retval == SOCKET_ERROR) {
				PRINT_ERROR("WSAEnumNetworkEvents");
				continue;
			}

			// 이벤트: FD_ACCEPT
			if (networkEvents.lNetworkEvents & FD_ACCEPT) {
				if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
					PRINT_ERROR("FD_ACCEPT");
					continue;
				}

				if (clientCnt >= WSA_MAXIMUM_WAIT_EVENTS) {
					std::cerr << "! 더 이상 클라이언트 수용 불가\n";
					continue;
				}

				clientSock = accept(servSock, (SOCKADDR*)&clientAddr, &clientAddrLen);
				if (clientSock == INVALID_SOCKET) {
					PRINT_ERROR("accept");
					continue;
				}

				AddSession(clientSock);

				retval = WSAEventSelect(clientSock, events[clientCnt - 1], FD_READ | FD_CLOSE);
				if (retval == SOCKET_ERROR) {
					PRINT_ERROR("WSAEventSelect");
					RemoveSession(clientCnt - 1);
				}

				std::cout << "# 새로운 클라이언트 연결: " << clientCnt - 1 << "\n";
			}

			// 이벤트: FD_READ & FD_WRITE
			if (networkEvents.lNetworkEvents & FD_READ || networkEvents.lNetworkEvents & FD_WRITE) {
				if (networkEvents.iErrorCode[FD_READ_BIT] != 0) {
					PRINT_ERROR("FD_READ");
					continue;
				}

				retval = recv(target->socket, target->buf, BUFSIZE, 0);
				if (retval <= 0) {
					if (retval == 0) {
						RemoveSession(startIdx);
						printf("# 클라이언트 연결 종료\n");
						break;
					}
					else {
						if (WSAGetLastError() != WSAEWOULDBLOCK) {
							PRINT_ERROR("recv");
							break;
						}
						continue;
					}
				}

				target->recvBytes = retval;
				target->buf[retval] = '\0';
				std::cout << target->buf << '\n';

				if (target->sendBytes < target->recvBytes) {
					retval = send(target->socket, target->buf, target->recvBytes, 0);
					if (retval == SOCKET_ERROR) {
						PRINT_ERROR("send");
						continue;
					}
					target->sendBytes += retval;

					if (target->sendBytes = target->recvBytes) {
						target->sendBytes = target->recvBytes = 0;
					}
				}
			}

			if (networkEvents.lNetworkEvents & FD_CLOSE) {
				if (networkEvents.iErrorCode[FD_CLOSE_BIT] != 0) {
					PRINT_ERROR("FD_CLOSE");
				}
				RemoveSession(index);
				printf("# 클라이언트 연결 종료\n");
			}
		}
	}

	closesocket(servSock);
	WSACleanup();

	return 0;
}

void AddSession(SOCKET socket)
{
	Session* newSession = new Session;
	newSession->socket = socket;
	newSession->recvBytes = 0;
	newSession->sendBytes = 0;
	sessions[clientCnt] = newSession;

	WSAEVENT hNewEvent = WSACreateEvent();
	if (hNewEvent == WSA_INVALID_EVENT) {
		PRINT_ERROR("WSACreateEvent");
	}
	events[clientCnt] = hNewEvent;

	++clientCnt;
}

void RemoveSession(int index)
{
	closesocket(sessions[index]->socket);
	delete sessions[index];
	WSACloseEvent(events[index]);

	if (index != clientCnt - 1) {
		sessions[index] = sessions[clientCnt - 1];
		events[index] = events[clientCnt - 1];
	}
	--clientCnt;
}

void PrintErrorLog(int error, std::string funcName, int line)
{
	std::cerr << std::format("! {}: {} failed with {}\n", line, funcName, error);
}
