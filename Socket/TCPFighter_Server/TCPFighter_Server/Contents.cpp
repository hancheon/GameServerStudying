#include <iostream>
#include <Windows.h>
#include "Session.h"
#include "Contents.h"
#include "Network.h"
#include "myList.h"

extern int oldTick;

extern myList<SESSION*> users;
extern myList<SESSION*> disconnects;

void update()
{
    // 50 프레임
    if (timeGetTime() - oldTick < 20)
        return;

    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        // 사망 처리
        if ((*iter)->HP <= 0)
        {
            disconnect(*iter);
        }
        else // 이동처리
        {
            int nextXPos;
            int nextYPos;
            switch ((*iter)->action)
            {
            case dfPACKET_MOVE_DIR_LL:
            {
                (*iter)->xPos = moveLeft((*iter)->xPos);
                printf("# [MOVE] SessionID: %d | direction: LL | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->xPos, (*iter)->yPos);
                break;
            }
            case dfPACKET_MOVE_DIR_LU:
            {
                nextXPos = moveLeft((*iter)->xPos);
                nextYPos = moveUp((*iter)->yPos);
                if (nextXPos != dfRANGE_MOVE_LEFT && nextYPos != dfRANGE_MOVE_TOP)
                {
                    (*iter)->xPos = nextXPos;
                    (*iter)->yPos = nextYPos;
                }
                printf("# [MOVE] SessionID: %d | direction: LU | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->xPos, (*iter)->yPos);
                break;
            }
            case dfPACKET_MOVE_DIR_UU:
            {
                (*iter)->yPos = moveUp((*iter)->yPos);
                printf("# [MOVE] SessionID: %d | direction: UU | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->xPos, (*iter)->yPos);
                break;
            }
            case dfPACKET_MOVE_DIR_RU:
            {
                nextXPos = moveRight((*iter)->xPos);
                nextYPos = moveUp((*iter)->yPos);
                if (nextXPos != dfRANGE_MOVE_RIGHT && nextYPos != dfRANGE_MOVE_TOP)
                {
                    (*iter)->xPos = nextXPos;
                    (*iter)->yPos = nextYPos;
                }
                printf("# [MOVE] SessionID: %d | direction: RU | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->xPos, (*iter)->yPos);
                break;
            }
            case dfPACKET_MOVE_DIR_RR:
            {
                (*iter)->xPos = moveRight((*iter)->xPos);
                printf("# [MOVE] SessionID: %d | direction: RR | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->xPos, (*iter)->yPos);
                break;
            }
            case dfPACKET_MOVE_DIR_RD:
            {
                nextXPos = moveRight((*iter)->xPos);
                nextYPos = moveDown((*iter)->yPos);
                if (nextXPos != dfRANGE_MOVE_RIGHT && nextYPos != dfRANGE_MOVE_BOTTOM)
                {
                    (*iter)->xPos = nextXPos;
                    (*iter)->yPos = nextYPos;
                }
                printf("# [MOVE] SessionID: %d | direction: RD | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->xPos, (*iter)->yPos);
                break;
            }
            case dfPACKET_MOVE_DIR_DD:
            {
                (*iter)->yPos = moveDown((*iter)->yPos);
                printf("# [MOVE] SessionID: %d | direction: DD | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->xPos, (*iter)->yPos);
                break;
            }
            case dfPACKET_MOVE_DIR_LD:
            {
                nextXPos = moveLeft((*iter)->xPos);
                nextYPos = moveDown((*iter)->yPos);
                if (nextXPos != dfRANGE_MOVE_LEFT && nextYPos != dfRANGE_MOVE_BOTTOM)
                {
                    (*iter)->xPos = nextXPos;
                    (*iter)->yPos = nextYPos;
                }
                printf("# [MOVE] SessionID: %d | direction: LD | xPos: %d | yPos: %d\n", (*iter)->sessionID, (*iter)->xPos, (*iter)->yPos);
                break;
            }
            }
        }
    }

    oldTick += 20;
}

//====================================================================//
// X 좌표 이동 함수
//====================================================================//
short moveLeft(short xPos)
{
    return max(xPos - FP_X, dfRANGE_MOVE_LEFT);
}

short moveRight(short xPos)
{
    return min(xPos + FP_X, dfRANGE_MOVE_RIGHT);
}

//====================================================================//
// Y 좌표 이동 함수
//====================================================================//
short moveUp(short yPos)
{
    return max(yPos - FP_Y, dfRANGE_MOVE_TOP);
}

short moveDown(short yPos)
{
    return min(yPos + FP_Y, dfRANGE_MOVE_BOTTOM);
}
