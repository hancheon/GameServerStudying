#include <stdio.h>
#include <Windows.h>
#include "Console.h"

HANDLE hConsole;

// �ܼ� ��� ���� �ʱ� �۾�
void consoleInit(void)
{
	CONSOLE_CURSOR_INFO stConsoleCursor;

	// Ŀ�� ����
	stConsoleCursor.dwSize = 1;
	stConsoleCursor.bVisible = FALSE;

	// ǥ����� �ڵ� ����
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}

// Ŀ�� �̵�
void moveCursor(int xpos, int ypos)
{
	// COORD ����ü: �� Ŀ�� ��ġ�� ���� ������ �����ϴ� ����ü
	COORD stCoord;

	stCoord.X = xpos;
	stCoord.Y = ypos;

	// Ŀ�� ��ġ �̵�
	SetConsoleCursorPosition(hConsole, stCoord);
}