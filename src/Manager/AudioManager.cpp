#include "BeatEngine/Manager/AudioManager.h"

#include "BeatEngine/Asset/AudioStream.h"
#include "BeatEngine/Enum/LogType.h"
#include "BeatEngine/Events/AudioEvent.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/GameState.h"
#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/AudioSignals.h"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/Logger.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <portaudio.h>
#ifdef __linux__
#include <pa_linux_pulseaudio.h>
#endif
#include <string>
#include <format>

int AudioManager::SoundCallback(
	const void* inputBuffer, 
	void* outputBuffer, 
	unsigned long framesPerBuffer, 
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags, 
	void* userData
) {
    (void)inputBuffer;
    (void)timeInfo;
    (void)statusFlags;

	auto out = static_cast<int16_t*>(outputBuffer);
	auto _this = static_cast<AudioManager*>(userData);
    
	memset(outputBuffer, 0, framesPerBuffer * 2 * sizeof(int16_t));
	int read = _this->m_SoundReadIndex.load();
    
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
		if (const auto& stream = _this->m_PendingRemovalStreams[read]) {
			if (auto it = std::ranges::find(_this->m_Streams, stream); it != _this->m_Streams.end()) {
                auto name = stream->GetName();
                stream->Stop();
				_this->m_Streams.erase(it);
				_this->m_PendingRemovalStreams[read] = nullptr;
                Logger::AddDebug(typeid(AudioManager), "Removed \"{}\"", name);
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
                    EventManager::GetInstance()->Send(std::make_shared<EventSoundStoped>(sound->GetName()));
					it = _this->m_Sounds.erase(it);
                } 
			}
		}
	}
	if (!_this->m_Streams.empty()) {
        for (auto it = _this->m_Streams.begin(); it != _this->m_Streams.end();) {
            const auto& stream = *it;

            if (stream->Erase()) {
                    stream->Stop();
					it = _this->m_Streams.erase(it);
                    EventManager::GetInstance()->Send(std::make_shared<EventAudioStreamStoped>(stream->GetName())); 
                    continue;
            }
            if (!stream->IsPlaying()) {
                it++;
                continue;
            } else {
                 for (size_t frame = 0; frame < framesPerBuffer; frame++) {
                    auto streamFrame = stream->GetNextFrame();

                    mixBuffer[frame * 2] += streamFrame[0];
                    mixBuffer[frame * 2 + 1] += streamFrame[1];
                }
                stream->CalcTranscurredSeconds();
            }


            it++;
		}
	}

	for (size_t i = 0; i < framesPerBuffer * 2; i++)
		out[i] = static_cast<int16_t>(std::clamp<float>(mixBuffer[i], -1.0, 1.0) * _this->m_SampleRate);

	return paContinue;
}

AudioManager::AudioManager(GameContext* context, GameState* state): m_Context(context), m_State(state) {
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
			Logger::AddWarning(typeid(AudioManager), "Tried to play an empty AudioStream!");
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
				if (stream->IsPlaying()) {
					stream->Pause();
                    Logger::AddDebug(typeid(AudioManager), "Stream \"{}\" paused!", stream->GetName());
                }
				else {
					stream->Resume();
                    Logger::AddDebug(typeid(AudioManager), "Stream \"{}\" Resumed!", stream->GetName());
                }

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
}

