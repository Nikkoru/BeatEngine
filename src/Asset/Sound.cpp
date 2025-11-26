#include "BeatEngine/Asset/Sound.h"

Sound::Sound(std::vector<float> data, uint64_t frameCount, uint64_t sampleRate) : m_Data(data), m_FrameCount(frameCount), m_SampleRate(sampleRate) {

}

uint64_t Sound::GetFrameCount() const {
	return m_FrameCount;
}

uint64_t Sound::GetSampleRate() const {
	return m_SampleRate;
}

std::array<float, 2> Sound::GetFrame(uint64_t frame) const {
	std::array<float, 2> dataFrame = { 0, 0 };
	
	if (frame < m_FrameCount) {
		dataFrame[0] = m_Data[frame * 2];
		dataFrame[1] = m_Data[frame * 2 + 1];
	}

	return dataFrame;
}
