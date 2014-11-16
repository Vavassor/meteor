#ifndef GAME_H
#define GAME_H

#include "Message.h"
#include "CameraData.h"

#include <stddef.h>

#if defined(_WIN32)
#define THREAD_RETURN_TYPE unsigned long __stdcall
#elif defined(__unix__)
#define THREAD_RETURN_TYPE void*
#endif

namespace Game
{
	THREAD_RETURN_TYPE Main(void* param);
	void Signal();
	void Quit();
	bool PumpMessage(Message& message);
	void GiveMessage(int type, void* data, size_t dataSize);
	void GetCameraData(CameraData* data);
}

#endif
