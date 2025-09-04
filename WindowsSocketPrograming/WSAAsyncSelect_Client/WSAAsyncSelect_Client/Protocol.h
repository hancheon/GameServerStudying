#pragma once

struct HEADER
{
	unsigned short len;
};

struct DRAW_PACKET
{
	int startX;
	int startY;
	int endX;
	int endY;
};