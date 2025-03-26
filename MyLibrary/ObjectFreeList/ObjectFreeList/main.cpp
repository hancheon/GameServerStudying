#include <iostream>
#include "ObjectFreeList.h"

struct Player
{
	int ID;
	int attr1;
	int attr2;
};

int main()
{
	myLibrary::ObjectFreeList<Player> playerList;
	Player* newPlayer = playerList.Alloc();
	newPlayer->ID = 3;
	newPlayer->attr1 = 2;
	newPlayer->attr2 = 1;
	playerList.Free(newPlayer);
	printf("123");
}