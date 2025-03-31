// AStar.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "AStar.h"
#include "windowsx.h"
#include <iostream>
#include <list>

#define MAX_LOADSTRING 100

#define GRID_SIZE 16
#define GRID_WIDTH 100
#define GRID_HEIGHT 50

int increaseX[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
int increaseY[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };

enum BLOCK_COLOR
{
    OBSTACLE = 1, BEGIN, END, OPEN, CLOSE, PATH
};

struct Node
{
    int x;
    int y;
    int g; // 출발지에서 노드까지 비용 -> 맨해튼
    double h; // 노드에서 목적지까지 비용 -> 유클리드
    int f; // 출발지에서 해당 노드 거쳐 목적지까지 비용
};

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

HBITMAP hMemDCBitmap;
HBITMAP hOldMemDCBitmap;
HDC hMemDC;
RECT memDCRect;

HBRUSH hObstacleBrush;
HBRUSH hBeginBrush;
HBRUSH hEndBrush;
HBRUSH hCloseBrush;
HBRUSH hOpenBrush;
HPEN hGridPen;
char tile[GRID_HEIGHT][GRID_WIDTH]; // 0: 장애물 없음, 1: 장애물 있음

Node* beginNode;
Node* endNode;

//int beginToNodeMode = MANHATTAN;
//int nodeToEndMode = EUCLID;

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

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
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

   HWND hWnd = CreateWindowW(szWindowClass, L"Path Finding", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//===================================================================//
// 그리드 출력
//===================================================================//
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

//===================================================================//
// 칸 색칠
// 0: 빈칸, 1: 장애물, 2: 시작점, 3: 도착점
// 4: OpenList, 5: CloseList, 6: 탐색된 길
//===================================================================//
void RenderBlock(HDC hdc)
{
    int x = 0;
    int y = 0;

    HBRUSH hOldBrush = NULL;

    for (int wCnt = 0; wCnt < GRID_WIDTH; wCnt++)
    {
        for (int hCnt = 0; hCnt < GRID_HEIGHT; hCnt++)
        {
            switch (tile[hCnt][wCnt])
            {
            case 1:
            {
                hOldBrush = (HBRUSH)SelectObject(hdc, hObstacleBrush);
                SelectObject(hdc, GetStockObject(NULL_PEN));
                x = wCnt * GRID_SIZE;
                y = hCnt * GRID_SIZE;
                Rectangle(hdc, x, y, x + GRID_SIZE + 2, y + GRID_SIZE + 2);
            }
                break;
            case 2:
            {
                hOldBrush = (HBRUSH)SelectObject(hdc, hBeginBrush);
                SelectObject(hdc, GetStockObject(NULL_PEN));
                x = wCnt * GRID_SIZE;
                y = hCnt * GRID_SIZE;
                Rectangle(hdc, x, y, x + GRID_SIZE + 2, y + GRID_SIZE + 2);
            }
                break;
            case 3:
            {
                hOldBrush = (HBRUSH)SelectObject(hdc, hEndBrush);
                SelectObject(hdc, GetStockObject(NULL_PEN));
                x = wCnt * GRID_SIZE;
                y = hCnt * GRID_SIZE;
                Rectangle(hdc, x, y, x + GRID_SIZE + 2, y + GRID_SIZE + 2);
            }
                break;
            default:
                hOldBrush = (HBRUSH)SelectObject(hdc, hObstacleBrush);
                break;
            }
        }
    }

    SelectObject(hdc, hOldBrush);
}

//===================================================================//
// A* 길찾기 로직
//===================================================================//
int BeginToNode(Node* node)
{
    node->g = abs(node->x - beginNode->x) + abs(node->y - beginNode->y);
}

double NodeToEnd(Node* node)
{
    node->h = sqrt(pow(endNode->x - node->x, 2) + pow(endNode->y - node->y, 2));
}

double Matrix(Node* node)
{
    node->f = node->g + node->h;
}

bool isArrive(Node* node)
{
    return (node->x == endNode->x) && (node->y == endNode->y);
}

void FindPath()
{
    if (beginNode == NULL || endNode == NULL)
        return;

    std::list<Node*> OpenList;
    std::list<Node*> CloseList;
    OpenList.push_back(beginNode);
    while (OpenList.size() != 0)
    {
        Node* node = OpenList.back();
        OpenList.pop_back();
        CloseList.push_back(node);
        
        for (int cnt = 0; cnt < 8; cnt++)
        {
            Node* nextNode = new Node;
            nextNode->x = node->x + increaseX[cnt];
            nextNode->y = node->y + increaseY[cnt];

            // 유효한 좌표인지 확인
            if (nextNode->x < 0 || nextNode->x > GRID_WIDTH || nextNode->y < 0 || nextNode->y > GRID_HEIGHT)
            {
                delete nextNode;
                continue;
            }

            // 목적지인지 확인
            if (isArrive(nextNode))
                return;

            // CloseList에 있는지 확인
            std::list<Node*>::iterator iter;
            for (iter = CloseList.begin(); iter != CloseList.end(); iter++)
            {
                if ((*iter)->x == nextNode->x && (*iter)->y == nextNode->y)
                {
                    break;
                }
            }

            OpenList.push_back(nextNode);
        }
    }
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
//  WM_LBUTTONDOWN  - 장애물 등록
//  WM_RBUTTONDOWN  - 장애물 취소
//

bool isErase = false;
bool isDrag = false;
bool isBegin = false;
bool isEnd = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_LBUTTONDBLCLK: // 출발지 설정
    {
        if (!isBegin)
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int tileX = xPos / GRID_SIZE;
            int tileY = yPos / GRID_SIZE;

            if (tileX >= 0 && tileX < GRID_WIDTH && tileY >= 0 && tileY < GRID_HEIGHT)
            {
                tile[tileY][tileX] = BEGIN;
                InvalidateRect(hWnd, NULL, false);
            }

            beginNode = new Node;
            beginNode->x = tileX;
            beginNode->y = tileY;

            isBegin = true;
        }
        else
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int tileX = xPos / GRID_SIZE;
            int tileY = yPos / GRID_SIZE;

            if (tileX >= 0 && tileX < GRID_WIDTH && tileY >= 0 && tileY < GRID_HEIGHT)
            {
                tile[beginNode->y][beginNode->x] = 0;
                if (beginNode->x == tileX && beginNode->y == tileY)
                {
                    delete beginNode;
                    isBegin = false;
                    InvalidateRect(hWnd, NULL, false);
                    break;
                }
                tile[tileY][tileX] = BEGIN;
                InvalidateRect(hWnd, NULL, false);
            }

            beginNode->x = tileX;
            beginNode->y = tileY;

            isBegin = true;
        }
    }
    break;
    case WM_RBUTTONDBLCLK: // 도착지 설정
    {
        if (!isEnd)
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int tileX = xPos / GRID_SIZE;
            int tileY = yPos / GRID_SIZE;

            if (tileX >= 0 && tileX < GRID_WIDTH && tileY >= 0 && tileY < GRID_HEIGHT)
            {
                tile[tileY][tileX] = END;
                InvalidateRect(hWnd, NULL, false);
            }

            endNode = new Node;
            endNode->x = tileX;
            endNode->y = tileY;

            isEnd = true;
        }
        else
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int tileX = xPos / GRID_SIZE;
            int tileY = yPos / GRID_SIZE;

            if (tileX >= 0 && tileX < GRID_WIDTH && tileY >= 0 && tileY < GRID_HEIGHT)
            {
                tile[endNode->y][endNode->x] = 0;
                if (endNode->x == tileX && endNode->y == tileY)
                {
                    delete endNode;
                    isEnd = false;
                    InvalidateRect(hWnd, NULL, false);
                    break;
                }
                tile[tileY][tileX] = END;
                InvalidateRect(hWnd, NULL, false);
            }

            endNode->x = tileX;
            endNode->y = tileY;

            isEnd = true;
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
                InvalidateRect(hWnd, NULL, false);
            }
        }
    }
    break;
    case WM_KEYDOWN:
    {
        if (wParam == VK_DELETE)
        {
            memset(tile, 0, sizeof(tile));
            InvalidateRect(hWnd, NULL, false);
        }
        else if (wParam == VK_RETURN)
        {
            FindPath();
        }
    }
    case WM_CREATE:
    {
        hdc = GetDC(hWnd);
        GetClientRect(hWnd, &memDCRect);
        hMemDCBitmap = CreateCompatibleBitmap(hdc, memDCRect.right, memDCRect.bottom);
        hMemDC = CreateCompatibleDC(hdc);
        ReleaseDC(hWnd, hdc);
        hOldMemDCBitmap = (HBITMAP)SelectObject(hMemDC, hMemDCBitmap);
        hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        hObstacleBrush = CreateSolidBrush(RGB(100, 100, 100));
        hBeginBrush = CreateSolidBrush(RGB(0, 200, 0));
        hEndBrush = CreateSolidBrush(RGB(200, 0, 0));
        hCloseBrush = CreateSolidBrush(RGB(200, 0, 200));
        hOpenBrush = CreateSolidBrush(RGB(0, 200, 200));
        break;
    }
    case WM_PAINT:
    {
        // 메모리 DC 클리어
        PatBlt(hMemDC, 0, 0, memDCRect.right, memDCRect.bottom, WHITENESS);

        // 메모리 DC에 출력
        RenderBlock(hMemDC);
        RenderGrid(hMemDC);

        // 메모리 DC -> 윈도우 DC Swap
        hdc = BeginPaint(hWnd, &ps);
        BitBlt(hdc, 0, 0, memDCRect.right, memDCRect.bottom, hMemDC, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        SelectObject(hMemDC, hOldMemDCBitmap);
        DeleteObject(hMemDC);
        DeleteObject(hMemDCBitmap);
        DeleteObject(hObstacleBrush);
        PostQuitMessage(0);
        break;
    case WM_SIZE:
    {
        SelectObject(hMemDC, hOldMemDCBitmap);
        DeleteObject(hMemDC);
        DeleteObject(hMemDCBitmap);

        hdc = GetDC(hWnd);

        GetClientRect(hWnd, &memDCRect);
        hMemDCBitmap = CreateCompatibleBitmap(hdc, memDCRect.right, memDCRect.bottom);
        hMemDC = CreateCompatibleDC(hdc);
        ReleaseDC(hWnd, hdc);

        hOldMemDCBitmap = (HBITMAP)SelectObject(hMemDC, hMemDCBitmap);
    }
    case WM_MOUSEWHEEL: // 확대 축소 (각 노드 속성 보기 위해서)
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
