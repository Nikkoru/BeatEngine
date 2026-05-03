#pragma once

#include "BeatEngine/Asset/Sound.h"
#include "BeatEngine/Asset/AudioStream.h"

#include <portaudio.h>
#include <atomic>
#include <array>
#include <vector>
#include <cstdint>

class GameContext;
class GameState;
class AudioManager {
private:
	PaStream* m_AudioStream{ nullptr };
    PaStreamParameters m_StreamParameters{};
	uint64_t m_SampleRate{ 48000 };

	std::vector<std::shared_ptr<Sound>> m_Sounds;
    std::map<std::string, std::array<float, 1024>> m_SoundFrames;
	std::vector<std::shared_ptr<AudioStream>> m_Streams;

	static constexpr int MAX_PENDING_SOUNDS{ 64 };
	std::array<std::shared_ptr<Sound>, MAX_PENDING_SOUNDS> m_PendingSounds;
	std::atomic<int> m_SoundReadIndex{};
	std::atomic<int> m_SoundWriteIndex{};

	static constexpr int MAX_PENDING_STREAMS{ 8 };
	std::array<std::shared_ptr<AudioStream>, MAX_PENDING_STREAMS> m_PendingStreams;
	std::atomic<int> m_StreamReadIndex{};
	std::atomic<int> m_StreamWriteIndex{};

	std::array<std::shared_ptr<AudioStream>, MAX_PENDING_STREAMS> m_PendingRemovalStreams;
	std::atomic<int> m_StreamRemoveReadIndex{};
	std::atomic<int> m_StreamRemoveWriteIndex{};

    bool m_PlayerWindow{ false };
    bool m_CalculateRealSize{ true };
private:
    GameContext* m_Context{ nullptr };
    GameState* m_State{ nullptr };
public:
	static int SoundCallback(
		const void* inputBuffer, 
		void* outputBuffer, 
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo, 
		PaStreamCallbackFlags statusFlags,
		void* userData
	);
	static int AudioStreamCallback(
		const void* inputBuffer, 
		void* outputBuffer, 
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo, 
		PaStreamCallbackFlags statusFlags,
		void* userData
	);
public:
    AudioManager() : AudioManager(nullptr, nullptr) {}
	AudioManager(GameContext* context, GameState* state);
	~AudioManager();

    AudioManager& operator=(const AudioManager& other) {
        if (this != &other) {
            m_SoundReadIndex.store(other.m_SoundReadIndex.load());
            m_SoundWriteIndex.store(other.m_SoundWriteIndex.load());

            m_StreamReadIndex.store(other.m_StreamReadIndex.load());
            m_StreamWriteIndex.store(other.m_StreamWriteIndex.load());

            m_StreamRemoveReadIndex.store(other.m_StreamRemoveReadIndex.load());
            m_StreamRemoveWriteIndex.store(other.m_StreamRemoveWriteIndex.load());

            m_Context = other.m_Context;
            m_State = other.m_State;

            m_Streams = other.m_Streams;
            m_PendingStreams = other.m_PendingStreams;
            m_PendingRemovalStreams = other.m_PendingRemovalStreams;

            m_Sounds = other.m_Sounds;
            m_PendingSounds = other.m_PendingSounds;
        }
        return *this;
    }
public:
    void Init();
    void Uninit();

	void PlaySound(std::shared_ptr<Sound> sound);
	void PlayStream(std::shared_ptr<AudioStream> stream);
	void StopStream(std::shared_ptr<AudioStream> stream);
	void StopStream(std::string streamName);

    bool IsStreamPlaying(std::shared_ptr<AudioStream> stream);
    bool IsStreamPlaying(std::string streamName);

    bool IsSoundPlaying(std::shared_ptr<Sound> stream);
    bool IsSoundPlaying(std::string sound);

	bool AllSoundsDone() const;

    void ShowImGuiDebugWindow();
};
