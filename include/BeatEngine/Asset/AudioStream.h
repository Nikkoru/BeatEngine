#pragma once

#include "BeatEngine/Base/Asset.h"

#include <miniaudio.h>
#include <taglib/fileref.h>
#include <samplerate.h>
#include <taglib/tag.h>
#include <vector>
#include <string>
#include <array>
#include <atomic>
#include <thread>
#include <cstdint>
#include <mutex>

struct AudioStreamMetadata {
private:
    friend class AudioStream;
public:
    TagLib::String Artist = "";
    TagLib::String Title = "";
    unsigned int TrackNum = 0;
    unsigned int Year = 0;
public:
    AudioStreamMetadata() = default;
};

class AudioStream : public Base::Asset {
private:
    friend class AudioManager;

	std::string m_Name = "";

    TagLib::FileRef m_MetadataReference;
    AudioStreamMetadata m_Metadata;

	ma_decoder m_Decoder;
	SRC_STATE* m_SrcState;
	SRC_DATA m_SrcData;

	uint64_t m_DataBufferFrameCount = 2048;
	uint64_t m_DefaultSampleRate = 0;
	uint64_t m_TargetSampleRate = 0;
	double m_SrcRatio = 1.0;
	uint64_t m_OutputFrameCount = 0;

	std::vector<float> m_ResampledBuffer[2];
    std::vector<float> m_TotalFrameData{};
	std::vector<float> m_BufferToResample;

	std::jthread m_BufferThread;
	std::mutex m_Mutex;
	std::atomic<bool> m_Fill{ false };
	std::atomic<bool> m_IsBufferReady[2]{ false ,false };

	int m_CurrentBuffer = 0;

	float m_TotalSeconds = -1;
    float m_TranscurredSeconds = 0;

    uint64_t m_TotalFrames = 0;
    uint64_t m_TotalReadFrames = 0;

	uint64_t m_CurrentFrame = 0;
	uint64_t m_ResampledFrameCount = 0;
	uint64_t m_BufferedFrameCount[2] = { 0, 0 };

	float m_Volume = 1.0f;
	float m_Pan = 0.5f;

	bool m_Playing = false;
	bool m_Erase = false;
	bool m_Loop = false;
	bool m_LastBufferRound = false;
private:
	void FillBuffers();
    void AsyncFillBuffers();
public:
	AudioStream(std::string name, ma_decoder decoder, uint64_t defaultSampleRate, uint64_t targetSampleRate, TagLib::FileRef fileRef, float totalSeconds = -1, uint64_t totalFrames = -1);
	AudioStream(AudioStream& other) : 
        m_Name(other.m_Name), 
        m_MetadataReference(other.m_MetadataReference), 
        m_Metadata(other.m_Metadata), 
        m_Decoder(other.m_Decoder), 
        m_SrcState(other.m_SrcState), 
        m_SrcData(other.m_SrcData),
        m_TargetSampleRate(other.m_TargetSampleRate),
        m_TotalSeconds(other.m_TotalSeconds),
        m_TotalFrames(other.m_TotalFrames)
    {}
	~AudioStream() override;

	std::array<float, 2> GetNextFrame();

	void SetVolume(float vol);
	void SetLoop(bool loop);

    void SetDataBufferFrameCount(uint64_t bufferSize);

	void Play();
	void Pause();
	void Stop();
	void Resume();

    void AddSeconds(float seconds);
    void CalcTranscurredSeconds();
    void ResetSeconds();

    void AddReadFrames(uint64_t frames);
    void ResetReadFrames();

	std::string GetName() const;
	float GetVolume() const;
	bool IsLooping() const;
	bool IsPlaying() const;

    AudioStreamMetadata GetMetadata();
    TagLib::FileRef GetFileReference();

    const std::vector<float> GetResampledBufferL();
    int GetCurrentFrameOffset();
    const std::vector<float> GetResampledBufferR();

    const std::vector<float>& GetAllFrames();
    void FreeTotalFrameData();

    float GetTotalSeconds();
    float GetTranscurredSeconds();

    uint64_t GetDataBufferFrameCount() const;

	bool Erase() const;

    void ShowImGuiDetails(bool* open) override;
};
