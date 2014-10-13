#include "Game.h"

#include "ThreadMessages.h"
#include "Camera.h"
#include "CameraData.h"

#include "utilities/Benaphore.h"
#include "utilities/Mutex.h"
#include "utilities/Timer.h"
#include "utilities/input/Input.h"
#include "utilities/Maths.h"
#include "utilities/Logging.h"
#include "utilities/Assert.h"
#include "utilities/collections/LinkedQueue.h"

#if defined(_MSC_VER) && defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

#include <cstring>
#include <cstdio>

namespace Game
{
	Benaphore updateSignal;
	volatile bool isRunning;

	double tickStartTime, tickEndTime;
	bool paused;

	LinkedQueue<Message> incomingMessages;
	LinkedQueue<Message> outgoingMessages;

	Camera camera;
	CameraData cameraData;
	Mutex cameraMutex;

	void Initialize();
	void Terminate();
	void Update(double deltaTime);
	void ThreadMessageLoop();

	void OutMessage(int type, void* data, size_t dataSize);
	void TogglePause();
}

void Game::Initialize()
{
	Input::Initialize();
	Input::DetectDevices();
}

void Game::Terminate()
{
	Input::Terminate();
}

THREAD_RETURN_TYPE Game::Main(void* param)
{
	Initialize();

	tickStartTime = Timer::GetTime();
	tickEndTime = tickStartTime - 16.0;

	paused = false;

	isRunning = true;
	while(isRunning)
	{
		updateSignal.Lock();

		// update time counters
		static unsigned long lastFPSTime = Timer::GetMilliseconds();
		static int fps = 0;
		static int fpsSample = 0;

		unsigned long time = Timer::GetMilliseconds();

		if(time - lastFPSTime > 1000)
		{
			#if defined(_DEBUG)
			{
				#if defined(_MSC_VER) && defined(_WIN32)
					wchar_t out[64];
					wsprintf(out, L"Game Thread : %i TPS\n", fpsSample);
					OutputDebugString(out);
				#else
					printf("Game Thread : %i TPS\n", fpsSample);
				#endif
			}
			#endif

			fpsSample = fps;
			lastFPSTime = time;
			fps = 0;
		}
		else
		{
			fps++;
		}

		tickStartTime = tickEndTime;
		tickEndTime = Timer::GetTime();

		// update game logic
		Update(tickEndTime - tickStartTime);
	}

	Terminate();

	LOG_INFO("Game thread shut down successfully");

	return 0;
}

void Game::Signal()
{
	updateSignal.Unlock();
}

void Game::Quit()
{
	isRunning = false;
	Signal();
}

bool Game::PumpMessage(Message& message)
{
	return outgoingMessages.Dequeue(message);
}

void Game::GiveMessage(int type, void* data, size_t dataSize)
{
	DEBUG_ASSERT(dataSize <= SIZE_MESSAGE);

	Message message = {};
	message.type = type;
	memcpy(message.data, data, dataSize);

	incomingMessages.Enqueue(message);
}

void Game::OutMessage(int type, void* data, size_t dataSize)
{
	DEBUG_ASSERT(dataSize <= SIZE_MESSAGE);

	Message message = {};
	message.type = type;
	memcpy(message.data, data, dataSize);

	outgoingMessages.Enqueue(message);
}

void Game::GetCameraData(CameraData* data)
{
	cameraMutex.Acquire();
	{
		memcpy(data, &cameraData, sizeof(CameraData));
	}
	cameraMutex.Release();
}

void Game::ThreadMessageLoop()
{
	Message message = {};
	while(incomingMessages.Dequeue(message))
	{
		switch(message.type)
		{
			case MESSAGE_RESIZE:
			{
				ViewportData* viewport = (ViewportData*) message.data;
				camera.Resize(viewport->width, viewport->height);
				camera.ResetZoom();
				break;
			}
		}
	}
}

void Game::Update(double deltaTime)
{
	ThreadMessageLoop();

	Input::Poll();

	{
		Input::Controller* input = Input::GetController(Input::PLAYER_1);

		static const float CAMERA_MOVE_SPEED = 2.0f;
		static const float CAMERA_TURN_SPEED = M_PI / 70.0f;

		camera.Rotate(CAMERA_TURN_SPEED * vec2(-input->rightAnalog[0], input->rightAnalog[1]));

		vec3 cameraMovement = camera.rotation * vec3(-input->leftAnalog[0], 0.0f, input->leftAnalog[1]);
		camera.Move(CAMERA_MOVE_SPEED * cameraMovement);

		float zoom = input->rightTrigger - input->leftTrigger;
		camera.Zoom(zoom);

		// pause game
		if(input->GetButtonTapped(Input::START))
		{
			TogglePause();

			bool mouseRelative = !paused;
			Input::SetMouseMode(mouseRelative);
		}
	}

	camera.Tick(deltaTime);

	// update camera data
	{
		cameraMutex.Acquire();

		cameraData.projection = camera.projection;
		cameraData.isOrtho = camera.GetIsOrtho();
		cameraData.position = camera.position;
		cameraData.viewX = camera.viewX;
		cameraData.viewY = camera.viewY;
		cameraData.viewZ = camera.viewZ;
		cameraData.fov = camera.fov;
		cameraData.nearPlane = camera.nearPlane;
		cameraData.farPlane = camera.farPlane;

		cameraMutex.Release();
	}
}

void Game::TogglePause()
{
	paused = !paused;
}
