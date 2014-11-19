#include "Sound.h"

#include "utilities/Logging.h"

#include <cstring>

namespace
{
	const int MAX_CHANNELS = 8;
	FMOD_SYSTEM* fmodSystem;
	FMOD_SOUNDGROUP *musicGroup, *noiseGroup;

	static const int MAX_SOUNDS = 32;
	Sound* sounds[MAX_SOUNDS];
	int numSounds = 0;

	void check_error(FMOD_RESULT result);
	const char* fmoderr_text(FMOD_RESULT errorCode);
}

void Sound::Initialize()
{
    FMOD_RESULT result = FMOD_OK;

	result = FMOD_System_Create(&fmodSystem);
	check_error(result);

	unsigned int version;
	result = FMOD_System_GetVersion(fmodSystem, &version);
	check_error(result);
	if(version < FMOD_VERSION)
	{
		LOG_ISSUE("AUDIO ERROR: fmodex.dll is an older version than needed. "
			"FMOD version should be at least %u.", FMOD_VERSION);
	}

	int numDrivers;
	result = FMOD_System_GetNumDrivers(fmodSystem, &numDrivers);
	check_error(result);

	if(numDrivers == 0)
	{
		result = FMOD_System_SetOutput(fmodSystem, FMOD_OUTPUTTYPE_NOSOUND);
		check_error(result);
	}
	else
	{
		FMOD_CAPS capabilities;
		FMOD_SPEAKERMODE speakerMode;
		result = FMOD_System_GetDriverCaps(fmodSystem, 0, &capabilities, nullptr, &speakerMode);
		check_error(result);

		result = FMOD_System_SetSpeakerMode(fmodSystem, speakerMode);
		check_error(result);

		// if hardware acceleration is not available,
		// extend the buffer size to make sure there is enough room
		if(capabilities & FMOD_CAPS_HARDWARE_EMULATED)
		{
			result = FMOD_System_SetDSPBufferSize(fmodSystem, 1024, 10);
			check_error(result);
		}

		char name[256];
		result = FMOD_System_GetDriverInfo(fmodSystem, 0, name, 256, nullptr);
		check_error(result);

		// SigmaTel sound devices crackle when the sound format is PCM 16-bit
		// PCM Floating-Point seems to fix it
		if(strstr(name, "SigmaTel"))
		{
			result = FMOD_System_SetSoftwareFormat(fmodSystem, 48000, FMOD_SOUND_FORMAT_PCMFLOAT,
				0, 0, FMOD_DSP_RESAMPLER_LINEAR);
			check_error(result);
		}
	}

	result = FMOD_System_Init(fmodSystem, MAX_CHANNELS, FMOD_INIT_NORMAL, 0);
	check_error(result);

	result = FMOD_System_CreateSoundGroup(fmodSystem, "Music", &musicGroup);
	check_error(result);
	result = FMOD_System_CreateSoundGroup(fmodSystem, "Sound Effects", &noiseGroup);
	check_error(result);

	numSounds = 0;
	for(int i = 0; i < MAX_SOUNDS; i++)
		sounds[i] = nullptr;
}

void Sound::Terminate()
{
	for(int i = 0; i < MAX_SOUNDS; i++)
	{
		Sound* sound = sounds[i];
		if(sound == nullptr) continue;
		delete sound;
	}

	FMOD_RESULT s_result = FMOD_OK;

	s_result = FMOD_SoundGroup_Release(musicGroup);
	check_error(s_result);
	s_result = FMOD_SoundGroup_Release(noiseGroup);
	check_error(s_result);

	s_result = FMOD_System_Close(fmodSystem);
	check_error(s_result);
	s_result = FMOD_System_Release(fmodSystem);
	check_error(s_result);
}

Sound* Sound::Create_Sound(Type type) 
{
	if(numSounds < MAX_SOUNDS)
	{
		Sound* sound = new Sound();
		for(int i = 0; i < MAX_SOUNDS; i++)
		{
			if(sounds[i] == nullptr)
			{
				sounds[i] = sound;
				break;
			}
		}
		numSounds++;
		sound->type = type;
		return sound;
	}
	return nullptr;
}

void Sound::Destroy_Sound(Sound* sound)
{
	for(int i = 0; i < MAX_SOUNDS; i++)
	{
		if(sounds[i] == sound)
		{
			sounds[i] = nullptr;
			delete sound;
			numSounds--;
			break;
		}
	}
}

