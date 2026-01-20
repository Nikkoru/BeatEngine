#include "BeatEngine/Manager/AudioManager.h"

#include "BeatEngine/Asset/AudioStream.h"
#include "BeatEngine/Enum/LogType.h"
#include "BeatEngine/Events/AudioEvent.h"
#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/AudioSignals.h"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/Logger.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <portaudio.h>
#ifdef __linux__
#include <pa_linux_pulseaudio.h>
#endif
#include <string>
#include <format>

int AudioManager::AudioCallback(
	const void* inputBuffer, 
	void* outputBuffer, 
	unsigned long framesPerBuffer, 
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags, 
	void* userData
) {
	auto out = static_cast<int16_t*>(outputBuffer);
	auto _this = static_cast<AudioManager*>(userData);
    
	memset(outputBuffer, 0, framesPerBuffer * 2 * sizeof(int16_t));
	size_t read = _this->m_SoundReadIndex.load();
    
	while (read != _this->m_SoundWriteIndex.load(std::memory_order_acquire)) {
		if (auto& sound = _this->m_PendingSounds[read]) {
			_this->m_Sounds.emplace_back(std::move(sound));
			_this->m_PendingSounds.back()->Play();

			_this->m_PendingSounds[read] = nullptr;
		}

		read = (read + 1) % MAX_PENDING_SOUNDS;
	}
	_this->m_SoundReadIndex.store(read);
	read = _this->m_StreamReadIndex.load();

	while (read != _this->m_StreamWriteIndex.load(std::memory_order_acquire)) {
		if (auto& stream = _this->m_PendingStreams[read]) {
			if (std::ranges::find(_this->m_Streams, stream) == _this->m_Streams.end()) {
				_this->m_Streams.emplace_back(stream);

				stream->Play();

				_this->m_PendingStreams[read] = nullptr;
			}
		}

		read = (read + 1) % MAX_PENDING_STREAMS;
	}
	_this->m_StreamReadIndex.store(read);

	read = _this->m_StreamRemoveReadIndex;
	while (read != _this->m_StreamRemoveWriteIndex.load(std::memory_order_acquire)) {
		if (auto& stream = _this->m_PendingRemovalStreams[read]) {
			if (auto it = std::ranges::find(_this->m_Streams, stream); it != _this->m_Streams.end()) {
				stream->Stop();
                Logger::AddInfo(typeid(AudioManager), "Stoped \"{}\"", stream->GetName());
                EventManager::GetInstance()->Send(std::make_shared<AudioStreamStopedEvent>(stream->GetName()));
				_this->m_Streams.erase(it);
				_this->m_PendingRemovalStreams[read] = nullptr;
			}
		}

		read = (read + 1) % MAX_PENDING_STREAMS;
	}
	_this->m_StreamRemoveReadIndex.store(read);

	std::vector<float> mixBuffer(framesPerBuffer * 2, 0);

	if (!_this->m_Sounds.empty()) {
		for (size_t frame = 0; frame < framesPerBuffer; frame++) {
			for (auto it = _this->m_Sounds.begin(); it != _this->m_Sounds.end();) {
				auto& sound = *it;

				if (sound->IsPlaying()) {
					auto soundFrame = sound->GetNextFrame();

					mixBuffer[frame * 2] += soundFrame[0];
					mixBuffer[frame * 2 + 1] += soundFrame[1];
					it++;
				}
				else {
                    EventManager::GetInstance()->Send(std::make_shared<SoundStopedEvent>(sound->GetName()));
					it = _this->m_Sounds.erase(it);
                } 
			}
		}
	}
	if (!_this->m_Streams.empty()) {
		for (size_t frame = 0; frame < framesPerBuffer; frame++) {
			for (auto it = _this->m_Streams.begin(); it != _this->m_Streams.end();) {
				auto& stream = *it;

				if (stream->IsPlaying()) {
					auto streamFrame = stream->GetNextFrame();

					mixBuffer[frame * 2] += streamFrame[0];
					mixBuffer[frame * 2 + 1] += streamFrame[1];
					it++;

                    stream->AddReadFrames(framesPerBuffer);

                    stream->CalcTranscurredSeconds();
				}
				if (stream->Erase()) {
                    Logger::AddInfo(typeid(AudioManager), "Stoped \"{}\"", stream->GetName());
                    EventManager::GetInstance()->Send(std::make_shared<AudioStreamStopedEvent>(stream->GetName())); 
					it = _this->m_Streams.erase(it);
                }
			}
		}
	}

	for (size_t i = 0; i < framesPerBuffer * 2; i++)
		out[i] = static_cast<int16_t>(std::clamp<float>(mixBuffer[i], -1.0, 1.0) * _this->m_SampleRate);

	return paContinue;
}
AudioManager::AudioManager() {
	Pa_Initialize();

	PaStreamParameters outputParams{};

	outputParams.channelCount = 2;
	outputParams.sampleFormat = paInt16;
	outputParams.hostApiSpecificStreamInfo = nullptr;

	std::vector<int> preferredRates = { 48000, 44100, 96000 };

	std::string deviceName;
	std::string apiName;
    

    int platformApiIndex = -1;
#ifdef _WIN32
    Logger::AddInfo(typeid(AudioManager), "Windows detected. Using WASAPI");

	for (int i = 0; i < Pa_GetHostApiCount(); ++i) {
		const PaHostApiInfo* info = Pa_GetHostApiInfo(i);
		if (info->type == paWASAPI) {
			platformApiIndex = i;
			break;
		}
	}
#elif defined(__linux__)
    Logger::AddInfo(typeid(AudioManager), "Linux detected. Using PulseAudio API");

	for (int i = 0; i < Pa_GetHostApiCount(); ++i) {
		const PaHostApiInfo *info = Pa_GetHostApiInfo(i);
		if (info->type == paPulseAudio) {
			platformApiIndex = i;
			break;
		}
	}
#endif
	apiName = Pa_GetHostApiInfo(platformApiIndex)->name;
	PaDeviceIndex deviceIndex = Pa_GetHostApiInfo(platformApiIndex)->defaultOutputDevice;

	if (deviceIndex == paNoDevice)
		Logger::AddError(typeid(AudioManager), "No Default {} device");

	const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(deviceIndex);
	outputParams.device = deviceIndex;
	outputParams.suggestedLatency = deviceInfo->defaultLowOutputLatency;

	bool supported = false;

	for (int rate : preferredRates) {
		if (Pa_IsFormatSupported(nullptr, &outputParams, rate) == paFormatIsSupported) {
			m_SampleRate = rate;
			supported = true;
			break;
		}
	}

	if (!supported) {
		std::string msg = std::format("Format given (rate=({}, {}, {}), device={}, api={}) is not supported", 
			*preferredRates.begin(), 
			*(preferredRates.begin() + 1), 
			*(preferredRates.begin() + 2),
			deviceInfo->name,
			Pa_GetHostApiInfo(platformApiIndex)->name
		);

		Logger::AddCritical(typeid(AudioManager), msg);
		THROW_RUNTIME_ERROR(msg);
	}
    Logger::AddInfo(typeid(AudioManager), "trying to get output devices");

    for (int i = 0; i <= Pa_GetHostApiInfo(platformApiIndex)->deviceCount; i++) {
        auto device = Pa_GetDeviceInfo(i);
        if (!device)
            Logger::AddWarning(typeid(AudioManager), "invalid index : {}", i);
        Logger::AddLog(LogType::DebugTarget, typeid(AudioManager), device->name);
    }

    deviceName = deviceInfo->name;

	auto err = Pa_OpenStream(&m_AudioStream, nullptr, &outputParams, m_SampleRate, 1024, paClipOff, AudioCallback, this);

	if (err != paNoError || Pa_StartStream(m_AudioStream) != paNoError) {
		std::string msg = "Failed to initialize audio stream";

		Logger::AddCritical(typeid(AudioManager), msg);
		THROW_RUNTIME_ERROR(msg);
	}

	Logger::AddInfo(typeid(AudioManager), "Audio Stream opened. Sample Rate = {} Device = {}, API = {}, Latency = {}", m_SampleRate, deviceName, apiName, outputParams.suggestedLatency);

	SignalManager::GetInstance()->RegisterCallback<PlayAudioStreamSignal>(typeid(AudioManager), [this](const std::shared_ptr<Base::Signal> sig) {
		auto audioSignal = std::static_pointer_cast<PlayAudioStreamSignal>(sig);
		if (audioSignal->AudioStreamHandle) {
			auto audioStream = audioSignal->AudioStreamHandle.Get();
			for (auto& stream : m_Streams) {
				if (stream->GetName() == audioStream->GetName())
					StopStream(stream);
			}

			PlayStream(audioStream);
		}
		else {
			Logger::AddError(typeid(AudioManager), "Tried to play an empty AudioStream");
		}

	});
	SignalManager::GetInstance()->RegisterCallback<StopAudioStreamSignal>(typeid(AudioManager), [this](const std::shared_ptr<Base::Signal> sig) {
		auto audioSignal = std::static_pointer_cast<StopAudioStreamSignal>(sig);
		auto nameStream = audioSignal->AudioStreamName;

		StopStream(nameStream);
	});
	SignalManager::GetInstance()->RegisterCallback<PauseAudioStreamSignal>(typeid(AudioManager), [this](const std::shared_ptr<Base::Signal> sig) {
		auto audioSignal = std::static_pointer_cast<PauseAudioStreamSignal>(sig);
		auto& nameStream = audioSignal->AudioStreamName;

		for (auto& stream : m_Streams) {
			if (stream->GetName() == nameStream) {
				if (stream->IsPlaying())
					stream->Pause();
				else
					stream->Resume();

				return;
			}
		}

		Logger::AddError(typeid(AudioManager), "AudioStream \"{}\" was not found; did you add it to the queue?", nameStream);
	});
	SignalManager::GetInstance()->RegisterCallback<PlaySoundSignal>(typeid(AudioManager), [this](const std::shared_ptr<Base::Signal> sig) {
		auto audioSignal = std::static_pointer_cast<PlaySoundSignal>(sig);

		PlaySound(audioSignal->SoundToPlay);
	});
}

