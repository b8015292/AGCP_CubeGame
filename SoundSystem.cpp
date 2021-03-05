#include "SoundSystem.h"

//Init comes from FMOD manual
bool SoundSystemClass::InitFMOD()
{
	FMOD_RESULT      result;
	unsigned int     version;
	FMOD_SPEAKERMODE speakermode;
	FMOD_CAPS        caps;

	/*
	Create a System object and initialize.
	*/
	if (FMOD::System_Create(&mpSystem) != FMOD_OK)
	{
		mpSystem = NULL;
		return false;
	}

	int driverCount = 0;
	mpSystem->getNumDrivers(&driverCount);

	if (driverCount == 0)
	{
		return false;
	}

	if (mpSystem->getVersion(&version) != FMOD_OK)
		goto shutdown_error;

	if (version < FMOD_VERSION)
	{
		assert(false);	//, "Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
		goto shutdown_error;
	}

	if (mpSystem->getDriverCaps(0, &caps, 0, 0, &speakermode) != FMOD_OK)
		goto shutdown_error;

	if (mpSystem->setSpeakerMode(speakermode) != FMOD_OK)       /* Set the user selected speaker mode. */
		goto shutdown_error;

	if (caps & FMOD_CAPS_HARDWARE_EMULATED)             /* The user has the 'Acceleration' slider set to off!  This is really bad for latency!. */
	{                                                   /* You might want to warn the user about this. */
		if (mpSystem->setDSPBufferSize(1024, 10) != FMOD_OK)    /* At 48khz, the latency between issuing an fmod command and hearing it will now be about 213ms. */
			goto shutdown_error;
	}

	// Initialize our Instance with  Channels
	result = mpSystem->init(100, FMOD_INIT_NORMAL, 0);
	if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         /* Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo... */
	{
		if (mpSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO) != FMOD_OK)
			goto shutdown_error;

		if (mpSystem->init(100, FMOD_INIT_NORMAL, 0) != FMOD_OK) /* Replace with whatever channel count and flags you use! */
			goto shutdown_error;
	}

	mpSystem->setHardwareChannels(0, 32, 0, 32);

	return true;

shutdown_error:
	Shutdown();
	return false;
}

void SoundSystemClass::Shutdown()
{
	//shut down fmod
	if (!mpSystem)
		return;
	mpSystem->release();
	mpSystem = NULL;
}

void SoundSystemClass::Initialise()
{
	mChannels.reserve(100);
}

void SoundSystemClass::CreateSound(SoundClass* pSound, const char* pFile)
{
	mpSystem->createSound(pFile, FMOD_HARDWARE, 0, pSound);
}

FMOD::Channel* SoundSystemClass::GetChannel(const unsigned int channelHandle) const
{
	for (size_t i = 0; i < mChannels.size(); ++i)
	{
		if (channelHandle == mChannels[i]._channelHandle)
			return mChannels[i]._pChannel;
	}
	return NULL;
}

//is a sound currently playing back
bool SoundSystemClass::IsPlaying(const unsigned int channelHandle)
{
	FMOD::Channel* pCh = GetChannel(channelHandle);
	bool playing;
	pCh->isPlaying(&playing);
	return playing;
}

bool SoundSystemClass::Play(SoundClass pSound, bool Loop, const unsigned int handle)
{
	bool inChannel = false;
	//play sound if its in a channel
	for (int i(0); i < mChannels.size(); ++i)
	{
		
		if(mChannels.at(i)._channelHandle == handle)
		{
			inChannel = true;
			FMOD::Channel* pCh = mChannels.at(i)._pChannel;
			bool playing = IsPlaying(handle);
			if (playing == false)
			{
				if (!Loop)
					pSound->setMode(FMOD_LOOP_OFF);
				else
				{
					pSound->setMode(FMOD_LOOP_NORMAL);
					pSound->setLoopCount(-1);
				}
				//specify FMOD_CHANNEL_REUSE to re-use the same channel multiple times
				if (mpSystem->playSound(FMOD_CHANNEL_REUSE, pSound, false, &pCh) != FMOD_OK)
					return false;
				mChannels.at(i) = (ChannelData(pCh, handle));
			}
			break;
		}
	}

	//if sound is not already in a channel put it in one and play sound
	if (!inChannel) 
	{
		//keep track of the new sound playing
		FMOD::Channel* pCh;
		if (!Loop)
			pSound->setMode(FMOD_LOOP_OFF);
		else
		{
			pSound->setMode(FMOD_LOOP_NORMAL);
			pSound->setLoopCount(-1);
		}
		if (mpSystem->playSound(FMOD_CHANNEL_FREE, pSound, false, &pCh) != FMOD_OK)
			return false;
		mChannels.push_back(ChannelData(pCh, handle));
	}
}


void SoundSystemClass::ReleaseSound(SoundClass pSound)
{
	pSound->release();
}
