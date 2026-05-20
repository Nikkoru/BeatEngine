#include "BeatEngine/Asset/AudioStream.h"

#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/Logger.h"

#include "BeatEngine/Util/Math.h"
#include <cmath>
#include <cstdint>
#include <extras/nodes/ma_vocoder_node/ma_vocoder_node.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <thread>
#include <vector>

void AudioStream::FillBuffers() {
    using namespace std::chrono_literals;

    while (m_Fill.load()) {
        int bufferToFill = -1;

        for (int i = 0; i < 2; i++) {
            if (!m_IsBufferReady[i].load()) {
                bufferToFill = i;
                break;
            }
        }

        if (bufferToFill == -1) {
            m_Fill.store(false);
            break;
        }


        memset(m_ResampledBuffer[bufferToFill].data(), 0, m_ResampledBuffer[bufferToFill].size() * sizeof(float));

        ma_uint64 framesCount = 0;
        ma_decoder_read_pcm_frames(&m_Decoder, m_BufferToResample.data(), m_DataBufferFrameCount, &framesCount);

        if (framesCount < m_DataBufferFrameCount) {
            if (m_Loop) {
                m_CurrentBuffer = 0;
                ma_decoder_seek_to_pcm_frame(&m_Decoder, 0);

                ResetReadFrames();
                ResetSeconds();
            }
            else {
                memset(m_BufferToResample.data() + framesCount * 2, 0, ((m_DataBufferFrameCount - framesCount) * 2) * sizeof(int16_t));

                m_LastBufferRound = true;
            }
        }

        m_SrcData.data_in = m_BufferToResample.data();
        m_SrcData.data_out = m_ResampledBuffer[bufferToFill].data();
        m_SrcData.input_frames = framesCount;
        m_SrcData.output_frames = m_OutputFrameCount;
        m_SrcData.src_ratio = m_SrcRatio;
        m_SrcData.end_of_input = m_LastBufferRound ? 1 : 0;

int err = src_process(m_SrcState, &m_SrcData);
        if (err) {
            std::string msg = "Sample rate conversion failed: " + std::string(src_strerror(err));
            Logger::AddCritical(typeid(AudioStream), msg);
            THROW_RUNTIME_ERROR(msg);
        }
        else {
            m_BufferedFrameCount[bufferToFill] = m_SrcData.output_frames_gen;

            if (m_SrcData.output_frames_gen == 0) {
                if (m_LastBufferRound) {
                    m_Playing = false;
                    m_Erase = true;
                    m_LastBufferRound = false;

                    break;
                }
                else 
                    Play();
            }

            m_IsBufferReady[bufferToFill].store(true);
        }
    }
}

void AudioStream::AsyncFillBuffers() {
    if (!m_Fill && m_BufferThread.joinable()) {
        m_BufferThread.join();
        m_Fill.store(true);

        m_BufferThread = std::jthread(&AudioStream::FillBuffers, this);
    }
}

AudioStream::AudioStream(std::string name, ma_decoder decoder, uint64_t defaultSampleRate, uint64_t targetSampleRate, TagLib::FileRef fileRef, float totalSeconds, uint64_t totalFrames)
    : m_Name(name), m_MetadataReference(fileRef), m_Decoder(decoder), m_DefaultSampleRate(defaultSampleRate), m_TargetSampleRate(targetSampleRate), m_TotalSeconds(totalSeconds), m_TotalFrames(totalFrames) {

    m_SrcRatio = static_cast<double>(m_TargetSampleRate)  / static_cast<double>(m_DefaultSampleRate);
    m_OutputFrameCount = static_cast<uint64_t>(m_SrcRatio * m_DataBufferFrameCount);

    int err = 0;

    m_SrcState = src_new(SRC_SINC_BEST_QUALITY, 2, &err);

    if (!m_SrcState && err > 0) {
        std::string msg = "Failed to initialized libsamplerate converter" + std::string(src_strerror(err));
        Logger::AddCritical(typeid(AudioStream), msg);
        THROW_RUNTIME_ERROR(msg);
    }

    m_Metadata.Title = m_MetadataReference.tag()->title();
    m_Metadata.Artist = m_MetadataReference.tag()->artist();
    m_Metadata.TrackNum = m_MetadataReference.tag()->track();
    m_Metadata.Year = m_MetadataReference.tag()->year();

    m_ResampledBuffer[0].resize(m_OutputFrameCount * 2);
    m_ResampledBuffer[1].resize(m_OutputFrameCount * 2);
    m_BufferToResample.resize(m_DataBufferFrameCount * 2);

    m_IsBufferReady[0].store(true);
    m_IsBufferReady[1].store(true);

    m_CurrentBuffer = 0;
    m_CurrentFrame = 0;
    m_ResampledFrameCount = 0;
}

