// AStar.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "AStar.h"
#include <windowsx.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#define MAX_LOADSTRING 100

#define GRID_SIZE 16
#define GRID_SIZE_ENLARGEMENT 45
#define GRID_WIDTH 100
#define GRID_HEIGHT 50

int increaseX[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
int increaseY[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };

enum BLOCK_COLOR
{
	BLANK = 0, OBSTACLE, BEGIN, END, OPEN, CLOSE, PATH
};

struct Node
{
	Node* parent; // 부모노드

	int x;
	int y;
	double g; // 출발지에서 노드까지 비용 -> 유클리드
	int h; // 노드에서 목적지까지 비용 -> 맨해튼
	double f; // 출발지에서 해당 노드 거쳐 목적지까지 비용
};

struct TileCost
{
	double g;
	int h;
	double f;
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
HBRUSH hPathBrush;

HPEN hGridPen;
HPEN hArrowPen;

char tile[GRID_HEIGHT][GRID_WIDTH]; // 0: 장애물 없음, 1: 장애물 있음
TileCost tileCost[GRID_HEIGHT][GRID_WIDTH];

int gridSize = GRID_SIZE;

std::vector<Node*> OpenList; // 후보군 리스트
std::vector<Node*> CloseList; // 확인한 노드 리스트
std::vector<Node*>::iterator iter;
Node* beginNode; // 출발지
Node* endNode; // 도착지

bool isEnlarge = false;
bool isDelete = false;

void RenderNodeAttr(int, int);
void RenderArrow(std::vector<Node*>);

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
	if (!InitInstance(hInstance, nCmdShow))
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

	return (int)msg.wParam;
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

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; // 더블클릭추가
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASTAR));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ASTAR);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
void RenderGrid()
{
	int x = 0;
	int y = 0;
	HPEN hOldPen = (HPEN)SelectObject(hMemDC, hGridPen);

	for (int wCnt = 0; wCnt <= GRID_WIDTH; wCnt++)
	{
		MoveToEx(hMemDC, x, 0, NULL);
		LineTo(hMemDC, x, GRID_HEIGHT * gridSize);
		x += gridSize;
	}
	for (int hCnt = 0; hCnt <= GRID_HEIGHT; hCnt++)
	{
		MoveToEx(hMemDC, 0, y, NULL);
		LineTo(hMemDC, GRID_WIDTH * gridSize, y);
		y += gridSize;
	}
	SelectObject(hMemDC, hOldPen);
}

//===================================================================//
// 칸 색칠
// 0: 빈칸, 1: 장애물(회색), 2: 시작점(초록색), 3: 도착점(빨간색)
// 4: OpenList(파란색), 5: CloseList(분홍색), 6: 탐색된 길(노란색)
//===================================================================//
void RenderBlock()
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
			case OBSTACLE:
			{
				hOldBrush = (HBRUSH)SelectObject(hMemDC, hObstacleBrush);
				SelectObject(hMemDC, GetStockObject(NULL_PEN));
				x = wCnt * gridSize;
				y = hCnt * gridSize;
				Rectangle(hMemDC, x, y, x + gridSize + 2, y + gridSize + 2);
			}
			break;
			case BEGIN:
			{
				hOldBrush = (HBRUSH)SelectObject(hMemDC, hBeginBrush);
				SelectObject(hMemDC, GetStockObject(NULL_PEN));
				x = wCnt * gridSize;
				y = hCnt * gridSize;
				Rectangle(hMemDC, x, y, x + gridSize + 2, y + gridSize + 2);
			}
			break;
			case END:
			{
				hOldBrush = (HBRUSH)SelectObject(hMemDC, hEndBrush);
				SelectObject(hMemDC, GetStockObject(NULL_PEN));
				x = wCnt * gridSize;
				y = hCnt * gridSize;
				Rectangle(hMemDC, x, y, x + gridSize + 2, y + gridSize + 2);
			}
			break;
			case OPEN:
			{
				hOldBrush = (HBRUSH)SelectObject(hMemDC, hOpenBrush);
				SelectObject(hMemDC, GetStockObject(NULL_PEN));
				x = wCnt * gridSize;
				y = hCnt * gridSize;
				Rectangle(hMemDC, x, y, x + gridSize + 2, y + gridSize + 2);
				RenderNodeAttr(wCnt, hCnt);
			}
			break;
			case CLOSE:
			{
				hOldBrush = (HBRUSH)SelectObject(hMemDC, hCloseBrush);
				SelectObject(hMemDC, GetStockObject(NULL_PEN));
				x = wCnt * gridSize;
				y = hCnt * gridSize;
				Rectangle(hMemDC, x, y, x + gridSize + 2, y + gridSize + 2);
				RenderNodeAttr(wCnt, hCnt);
			}
			break;
			case PATH:
			{
				hOldBrush = (HBRUSH)SelectObject(hMemDC, hPathBrush);
				SelectObject(hMemDC, GetStockObject(NULL_PEN));
				x = wCnt * gridSize;
				y = hCnt * gridSize;
				Rectangle(hMemDC, x, y, x + gridSize + 2, y + gridSize + 2);
				RenderNodeAttr(wCnt, hCnt);
			}
			break;
			default:
				hOldBrush = (HBRUSH)SelectObject(hMemDC, hObstacleBrush);
				break;
			}
		}
	}

	SelectObject(hMemDC, hOldBrush);
}