void Sound::Update()
{
	FMOD_System_Update(fmodSystem);

	for(int i = 0; i < MAX_SOUNDS; i++)
	{
		Sound* sound = sounds[i];
		if(sound == nullptr) continue;

		if(sound->onlyPlayOnce && !sound->Is_Playing())
			Destroy_Sound(sound);
	}
}

void Sound::Set_Group_Volume(float volume, Type channelType)
{
	if(volume > 1.0f || volume < 0.0f) return;

	switch(channelType)
	{
		case SYSTEM:
			FMOD_SoundGroup_SetVolume(musicGroup, volume);
			FMOD_SoundGroup_SetVolume(noiseGroup, volume);
			break;
		case MUSIC:
			FMOD_SoundGroup_SetVolume(musicGroup, volume);
			break;
		case SOUND_EFFECT:
			FMOD_SoundGroup_SetVolume(noiseGroup, volume);
			break;
	}
}

float Sound::Get_Group_Volume(Type channelType)
{
	float currentVolume = 0.0f;
	switch(channelType)
	{
		case SYSTEM:
		{
			FMOD_SoundGroup_GetVolume(musicGroup, &currentVolume);
			float avg = currentVolume;
			FMOD_SoundGroup_GetVolume(noiseGroup, &currentVolume);
			currentVolume = (avg + currentVolume) / 2.0f;
			break;
		}
		case MUSIC:
			FMOD_SoundGroup_GetVolume(musicGroup, &currentVolume);
			break;
		case SOUND_EFFECT:
			FMOD_SoundGroup_GetVolume(noiseGroup, &currentVolume);
			break;
	}
	return currentVolume;
}

Sound::Sound():
	sound(nullptr),
	channel(nullptr),
	isLooping(false),
	onlyPlayOnce(false),
	type(SOUND_EFFECT)
{
	FMOD_Channel_SetVolume(channel, 0.0f);
}

Sound::~Sound()
{
	Unload();
}

void Sound::Set_Volume(float vol) 
{
    if (vol >= 0.0f && vol <= 1.0f) 
	{
        FMOD_Channel_SetVolume(channel, vol);
    }
}

void Sound::Load_Audio(const String& fileName)
{
	if(fileName.Count() == 0) return;

	name = fileName;

	FMOD_RESULT s_result = FMOD_OK;
    s_result = FMOD_Sound_Release(sound);
	check_error(s_result);

	String path("data/sounds/");
	path.Append(fileName);

    s_result = FMOD_System_CreateSound(fmodSystem, path.Data(),
		FMOD_SOFTWARE | FMOD_UNICODE, nullptr, &sound);
	check_error(s_result);

	switch(type)
	{
		case MUSIC:
			FMOD_Sound_SetSoundGroup(sound, musicGroup);
			break;

		case SOUND_EFFECT:
			FMOD_Sound_SetSoundGroup(sound, noiseGroup);
			break;
	}
}

void Sound::Load_Stream(const String& fileName) 
{
	if(fileName.Count() == 0) return;

	name = fileName;

	String path("data/sounds/");
	path.Append(fileName);

	FMOD_RESULT s_result = FMOD_OK;

	if(sound != nullptr)
	{
		s_result = FMOD_Sound_Release(sound);
		check_error(s_result);
	}

    s_result = FMOD_System_CreateSound(fmodSystem, path.Data(),
		FMOD_SOFTWARE | FMOD_UNICODE | FMOD_CREATESTREAM, nullptr, &sound);
	check_error(s_result);

	switch(type)
	{
		case MUSIC:
			FMOD_Sound_SetSoundGroup(sound, musicGroup);
			break;

		case SOUND_EFFECT:
			FMOD_Sound_SetSoundGroup(sound, noiseGroup);
			break;
	}
}

void Sound::Unload()
{
	FMOD_RESULT s_result = FMOD_OK;
	s_result = FMOD_Sound_Release(sound);
}

void Sound::Play(bool pause)
{
	FMOD_RESULT s_result = FMOD_OK;
	s_result = FMOD_System_PlaySound(fmodSystem, FMOD_CHANNEL_FREE, sound, pause, &channel);
	if(s_result == FMOD_OK && isLooping)
		FMOD_Channel_SetMode(channel, FMOD_LOOP_NORMAL);
}

void Sound::Set_Pause(bool pause)
{
	FMOD_Channel_SetPaused(channel, pause);
}

void Sound::Toggle_Pause() 
{
    FMOD_BOOL p;
    FMOD_Channel_GetPaused(channel, &p);
    FMOD_Channel_SetPaused(channel, !p);
}

