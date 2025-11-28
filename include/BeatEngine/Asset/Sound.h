#pragma once

#include "BeatEngine/Base/Asset.h"

#include <vector>
#include <cstdint>
#include <array>

class AudioManager;
class Sound : public Base::Asset {
private:
	std::vector<float> m_Data;
	uint64_t m_FrameCount = 0;
	uint64_t m_SampleRate = 0;

	uint64_t m_CurrentFrame = 0;

	float m_Volume = 1.0f;
	float m_Pitch = 1.0f;

	bool m_Playing = false;

public:
	Sound(std::vector<float> data, uint64_t frameCount, uint64_t sampleRate);
	uint64_t GetFrameCount() const;
	uint64_t GetSampleRate() const;
	std::array<float, 2> GetFrame(uint64_t frame) const;

	void SetVolume(float vol);
	void SetPitch(float pitch);

	float GetVolume() const;
	float GetPitch() const;
	bool IsPlaying() const;
private:
	friend class AudioManager;
	void Play();
	std::array<float, 2> GetNextFrame();
};