void RenderNodeAttr(int x, int y)
{
	if (isEnlarge == true)
	{
		SetBkMode(hMemDC, TRANSPARENT);
		if (tile[y][x] == OPEN)
		{
			SetTextColor(hMemDC, RGB(255, 255, 255));
			std::wstring gValue = std::to_wstring(tileCost[y][x].g);
			std::wstring hValue = std::to_wstring(tileCost[y][x].h);
			std::wstring fValue = std::to_wstring(tileCost[y][x].f);
			std::wstring nodeAttr = L"G: " + gValue + L"\nH: " + hValue + L"\nF: " + fValue;
			RECT pos = { x * gridSize, y * gridSize , (x + 1) * gridSize, (y + 1) * gridSize };
			DrawText(hMemDC, nodeAttr.c_str(), nodeAttr.length(), &pos, DT_LEFT);
		}
		else
		{
			SetTextColor(hMemDC, RGB(0, 0, 0));
			std::wstring gValue = std::to_wstring(tileCost[y][x].g);
			std::wstring hValue = std::to_wstring(tileCost[y][x].h);
			std::wstring fValue = std::to_wstring(tileCost[y][x].f);
			std::wstring nodeAttr = L"G: " + gValue + L"\nH: " + hValue + L"\nF: " + fValue;
			RECT pos = { x * gridSize, y * gridSize , (x + 1) * gridSize, (y + 1) * gridSize };
			DrawText(hMemDC, nodeAttr.c_str(), nodeAttr.length(), &pos, DT_LEFT);

		}
	}
}

void RenderArrow(std::vector<Node*> list)
{
	for (iter = list.begin(); iter != list.end(); iter++)
	{
		if (*iter == beginNode)
			continue;
		int startX = ((*iter)->x * gridSize) + (gridSize / 2);
		int startY = ((*iter)->y * gridSize) + (gridSize / 2);
		int endX = (((*iter)->x * gridSize) + (gridSize / 2) + ((*iter)->parent->x * gridSize) + (gridSize / 2)) / 2;
		int endY = (((*iter)->y * gridSize) + (gridSize / 2) + ((*iter)->parent->y * gridSize) + (gridSize / 2)) / 2;

		HPEN hOldPen = (HPEN)SelectObject(hMemDC, hArrowPen);
		MoveToEx(hMemDC, startX, startY, NULL);
		LineTo(hMemDC, endX, endY);
	}
}
//===================================================================//
// A* 길찾기 로직
//===================================================================//
bool isArrive(int x, int y)
{
	return (x == endNode->x) && (y == endNode->y);
}

bool cmp(Node* first, Node* second)
{
	return first->f > second->f;
}

