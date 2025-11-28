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

void Sound::SetVolume(float vol) {
	m_Volume = vol;
}

void Sound::SetPitch(float pitch) {
	m_Pitch = pitch;
}

float Sound::GetVolume() const {
	return m_Volume;
}

float Sound::GetPitch() const {
	return m_Pitch;
}

bool Sound::IsPlaying() const {
	return m_Playing;
}

void Sound::Play() {
	m_Playing = true;
}

std::array<float, 2> Sound::GetNextFrame() {
	std::array<float, 2> frame = { 0, 0 };
	if (m_CurrentFrame < m_FrameCount) {
		frame = GetFrame(m_CurrentFrame);

		float left = frame[0] * m_Volume;
		float right = frame[1] * m_Volume;

		m_CurrentFrame++;
	}
	else
		m_Playing = false;

	return frame;
}
