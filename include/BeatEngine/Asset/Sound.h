#pragma once

#include "BeatEngine/Base/Asset.h"

#include <vector>
#include <cstdint>
#include <array>

class Sound : public Base::Asset {
private:
	std::vector<float> m_Data;
	uint64_t m_FrameCount = 0;
	uint64_t m_SampleRate = 0;
public:
	Sound(std::vector<float> data, uint64_t frameCount, uint64_t sampleRate);
	uint64_t GetFrameCount() const;
	uint64_t GetSampleRate() const;
	std::array<float, 2> GetFrame(uint64_t frame) const;
};