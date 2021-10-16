#pragma once
#include <xaudio2.h>
#include <string>
#include <vector>
#include <map>

//Base class for audio data
struct AudioBase : IXAudio2VoiceCallback
{
	WAVEFORMATEXTENSIBLE waveFormat = {};
	XAUDIO2_BUFFER buffer = {};
	std::string audioFilename;

	std::vector<IXAudio2SourceVoice*> sourceVoicesPlaying;

	bool isPlaying = false;

	AudioBase(std::string filename);
	~AudioBase();

	// Inherited via IXAudio2VoiceCallback
	virtual void OnVoiceProcessingPassStart(UINT32 BytesRequired) override;
	virtual void OnVoiceProcessingPassEnd(void) override;
	virtual void OnStreamEnd(void) override;
	virtual void OnBufferStart(void* pBufferContext) override;
	virtual void OnBufferEnd(void* pBufferContext) override;
	virtual void OnLoopEnd(void* pBufferContext) override;
	virtual void OnVoiceError(void* pBufferContext, HRESULT Error) override;
};
