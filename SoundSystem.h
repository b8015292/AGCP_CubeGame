#pragma once
#include "fmod.hpp"
#include "fmod_errors.h"
#include <assert.h>
#include <vector>


typedef FMOD::Sound* SoundClass;

class SoundSystemClass
{
public:

	// Pointer to the FMOD instance
	FMOD::System* mpSystem;

	SoundSystemClass()
	{
		InitFMOD();
	}
	~SoundSystemClass() { Shutdown(); };

	//a data structure only used by the fmod audio group and its manager
	struct ChannelData
	{
		ChannelData(FMOD::Channel* const pChannel, const unsigned int channelHandle)
			:_pChannel(pChannel), _channelHandle(channelHandle) {};
		FMOD::Channel* _pChannel;
		unsigned int _soundIdx;
		unsigned int _channelHandle;
	};

	virtual bool InitFMOD();
	virtual void Initialise();
	bool IsPlaying(const unsigned int channelHandle);
	FMOD::Channel* GetChannel(const unsigned int channelHandle) const;
	virtual void Shutdown();
	virtual void CreateSound(SoundClass* pSound, const char* pFile);
	virtual bool Play(SoundClass pSound, bool Loop, const unsigned int handle);
	virtual void ReleaseSound(SoundClass pSound);

private:

	typedef std::vector<ChannelData> Channels;
	Channels mChannels; //an array of all the channels we are using
};
