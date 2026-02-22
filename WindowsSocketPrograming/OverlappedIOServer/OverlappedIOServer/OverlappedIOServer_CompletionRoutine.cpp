// OverlappedIO_CompletionRoutine.cpp
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
void CALLBACK CompletionRoutine(DWORD dwError, DWORD result, LPWSAOVERLAPPED lpOverlapped, DWORD flags);
void PrintErrorLog(int error, std::string funcName, int line);

int clientCnt = 0;

SOCKET clientSock;
HANDLE hReadEvent, hWriteEvent;

int main()
{
	int retval;

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

	// CreateEvent에서 3번째 인자(bInitialState): 초기 상태 결정(True: Signaled, False: Non-Signaled)
	hReadEvent = CreateEventW(nullptr, FALSE, TRUE, NULL);
	if (hReadEvent == NULL) {
		PRINT_ERROR("CreateEvent");
		closesocket(servSock);
		WSACleanup();
		return -1;
	}
	hWriteEvent = CreateEventW(nullptr, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) {
		PRINT_ERROR("CreateEvent");
		closesocket(servSock);
		WSACleanup();
		return -1;
	}

	HANDLE hThread = CreateThread(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
	if (hThread == NULL) {
		PRINT_ERROR("CreateThread");
		closesocket(servSock);
		WSACleanup();
		return -1;
	}
	CloseHandle(hThread);

	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);

	while (1) {
		WaitForSingleObject(hReadEvent, INFINITE);

		// accept
		clientSock = accept(servSock, (SOCKADDR*)&clientAddr, &clientAddrLen);
		if (clientSock == INVALID_SOCKET) {
			PRINT_ERROR("accept");
			break;
		}

		std::cout << "# 새로운 클라이언트: " << clientCnt + 1 << '\n';

		WSASetEvent(hWriteEvent);
	}

	closesocket(servSock);
	WSACleanup();
	return 0;
}

DWORD __stdcall WorkerThread(LPVOID args)
{
	int retval;

	while (1) {
		while (1) {
			// alertable wait
			DWORD index = WaitForSingleObjectEx(hWriteEvent, INFINITE, TRUE);
			if (index == WAIT_OBJECT_0) break; // 이벤트 발생 -> IO 작업 실행
			if (index != WAIT_IO_COMPLETION) return -1; // WAIT_IO_COMPLETION: alertable wait에서 벗어남
		}

		Session* newSession = new Session;
		ZeroMemory(&newSession->overlapped, sizeof(newSession->overlapped));
		newSession->socket = clientSock;
		SetEvent(hReadEvent);
		newSession->recvBytes = 0;
		newSession->sendBytes = 0;
		newSession->wsaBuf.buf = newSession->buf;
		newSession->wsaBuf.len = BUFSIZE;

		DWORD result, flag;
		flag = 0;
		retval = WSARecv(newSession->socket, &newSession->wsaBuf, 1, &result, &flag, &newSession->overlapped, CompletionRoutine);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				PRINT_ERROR("WSARecv");
				return -1;
			}
		}
	}

	return 0;
}

void CALLBACK CompletionRoutine(DWORD dwError, DWORD dResultBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	int retval;

	Session* session = (Session*)lpOverlapped;

	if (dwError != 0 || dResultBytes == 0) {
		if (dwError != 0) PRINT_ERROR("WSARecv");
		closesocket(clientSock);
		std::cout << "# 클라이언트 연결 종료\n";
		delete session;
		return;
	}

	if (session->recvBytes == 0) {
		session->recvBytes = dResultBytes;
		session->sendBytes = 0;
		session->buf[session->recvBytes] = '\0';
		std::cout << "받은 데이터: " << session->buf << '\n';
	}
	else {
		session->sendBytes += dResultBytes;
	}

	// 보낼 게 있는 경우 보내기
	if (session->recvBytes > session->sendBytes) {
		// 데이터 전송
		ZeroMemory(&session->overlapped, sizeof(session->overlapped));
		session->wsaBuf.buf = session->buf + session->sendBytes;
		session->wsaBuf.len = session->recvBytes - session->sendBytes;

		DWORD result;
		retval = WSASend(session->socket, &session->wsaBuf, 1, &result, NULL, &session->overlapped, nullptr);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				PRINT_ERROR("WSASend");
			}
			return;
		}
	}
	else { // 받은 거 이미 다 보낸 경우: 이벤트 = recv
		session->recvBytes = 0;
		ZeroMemory(&session->overlapped, sizeof(session->overlapped));
		session->wsaBuf.buf = session->buf;
		session->wsaBuf.len = BUFSIZE;

		DWORD result, flag;
		flag = 0;
		retval = WSARecv(session->socket, &session->wsaBuf, 1, &result, &flag, &session->overlapped, nullptr);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				PRINT_ERROR("WSARecv");
			}
			return;
		}
	}
}

void PrintErrorLog(int error, std::string funcName, int line)
{
	std::cerr << std::format("! {}: {} failed with {}\n", line, funcName, error);
}