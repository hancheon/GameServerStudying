#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windowsx.h>
#include "Protocol.h"
#include "RingBuffer.h"
#pragma comment(lib, "ws2_32")

#define UM_SOCKET (WM_USER+1)
#define SERVER_IP L"192.168.30.14"
#define SERVER_PORT 25000
#define MAX_BUFSIZE 1024

// 전역 변수
SOCKET sock;
RingBuffer readQ;
RingBuffer writeQ;

bool isConnected = false;
bool isDrag = false;

int oldX;
int oldY;

HWND hWnd;
HPEN hPen;

// 통신 관련 함수
void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);
void readProc();
void moveProc(LPARAM);
void writeProc();

// WinApi 함수
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int main(int argc, char** argv)
{
	int retval;

	WNDCLASSEX wndClass;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = NULL;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = L"PaintBoardWndClass";
	wndClass.hIconSm = NULL;
	if (!RegisterClassEx(&wndClass)) return -1;

	// 윈도우 생성
	hWnd = CreateWindow(
		L"PaintBoardWndClass", L"NET 그림판", WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, NULL, NULL, NULL, NULL
	);
	if (!hWnd) return -1;
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("%d\n", GetLastError());
		return -1;
	}

	// socket()
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printf("%d\n", GetLastError());
		return -1;
	}

	// WSAAsyncSelect()
	retval = WSAAsyncSelect(sock, hWnd, UM_SOCKET, FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE);
	if (retval == SOCKET_ERROR)
	{
		printf("%d\n", GetLastError());
		return -1;
	}

	// connect()
	SOCKADDR_IN servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVER_PORT);
	InetPton(AF_INET, SERVER_IP, &servAddr.sin_addr);
	retval = connect(sock, (SOCKADDR*)&servAddr, sizeof(servAddr));
	if (retval == SOCKET_ERROR)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			printf("%d\n", GetLastError());
			return -1;
		}
	}

	// 메세지 루프
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 윈속 종료
	WSACleanup();
	return (int)msg.wParam;
}

void ProcessSocketMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam)) // 에러 확인
		return;
	switch (WSAGETSELECTEVENT(lParam)) // 소켓 관련 메세지
	{
	case FD_CONNECT:
		isConnected = true;
		break;
	case FD_CLOSE:
		PostMessage(hWnd, WM_DESTROY, 0, 0);
		break;
	case FD_READ:
		if (isConnected)
			readProc();
		break;
	case FD_WRITE:
		if (isConnected)
			writeProc();
		break;
	}
}

