#pragma once

// 이동 가능 경계
#define dfRANGE_MOVE_TOP	50
#define dfRANGE_MOVE_LEFT	10
#define dfRANGE_MOVE_RIGHT	630
#define dfRANGE_MOVE_BOTTOM	470

// 프레임 당 이동거리
#define FP_X 3
#define FP_Y 2

void update();

short moveLeft(short xPos);
short moveRight(short xPos);
short moveUp(short yPos);
short moveDown(short yPos);