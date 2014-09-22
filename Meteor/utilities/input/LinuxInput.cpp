#if defined(__linux__)

#include "LinuxInput.h"

#include "InternalGlobals.h"
#include "LinuxEvDevUtils.h"

#include "utilities/Logging.h"
#include "utilities/StringUtils.h"

#include <libudev.h>

#include <linux/input.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <stdio.h>

namespace Input
{
	enum DeviceClass
	{
		UDEV_DEVICE_MOUSE		= 0x0001,
		UDEV_DEVICE_KEYBOARD	= 0x0002,
		UDEV_DEVICE_JOYSTICK	= 0x0004,
	};

	struct Device
	{
		struct AxisSpecification
		{
			int deadband[2];
			float coefficients[2];
		};

		int file;
		int deviceClass;
		char name[128];

		int numButtons;
		unsigned char buttonMap[KEY_MAX - BTN_MISC];
		unsigned char absoluteMap[ABS_MAX];

		int numAxes;
		AxisSpecification axisAttributes[ABS_MAX];

		int numHats;
		int numBalls;
	};

	struct udev* udev;
	udev_monitor* deviceMonitor;

	Device devices[5];
	int numDevices = 0;

	void SetupDevice(udev_device* device);
	bool ConfigureDevice(Device* device);
	void OnDeviceChange(udev_device* device, const char* action);

	void PollEvents(
		const Device& device,
		const input_event events[],
		int numEvents,
		Controller* controller);

	void PollButton(int code, int state, Controller* controller);
	void PollAbsolute(const Device& device, int code, int state, Controller* controller);
}

void Input::RegisterMonitor()
{
	// create library interface
	udev = udev_new();
	if(!udev)
	{
		LOG_ISSUE("Can't create low-level input: udev");
	}

	// create monitor for device-changed notifications
	deviceMonitor = udev_monitor_new_from_netlink(udev, "udev");
	if(deviceMonitor == NULL)
	{
		LOG_ISSUE("Could not create monitor for detecting device connects/disconnects");
	}

	udev_monitor_filter_add_match_subsystem_devtype(deviceMonitor, "input", NULL);
	udev_monitor_enable_receiving(deviceMonitor);
}

void Input::UnregisterMonitor()
{
	udev_monitor_unref(deviceMonitor);

	udev_unref(udev);

	for(int i = 0; i < numDevices; i++)
	{
		Device& device = devices[i];

		close(device.file);
		LOG_INFO("closed input device: %s", device.name);
	}
}

void Input::DetectDevices()
{
	// Create a list of the devices in the 'hidraw' subsystem.
	udev_enumerate* enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "input");
	udev_enumerate_scan_devices(enumerate);
	udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry* entry;
	udev_list_entry_foreach(entry, devices)
	{
		const char* path = udev_list_entry_get_name(entry);
		udev_device* device = udev_device_new_from_syspath(udev, path);
		if(device == NULL) continue;

		SetupDevice(device);

		udev_device_unref(device);
	}

	udev_enumerate_unref(enumerate);
}

static bool check_monitor(udev_monitor* deviceMonitor)
{
	if(deviceMonitor == nullptr) return false;

	const int fd = udev_monitor_get_fd(deviceMonitor);

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if((select(fd + 1, &fds, NULL, NULL, &tv) > 0) && (FD_ISSET(fd, &fds)))
		return true;

	return false;
}

void Input::CheckMonitor()
{
	if(deviceMonitor != nullptr)
	{
		while(check_monitor(deviceMonitor))
		{
			udev_device* device = udev_monitor_receive_device(deviceMonitor);
			if(device == NULL) break;

			const char* action = udev_device_get_action(device);
			OnDeviceChange(device, action);

			udev_device_unref(device);
		}
	}
}

void Input::SetupDevice(udev_device* device)
{
	const char* devicePath = udev_device_get_devnode(device);
	if(devicePath == NULL) return;

	// query which classes are supported by the device
	int deviceClass = 0;

	const char* subsystem = udev_device_get_subsystem(device);
	if(strcmp(subsystem, "input") == 0)
	{
		// parse out device class from udev properties
		const char* val = udev_device_get_property_value(device, "ID_INPUT_JOYSTICK");
		if(val != nullptr && strcmp(val, "1") == 0)
			deviceClass |= UDEV_DEVICE_JOYSTICK;

		val = udev_device_get_property_value(device, "ID_INPUT_MOUSE");
		if(val != nullptr && strcmp(val, "1") == 0)
			deviceClass |= UDEV_DEVICE_MOUSE;

		val = udev_device_get_property_value(device, "ID_INPUT_KEYBOARD");
		if(val != nullptr && strcmp(val, "1") == 0 )
			deviceClass |= UDEV_DEVICE_KEYBOARD;

		if(deviceClass == 0)
		{
			// Fall back to old style input classes
			val = udev_device_get_property_value(device, "ID_CLASS");
			if(val != NULL)
			{
				if(strcmp(val, "joystick") == 0)	deviceClass = UDEV_DEVICE_JOYSTICK;
				else if(strcmp(val, "mouse") == 0)	deviceClass = UDEV_DEVICE_MOUSE;
				else if(strcmp(val, "kbd") == 0)	deviceClass = UDEV_DEVICE_KEYBOARD;
			}
		}
	}

	// get miscellaneous device info
	dev_t deviceNumber = udev_device_get_devnum(device);

	// open device
	int file = open(devicePath, O_RDONLY, 0);
	if(file < 0) return;

	// set read mode to non-blocking
	fcntl(file, F_SETFL, O_NONBLOCK);

	LOG_INFO("input device #%lu opened at: %s", deviceNumber, devicePath);

	Device item = {};
	item.file = file;
	item.deviceClass = deviceClass;

	// determine what input system to use
	bool configured = false;

	const char* systemName = udev_device_get_sysname(device);
	if(starts_with(systemName, "event"))
	{
		// use evdev system
		configured = ConfigureDevice(&item);
	}
	else if(starts_with(systemName, "js"))
	{
		// use joystick api
	}

	if(configured)
	{
		devices[numDevices++] = item;

		Controller& controller = controllers[num_controllers++];
		controller.type = GAMEPAD_EVDEV;
	}
	else
	{
		close(file);
		LOG_INFO("input device #%lu closed", deviceNumber);
	}
}

