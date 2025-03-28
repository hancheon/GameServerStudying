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
	myLibrary::MemoryPool<Player> playerList(10, 0, true);
	myLibrary::MemoryPool<Player> playerList2(10, 0, false);

	printf("123\n");

	for (int i = 0; i < 11; i++)
	{
		Player* player = playerList2.Alloc();
	}
}