AudioStream::~AudioStream() {
    Stop();

    if (m_SrcState)
        src_delete(m_SrcState);
}

std::array<float, 2> AudioStream::GetNextFrame() {
    std::array<float, 2> frame = { 0, 0 };

    if (!m_Playing)
        return frame;
    if (m_CurrentFrame >= m_ResampledFrameCount) {
        int nextBuffer = 1 - m_CurrentBuffer;

        if (m_IsBufferReady[nextBuffer].load()) {
            m_IsBufferReady[m_CurrentBuffer].store(false);

            m_CurrentBuffer = nextBuffer;
            m_CurrentFrame = 0;

            m_ResampledFrameCount = m_BufferedFrameCount[m_CurrentBuffer];

            if (m_ResampledFrameCount == 0)
                return frame;

            AsyncFillBuffers();
        }
        else
            return frame;
    }

    float angle = m_Pan * (PI / 2.f);
    float leftPan = std::cos(angle);
    float rightPan = std::sin(angle);

    float left = m_ResampledBuffer[m_CurrentBuffer][m_CurrentFrame * 2] * leftPan * m_Volume;
    float right = m_ResampledBuffer[m_CurrentBuffer][m_CurrentFrame * 2 + 1] * rightPan * m_Volume;

    frame[0] = left;
    frame[1] = right;

    m_CurrentFrame++;
    m_TotalReadFrames++;


    return frame;
}

void AudioStream::SetVolume(float vol) {
    m_Volume = vol;
}

void AudioStream::SetLoop(bool loop) {
    m_Loop = loop;
}

void AudioStream::SetDataBufferFrameCount(uint64_t bufferSize) {
    m_DataBufferFrameCount = bufferSize;
}


void AudioStream::Play() {
    // NOTE: for some reason in XOrg the stream playback saturates the bass and also makes the pan value useless

    m_LastBufferRound = false;

    if (!m_Fill.load()) {
        m_Fill.store(true);
        m_BufferThread = std::jthread(&AudioStream::FillBuffers, this);
    }

    m_Playing = true;
    m_Erase = false;

    Logger::AddDebug("", "Stream \"{}\" playing!", m_Name);
}

void AudioStream::Pause() {
    m_Fill.store(false);
    m_Playing = false;
    Logger::AddDebug("", "Stream \"{}\" paused!", m_Name);
}

void AudioStream::Stop() {
    m_Fill.store(false);
    if (m_BufferThread.joinable())
        m_BufferThread.join();

    m_Playing = false;
    m_LastBufferRound = false;
    m_CurrentFrame = 0;
    m_CurrentBuffer = 0;

    m_IsBufferReady[0].store(false);
    m_IsBufferReady[1].store(false);

    ma_decoder_seek_to_pcm_frame(&m_Decoder, 0);

    if (m_SrcState)
        src_reset(m_SrcState);

    ResetReadFrames();
    ResetSeconds();

    Logger::AddDebug("", "Stream \"{}\" stoped!", m_Name);
}

void AudioStream::Resume() {
    m_Fill.store(false);
    m_Playing = true;
    Logger::AddDebug("", "Stream \"{}\" resumed!", m_Name);
}

