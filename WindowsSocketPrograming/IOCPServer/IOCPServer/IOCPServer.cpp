// IOCPServer.cpp: 간단한 IOCP 에코 서버
//

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <list>
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

DWORD WINAPI WorkerThread(LPVOID arg);
void PrintErrorLog(int error, std::string funcName, int line);

std::list<Session*> sessions;

int main()
{
	int retval;

	WSADATA wsaData;
	retval = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (retval != 0) {
		PRINT_ERROR("WSAStartup");
		return -1;
	}

	HANDLE hCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hCP == NULL) {
		PRINT_ERROR("CreateIoCompletionPort");
		WSACleanup();
		return -1;
	}

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	HANDLE hThread;
	for (int cnt = 0; cnt < si.dwNumberOfProcessors * 2; cnt++) {
		hThread = CreateThread(NULL, 0, WorkerThread, hCP, 0, NULL);
		if (hThread == NULL) {
			PRINT_ERROR("CreateThread");
			WSACleanup();
			return -1;
		}
		CloseHandle(hThread);
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

	SOCKET clientSock;
	SOCKADDR_IN clientSockAddr;
	int clientSockAddrLen = sizeof(clientSockAddr);
	DWORD flags, result;

	while (1) {
		// accept
		clientSock = accept(servSock, (SOCKADDR*)&clientSockAddr, &clientSockAddrLen);
		if (clientSock == INVALID_SOCKET) {
			PRINT_ERROR("accept");
			continue;
		}

		CreateIoCompletionPort((HANDLE)clientSock, hCP, clientSock, 0);

		Session* newSession = new Session;
		ZeroMemory(&newSession->overlapped, sizeof(newSession->overlapped));
		newSession->socket = clientSock;
		newSession->recvBytes = 0;
		newSession->sendBytes = 0;
		newSession->wsaBuf.buf = newSession->buf;
		newSession->wsaBuf.len = BUFSIZE;
		sessions.push_back(newSession);

		flags = 0;
		retval = WSARecv(newSession->socket, &newSession->wsaBuf, 1, &result, &flags, &newSession->overlapped, nullptr);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				PRINT_ERROR("WSARecv");
				// 세션 삭제
			}
			continue;
		}

		std::cout << "# 새로운 클라이언트: " << sessions.size() << '\n';
	}

	closesocket(servSock);
	WSACleanup();
	return 0;
}

DWORD __stdcall WorkerThread(LPVOID arg)
{
	int retval;

	HANDLE hCP = (HANDLE)arg;

	while (1) {
		// 비동기 입출력 대기
		DWORD result;
		SOCKET clientSock;
		Session* session;
		retval = GetQueuedCompletionStatus(hCP, &result, (PULONG_PTR)&clientSock, (LPOVERLAPPED*)&session, INFINITE);

		if (retval == 0 || result == 0) {
			if (retval == 0) {
				DWORD temp1, temp2;
				WSAGetOverlappedResult(clientSock, &session->overlapped, &temp1, FALSE, &temp2);
				PRINT_ERROR("GetQueuedCompletionStatus");
			}
			closesocket(clientSock);
			std::cout << "# 클라이언트 접속 종료\n";
			delete session;
			continue;
		}

		// 데이터 전송량 갱신
		if (session->recvBytes == 0) {
			session->recvBytes = result;
			session->sendBytes = 0;

			// 받은 데이터 출력
			session->buf[session->recvBytes] = '\0';
			std::cout << "받은 데이터: " << session->buf << '\n';
		}
		else {
			session->sendBytes += result;
		}

		if (session->recvBytes > session->sendBytes) {
			// 데이터 전송
			ZeroMemory(&session->overlapped, sizeof(session->overlapped));
			session->wsaBuf.buf = session->buf + session->sendBytes;
			session->wsaBuf.len = session->recvBytes - session->sendBytes;
			
			DWORD result;
			DWORD flags = 0;
			retval = WSASend(session->socket, &session->wsaBuf, 1, &result, flags, &session->overlapped, nullptr);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					PRINT_ERROR("WSASend");
				}
				continue;
			}
		} 
		else {
			session->recvBytes = 0;

			// 데이터 수신
			ZeroMemory(&session->overlapped, sizeof(session->overlapped));
			session->wsaBuf.buf = session->buf;
			session->wsaBuf.len = BUFSIZE;

			DWORD result;
			DWORD flag = 0;
			retval = WSARecv(session->socket, &session->wsaBuf, 1, &result, &flag, &session->overlapped, nullptr);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					PRINT_ERROR("WSARecv");
				}
				continue;
			}
		}
	}

	return 0;
}

void PrintErrorLog(int error, std::string funcName, int line)
{
    std::cerr << std::format("! {}: {} failed with {}\n", line, funcName, error);
}