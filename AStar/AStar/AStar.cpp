// AStar.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "AStar.h"
#include "windowsx.h"

#define MAX_LOADSTRING 100

#define GRID_SIZE 16
#define GRID_WIDTH 100
#define GRID_HEIGHT 50

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

HBRUSH hTileBrush;
HPEN hGridPen;
char tile[GRID_HEIGHT][GRID_WIDTH]; // 0: 장애물 없음, 1: 장애물 있음

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
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ASTAR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ASTAR));

    MSG msg;

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASTAR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ASTAR);
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

bool isErase = false;
bool isDrag = false;

void RenderGrid(HDC hdc)
{
    int x = 0;
    int y = 0;
    HPEN hOldPen = (HPEN)SelectObject(hdc, hGridPen);

    for (int wCnt = 0; wCnt <= GRID_WIDTH; wCnt++)
    {
        MoveToEx(hdc, x, 0, NULL);
        LineTo(hdc, x, GRID_HEIGHT * GRID_SIZE);
        x += GRID_SIZE;
    }
    for (int hCnt = 0; hCnt <= GRID_HEIGHT; hCnt++)
    {
        MoveToEx(hdc, 0, y, NULL);
        LineTo(hdc, GRID_WIDTH * GRID_SIZE, y);
        y += GRID_SIZE;
    }
    SelectObject(hdc, hOldPen);
}

void RenderObstacle(HDC hdc)
{
    int x = 0;
    int y = 0;
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hTileBrush);
    SelectObject(hdc, GetStockObject(NULL_PEN));

    for (int wCnt = 0; wCnt < GRID_WIDTH; wCnt++)
    {
        for (int hCnt = 0; hCnt < GRID_HEIGHT; hCnt++)
        {
            if (tile[hCnt][wCnt] == 1)
            {
                x = wCnt * GRID_SIZE;
                y = hCnt * GRID_SIZE;

                Rectangle(hdc, x, y, x + GRID_SIZE + 2, y + GRID_SIZE + 2);
            }
        }
    }

    SelectObject(hdc, hOldBrush);
}

void RenderStart(HDC hdc)
{
    int x = 0;
    int y = 0;
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, CreateSolidBrush(RGB(100, 0, 100)));
    SelectObject(hdc, GetStockObject(NULL_PEN));

    for (int wCnt = 0; wCnt < GRID_WIDTH; wCnt++)
    {
        for (int hCnt = 0; hCnt < GRID_HEIGHT; hCnt++)
        {
            if (tile[hCnt][wCnt] == 2)
            {
                x = wCnt * GRID_SIZE;
                y = hCnt * GRID_SIZE;

                Rectangle(hdc, x, y, x + GRID_SIZE + 2, y + GRID_SIZE + 2);
            }
        }
    }

    SelectObject(hdc, hOldBrush);
}

void RenderEnd(HDC hdc)
{
    int x = 0;
    int y = 0;
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, CreateSolidBrush(RGB(0, 100, 0)));
    SelectObject(hdc, GetStockObject(NULL_PEN));

    for (int wCnt = 0; wCnt < GRID_WIDTH; wCnt++)
    {
        for (int hCnt = 0; hCnt < GRID_HEIGHT; hCnt++)
        {
            if (tile[hCnt][wCnt] == 3)
            {
                x = wCnt * GRID_SIZE;
                y = hCnt * GRID_SIZE;

                Rectangle(hdc, x, y, x + GRID_SIZE + 2, y + GRID_SIZE + 2);
            }
        }
    }

    SelectObject(hdc, hOldBrush);
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//  WM_LBUTTONDBLCLK    - 출발점 설정
//  WM_RBUTTONDBLCLK    - 도착점 설정
//  WM_LBUTTONDOWN  - 장애물 등록: 0
//  WM_RBUTTONDOWN  - 장애물 취소: 1
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_LBUTTONDBLCLK: // 출발지 설정
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        int tileX = xPos / GRID_SIZE;
        int tileY = yPos / GRID_SIZE;

        if (tileX >= 0 && tileX < GRID_WIDTH && tileY >= 0 && tileY < GRID_HEIGHT)
        {
            tile[tileY][tileX] = 2;
            InvalidateRect(hWnd, NULL, true);
        }
    }
    break;
    case WM_RBUTTONDBLCLK: // 도착지 설정
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        int tileX = xPos / GRID_SIZE;
        int tileY = yPos / GRID_SIZE;

        if (tileX >= 0 && tileX < GRID_WIDTH && tileY >= 0 && tileY < GRID_HEIGHT)
        {
            tile[tileY][tileX] = 3;
            InvalidateRect(hWnd, NULL, true);
        }
    }
    break;
    case WM_LBUTTONDOWN: // 장애물 등록 시작
        isDrag = true;
        isErase = false;
        break;
    case WM_LBUTTONUP: // 장애물 등록 끝
        isDrag = false;
        break;
    case WM_RBUTTONDOWN: // 장애물 취소 시작
        isDrag = true;
        isErase = true;
        break;
    case WM_RBUTTONUP: // 장애물 취소 끝
        isDrag = false;
        break;
    case WM_MOUSEMOVE:
    {
        if (isDrag)
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int tileX = xPos / GRID_SIZE;
            int tileY = yPos / GRID_SIZE;

            if (tileX >= 0 && tileX < GRID_WIDTH && tileY >= 0 && tileY < GRID_HEIGHT)
            {
                tile[tileY][tileX] = (!isErase);
                InvalidateRect(hWnd, NULL, true);
            }
        }
    }
    break;
    case WM_CREATE:
        hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        hTileBrush = CreateSolidBrush(RGB(100, 100, 100));
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        RenderObstacle(hdc);
        RenderGrid(hdc);
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        DeleteObject(hTileBrush);
        DeleteObject(hGridPen);
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
