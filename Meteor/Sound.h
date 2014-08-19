#ifndef SOUND_H
#define SOUND_H

#include <fmod.h>

#include "BString.h"

class Sound
{
public:
	enum Type { SYSTEM, MUSIC, SOUND_EFFECT, NUM_TYPES };

	static void Initialize();
	static void Terminate();
	static Sound* CreateSound(Type type = SOUND_EFFECT);
	static void DestroySound(Sound* sound);
	static void Update();
	static void SetGroupVolume(float volume, Type channelType = SYSTEM);
	static float GetGroupVolume(Type channelType = SYSTEM);

	FMOD_SOUND* sound;
	FMOD_CHANNEL* channel;
	bool isLooping, onlyPlayOnce;
	String name;
	Type type;

	Sound();
	~Sound();

	void SetVolume(float vol);
	void LoadAudio(const String& fileName);
	void LoadStream(const String& fileName);
	void Unload();
	void Play(bool pause = false);
	void SetPause(bool pause);
	void TogglePause();
	bool IsPlaying() const;

private:
	static const int MAX_SOUNDS = 32;
	static Sound* sounds[MAX_SOUNDS];
	static int numSounds;
};

#endif
