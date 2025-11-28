#pragma once

#include "BeatEngine/Base/Asset.h"

#include <miniaudio.h>
#include <samplerate.h>
#include <vector>
#include <array>
#include <atomic>
#include <thread>
#include <cstdint>
#include <mutex>

class AudioStream : public Base::Asset {
private:
	ma_decoder m_Decoder;
	SRC_STATE* m_SrcState;
	SRC_DATA m_SrcData;

	uint64_t m_DataBufferFrameCount = 2048;
	uint64_t m_DefaultSampleRate = 0;
	uint64_t m_TargetSampleRate = 0;
	double m_SrcRatio = 1.0;
	uint64_t m_OutputFrameCount = 0;

	std::vector<float> m_ResampledBuffer[2];
	std::vector<float> m_BufferToResample;

	std::jthread m_BufferThread;
	std::mutex m_Mutex;
	std::atomic<bool> m_Fill{ false };
	std::atomic<bool> m_IsBufferReady[2]{ false ,false };

	int m_CurrentBuffer = 0;
	uint64_t m_CurrentFrame = 0;
	uint64_t m_ResampledFrameCount = 0;
	uint64_t m_BufferedFrameCount[2] = { 0, 0 };

	float m_Volume = 1.0f;
	float m_Pan = 0.0f;

	bool m_Playing = false;
	bool m_Loop = false;
	bool m_LastBufferRound = false;
private:
	void FillBuffers();
public:
	AudioStream(ma_decoder decoder, uint64_t defaultSampleRate, uint64_t targetSampleRate);
	~AudioStream() override;

	std::array<float, 2> GetNextFrame();

	void SetVolume(float vol);
	void SetLoop(bool loop);

	void Play();
	void Pause();
	void Stop();
	void Resume();

	float GetVolume() const;
	bool IsLooping() const;
	bool IsPlaying() const;
};
