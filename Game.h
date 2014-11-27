#ifndef GAME_H
#define GAME_H

#include "CameraData.h"

#include <stddef.h>

namespace Game
{
	void Start();
	void Quit();
	void Update(double deltaTime);
	CameraData Get_Camera_Data();
	void Toggle_Pause();
}

#endif
