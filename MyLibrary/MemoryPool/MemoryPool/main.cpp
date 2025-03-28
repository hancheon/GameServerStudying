#include <iostream>
#include "MemoryPool.h"

class Player
{
public:
	Player() : ID(0), attr1(100), attr2(200) { printf("Player »ý¼º\n"); }
	~Player() {}
	int ID;
	int attr1;
	int attr2;
};

int main()
{
	myLibrary::MemoryPool<Player> playerList(0, 0, true);

	for (int i = 0; i < 1000; i++)
	{
		Player* player = playerList.Alloc();
		playerList.Free(player);
	}

	printf("1234");
}