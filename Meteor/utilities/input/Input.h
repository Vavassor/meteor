#ifndef INPUT_H
#define INPUT_H

namespace Input
{
	class Controller;

	enum Button
	{
		A, B, X, Y,
		START, SELECT,
		L_SHOULDER, R_SHOULDER,
		D_RIGHT, D_UP, D_LEFT, D_DOWN,
		NUM_BUTTONS,
	};

	enum ControllerType { KEYBOARD_AND_MOUSE, GAMEPAD_XINPUT, GAMEPAD_EVDEV, };
	enum PlayerSlot { PLAYER_1, PLAYER_2, PLAYER_3, PLAYER_4, MAX_PLAYERS };

	void Initialize();
	void Terminate();

	Controller* GetController(PlayerSlot slot);
	void GetMousePosition(int point[2]);
	void SetMouseMode(bool relative);

	void DetectDevices();
	void Poll();

	class Controller
	{
	public:
		ControllerType type;
		bool buttons[NUM_BUTTONS];
		float leftAnalog[2];
		float rightAnalog[2];
		float leftTrigger, rightTrigger;

		bool GetButtonDown(Button button) const;
		bool GetButtonReleased(Button button) const;
		bool GetButtonTapped(Button button) const;

	private:
		enum ButtonState { PRESSED, RELEASED, ONCE };

		ButtonState buttonStates[NUM_BUTTONS];

		friend void Input::Poll();

		void Update();
	};

} // namespace Input

#endif
