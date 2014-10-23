#ifndef INTERNAL_GLOBALS_H
#define INTERNAL_GLOBALS_H

#include "Input.h"

namespace Input
{
	int DetectDevices(ControllerType types[]);
	void DetectControllers();
	void PollMouse(Controller* controller);
}

#endif