void readProc()
{
	if (readQ.GetFreeSize() == 0) // 문제상황 연결 중단?
	{
		printf("처리 불가능한 패킷으로 수신 링버퍼 가득 참\n");
		PostMessage(hWnd, WM_DESTROY, 0, 0);
		return;
	}

	int retval;
	char buffer[MAX_BUFSIZE];

	// tcp 수신버퍼 -> 지역버퍼
	retval = recv(sock, buffer, min(MAX_BUFSIZE, readQ.GetFreeSize()), 0);
	if (retval == SOCKET_ERROR) // 에러처리
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			printf("%d\n", WSAGetLastError());
			PostMessage(hWnd, WM_DESTROY, 0, 0);
			return;
		}
	}
	else if (retval == 0) // 연결 종료
	{
		printf("서버측 연결 종료\n");
		PostMessage(hWnd, WM_DESTROY, 0, 0);
		return;
	}

	// 지역버퍼 -> 링버퍼
	int retval_enq = readQ.Enqueue(buffer, retval);
	if (retval_enq != retval)
	{
		printf("[수신] 인큐 요청 크기와 인큐 크기 다름\n"); // 문제 계속 발생 및 내 그림이 느림
		PostMessage(hWnd, WM_DESTROY, 0, 0);
		return;
	}

	while (1)
	{
		// 링버퍼 내 처리 가능한 헤더 있는지 확인
		if (readQ.GetUsedSize() < sizeof(HEADER))
			break;

		// 헤더로 페이로드 길이 확인
		HEADER header;
		retval = readQ.Peek((char*)&header, sizeof(HEADER));
		if (retval != sizeof(HEADER))
		{
			printf("[수신] 잘못된 헤더 크기: %d\n", retval);
			PostMessage(hWnd, WM_DESTROY, 0, 0);
			break;
		}

		if (header.len != sizeof(DRAW_PACKET))
		{
			printf("[수신] 잘못된 패킷 크기: %d\n", header.len);
			PostMessage(hWnd, WM_DESTROY, 0, 0);
			break;
		}

		// 아직 페이로드 다 안들어온 경우
		if (readQ.GetUsedSize() < sizeof(HEADER) + header.len)
			break;

		// 헤더, 페이로드 디큐
		DRAW_PACKET payload;
		retval = readQ.Dequeue((char*)&header, sizeof(HEADER));
		if (retval != sizeof(HEADER))
		{
			printf("[수신] 헤더 디큐 실패: %d\n", retval);
			PostMessage(hWnd, WM_DESTROY, 0, 0);
			break;
		}
		retval = readQ.Dequeue((char*)&payload, header.len);
		if (retval != header.len)
		{
			printf("[수신] 페이로드 디큐 실패: %d\n", retval);
			PostMessage(hWnd, WM_DESTROY, 0, 0);
			break;
		}

		printf("수신한 패킷: startX=%d, startY=%d, endX=%d, endY=%d\n",
			payload.startX, payload.startY, payload.endX, payload.endY);


		// 그리기 로직
		HDC hdc = GetDC(hWnd);
		HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
		MoveToEx(hdc, payload.startX, payload.startY, NULL);
		LineTo(hdc, payload.endX, payload.endY);
		SelectObject(hdc, oldPen);
		ReleaseDC(hWnd, hdc);
	}
}

void moveProc(LPARAM lParam)
{
	int retval;

	int xPos = GET_X_LPARAM(lParam);
	int yPos = GET_Y_LPARAM(lParam);

	DRAW_PACKET payload;
	payload.startX = oldX;
	payload.startY = oldY;
	payload.endX = xPos;
	payload.endY = yPos;

	HEADER header;
	header.len = sizeof(payload);

	retval = writeQ.Enqueue((char*)&header, sizeof(HEADER));
	if (retval != sizeof(HEADER))
	{
		printf("[송신] 헤더 인큐 실패: %d\n", retval);
		PostMessage(hWnd, WM_DESTROY, 0, 0);
		return;
	}
	retval = writeQ.Enqueue((char*)&payload, header.len);
	if (retval != header.len)
	{
		printf("[송신] 페이로드 인큐 실패: %d\n", retval);
		PostMessage(hWnd, WM_DESTROY, 0, 0);
		return;
	}

	oldX = xPos;
	oldY = yPos;

	writeProc();
}
void writeProc()
{
	int retval;
	char buffer[MAX_BUFSIZE];

	while (1)
	{
		if (writeQ.GetUsedSize() == 0)
			break;

		retval = writeQ.Dequeue(buffer, min(writeQ.GetUsedSize(), MAX_BUFSIZE));

		retval = send(sock, buffer, retval, 0);
		if (retval == SOCKET_ERROR)
		{
			if (GetLastError() != WSAEWOULDBLOCK)
			{
				printf("%d\n", GetLastError());
				PostMessage(hWnd, WM_DESTROY, 0, 0);
				break;
			}
		}
	}
}

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		break;
	case WM_LBUTTONDOWN:
		isDrag = true;
		oldX = GET_X_LPARAM(lParam);
		oldY = GET_Y_LPARAM(lParam);
		break;
	case WM_LBUTTONUP:
		isDrag = false;
		break;
	case WM_MOUSEMOVE:
		if (isDrag)
			moveProc(lParam);
		break;
	case UM_SOCKET:
		ProcessSocketMessage(hWnd, msg, wParam, lParam);
		break;
	case WM_DESTROY:
		DeletePen(hPen);
		closesocket(sock);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}