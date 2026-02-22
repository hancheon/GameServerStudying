// OverlappedIO_Event.cpp
//

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <format>
#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define BUFSIZE 516
#define PRINT_ERROR(X) PrintErrorLog(WSAGetLastError(), X, __LINE__)

struct Session
{
	WSAOVERLAPPED overlapped;
	SOCKET socket;
	char buf[BUFSIZE + 1];
	int recvBytes;
	int sendBytes;
	WSABUF wsaBuf;
};

DWORD WINAPI WorkerThread(LPVOID args);
void AddSession(SOCKET);
void RemoveSession(int);
void PrintErrorLog(int error, std::string funcName, int line);

int clientCnt = 0;
Session* sessions[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS];
CRITICAL_SECTION cs;

int main()
{
	int retval;
	InitializeCriticalSection(&cs);

	WSADATA wsaData;

	retval = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (retval != 0) {
		PRINT_ERROR("WSAStartup");
		return -1;
	}

	SOCKET servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (servSock == INVALID_SOCKET) {
		PRINT_ERROR("socket");
		WSACleanup();
		return -1;
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
		return -1;
	}

	retval = listen(servSock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		PRINT_ERROR("listen");
		closesocket(servSock);
		WSACleanup();
		return -1;
	}

	// 더미 이벤트
	// 최초에 서버가 켜졌을 때, WSAWaitForMultipleEvents()를 실행하기 위해 등록할 이벤트
	// 또 새로운 클라이언트가 추가되었다는 것을 알려줌
	WSAEVENT hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT) {
		PRINT_ERROR("WSACreateEvent");
		closesocket(servSock);
		WSACleanup();
		return -1;
	}
	events[clientCnt] = hEvent;
	clientCnt++;

	HANDLE hThread = CreateThread(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
	if (hThread == NULL) {
		PRINT_ERROR("CreateThread");
		closesocket(servSock);
		WSACleanup();
		return -1;
	}
	CloseHandle(hThread);

	SOCKET clientSock;
	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	DWORD recvBytes, flag;

	while (1) {
		clientSock = accept(servSock, (SOCKADDR*)&clientAddr, &clientAddrLen);
		if (clientSock == INVALID_SOCKET) {
			PRINT_ERROR("accept");
			continue;
		}

		if (clientCnt >= WSA_MAXIMUM_WAIT_EVENTS) {
			std::cout << "! 더 이상 클라이언트를 수용할 수 없습니다.\n";
			continue;
		}

		AddSession(clientSock);

		flag = 0;
		Session* newSession = sessions[clientCnt - 1];
		// 일단은 감시 이벤트 recv만 걸어둠
		retval = WSARecv(newSession->socket, &newSession->wsaBuf, 1, &recvBytes, &flag, &newSession->overlapped, nullptr);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				PRINT_ERROR("WSARecv()");
				RemoveSession(clientCnt - 1);
				continue;
			}
		}

		std::cout << "# 새로운 클라이언트: " << clientCnt - 1 << '\n';

		// WSAEventSelect에서는 리슨 소켓도 이벤트으로 발생을 알았는데,
		// 중첩 IO에서는 리슨 소켓은 그냥 계속 들어오면 accept
		// 새로운 클라이언트 추가되었다고 워커 스레드에 직접 알림
		WSASetEvent(events[0]);
	}

	closesocket(servSock);
	WSACleanup();
	return 0;
}

DWORD __stdcall WorkerThread(LPVOID args)
{
	int retval;

	while (1) {
		DWORD index = WSAWaitForMultipleEvents(clientCnt, events, FALSE, 0, FALSE);
		if (index == WSA_WAIT_FAILED) continue;
		int startIdx = index - WSA_WAIT_EVENT_0;
		ResetEvent(events[startIdx]);
		if (startIdx == 0) continue;

		for (int cnt = startIdx; cnt < clientCnt; cnt++) {
			DWORD result, flag;
			Session* target = sessions[cnt];
			retval = WSAGetOverlappedResult(target->socket, &target->overlapped, &result, FALSE, &flag);
			if (retval == FALSE || result == 0) {
				RemoveSession(startIdx);
				std::cout << "# 클라이언트 연결 종료\n";
				continue;
			}

			if (target->recvBytes == 0) {
				target->recvBytes = result;
				target->sendBytes = 0;
				target->buf[target->recvBytes] = '\0';
				std::cout << "받은 데이터: " << target->buf << '\n';
			}
			else {
				target->sendBytes += result;
			}

			// 보낼 게 있는 경우 보내기
			if (target->recvBytes > target->sendBytes) {
				// 데이터 전송
				ZeroMemory(&target->overlapped, sizeof(target->overlapped));
				target->overlapped.hEvent = events[cnt];
				target->wsaBuf.buf = target->buf + target->sendBytes;
				target->wsaBuf.len = target->recvBytes - target->sendBytes;

				DWORD result;
				retval = WSASend(target->socket, &target->wsaBuf, 1, &result, NULL, &target->overlapped, nullptr);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() != WSA_IO_PENDING) {
						PRINT_ERROR("WSASend");
					}
					continue;
				}
			}
			else { // 받은 거 이미 다 보낸 경우: 이벤트 = recv
				target->recvBytes = 0;
				ZeroMemory(&target->overlapped, sizeof(target->overlapped));
				target->overlapped.hEvent = events[cnt];
				target->wsaBuf.buf = target->buf;
				target->wsaBuf.len = BUFSIZE;

				DWORD result, flag;
				flag = 0;
				retval = WSARecv(target->socket, &target->wsaBuf, 1, &result, &flag, &target->overlapped, nullptr);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() != WSA_IO_PENDING) {
						PRINT_ERROR("WSARecv");
					}
					continue;
				}
			}
		}
	}

	return 0;
}

void AddSession(SOCKET socket)
{
	EnterCriticalSection(&cs);

	WSAEVENT hNewEvent = WSACreateEvent();
	if (hNewEvent == WSA_INVALID_EVENT) {
		PRINT_ERROR("WSACreateEvent");
	}

	Session* newSession = new Session;
	ZeroMemory(&newSession->overlapped, sizeof(newSession->overlapped));
	newSession->overlapped.hEvent = hNewEvent;
	newSession->socket = socket;
	newSession->recvBytes = 0;
	newSession->sendBytes = 0;
	newSession->wsaBuf.buf = newSession->buf;
	newSession->wsaBuf.len = BUFSIZE;
	sessions[clientCnt] = newSession;
	events[clientCnt] = hNewEvent;
	++clientCnt;

	LeaveCriticalSection(&cs);
}

void RemoveSession(int index)
{
	EnterCriticalSection(&cs);

	closesocket(sessions[index]->socket);
	delete sessions[index];
	WSACloseEvent(events[index]);

	if (index != clientCnt - 1) {
		sessions[index] = sessions[clientCnt - 1];
		events[index] = events[clientCnt - 1];
	}
	--clientCnt;

	LeaveCriticalSection(&cs);
}

void PrintErrorLog(int error, std::string funcName, int line)
{
	std::cerr << std::format("! {}: {} failed with {}\n", line, funcName, error);
}