bool Sound::Is_Playing() const
{
	FMOD_BOOL isPlaying;
	FMOD_Channel_IsPlaying(channel, &isPlaying);
	return isPlaying != 0;
}

namespace
{
	void check_error(FMOD_RESULT result)
	{
		if(result != FMOD_OK)
		{
			LOG_ISSUE("FMOD Error: %s", fmoderr_text(result));
		}
	}

	const char* fmoderr_text(FMOD_RESULT errorCode)
	{
		switch(errorCode)
		{
			case FMOD_ERR_ALREADYLOCKED:          return "Tried to call lock a second time before unlock was called. ";
			case FMOD_ERR_BADCOMMAND:             return "Tried to call a function on a data type that does not allow this type of functionality (ie calling Sound::lock on a streaming sound). ";
			case FMOD_ERR_CDDA_DRIVERS:           return "Neither NTSCSI nor ASPI could be initialised. ";
			case FMOD_ERR_CDDA_INIT:              return "An error occurred while initialising the CDDA subsystem. ";
			case FMOD_ERR_CDDA_INVALID_DEVICE:    return "Couldn't find the specified device. ";
			case FMOD_ERR_CDDA_NOAUDIO:           return "No audio tracks on the specified disc. ";
			case FMOD_ERR_CDDA_NODEVICES:         return "No CD/DVD devices were found. ";
			case FMOD_ERR_CDDA_NODISC:            return "No disc present in the specified drive. ";
			case FMOD_ERR_CDDA_READ:              return "A CDDA read error occurred. ";
			case FMOD_ERR_CHANNEL_ALLOC:          return "Error trying to allocate a channel. ";
			case FMOD_ERR_CHANNEL_STOLEN:         return "The specified channel has been reused to play another sound. ";
			case FMOD_ERR_COM:                    return "A Win32 COM related error occurred. COM failed to initialize or a QueryInterface failed meaning a Windows codec or driver was not installed properly. ";
			case FMOD_ERR_DMA:                    return "DMA Failure.  See debug output for more information. ";
			case FMOD_ERR_DSP_CONNECTION:         return "DSP connection error.  Connection possibly caused a cyclic dependancy. ";
			case FMOD_ERR_DSP_FORMAT:             return "DSP Format error.  A DSP unit may have attempted to connect to this network with the wrong format. ";
			case FMOD_ERR_DSP_NOTFOUND:           return "DSP connection error.  Couldn't find the DSP unit specified. ";
			case FMOD_ERR_DSP_RUNNING:            return "DSP error.  Cannot perform this operation while the network is in the middle of running.  This will most likely happen if a connection or disconnection is attempted in a DSP callback. ";
			case FMOD_ERR_DSP_TOOMANYCONNECTIONS: return "DSP connection error.  The unit being connected to or disconnected should only have 1 input or output. ";
			case FMOD_ERR_FILE_BAD:               return "Error loading file. ";
			case FMOD_ERR_FILE_COULDNOTSEEK:      return "Couldn't perform seek operation.  This is a limitation of the medium (ie netstreams) or the file format. ";
			case FMOD_ERR_FILE_EOF:               return "End of file unexpectedly reached while trying to read essential data (truncated data?). ";
			case FMOD_ERR_FILE_NOTFOUND:          return "File not found. ";
			case FMOD_ERR_FILE_UNWANTED:          return "Unwanted file access occurred.";
			case FMOD_ERR_FORMAT:                 return "Unsupported file or audio format. ";
			case FMOD_ERR_HTTP:                   return "A HTTP error occurred. This is a catch-all for HTTP errors not listed elsewhere. ";
			case FMOD_ERR_HTTP_ACCESS:            return "The specified resource requires authentication or is forbidden. ";
			case FMOD_ERR_HTTP_PROXY_AUTH:        return "Proxy authentication is required to access the specified resource. ";
			case FMOD_ERR_HTTP_SERVER_ERROR:      return "A HTTP server error occurred. ";
			case FMOD_ERR_HTTP_TIMEOUT:           return "The HTTP request timed out. ";
			case FMOD_ERR_INITIALIZATION:         return "FMOD was not initialized correctly to support this function. ";
			case FMOD_ERR_INITIALIZED:            return "Cannot call this command after System::init. ";
			case FMOD_ERR_INTERNAL:               return "An error occurred that wasnt supposed to.  Contact support. ";
			case FMOD_ERR_INVALID_HANDLE:         return "An invalid object handle was used. ";
			case FMOD_ERR_INVALID_PARAM:          return "An invalid parameter was passed to this function. ";
			case FMOD_ERR_INVALID_SPEAKER:        return "An invalid speaker was passed to this function based on the current speaker mode. ";
			case FMOD_ERR_MEMORY:                 return "Not enough memory or resources. ";
			case FMOD_ERR_MEMORY_SRAM:            return "Not enough memory or resources on console sound ram. ";
			case FMOD_ERR_NEEDS2D:                return "Tried to call a command on a 3d sound when the command was meant for 2d sound. ";
			case FMOD_ERR_NEEDS3D:                return "Tried to call a command on a 2d sound when the command was meant for 3d sound. ";
			case FMOD_ERR_NEEDSHARDWARE:          return "Tried to use a feature that requires hardware support.  (ie trying to play a VAG compressed sound in software on PS2). ";
			case FMOD_ERR_NEEDSSOFTWARE:          return "Tried to use a feature that requires the software engine.  Software engine has either been turned off, or command was executed on a hardware channel which does not support this feature. ";
			case FMOD_ERR_NET_CONNECT:            return "Couldn't connect to the specified host. ";
			case FMOD_ERR_NET_SOCKET_ERROR:       return "A socket error occurred.  This is a catch-all for socket-related errors not listed elsewhere. ";
			case FMOD_ERR_NET_URL:                return "The specified URL couldn't be resolved. ";
			case FMOD_ERR_NOTREADY:               return "Operation could not be performed because specified sound is not ready. ";
			case FMOD_ERR_OUTPUT_ALLOCATED:       return "Error initializing output device, but more specifically, the output device is already in use and cannot be reused. ";
			case FMOD_ERR_OUTPUT_CREATEBUFFER:    return "Error creating hardware sound buffer. ";
			case FMOD_ERR_OUTPUT_DRIVERCALL:      return "A call to a standard soundcard driver failed, which could possibly mean a bug in the driver or resources were missing or exhausted. ";
			case FMOD_ERR_OUTPUT_FORMAT:          return "Soundcard does not support the minimum features needed for this soundsystem (16bit stereo output). ";
			case FMOD_ERR_OUTPUT_INIT:            return "Error initializing output device. ";
			case FMOD_ERR_OUTPUT_NOHARDWARE:      return "FMOD_HARDWARE was specified but the sound card does not have the resources necessary to play it. ";
			case FMOD_ERR_OUTPUT_NOSOFTWARE:      return "Attempted to create a software sound but no software channels were specified in System::init. ";
			case FMOD_ERR_PAN:                    return "Panning only works with mono or stereo sound sources. ";
			case FMOD_ERR_PLUGIN:                 return "An unspecified error has been returned from a 3rd party plugin. ";
			case FMOD_ERR_PLUGIN_MISSING:         return "A requested output, dsp unit type or codec was not available. ";
			case FMOD_ERR_PLUGIN_RESOURCE:        return "A resource that the plugin requires cannot be found. (ie the DLS file for MIDI playback) ";
			case FMOD_ERR_RECORD:                 return "An error occurred trying to initialize the recording device. ";
			case FMOD_ERR_REVERB_INSTANCE:        return "Specified Instance in FMOD_REVERB_PROPERTIES couldn't be set. Most likely because another application has locked the EAX4 FX slot. ";
			case FMOD_ERR_SUBSOUNDS:              return " The error occurred because the sound referenced contains subsounds.  (ie you cannot play the parent sound as a static sample, only its subsounds.)";
			case FMOD_ERR_SUBSOUND_ALLOCATED:     return "This subsound is already being used by another sound, you cannot have more than one parent to a sound.  Null out the other parent's entry first. ";
			case FMOD_ERR_TAGNOTFOUND:            return "The specified tag could not be found or there are no tags. ";
			case FMOD_ERR_TOOMANYCHANNELS:        return "The sound created exceeds the allowable input channel count.  This can be increased with System::setMaxInputChannels. ";
			case FMOD_ERR_UNIMPLEMENTED:          return "Something in FMOD hasn't been implemented when it should be! contact support! ";
			case FMOD_ERR_UNINITIALIZED:          return "This command failed because System::init or System::setDriver was not called. ";
			case FMOD_ERR_UNSUPPORTED:            return "A command issued was not supported by this object.  Possibly a plugin without certain callbacks specified. ";
			case FMOD_ERR_UPDATE:                 return "On PS2, System::update was called twice in a row when System::updateFinished must be called first. ";
			case FMOD_ERR_VERSION:                return "The version number of this file format is not supported. ";
			case FMOD_OK:                         return "No errors.";
			default:                              return "Unknown error.";
		}
	}
}
