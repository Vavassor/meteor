#ifndef WINDOWS_INPUT_H
#define WINDOWS_INPUT_H

#include "Input.h"

namespace Input
{
	void InitializeWindow();
	void TerminateWindow();
	void CaptureMouse(bool enable);
	void MessageLoop();
	void PollXInputDevice(int deviceNum, Controller* pad);
}

#endif
