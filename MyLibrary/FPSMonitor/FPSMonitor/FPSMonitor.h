#ifndef __FPS__
#define __FPS__
#include "Windows.h"
#define DEFAULT_FPS 60

class FPSMonitor
{
public:
	FPSMonitor();
	FPSMonitor(int);

	void CountFrame();
protected:
	DWORD m_oldTick;
	int m_fps;
	int m_frameCnt;
};

#endif