#define BIT_COUNT(x) ((((x)-1)/(sizeof(long) * 8))+1)

#define CHECK_BIT(array, n) \
    (((1UL << ((n) % (sizeof(long) * 8))) & ((array)[(n) / (sizeof(long) * 8)])) != 0)

bool Input::ConfigureDevice(Device* device)
{
	int file = device->file;

	// get basic identifying information
	if(ioctl(file, EVIOCGNAME(sizeof device->name), device->name) < 0)
	{
		return false;
	}

	input_id id;
	if(ioctl(file, EVIOCGID, &id) < 0)
	{
		return false;
	}

	// determine capabilities
	unsigned long keybit[BIT_COUNT(KEY_MAX)] = { 0 };
	unsigned long absbit[BIT_COUNT(ABS_MAX)] = { 0 };
	unsigned long relbit[BIT_COUNT(REL_MAX)] = { 0 };

	if (ioctl(file, EVIOCGBIT(EV_KEY, sizeof keybit), keybit) < 0 ||
		ioctl(file, EVIOCGBIT(EV_ABS, sizeof absbit), absbit) < 0 ||
		ioctl(file, EVIOCGBIT(EV_REL, sizeof relbit), relbit) < 0)
	{
		return false;
	}

	//--- Get Buttons ---

	// get normal joystick buttons first
	for(int i = BTN_JOYSTICK; i < KEY_MAX; ++i)
	{
		if(CHECK_BIT(keybit, i))
		{
			DEBUG_LOG("Joystick has button: 0x%x %s", i, button_code_text(i));

			device->buttonMap[i - BTN_MISC] = device->numButtons;
			device->numButtons++;
		}
	}
	// then, get miscellaneous buttons
	for(int i = BTN_MISC; i < BTN_JOYSTICK; ++i)
	{
		if(CHECK_BIT(keybit, i))
		{
			DEBUG_LOG("Joystick has button: 0x%x %s", i, button_code_text(i));

			device->buttonMap[i - BTN_MISC] = device->numButtons;
			device->numButtons++;
		}
	}

	//--- Get Axes ---
	for(int i = 0; i < ABS_MISC; ++i)
	{
		// skip hats
		if(i == ABS_HAT0X)
		{
			i = ABS_HAT3Y;
			continue;
		}

		// get axis specification
		if(CHECK_BIT(absbit, i))
		{
			input_absinfo absinfo;
			if(ioctl(file, EVIOCGABS(i), &absinfo) < 0)
				continue;

			device->absoluteMap[i] = device->numAxes;

			int min = absinfo.minimum;
			int max = absinfo.maximum;

			int deadZone = absinfo.flat + (max - min) / 9;

			int deadMin = 0;
			float normalizedMin = 0.0f;
			if(min < 0)
			{
				deadMin = -deadZone;
				normalizedMin = -1.0f;
			}
			device->axisAttributes[i].deadband[0] = deadMin;

			int deadMax = 0;
			float normalizedMax = 0.0f;
			if(max > 0)
			{
				deadMax = deadZone;
				normalizedMax = 1.0f;
			}
			device->axisAttributes[i].deadband[1] = deadMax;

			float a = (normalizedMax - normalizedMin) / float((max - deadMax) + (deadMin - min));
			float b = normalizedMax - a * float(max - deadMax);

			device->axisAttributes[i].coefficients[0] = a;
			device->axisAttributes[i].coefficients[1] = b;

			device->numAxes++;

			DEBUG_LOG(
				"Joystick has absolute axis: 0x%x %s\t"
				"current:%i   range:[%i, %i]   "
				"fuzz:%i   flat:%i   resolution:%i   "
				"deadband:[%i, %i]",
				i, abs_code_text(i),
				absinfo.value, absinfo.minimum, absinfo.maximum,
				absinfo.fuzz, absinfo.flat, absinfo.resolution,
				deadMin, deadMax);
		}
	}

	//--- Get Hat Switches ---
	for(int i = ABS_HAT0X; i <= ABS_HAT3Y; i += 2)
	{
		if(CHECK_BIT(absbit, i) || CHECK_BIT(absbit, i + 1))
		{
			input_absinfo absinfo;
			if(ioctl(file, EVIOCGABS(i), &absinfo) < 0)
				continue;

			DEBUG_LOG(
				"Joystick has hat: 0x%x %s\t"
				"Values = { %i, %i, %i, %i, %i, %i }",
				(i - ABS_HAT0X) / 2, abs_code_text(i),
				absinfo.value, absinfo.minimum, absinfo.maximum,
				absinfo.fuzz, absinfo.flat, absinfo.resolution);

			device->numHats++;
		}
	}

	//--- Get Balls ---
	if(CHECK_BIT(relbit, REL_X) || CHECK_BIT(relbit, REL_Y))
	{
		device->numBalls++;
	}

	return true;
}