void AudioStream::AddSeconds(float seconds) {
    if (m_TranscurredSeconds <= m_TotalSeconds)
        m_TranscurredSeconds += seconds;

    if (m_TranscurredSeconds > m_TotalSeconds)
        m_TranscurredSeconds = m_TotalSeconds;
}

void AudioStream::CalcTranscurredSeconds() {
    m_TranscurredSeconds = (static_cast<float>(m_TotalReadFrames) / static_cast<float>(m_TargetSampleRate));
}

void AudioStream::ResetSeconds() {
    m_TranscurredSeconds = 0;
}

void AudioStream::AddReadFrames(uint64_t frames) {
    this->m_TotalReadFrames += frames;
}

void AudioStream::ResetReadFrames() {
    this->m_TotalReadFrames = 0;
}

std::string AudioStream::GetName() const {
    return m_Name;
}

float AudioStream::GetVolume() const {
    return m_Volume;
}

bool AudioStream::IsLooping() const {
    return m_Loop;
}

bool AudioStream::IsPlaying() const {
    return m_Playing;
}

AudioStreamMetadata AudioStream::GetMetadata() {
    return m_Metadata;
}

TagLib::FileRef AudioStream::GetFileReference() {
    return m_MetadataReference;
}

const std::vector<float> AudioStream::GetResampledBufferL() {
    return m_ResampledBuffer[0];
}

int AudioStream::GetCurrentFrameOffset() {
    return m_CurrentFrame;
}

const std::vector<float> AudioStream::GetResampledBufferR() {
    return m_ResampledBuffer[1];
}

const std::vector<float>& AudioStream::GetAllFrames() {
    std::vector<float> rawFrames;

    if (m_TotalFrameData.empty()) {
        ma_uint64 totalFrames;
        ma_uint64 framesRead;

        m_Playing = false;
        ma_decoder_seek_to_pcm_frame(&m_Decoder, 0);
        ma_decoder_get_length_in_pcm_frames(&m_Decoder, &totalFrames);
        Logger::AddDebug("", "totalFrames {}", totalFrames);
        rawFrames.resize(totalFrames * 2);
        ma_decoder_read_pcm_frames(&m_Decoder, rawFrames.data(), totalFrames, &framesRead);
        Logger::AddDebug("", "totalReadFrames {}", framesRead);
        ma_decoder_seek_to_pcm_frame(&m_Decoder, m_TotalReadFrames);
        rawFrames.shrink_to_fit();
        m_TotalFrameData.resize(rawFrames.size());

        if (rawFrames.data() == nullptr)
            THROW_RUNTIME_ERROR("rawFrames is nullptr");

        if (m_TotalFrameData.data() == nullptr)
            THROW_RUNTIME_ERROR("m_TotalFrameData is nullptr");

        auto srcCopy = m_SrcData;

        m_SrcData.data_in = rawFrames.data();
        m_SrcData.data_out = m_TotalFrameData.data();
        m_SrcData.input_frames = totalFrames;
        m_SrcData.output_frames = 0;
        m_SrcData.src_ratio = m_SrcRatio;
        m_SrcData.end_of_input = 0;

        int err = src_process(m_SrcState, &m_SrcData);
        if (err) {
            std::string msg = "Sample rate conversion failed: " + std::string(src_strerror(err));
            Logger::AddCritical(typeid(AudioStream), msg);
            THROW_RUNTIME_ERROR(msg);
        }

        m_SrcData = srcCopy;
        m_Playing = true;
    }

    return m_TotalFrameData;
}

void AudioStream::FreeTotalFrameData() {
    m_TotalFrameData.clear();
}

float AudioStream::GetTotalSeconds() {
    return m_TotalSeconds;
}

float AudioStream::GetTranscurredSeconds() {
    return m_TranscurredSeconds;
}

uint64_t AudioStream::GetDataBufferFrameCount() const {
    return m_DataBufferFrameCount;
}

bool AudioStream::Erase() const {
    return m_Erase;
}