void FindPath()
{
	if (beginNode == NULL || endNode == NULL)
		return;

	OpenList.clear();
	CloseList.clear();

	// 시작 노드를 후보군 리스트에 넣고 시작
	OpenList.push_back(beginNode);

	while (OpenList.size() != 0) // 후보군 리스트가 0이 되면 종료 (더 이상 길이 없음)
	{
		// OpenList에서 가장 f 작은 노드 뽑기
		Node* node = OpenList.back();
		OpenList.pop_back();

		// 뽑은 노드 CloseList에 넣기
		CloseList.push_back(node);

		if (node != beginNode) // 시작 노드 색 지키기
			tile[node->y][node->x] = CLOSE;

		// 노드 8방향 확인 후 OpenList 설정
		for (int cnt = 0; cnt < 8; cnt++)
		{
			int nextX = node->x + increaseX[cnt];
			int nextY = node->y + increaseY[cnt];

			// 유효한 좌표인지 확인 (맵 밖, 장애물 확인)
			if (nextX < 0 || nextX > GRID_WIDTH || nextY < 0 || nextY > GRID_HEIGHT || tile[nextY][nextX] == OBSTACLE)
			{
				continue;
			}

			// 목적지인지 확인
			if (isArrive(nextX, nextY))
			{
				// 길 그리기
				Node* pathNode = node;
				while (node != beginNode)
				{
					tile[node->y][node->x] = PATH;
					node = node->parent;
				}
				return;
			}

			// 위 조건 해당하지 않으면 노드 생성
			Node* nextNode = new Node;
			nextNode->x = nextX;
			nextNode->y = nextY;
			nextNode->parent = node;
			// 유클리드 거리방식
			if (increaseX[cnt] == 0 || increaseY[cnt] == 0) // 직선이동
				nextNode->g = node->g + 1;
			else // 대각선 이동
				nextNode->g = node->g + 1.4;
			nextNode->h = abs(nextNode->x - endNode->x) + abs(nextNode->y - endNode->y); // 맨해튼 거리방식
			nextNode->f = nextNode->g + nextNode->h;

			bool isDup = false; // 중복확인
			// CloseList에 있는지 확인
			for (iter = CloseList.begin(); iter != CloseList.end(); iter++)
			{
				if ((*iter)->x == nextX && (*iter)->y == nextY)
				{
					isDup = true;
					break;
				}
			}

			// OpenList에 있는지 확인
			for (iter = OpenList.begin(); iter != OpenList.end(); iter++)
			{
				if ((*iter)->x == nextX && (*iter)->y == nextY) // 중복된 노드
				{
					if ((*iter)->g > nextNode->g) // 해당 노드의 새롭게 측정된 g가 더 작을 때 교체 (같은 위치니까 h는 동일)
					{
						(*iter)->parent = nextNode->parent;
						(*iter)->g = nextNode->g;
						(*iter)->f = nextNode->f;
					}
					isDup = true;
					break;
				}
			}

			if (isDup)
			{
				delete nextNode;
				continue;
			}

			// OpenList에 추가
			OpenList.push_back(nextNode);
			tile[nextNode->y][nextNode->x] = OPEN;
			tileCost[nextNode->y][nextNode->x].g = nextNode->g;
			tileCost[nextNode->y][nextNode->x].h = nextNode->h;
			tileCost[nextNode->y][nextNode->x].f = nextNode->f;
		}

		// Matrix 기준으로 OpenList 정렬
		std::sort(OpenList.begin(), OpenList.end(), cmp);
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
bool isLDrag = false;

bool beginExist = false;
bool endExist = false;

int oldX;
int oldY;
int moveX;
int moveY;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (message)
	{
	case WM_LBUTTONDBLCLK: // 출발지 설정
	{
		isDelete = true;
		if (!beginExist)
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int tileX = xPos / gridSize;
			int tileY = yPos / gridSize;

			if (tileX >= 0 && tileX < GRID_WIDTH && tileY >= 0 && tileY < GRID_HEIGHT)
			{
				tile[tileY][tileX] = BEGIN;
				InvalidateRect(hWnd, NULL, false);
			}

			beginNode = new Node;
			beginNode->x = tileX;
			beginNode->y = tileY;
			beginNode->g = 0;
			beginNode->parent = NULL;

			beginExist = true;
		}
		else
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int tileX = xPos / gridSize;
			int tileY = yPos / gridSize;

			if (tileX >= 0 && tileX < GRID_WIDTH && tileY >= 0 && tileY < GRID_HEIGHT)
			{
				tile[beginNode->y][beginNode->x] = BLANK;
				if (beginNode->x == tileX && beginNode->y == tileY)
				{
					delete beginNode;
					beginExist = false;
					InvalidateRect(hWnd, NULL, false);
					break;
				}

				for (int hCnt = 0; hCnt < GRID_HEIGHT; hCnt++)
				{
					for (int wCnt = 0; wCnt < GRID_WIDTH; wCnt++)
					{
						if (tile[hCnt][wCnt] != BEGIN && tile[hCnt][wCnt] != END && tile[hCnt][wCnt] != OBSTACLE)
						{
							tile[hCnt][wCnt] = BLANK;
						}
					}
				}

				tile[tileY][tileX] = BEGIN;
				InvalidateRect(hWnd, NULL, false);
			}

			beginNode->x = tileX;
			beginNode->y = tileY;
			beginNode->g = 0;
			beginNode->parent = NULL;

			beginExist = true;
		}
	}
	break;
	case WM_RBUTTONDBLCLK: // 도착지 설정
	{
		isDelete = true;
		if (!endExist)
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int tileX = xPos / gridSize;
			int tileY = yPos / gridSize;

			if (tileX >= 0 && tileX < GRID_WIDTH && tileY >= 0 && tileY < GRID_HEIGHT)
			{
				tile[tileY][tileX] = END;
				InvalidateRect(hWnd, NULL, false);
			}

			endNode = new Node;
			endNode->x = tileX;
			endNode->y = tileY;
			endNode->parent = NULL;

			endExist = true;
		}
		else
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int tileX = xPos / gridSize;
			int tileY = yPos / gridSize;

			if (tileX >= 0 && tileX < GRID_WIDTH && tileY >= 0 && tileY < GRID_HEIGHT)
			{
				tile[endNode->y][endNode->x] = BLANK;
				if (endNode->x == tileX && endNode->y == tileY)
				{
					delete endNode;
					endExist = false;
					InvalidateRect(hWnd, NULL, false);
					break;
				}

				for (int hCnt = 0; hCnt < GRID_HEIGHT; hCnt++)
				{
					for (int wCnt = 0; wCnt < GRID_WIDTH; wCnt++)
					{
						if (tile[hCnt][wCnt] != BEGIN && tile[hCnt][wCnt] != END && tile[hCnt][wCnt] != OBSTACLE)
						{
							tile[hCnt][wCnt] = BLANK;
						}
					}
				}

				tile[tileY][tileX] = END;
				InvalidateRect(hWnd, NULL, false);
			}

			endNode->x = tileX;
			endNode->y = tileY;
			endNode->parent = NULL;

			endExist = true;
		}
	}
	break;
	case WM_LBUTTONDOWN: // 장애물 등록 시작
	{
		isLDrag = true;

		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		int tileX = xPos / gridSize;
		int tileY = yPos / gridSize;

		if (tileX >= 0 && tileX < GRID_WIDTH && tileY >= 0 && tileY < GRID_HEIGHT)
		{
			if (tile[tileY][tileX] == 1)
				isErase = true;
			else
				isErase = false;
		}
	}
	break;
	case WM_LBUTTONUP: // 장애물 등록 끝
		isLDrag = false;
		break;
	case WM_MOUSEMOVE:
	{
		if (isLDrag)
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int tileX = xPos / gridSize;
			int tileY = yPos / gridSize;

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
		switch (wParam)
		{
		case VK_DELETE: // 일괄 삭제
		{
			memset(tile, 0, sizeof(tile));
			isDelete = true;
			InvalidateRect(hWnd, NULL, false);
			beginExist = false;
			endExist = false;
			delete beginNode;
			delete endNode;
		}
		break;
		case VK_RETURN:
		{
			for (int hCnt = 0; hCnt < GRID_HEIGHT; hCnt++)
			{
				for (int wCnt = 0; wCnt < GRID_WIDTH; wCnt++)
				{
					if (tile[hCnt][wCnt] != BEGIN && tile[hCnt][wCnt] != END && tile[hCnt][wCnt] != OBSTACLE)
					{
						tile[hCnt][wCnt] = BLANK;
					}
				}
			}
			isDelete = false;
			FindPath();
			InvalidateRect(hWnd, NULL, false);
		}
		break;
		case VK_SPACE: // 탐색 단계별로
			break;
		case VK_CONTROL: // 그리드 확대
		{
			gridSize = (gridSize == GRID_SIZE ? GRID_SIZE_ENLARGEMENT : GRID_SIZE);
			// 좌표 표기 활성화
			isEnlarge = (gridSize == GRID_SIZE ? false : true);
			InvalidateRect(hWnd, NULL, false);
		}
		break;
		default:
			break;
		}
	}
	break;
	case WM_CREATE:
	{
		hdc = GetDC(hWnd);
		GetClientRect(hWnd, &memDCRect);
		hMemDCBitmap = CreateCompatibleBitmap(hdc, memDCRect.right, memDCRect.bottom);
		hMemDC = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
		hOldMemDCBitmap = (HBITMAP)SelectObject(hMemDC, hMemDCBitmap);

		// 사용할 펜과 브러쉬
		hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
		hArrowPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
		hObstacleBrush = CreateSolidBrush(RGB(100, 100, 100));
		hBeginBrush = CreateSolidBrush(RGB(0, 255, 0));
		hEndBrush = CreateSolidBrush(RGB(255, 0, 0));
		hCloseBrush = CreateSolidBrush(RGB(200, 0, 200));
		hOpenBrush = CreateSolidBrush(RGB(0, 0, 255));
		hPathBrush = CreateSolidBrush(RGB(255, 255, 0));
		break;
	}
	case WM_PAINT:
	{
		// 메모리 DC 클리어
		PatBlt(hMemDC, 0, 0, memDCRect.right, memDCRect.bottom, WHITENESS);

		// 메모리 DC에 출력
		RenderBlock();
		if (!isDelete)
		{
			RenderArrow(OpenList);
			RenderArrow(CloseList);
		}
		RenderGrid();

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
