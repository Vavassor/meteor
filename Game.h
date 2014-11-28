#ifndef GAME_H
#define GAME_H

#include "CameraData.h"

namespace Game
{
	void Start();
	void Quit();
	void Update(double delta_time);
	CameraData Get_Camera_Data();
	void Toggle_Pause();
}

#endif
