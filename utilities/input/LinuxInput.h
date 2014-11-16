#ifndef LINUX_INPUT_H
#define LINUX_INPUT_H

#include "Input.h"

namespace Input
{
	void RegisterMonitor();
	void UnregisterMonitor();
	void CheckMonitor();

	void PollDevice(int deviceNum, Controller* controller);
}

#endif