void AudioManager::Init() {
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
    Logger::AddDebug(typeid(AudioManager), "Linux detected. Using PulseAudio API");

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
		Logger::AddError(typeid(AudioManager), "No Default {} device", apiName);

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
    Logger::AddDebug(typeid(AudioManager), "trying to get output devices");

    for (int i = 0; i <= Pa_GetHostApiInfo(platformApiIndex)->deviceCount; i++) {
        auto device = Pa_GetDeviceInfo(i);
        if (!device)
            Logger::AddWarning(typeid(AudioManager), "Invalid index : {}", i);
        Logger::AddLog(LogType::DebugTarget, typeid(AudioManager), device->name);
    }

    deviceName = deviceInfo->name;

    m_StreamParameters = outputParams;

	auto err = Pa_OpenStream(&m_AudioStream, nullptr, &outputParams, m_SampleRate, 1024, paClipOff, SoundCallback, this);

	if (err != paNoError || Pa_StartStream(m_AudioStream) != paNoError) {
		std::string msg = "Failed to initialize audio stream";

		Logger::AddCritical(typeid(AudioManager), msg);
		THROW_RUNTIME_ERROR(msg);
	}

	Logger::AddDebug(typeid(AudioManager), "Sound Stream opened. Sample Rate = {} Device = {}, API = {}, Latency = {}", m_SampleRate, deviceName, apiName, outputParams.suggestedLatency);

}

void AudioManager::Uninit() {
    Pa_StopStream(m_AudioStream);
	Pa_CloseStream(m_AudioStream);
	Pa_Terminate();

    m_Streams.clear();
    m_Sounds.clear();
}

void AudioManager::PlaySound(std::shared_ptr<Sound> sound) {
    EventManager::GetInstance()->Send(std::make_shared<EventSoundStarted>(sound->GetName()));

	size_t write = m_SoundWriteIndex.load();
	size_t nextWrite = (write + 1) % MAX_PENDING_SOUNDS;

	if (nextWrite != m_SoundReadIndex.load()) {
		m_PendingSounds[write] = std::move(sound);

		m_SoundWriteIndex.store(nextWrite, std::memory_order_release);
	}
}

void AudioManager::PlayStream(std::shared_ptr<AudioStream> stream) {
    if (auto it = std::ranges::find(m_Streams, stream); it != m_Streams.end()) {
        Logger::AddInfo(typeid(AudioManager), "Stream \"{}\" is already playing. Replaying", stream->GetName());
        (*it)->Stop();
        (*it)->Play();
        return;
    }
	Logger::AddDebug(typeid(AudioManager), "Adding \"{}\" onto the queue", stream->GetName());
    
    stream->m_IsBufferReady[0].store(false);
    stream->m_IsBufferReady[1].store(false);
    
    stream->m_Fill.store(false);

	size_t write = m_StreamWriteIndex.load();
	size_t nextWrite = (write + 1) % MAX_PENDING_STREAMS;

	if (nextWrite != m_StreamReadIndex.load()) {
		m_PendingStreams[write] = stream;

		m_StreamWriteIndex.store(nextWrite, std::memory_order_release);
	}

 //    auto err = Pa_OpenStream(&stream->m_Stream, nullptr, &m_StreamParameters, stream->m_DefaultSampleRate, 1024, paClipOff, AudioStreamCallback, &m_Streams.emplace_back(stream));
	//
	// if (err != paNoError || Pa_StartStream(stream->m_Stream) != paNoError) {
	// 	std::string msg = "Failed to initialize audio stream";
	//
	// 	Logger::AddCritical(typeid(AudioManager), msg);
	// 	THROW_RUNTIME_ERROR(msg);
	// }

    // stream->Play();

    EventManager::GetInstance()->Send(std::make_shared<EventAudioStreamStarted>(stream->GetName()));
}