AudioManager::~AudioManager() {
	Pa_StopStream(m_AudioStream);
	Pa_CloseStream(m_AudioStream);
	Pa_Terminate();
}

void AudioManager::PlaySound(std::shared_ptr<Sound> sound) {
    EventManager::GetInstance()->Send(std::make_shared<SoundStartedEvent>(sound->GetName()));

	size_t write = m_SoundWriteIndex.load();
	size_t nextWrite = (write + 1) % MAX_PENDING_SOUNDS;

	if (nextWrite != m_SoundReadIndex.load()) {
		m_PendingSounds[write] = std::move(sound);

		m_SoundWriteIndex.store(nextWrite, std::memory_order_release);
	}
}

void AudioManager::PlayStream(std::shared_ptr<AudioStream> stream) {
	Logger::AddInfo(typeid(AudioManager), "Adding \"{}\"onto the queue", stream->GetName());
    stream->ResetSeconds();

    EventManager::GetInstance()->Send(std::make_shared<AudioStreamStartedEvent>(stream->GetName()));

	size_t write = m_StreamWriteIndex.load();
	size_t nextWrite = (write + 1) % MAX_PENDING_STREAMS;

	if (nextWrite != m_StreamReadIndex.load()) {
		m_PendingStreams[write] = std::move(stream);

		m_StreamWriteIndex.store(nextWrite, std::memory_order_release);
	}
}

void AudioManager::StopStream(std::shared_ptr<AudioStream> stream) {
    std::string streamName = stream->GetName();

	size_t write = m_StreamRemoveWriteIndex.load();
	size_t nextWrite = (write + 1) % MAX_PENDING_STREAMS;

	if (nextWrite != m_StreamRemoveReadIndex.load()) {
		m_PendingRemovalStreams[write] = std::move(stream);

		m_StreamRemoveWriteIndex.store(nextWrite, std::memory_order_release);
	}
}

void AudioManager::StopStream(std::string streamName) {
	for (const auto& stream : m_Streams) {
		if (stream->GetName() == streamName) {
			StopStream(stream);
			return;
		}
	}
}

bool AudioManager::IsStreamPlaying(std::shared_ptr<AudioStream> stream) {
    auto streamName = stream->GetName();
    return IsStreamPlaying(streamName);
}

bool AudioManager::IsStreamPlaying(std::string streamName) {
    for (const auto& stream : m_Streams) {
        if (stream->GetName() == streamName)
            return true;
    }

    return false;
}

bool AudioManager::AllSoundsDone() const {
	return m_Sounds.empty();
}

void AudioManager::DrawImGuiDebug() {

}
