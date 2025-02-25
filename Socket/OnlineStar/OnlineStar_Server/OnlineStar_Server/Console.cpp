#include <stdio.h>
#include <Windows.h>
#include "Console.h"

HANDLE hConsole;

// 콘솔 제어를 위한 초기 작업
void consoleInit(void)
{
	CONSOLE_CURSOR_INFO stConsoleCursor;

	// 커서 숨김
	stConsoleCursor.dwSize = 1;
	stConsoleCursor.bVisible = FALSE;

	// 표준출력 핸들 저장
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}

// 커서 이동
void moveCursor(int xpos, int ypos)
{
	// COORD 구조체: 새 커서 위치를 문자 단위로 지정하는 구조체
	COORD stCoord;

	stCoord.X = xpos;
	stCoord.Y = ypos;

	// 커서 위치 이동
	SetConsoleCursorPosition(hConsole, stCoord);
}