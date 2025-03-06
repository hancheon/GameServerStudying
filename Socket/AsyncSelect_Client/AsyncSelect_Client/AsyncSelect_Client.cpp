// AsyncSelect_Client.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "AsyncSelect_Client.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windowsx.h>
#include "Protocol.h"
#include "RingBuffer.h"
#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define MAX_LOADSTRING 100
#define SERVER_PORT 25000
#define UM_SOCKET (WM_USER+1) // 소켓 관련 이벤트 번호 정의
#define MAX_BUFSIZE 512

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

void readProc(HWND);
void writeProc();

SOCKET sock;
RingBuffer recvQ(1000);
RingBuffer sendQ(1000);
bool isConnected = false;
bool isClicked = false;
int oldX;
int oldY;

HPEN hPen;
HBRUSH hBrush;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDC_ASYNCSELECTCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ASYNCSELECTCLIENT));

    MSG msg;
    //HWND hWnd;

    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return -1;

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
        return -1;

    // WSAAsyncSelect() 함수에 등록
    retval = WSAAsyncSelect(sock, msg.hwnd, UM_SOCKET, FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE);
    if (retval == SOCKET_ERROR)
    {
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
        return -1;
    }

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASYNCSELECTCLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ASYNCSELECTCLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, L"Net Drawing", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case UM_SOCKET:
    {
        if (WSAGETSELECTERROR(lParam))
            break;
        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_CONNECT:
            isConnected = true;
            break;
        case FD_CLOSE:
            PostQuitMessage(0);
            break;
        case FD_READ:
            readProc(hWnd);
            break;
        case FD_WRITE:
            writeProc();
            break;
        }
    }
    case WM_LBUTTONDOWN: // 그리기 시작
        isClicked = true;
        break;
    case WM_LBUTTONUP: // 그리기 끝
        isClicked = false;
        break;
    case WM_MOUSEMOVE: // 그리기 동선
    {
        if (isConnected && isClicked)
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);

            DRAW_PACKET payload;
            payload.startX = oldX;
            payload.startY = oldY;
            payload.endX = xPos;
            payload.endY = yPos;

            HEADER header;
            header.len = sizeof(payload);

            sendQ.Enqueue((char*)&header, sizeof(header));
            sendQ.Enqueue((char*)&payload, sizeof(payload));

            writeProc();

            oldX = xPos;
            oldY = yPos;
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void readProc(HWND hWnd)
{
    int retval;
    char buffer[MAX_BUFSIZE];

    while (1)
    {
        // tcp 수신버퍼 -> 링버퍼
        retval = recv(sock, buffer, MAX_BUFSIZE, 0);
        if (retval == SOCKET_ERROR)
        {
            if (GetLastError() == WSAEWOULDBLOCK)
            {
                break;
            }
            // 에러처리
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

        DRAW_PACKET payload;
        recvQ.Dequeue((char*)&header, sizeof(HEADER)); // 헤더 디큐
        recvQ.Dequeue((char*)&payload, header.len); // 페이로드 디큐

        // 그리기 처리
        HDC hdc = GetDC(hWnd);
        HPEN hPenOld = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, payload.startX, payload.startY, NULL);
        LineTo(hdc, payload.endX, payload.endY);
        ReleaseDC(hWnd, hdc);
    }
}

void writeProc()
{
    //int retval;

    while (1)
    {
        // 링버퍼 데이터 send()
    }
}