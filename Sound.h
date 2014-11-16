#ifndef SOUND_H
#define SOUND_H

#include "utilities/String.h"

#include <fmod.h>

class Sound
{
public:
	enum Type { SYSTEM, MUSIC, SOUND_EFFECT, NUM_TYPES };

	static void Initialize();
	static void Terminate();
	static Sound* Create_Sound(Type type = SOUND_EFFECT);
	static void Destroy_Sound(Sound* sound);
	static void Update();
	static void Set_Group_Volume(float volume, Type channelType = SYSTEM);
	static float Get_Group_Volume(Type channelType = SYSTEM);

	FMOD_SOUND* sound;
	FMOD_CHANNEL* channel;
	bool isLooping, onlyPlayOnce;
	String name;
	Type type;

	Sound();
	~Sound();

	void Set_Volume(float vol);
	void Load_Audio(const String& fileName);
	void Load_Stream(const String& fileName);
	void Unload();
	void Play(bool pause = false);
	void Set_Pause(bool pause);
	void Toggle_Pause();
	bool Is_Playing() const;
};

#endif
