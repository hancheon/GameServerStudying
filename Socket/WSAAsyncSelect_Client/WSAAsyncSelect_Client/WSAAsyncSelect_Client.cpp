#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <windowsx.h>
#include "Protocol.h"
#include "RingBuffer.h"
#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 25000
#define UM_SOCKET (WM_USER+1)
#define MAX_BUFSIZE 512

// 전역변수
SOCKET sock;
RingBuffer recvQ(1000);
RingBuffer sendQ(1000);
bool isConnected = false;
HPEN pen;
HBRUSH brush;

// 윈도우 메세지 처리 함수
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void readProc(SOCKET);
void writeProc(SOCKET);

// 로깅 함수
void errorLoging(int);

/*
* WinProc()
* {
*   switch (메세지)
*       case FD_CONNECT:
*           isConnect = true;
*       case FD_CLOSE:
*           연결 종료
*       case FD_READ:
*           readProc();
*       case FD_WRITE:
*           writeProc();
* }
*/

/*
* WM_MOUSEMOVE:
* 마우스 움직이는 순간에 send
* oldX oldY curX curY
* msg.startx = oldx
* msg.endx = curx
* 헤더 인큐 페이로드 인큐 따로
* header.len = sizeof(msg)
* /// select() 경우에는 폴링 방식이라 프레임 루프 돌면서 모아서 보낼 수 있었는데, async모델은 이벤트에 대한 반응이 필요하니까 모아서 보내기 안됨
* /// 그래서 바로 send()
* /// sendringbuffer의 목적 1. send 횟수 줄이기 2. 못보낸거 저장해두기
* 만약 보내고 싶은 걸 다 못보내서 링버퍼에 저장해두면 다음에 또 그걸 보내면서 실패가 일어날 수 있음
* 절차를 통일된 형태로? -> 항상 바로 sendQ에 넣고 거기서 dequeue하도록 방식 -> 성능은 떨어지지만 코드가 깔끔함
* 헤더 enqueue 페이로드 enqueue
* peek 해서 확인하고
* 지역 버퍼에 dequeue로 넣어서 ret = send()
* 지금 보낼 거 다 보내야함 while로다가 지역 버퍼가 작으면
* 보낼 메세지가 있으면 FD_WRITE는 다시 안오니까 바로 다 보내야함
* 보낼 수가 없었다가 보낼 수 있게 됨 -> 보내려고 했었는데 안됬다 (send의 실패) -> sendq에 못보낸거 보내기 -> 같은 코드
*/
/*
* writeProc()
* {
* }
*/

int main()
{
    int retval;

    // 윈도우 클래스 등록
    WNDCLASS wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = NULL;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = L"MyWndClass";
    if (!RegisterClass(&wndclass)) return 1;

    // 윈도우 생성
    HWND hWnd = CreateWindow(
        L"MyWndClass",
        L"Drawing Net", // 창 이름
        WS_OVERLAPPEDWINDOW, // 스타일
        0, 0, // 생성 위치
        600, 200, // 창 크기
        NULL,
        NULL,
        NULL,
        NULL);
    if (hWnd == NULL) return 1;
    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return -1;

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
        return -1;

    // WSAAsyncSelect() 함수에 등록
    retval = WSAAsyncSelect(sock, hWnd, UM_SOCKET, FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE);
    if (retval == SOCKET_ERROR)
    {
        errorLoging(GetLastError());
        return -1;
    }

    // connect()
    SOCKADDR_IN servAddr;
    wchar_t ipAddr[16] = L"127.0.0.1";
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVER_PORT);
    InetPton(AF_INET, ipAddr, &servAddr.sin_addr);
    retval = connect(sock, (SOCKADDR*)&servAddr, sizeof(servAddr));
    if (retval == SOCKET_ERROR)
    {
        errorLoging(retval);
        return -1;
    }

    return 0;
}

// wParam: socket, lParam: (상위 16비트) 오류 코드 + (하위 16비트) 네트워크 이벤트 종류
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case UM_SOCKET:
    {
        if (WSAGETSELECTERROR(lParam))
        {
            // 에러처리
            errorLoging(WSAGETSELECTERROR(lParam));
            return;
        }

        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_CONNECT:
            isConnected = true;
            break;
        case FD_CLOSE:
            // 연결 종료
            break;
        case FD_READ:
            if (!isConnected)
                return;
            readProc(wParam);
            break;
        case FD_WRITE:
            if (!isConnected)
                return;
            writeProc(wParam);
            break;
        }
    }
        break;
    case WM_LBUTTONDOWN: // 그리기 시작
        break;
    case WM_LBUTTONUP: // 그리기 끝
        break;
    case WM_MOUSEMOVE: // 그리기 동선
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void readProc()
{
    int retval;
    char buffer[MAX_BUFSIZE];

    while (1)
    {
        retval = recv(sock, buffer, MAX_BUFSIZE, 0);
        if (retval == SOCKET_ERROR)
        {
            if (GetLastError() == WSAEWOULDBLOCK)
            {
                break;
            }
            errorLoging(GetLastError());
            break;
        }
        recvQ.Enqueue(buffer, retval);

        if (recvQ.GetUsedSize() < sizeof(HEADER)) // 헤더조차도 안들어온 경우
        {
            break;
        }

        HEADER header;
        recvQ.Peek((char*)&header, sizeof(HEADER));
        if (recvQ.GetUsedSize() < sizeof(HEADER) + header.len) // 헤더 + 페이로드가 들어오지 않은 경우
        {
            break;
        }

        DRAW_PACKET packet;
        recvQ.Dequeue((char*)&header, sizeof(HEADER)); // 헤더 디큐
        recvQ.Dequeue((char*)&packet, header.len); // 페이로드 디큐

        // 그리기 처리
        // GetDC()
    }
}

void writeProc()
{
    int retval;


}

void errorLoging(int retval)
{
    printf("%d\n", retval);
}
