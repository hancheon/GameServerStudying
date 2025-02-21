#pragma once

struct ID_ALLOC
{
	int type;
	int id;
	int none1;
	int none2;
};

struct STAR_CREATE
{
	int type;
	int id;
	int xpos;
	int ypos;
};

struct STAR_DELETE
{
	int type;
	int id;
	int none1;
	int none2;
};

struct STAR_MOVE
{
	int type;
	int id;
	int xpos;
	int ypos;
};

struct PLAYER
{
	int id;
	int xpos;
	int ypos;
};