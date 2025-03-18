#include <iostream>
#include "FPSMonitor.h"
#pragma comment(lib, "Winmn")

FPSMonitor::FPSMonitor() : m_fps(DEFAULT_FPS), m_frameCnt(0)
{
	m_oldTick = timeGetTime();
}

FPSMonitor::FPSMonitor(int fps) : m_fps(fps), m_frameCnt(0)
{
	m_oldTick = timeGetTime();
}

void FPSMonitor::CountFrame()
{
	if ((timeGetTime() - m_oldTick) >= (1000 / m_fps))
	{
		printf("FPS: %d\n", m_frameCnt);
		m_oldTick += (1000 / m_fps);
		m_frameCnt = 0;
	}
	else
	{
		m_frameCnt++;
	}
}
