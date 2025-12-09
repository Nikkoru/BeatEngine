#include "BeatEngine/Manager/AudioManager.h"

#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/AudioSignals.h"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/Logger.h"

#include <algorithm>
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
				_this->m_Streams.erase(it);
				stream->Stop();
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
				else 
					it = _this->m_Sounds.erase(it);
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
				}
				else {
					if (stream->Erase())
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

#ifdef _WIN32
	int wasapiApiIndex = -1;

	for (int i = 0; i < Pa_GetHostApiCount(); ++i) {
		const PaHostApiInfo* info = Pa_GetHostApiInfo(i);
		if (info->type == paWASAPI) {
			wasapiApiIndex = i;
			break;
		}
	}

	PaDeviceIndex deviceIndex = Pa_GetHostApiInfo(wasapiApiIndex)->defaultOutputDevice;

	if (deviceIndex == paNoDevice)
		Logger::GetInstance()->AddError("Default output device via WASAPI was not found", typeid(AudioManager));

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
			Pa_GetHostApiInfo(wasapiApiIndex)->name
		);

		Logger::GetInstance()->AddCritical(msg, typeid(AudioManager));
		THROW_RUNTIME_ERROR(msg);
	}

	deviceName = deviceInfo->name;
	apiName = Pa_GetHostApiInfo(wasapiApiIndex)->name;
#elif defined(__linux__)
	int pulseApiIndex = -1;
	for (int i = 0; i < Pa_GetHostApiCount(); ++i) {
		const PaHostApiInfo *info = Pa_GetHostApiInfo(i);
		if (info->type == paPulseAudio) {
			pulseApiIndex = i;
			break;
		}
	}

	PaDeviceIndex deviceIndex = Pa_GetHostApiInfo(pulseApiIndex)->defaultOutputDevice;

	if (deviceIndex == paNoDevice)
		THROW_RUNTIME_ERROR("No default PulseAudio output device.");

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

	if (!supported)
		THROW_RUNTIME_ERROR("Neither 48000 Hz nor 44100 Hz supported with PulseAudio and paInt16.");

#endif

	auto err = Pa_OpenStream(& m_AudioStream, nullptr, & outputParams, m_SampleRate, 64, paClipOff, AudioCallback, this);

	if (err != paNoError || Pa_StartStream(m_AudioStream) != paNoError) {
		std::string msg = "Failed to initialize audio stream";

		Logger::GetInstance()->AddCritical(msg, typeid(AudioManager));
		THROW_RUNTIME_ERROR(msg);
	}

	Logger::GetInstance()->AddInfo(std::format("Audio Stream opened. Sample Rate = {} Device = {}, API = {}", m_SampleRate, deviceName, apiName), typeid(AudioManager));

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
			Logger::GetInstance()->AddError("Tried to play an empty AudioStream", typeid(AudioManager));
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

		Logger::GetInstance()->AddError("AudioStream \"" + nameStream + "\" was not found; did you add it to the queue", typeid(AudioManager));
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
	size_t write = m_SoundWriteIndex.load();
	size_t nextWrite = (write + 1) % MAX_PENDING_SOUNDS;

	if (nextWrite != m_SoundReadIndex.load()) {
		m_PendingSounds[write] = std::move(sound);

		m_SoundWriteIndex.store(nextWrite, std::memory_order_release);
	}
}

void AudioManager::PlayStream(std::shared_ptr<AudioStream> stream) {
	Logger::GetInstance()->AddInfo("Adding \"" + stream->GetName() + "\"onto the queue", typeid(AudioManager));

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

	Logger::GetInstance()->AddInfo("Stoped \"" + streamName + "\"", typeid(AudioManager));
}

void AudioManager::StopStream(std::string streamName) {
	for (auto& stream : m_Streams) {
		if (stream->GetName() == streamName) {
			StopStream(stream);
			return;
		}
	}
}

bool AudioManager::AllSoundsDone() const {
	return m_Sounds.empty();
}
