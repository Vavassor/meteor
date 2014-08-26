#ifndef INPUT_H
#define INPUT_H

class InputDevice;

namespace Input
{
	enum Button
	{
		A, B, X, Y,
		START, SELECT,
		L_SHOULDER, R_SHOULDER,
		D_RIGHT, D_UP, D_LEFT, D_DOWN,
		NUM_BUTTONS,
	};

	enum DeviceType { KEYBOARD_AND_MOUSE, GAMEPAD_XINPUT, GAMEPAD_JOYSTICK, };
	enum PlayerSlot { PLAYER_1, PLAYER_2, PLAYER_3, PLAYER_4, MAX_PLAYERS };

	void Initialize();
	void Terminate();

	InputDevice* GetDevice(PlayerSlot slot);
	void GetMousePosition(int point[2]);
	void SetMouseMode(bool relative);

	void DetectDevices();
	void Poll();
}

class InputDevice
{
public:
	Input::DeviceType type;
	float leftAnalog[2];
	float rightAnalog[2];
	float leftTrigger, rightTrigger;

	bool GetButtonDown(Input::Button button) const;
	bool GetButtonReleased(Input::Button button) const;
	bool GetButtonTapped(Input::Button button) const;
	void UpdateButtons(bool pressed[]);

private:
	enum ButtonState { PRESSED, RELEASED, ONCE };

	ButtonState buttons[Input::NUM_BUTTONS];
};

#endif