void AudioManager::StopStream(std::shared_ptr<AudioStream> stream) {
    Logger::AddDebug(typeid(AudioManager), "Adding {} to removal queue", stream->GetName());

	int write = m_StreamRemoveWriteIndex.load();
	int nextWrite = (write + 1) % MAX_PENDING_STREAMS;

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

void AudioManager::ShowImGuiDebugWindow() {
    ImGui::Begin("AudioManager Debug", nullptr, ImGuiWindowFlags_MenuBar);
    
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Debug Preferences")) {
            ImGui::MenuItem("Place player in different window", nullptr, &m_PlayerWindow);
            ImGui::SeparatorText("RemovalStreams");
            ImGui::MenuItem("Calculate removalstream real size", nullptr, &m_CalculateRealSize);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (!m_PlayerWindow) {
        if (ImGui::BeginTabBar("audioManagerTabBar")) {
            if (ImGui::BeginTabItem("Player")) {
                goto player;
            }
            if (ImGui::BeginTabItem("Status")) {
                goto status;
            }
        }
    }
    else {
        ImGui::Begin("Player");
    }
player:
    static char buf[150]{};
    ImGui::InputText("Asset name", buf, 150);
    ImGui::SameLine();
    if (ImGui::Button("Play")) {
        if (m_State->GetAssetMgr().Has(buf)) {
            auto stream = m_State->GetAssetMgr().Get<AudioStream>(buf);
            PlayStream(stream.Get());
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        StopStream(buf);
    }

    if (!m_Streams.empty()) {
        for (const auto& stream : m_Streams) {
            if (!stream) continue;
            
            if (stream->IsPlaying())
                ImGui::PushStyleColor(ImGuiCol_Text, { .0f, 1.0f, .0f, 1.0f });
            else
                ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, .0f, 1.0f });
            if (ImGui::TreeNode(stream->GetName().c_str())) {
                ImGui::PopStyleColor();
                auto metadata = stream->GetMetadata();
                
                ImGui::Text("Title: \"%s\"", metadata.Title.toCString(true));
                ImGui::SameLine();
                ImGui::Text("Artist: \"%s\"", metadata.Artist.toCString(true));

                ImGui::Text("Track N°%u", metadata.TrackNum);
                ImGui::SameLine();
                ImGui::Text("Year: %u", metadata.Year);

                if (stream->IsLooping())
                    ImGui::TextColored({ .0f, 1.0f, .0f, 1.0f }, "Looping");
                else
                    ImGui::TextColored({ 1.0f, .0f, .0f, 1.0f }, "Not Looping");
                ImGui::SameLine();
                ImGui::Text("|");
                ImGui::SameLine();
                if (stream->IsPlaying())
                    ImGui::TextColored({ .0f, 1.0f, .0f, 1.0f }, "Playing");
                else
                    ImGui::TextColored({ 1.0f, 1.0f, .0f, 1.0f }, "Paused");

                if (stream->Erase())
                    ImGui::TextColored({ 1.0f, .0f, .0f, 1.0f }, "Erase Requested");

                if (stream->m_Fill)
                    ImGui::TextColored({ .0f, .0f, 1.0f, 1.0f }, "Filling");
                else
                    ImGui::TextColored({ 1.0f, .0f, .0f, 1.0f }, "Not Filling");
                if (stream->m_LastBufferRound) {
                    ImGui::SameLine();
                    ImGui::Text("|");
                    ImGui::SameLine();
                    ImGui::TextColored({ 1.0f, 1.0f, .0f, 1.0f }, "Last Buffer Round");
                }

                ImGui::Text("Default sample rate: %lu", stream->m_DefaultSampleRate);
                ImGui::Text("Target sample rate: %lu", stream->m_TargetSampleRate);
                ImGui::Text("Output frame count: %lu", stream->m_OutputFrameCount);
                ImGui::Text("Data buffer frame count: %lu", stream->m_DataBufferFrameCount);


                if (ImGui::Button("Replay")) {
                    PlayStream(stream);
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop")) {
                    StopStream(stream);
                }
                ImGui::SameLine();
                if (stream->IsPlaying()) {
                    if (ImGui::Button("Pause")) {
                        stream->Pause();
                    }
                }
                else {
                    if (ImGui::Button("Resume")) {
                        stream->Resume();
                    }
                }
                ImGui::SameLine();
                if (stream->IsLooping()) {
                    if (ImGui::Button("Unloop")) {
                        stream->SetLoop(false);
                    }
                }
                else {
                    if (ImGui::Button("Loop")) {
                        stream->SetLoop(true);
                    }
                }

                auto totalSeconds = stream->GetTotalSeconds();
                auto transSeconds = stream->GetTranscurredSeconds();
                auto bufProg = std::format("{}/{} ({}%)", std::floor(transSeconds), totalSeconds, std::floor((transSeconds / totalSeconds) * 100));
                ImGui::ProgressBar(transSeconds / totalSeconds, ImVec2(.0f, .0f), bufProg.data());
                ImGui::SameLine();
                ImGui::Text("Progress");
                auto totalFrames = stream->m_TotalFrames;
                auto transFrames = stream->m_TotalReadFrames;
                auto bufRaw = std::format("{}/{} ({}%)", transFrames, totalFrames, std::floor((transFrames / totalFrames) * 100));
                ImGui::ProgressBar(static_cast<float>(transFrames) / static_cast<float>(totalFrames), ImVec2(.0f, .0f), bufRaw.data());
                ImGui::SameLine();
                ImGui::Text("Raw Progress");
                int volume = stream->GetVolume() * 100;
                ImGui::DragInt("##volume", &volume, 1.f, .0, 100);
                ImGui::SameLine();
                ImGui::Text("Volume");
                if ((static_cast<float>(volume) / 100) != stream->GetVolume()) {
                    stream->SetVolume(static_cast<float>(volume) / 100);
                }
                ImGui::DragFloat("##pan", &stream->m_Pan, 0.05, .0f, 1.0f);
                ImGui::SameLine();
                ImGui::Text("Pan");

                auto bufferL = stream->GetResampledBufferL();
                auto bufferR = stream->GetResampledBufferR();

                ImGui::PlotLines("Left", bufferL.data(), bufferL.size(), 0, nullptr, -1.0f, 1.0f, ImVec2(80.0f, 30.0f));
                ImGui::PlotLines("Right", bufferR.data(), bufferR.size(), 0, nullptr, -1.0f, 1.0f, ImVec2(80.0f, 30.0f));
                if (stream->m_IsBufferReady[0].load())
                    ImGui::TextColored({ .0f, 1.0f, .0f, 1.0f }, "LBuffer filled");
                else
                    ImGui::TextColored({ 1.0f, 1.0f, .0f, 1.0f }, "Filling LBuffer...");
                if (stream->m_IsBufferReady[1].load())
                    ImGui::TextColored({ .0f, 1.0f, .0f, 1.0f }, "RBuffer filled");
                else
                    ImGui::TextColored({ 1.0f, 1.0f, .0f, 1.0f }, "Filling RBuffer...");

                ImGui::TreePop();
            } 
            else 
                ImGui::PopStyleColor();
        }
    }

    if (!m_PlayerWindow) {
        ImGui::EndTabItem();
        if (ImGui::BeginTabItem("Status")) {
            goto status;
        }

        ImGui::EndTabBar();
        goto end;
    }
    else {
        ImGui::End();
    }
status:
    ImGui::Text("m_Streams size: %zu", m_Streams.size());
    ImGui::Text("m_Sounds size: %zu", m_Sounds.size());

    ImGui::Separator();

    ImGui::Text("m_StreamReadIndex: %i", m_StreamReadIndex.load());
    ImGui::Text("m_StreamWriteIndex: %i", m_StreamWriteIndex.load());

    ImGui::Separator();

    ImGui::Text("m_SoundReadIndex: %i", m_SoundReadIndex.load());
    ImGui::Text("m_SoundWriteIndex: %i", m_SoundWriteIndex.load());

    ImGui::SeparatorText("m_PendingRemovalStreams");

    ImGui::Text("Size: %zu", m_PendingRemovalStreams.size());

    if (m_CalculateRealSize) {
        size_t usedSpace{};
        for (const auto& stream : m_PendingRemovalStreams) {
            if (stream) usedSpace++;
        }
        ImGui::Text("Real size: %zu", usedSpace);
    }

    ImGui::Text("m_StreamRemoveReadIndex: %i", m_StreamRemoveReadIndex.load());
    ImGui::Text("m_StreamRemoveWriteIndex: %i", m_StreamRemoveWriteIndex.load());

    if (!m_PlayerWindow) {
        ImGui::EndTabItem();
        ImGui::EndTabBar();
    }
end:
    ImGui::End();
}
