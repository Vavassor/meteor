#ifndef WINDOWS_INPUT_H
#define WINDOWS_INPUT_H

#include "Input.h"

namespace Input
{
	void InitializeWindow();
	void TerminateWindow();
	void Capture_Mouse(bool enable);
	void MessageLoop();
	void PollXInputDevice(int deviceNum, Controller* pad);
}

#endif
