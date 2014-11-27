#include "Game.h"

#include "Camera.h"
#include "CameraData.h"

#include "utilities/Maths.h"
#include "utilities/Logging.h"
#include "utilities/input/Input.h"

namespace Game
{
	bool paused = true;

	Camera camera;
	CameraData camera_data;
}

void Game::Start()
{
	Input::Initialize();

	paused = false;
}

void Game::Quit()
{
	Input::Terminate();
}

void Game::Update(double deltaTime)
{
	// do input logic
	Input::Poll();

	{
		Input::Controller* input = Input::GetController(Input::PLAYER_1);

		const float CAMERA_MOVE_SPEED = 2.0f;
		const float CAMERA_TURN_SPEED = M_PI / 70.0f;

		camera.Rotate(CAMERA_TURN_SPEED * vec2(-input->rightAnalog[0], input->rightAnalog[1]));

		vec3 cameraMovement = camera.rotation * vec3(-input->leftAnalog[0], 0.0f, input->leftAnalog[1]);
		camera.Move(CAMERA_MOVE_SPEED * cameraMovement);

		float zoom = input->rightTrigger - input->leftTrigger;
		camera.Zoom(zoom);

		// pause game
		if(input->GetButtonTapped(Input::START))
		{
			Toggle_Pause();

			bool mouseRelative = !paused;
			Input::SetMouseMode(mouseRelative);
		}
	}

	// update camera
	{
		camera_data = camera.Update(deltaTime);
	}
}

CameraData Game::Get_Camera_Data()
{
	return camera_data;
}

void Game::Toggle_Pause()
{
	paused = !paused;
}
