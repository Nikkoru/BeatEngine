#include "BeatEngine/Manager/AssetManager.h"

#include <cstdint>
#include <miniaudio.h>
#include <sndfile.h>
#include <taglib/taglib.h>

#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Asset/Sound.h"
#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Asset/AudioStream.h"

#include "BeatEngine/GameContext.h"
#include "BeatEngine/Logger.h"

#include "BeatEngine/Util/Exception.h"

AssetManager::AssetManager(GameContext* context): m_Context(context) {}

AssetManager::~AssetManager() {
   m_GlobalAssets.clear();
   m_ViewAssets.clear();
}

template <> Base::AssetHandle<Texture> AssetManager::Load<Texture>(const fs::path& path, std::type_index viewID) {
	if (fs::exists(path)) {
		std::string name = path.stem().string();
		std::string fullpath = path.string();

		Base::AssetHandle<Texture> handle;

		bool global = viewID == typeid(nullptr);

		if (global) {
			if (!m_GlobalAssets.contains(name)) {
				// sf::Texture sfTexture(fullpath);
				// auto texture = std::make_shared<Texture>(sfTexture);
				//
				// handle = Base::AssetHandle<Texture>(texture);
				// m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(texture) };
			}
			else {
				Logger::AddWarning(typeid(AssetManager), "Asset already exists: \"{}\", returning existing asset", name);
				handle = Base::AssetHandle<Texture>::Cast(m_GlobalAssets[name].Handle);
			}
		}
		else {
			if (!m_ViewAssets.contains(viewID))
				m_ViewAssets[viewID];
			if (!m_ViewAssets.at(viewID).contains(name)) {
				// sf::Texture sfTexture(fullpath);
				// auto texture = std::make_shared<Texture>(sfTexture);

				// handle = Base::AssetHandle<Texture>(texture);
				// m_ViewAssets.at(viewID)[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(texture) };
			}
			else {
				Logger::AddWarning(typeid(AssetManager), "Asset already exists: \"{}\", returning existing asset", name);
				handle = Base::AssetHandle<Texture>::Cast(m_ViewAssets.at(viewID)[name].Handle); 
			}
		}

		return handle;
	}
	else {
		Logger::AddError(typeid(AssetManager), "Directory/File \"{}\" doesn't exist", path.string());
		return Base::AssetHandle<Texture>();
	}
}
template <> Base::AssetHandle<Sound> AssetManager::Load<Sound>(const fs::path& path, std::type_index viewID) {
	if (fs::exists(path)) {
		std::string name = path.stem().string();
		std::string fullpath = path.string();

		Base::AssetHandle<Sound> handle;

		bool global = viewID == typeid(nullptr);
		bool exists = false;

		if (global) {
			if (m_GlobalAssets.contains(name)) {
				Logger::AddError(typeid(AssetManager), "Asset \"{}\" already exists, returning existing asset", name);
				handle = Base::AssetHandle<Sound>::Cast(m_GlobalAssets[name].Handle);
				exists = true;
			}
			else
				m_GlobalAssets[name];
		}
		else {
			if (!m_ViewAssets.contains(viewID))
				m_ViewAssets[viewID];
			if (m_ViewAssets.at(viewID).contains(name)) {
				Logger::AddError(typeid(AssetManager), "Asset \"{}\" already exists, returning existing asset", name);
				handle = Base::AssetHandle<Sound>::Cast(m_ViewAssets.at(viewID)[name].Handle);
				exists = true;
			}
			else
				m_ViewAssets.at(viewID)[name];
		}
		if (!exists) {
			ma_result result;
			ma_decoder decoder;
			ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 2, 48000);

			
			
			result = ma_decoder_init_file(fullpath.c_str(), &config, &decoder);

			if (result != MA_SUCCESS) {
				std::string msg = "Couldn't read audio file: \"" + name + "\"";
				Logger::AddCritical(typeid(AssetManager), msg);
				ma_decoder_uninit(&decoder);
				THROW_RUNTIME_ERROR(msg);
			}

			uint64_t frameCount = 0;
			std::vector<float> data;
			uint8_t channels = 0;

			ma_uint64 maxFrames = decoder.outputSampleRate * 60;
			channels = decoder.outputChannels;
			data.resize(maxFrames * channels);

			result = ma_decoder_read_pcm_frames(&decoder, data.data(), maxFrames, (ma_uint64*)&frameCount);
			ma_decoder_uninit(&decoder);

			if (result != MA_SUCCESS) {
				std::string msg = "Unable to decode audio: \"" + name + "\"";
				Logger::AddCritical(typeid(AssetManager), msg);
				THROW_RUNTIME_ERROR(msg);
			}

			auto sound = std::make_shared<Sound>(name, data, frameCount, m_AudioSampleRate);

			handle = Base::AssetHandle<Sound>(sound);

			if (global)
				m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(sound) };
			else
				m_ViewAssets.at(viewID)[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(sound) };
		}

		return handle;
	}
	else {
		Logger::AddError(typeid(AssetManager), "Directory/File \"{}\" doesn't exist", path.string());
		return Base::AssetHandle<Sound>();
	}
}
template <> Base::AssetHandle<AudioStream> AssetManager::Load<AudioStream>(const fs::path& path, std::type_index viewID) {
	if (fs::exists(path)) {
		std::string name = path.stem().string();
		std::string fullpath = path.string();

		Base::AssetHandle<AudioStream> handle;

		bool global = viewID == typeid(nullptr);
		bool exists = false;

		if (global) {
			if (m_GlobalAssets.contains(name)) {
				exists = true;
			}
		}
		else {
			if (!m_ViewAssets.contains(viewID))
				m_ViewAssets[viewID];
			if (m_ViewAssets.at(viewID).contains(name)) {
				exists = true;
			}
		}

		if (!exists) {
			ma_result result;
			ma_decoder decoder;
			ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 2, 0);

			SF_INFO sfInfo;
            TagLib::FileRef ref;

			sf_count_t totalFrames = -1;
            
            float seconds = -1;

			auto sndFile = sf_open(fullpath.c_str(), SFM_READ, &sfInfo);
			if (sndFile) {
				totalFrames = sfInfo.frames;
                seconds = totalFrames / sfInfo.samplerate;
			}
			else {
                Logger::AddError(typeid(AssetManager), "Failed to retreive frame count data of \"{}\"", name);
			}
			sf_close(sndFile);

            ref = TagLib::FileRef(fullpath.c_str());
			
			result = ma_decoder_init_file(fullpath.c_str(), &config, &decoder);

			if (result != MA_SUCCESS) {
				THROW_RUNTIME_ERROR("failed");
				ma_decoder_uninit(&decoder);
			}

			auto stream = std::make_shared<AudioStream>(name, decoder, decoder.outputSampleRate, m_AudioSampleRate, ref, seconds, static_cast<uint64_t>(totalFrames));

			handle = Base::AssetHandle<AudioStream>(stream);

			m_ViewAssets.at(viewID)[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(stream) };
		}
		return handle;
	}
	else {
		Logger::AddError(typeid(AssetManager), "Directory/File \"{}\" doesn't exist", path.string());
		return Base::AssetHandle<AudioStream>();
	}
}
template <> Base::AssetHandle<Font> AssetManager::Load<Font>(const fs::path& path, std::type_index viewID) {
	if (fs::exists(path)) {
		std::string name = path.stem().string();
		std::string fullpath = path.string();

		Base::AssetHandle<Font> handle;

		bool global = viewID == typeid(nullptr);

		if (global) {
			if (!m_GlobalAssets.contains(name)) {
				// sf::Font sfFont(fullpath);
				// auto font = std::make_shared<Font>(sfFont);

				// handle = Base::AssetHandle<Font>(font);
				// m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(font) };
			}
			else {
				Logger::AddError(typeid(AssetManager), "Asset \"{}\" already exists, returning existing asset", name);
				handle = Base::AssetHandle<Font>::Cast(m_ViewAssets.at(viewID)[name].Handle);
			}

		}
		else {
			if (!m_ViewAssets.contains(viewID))
				m_ViewAssets[viewID];
			if (!m_ViewAssets.at(viewID).contains(name)) {
				// Font sfFont(fullpath);
				// auto font = std::make_shared<Font>(sfFont);

				// handle = Base::AssetHandle<Font>(font);
				// m_ViewAssets.at(viewID)[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(font) };
			}
			else {
				Logger::AddError(typeid(AssetManager), "Asset \"{}\" already exists, returning existing asset", name);
				handle = Base::AssetHandle<Font>::Cast(m_ViewAssets.at(viewID)[name].Handle);
			}
		}
		return handle;
	}
	else {
		Logger::AddError(typeid(AssetManager), "Directory/File \"{}\" doesn't exist", path.string());
		return Base::AssetHandle<Font>();
	}
}

bool AssetManager::Has(std::string name, const std::type_index viewID) {
	bool global = viewID == typeid(nullptr);

	if (global)
		return m_GlobalAssets.contains(name);
	else {
		if (m_ViewAssets.contains(viewID))
			return m_ViewAssets.at(viewID).contains(name);
		else
			return false;
	}
}

void AssetManager::DrawImGuiDebug() {
    // ImGui::Begin("BeatEngine AssetManager Debug Window");
    // ImGui::Text("Global Assets : %zu", m_GlobalAssets.size());
    // ImGui::Text("View Assets: %zu", m_ViewAssets.size());
    // ImGui::End();
}
