#ifndef GAME_H
#define GAME_H

#include <stddef.h>

#include "Message.h"
#include "CameraData.h"

namespace Game
{
	unsigned long __stdcall Main(void* param);
	void Signal();
	void Quit();
	bool PumpMessage(Message& message);
	void GiveMessage(int type, void* data, size_t dataSize);
	void GetCameraData(CameraData* data);
}

#endif
