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

bool update()
{
    int workTime = timeGetTime() - oldTick;
    if (workTime < 20)
        return true;

    myList<SESSION*>::iterator iter;
    for (iter = users.begin(); iter != users.end(); iter++)
    {
        if ((*iter)->hp <= 0)
        {
            disconnect(*iter);
        }
    }

    return true;
}