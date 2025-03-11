#include <iostream>
#include <Windows.h>
#include "Session.h"
#include "Contents.h"
#include "Network.h"
#include "myList.h"
#pragma comment(lib, "Winmm")

extern int oldTick;
extern myList<SESSION*> users;
extern myList<SESSION*> disconnects;

void update()
{
    int workTime = timeGetTime() - oldTick;
    if (workTime < 20)
        return;

    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        if ((*iter)->HP <= 0)
        {
            disconnect(*iter);
        }
        else // 이동처리
        {
            switch ((*iter)->action)
            {
            case dfPACKET_MOVE_DIR_LL:
                (*iter)->xPos -= 3;
                printf("# [MOVE] SessionID: %d | direction: %d | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->action, (*iter)->xPos, (*iter)->yPos);
                break;
            case dfPACKET_MOVE_DIR_LU:
                (*iter)->xPos -= 3;
                (*iter)->yPos -= 2;
                printf("# [MOVE] SessionID: %d | direction: %d | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->action, (*iter)->xPos, (*iter)->yPos);
                break;
            case dfPACKET_MOVE_DIR_UU:
                (*iter)->yPos -= 2;
                printf("# [MOVE] SessionID: %d | direction: %d | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->action, (*iter)->xPos, (*iter)->yPos);
                break;
            case dfPACKET_MOVE_DIR_RU:
                (*iter)->xPos += 3;
                (*iter)->yPos -= 2;
                printf("# [MOVE] SessionID: %d | direction: %d | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->action, (*iter)->xPos, (*iter)->yPos);
                break;
            case dfPACKET_MOVE_DIR_RR:
                (*iter)->xPos += 3;
                printf("# [MOVE] SessionID: %d | direction: %d | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->action, (*iter)->xPos, (*iter)->yPos);
                break;
            case dfPACKET_MOVE_DIR_RD:
                (*iter)->xPos += 3;
                (*iter)->yPos += 2;
                printf("# [MOVE] SessionID: %d | direction: %d | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->action, (*iter)->xPos, (*iter)->yPos);
                break;
            case dfPACKET_MOVE_DIR_DD:
                (*iter)->yPos += 2;
                printf("# [MOVE] SessionID: %d | direction: %d | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->action, (*iter)->xPos, (*iter)->yPos);
                break;
            case dfPACKET_MOVE_DIR_LD:
                (*iter)->xPos -= 3;
                (*iter)->yPos += 2;
                printf("# [MOVE] SessionID: %d | direction: %d | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->action, (*iter)->xPos, (*iter)->yPos);
                break;
            }
        }
    }

    oldTick += 20;
}