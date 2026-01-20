#pragma once

#include "BeatEngine/Asset/Sound.h"
#include "BeatEngine/Asset/AudioStream.h"

#include <portaudio.h>
#include <atomic>
#include <array>
#include <vector>
#include <cstdint>


class AudioManager {
private:
	PaStream* m_AudioStream;
	uint64_t m_SampleRate = 48000;

	std::vector<std::shared_ptr<Sound>> m_Sounds;
	std::vector<std::shared_ptr<AudioStream>> m_Streams;

	static constexpr int MAX_PENDING_SOUNDS = 64;
	std::array<std::shared_ptr<Sound>, MAX_PENDING_SOUNDS> m_PendingSounds;
	std::atomic<int> m_SoundReadIndex;
	std::atomic<int> m_SoundWriteIndex;

	static constexpr int MAX_PENDING_STREAMS = 8;
	std::array<std::shared_ptr<AudioStream>, MAX_PENDING_STREAMS> m_PendingStreams;
	std::atomic<int> m_StreamReadIndex;
	std::atomic<int> m_StreamWriteIndex;

	std::array<std::shared_ptr<AudioStream>, MAX_PENDING_STREAMS> m_PendingRemovalStreams;
	std::atomic<int> m_StreamRemoveReadIndex;
	std::atomic<int> m_StreamRemoveWriteIndex;

public:
	static int AudioCallback(
		const void* inputBuffer, 
		void* outputBuffer, 
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo, 
		PaStreamCallbackFlags statusFlags,
		void* userData
	);
public:
	AudioManager();
	~AudioManager();
public:
	void PlaySound(std::shared_ptr<Sound> sound);
	void PlayStream(std::shared_ptr<AudioStream> stream);
	void StopStream(std::shared_ptr<AudioStream> stream);
	void StopStream(std::string streamName);

    bool IsStreamPlaying(std::shared_ptr<AudioStream> stream);
    bool IsStreamPlaying(std::string streamName);

    bool IsSoundPlaying(std::shared_ptr<Sound> stream);
    bool IsSoundPlaying(std::string sound);

	bool AllSoundsDone() const;

    void DrawImGuiDebug();
};