void Input::OnDeviceChange(udev_device* device, const char* action)
{
	// action can be equal to "add" or "remove"

	SetupDevice(device);
}

void Input::PollDevice(int deviceNum, Controller* controller)
{
	Device& device = devices[deviceNum];

	input_event events[32];

	int numBytes;
	while((numBytes = read(device.file, events, sizeof events)) > 0)
	{
		int numEvents = numBytes / sizeof events[0];
		PollEvents(device, events, numEvents, controller);
	}
}

void Input::PollEvents(
	const Device& device,
	const input_event events[],
	int numEvents,
	Controller* controller)
{
	for(int i = 0; i < numEvents; ++i)
	{
		int code = events[i].code;
		int state = events[i].value;

		switch(events[i].type)
		{
			case EV_KEY:
			{
				PollButton(code, state, controller);
				break;
			}
			case EV_ABS:
			{
				PollAbsolute(device, code, state, controller);
				break;
			}
			case EV_REL:
			{
				switch(code)
				{
					case REL_X: case REL_Y:
					{
						// Ball Events
						code -= REL_X;

						break;
					}
				}
				break;
			}
			case EV_SYN:
			{
				switch(code)
				{
					case SYN_DROPPED:
					{
						break;
					}
				}
				break;
			}
		}
	}
}

void Input::PollButton(int code, int state, Controller* controller)
{
	if(code < BTN_MISC) return;

	Button button = NUM_BUTTONS;
	switch(code)
	{
		case BTN_SOUTH:  button = B;           break;
		case BTN_EAST:   button = A;           break;
		case BTN_NORTH:  button = X;           break;
		case BTN_WEST:   button = Y;           break;
		case BTN_TL:     button = L_SHOULDER;  break;
		case BTN_TR:     button = R_SHOULDER;  break;
		case BTN_SELECT: button = SELECT;      break;
		case BTN_START:  button = START;       break;

		case BTN_DPAD_UP:    button = D_UP;    break;
		case BTN_DPAD_DOWN:  button = D_DOWN;  break;
		case BTN_DPAD_LEFT:  button = D_LEFT;  break;
		case BTN_DPAD_RIGHT: button = D_RIGHT; break;
	}

	if(button >= NUM_BUTTONS) return;

	controller->buttons[button] = state != 0;
}

void Input::PollAbsolute(const Device& device, int code, int state, Controller* controller)
{
	if(code >= ABS_MISC) return;

	switch(code)
	{
		case ABS_HAT0X:
		{
			// D-Pad EW
			controller->buttons[D_LEFT] = state < 0;
			controller->buttons[D_RIGHT] = state > 0;
			break;
		}
		case ABS_HAT0Y:
		{
			// D-Pad NS
			controller->buttons[D_UP] = state < 0;
			controller->buttons[D_DOWN] = state > 0;
			break;
		}
		case ABS_HAT1X:
		case ABS_HAT1Y:
		case ABS_HAT3X:
		case ABS_HAT3Y:
		{
			// Ignore
			break;
		}
		default:
		{
			// Axis Events
			int deadMin = device.axisAttributes[code].deadband[0];
			int deadMax = device.axisAttributes[code].deadband[1];

			float magnitude = state;
			if(state < deadMin)
				magnitude -= deadMin;
			else if(state > deadMax)
				magnitude -= deadMax;
			else
				magnitude = 0.0f;

			float a = device.axisAttributes[code].coefficients[0];
			float b = device.axisAttributes[code].coefficients[1];
			float normalized = a * magnitude + b;

			switch(code)
			{
				// Trigger values
				case ABS_Z: case ABS_HAT2Y:
					controller->leftTrigger = normalized;
					break;

				case ABS_RZ: case ABS_HAT2X:
					controller->rightTrigger = normalized;
					break;

				// Stick values
				case ABS_X: controller->leftAnalog[0] = normalized; break;
				case ABS_Y: controller->leftAnalog[1] = normalized; break;

				case ABS_RX: controller->rightAnalog[0] = normalized; break;
				case ABS_RY: controller->rightAnalog[1] = normalized; break;
			}
			break;
		}
	}
}

#endif // defined(__linux__)
