#include "BeatEngine/Manager/AssetManager.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <miniaudio.h>
#include <sndfile.h>
#include <taglib/taglib.h>
#include <typeindex>

#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Asset/Sound.h"
#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Asset/AudioStream.h"

#include "BeatEngine/Base/Asset.h"
#include "BeatEngine/Enum/AssetType.h"
#include "BeatEngine/Manager/GraphicsManager.h"

#include "BeatEngine/GameContext.h"
#include "BeatEngine/GameState.h"
#include "BeatEngine/Logger.h"

#include "BeatEngine/Util/Exception.h"
#include "imgui.h"

AssetManager::AssetManager(GameContext* context, GameState* state)
    : m_Context(context), m_State(state) {}

AssetManager::~AssetManager() {
   m_GlobalAssets.clear();
   m_ViewAssets.clear();
}


void AssetManager::Uninit() {

}


template <> Base::AssetHandle<Texture> AssetManager::Load<Texture>(const fs::path& path, std::type_index viewID) {
	if (fs::exists(path)) {
		std::string name = path.stem().string();

		Base::AssetHandle<Texture> handle;

		bool global = viewID == typeid(nullptr);

		if (global) {
			if (!m_GlobalAssets.contains(name)) {
                auto texture = m_State->GetGraphicsMgr().CreateTexture(path);

				handle = Base::AssetHandle<Texture>(texture);
				m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(texture) };
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
                auto texture = m_State->GetGraphicsMgr().CreateTexture(path);

				handle = Base::AssetHandle<Texture>(texture);
				m_ViewAssets.at(viewID)[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(texture) };
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
				Logger::AddWarning(typeid(AssetManager), "Asset \"{}\" already exists, returning existing asset", name);
				handle = Base::AssetHandle<Font>::Cast(m_ViewAssets.at(viewID)[name].Handle);
			}
		}
		return handle;
	}
	else {
		Logger::AddError(typeid(AssetManager), "File \"{}\" doesn't exist", path.string());
		return Base::AssetHandle<Font>();
	}
}

Base::AssetHandle<Shader> AssetManager::LoadShader(const fs::path& path, Shader::Type type, const std::type_index viewID) {
    if (!fs::exists(path)) {
		Logger::AddError(typeid(AssetManager), "Directory/File \"{}\" doesn't exist", path.string());
		return Base::AssetHandle<Shader>();
    }

    std::string name = path.stem().string();

    Base::AssetHandle<Shader> handle;

    bool global = viewID == typeid(nullptr);

    if (global) {
        if (m_GlobalAssets.contains(name)) {
            Logger::AddWarning(typeid(AssetManager), "Asset \"{}\" already exists, returning existing asset", name);
            handle = Base::AssetHandle<Shader>::Cast(m_GlobalAssets[name].Handle);
        }
        else {
            auto shader = m_State->GetGraphicsMgr().CreateShader(path, type);
            handle = Base::AssetHandle<Shader>(shader);

            m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(shader) };
        }
    }
    else {
        if (!m_ViewAssets.contains(viewID))
            m_ViewAssets[viewID];
        if (!m_ViewAssets.at(viewID).contains(name)) {
            auto shader = m_State->GetGraphicsMgr().CreateShader(path, type);
            handle = Base::AssetHandle<Shader>(shader);

            m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(shader) };
        }
        else {
            Logger::AddWarning(typeid(AssetManager), "Asset \"{}\" already exists, returning existing asset", name);
            handle = Base::AssetHandle<Shader>::Cast(m_ViewAssets.at(viewID)[name].Handle);
        }
    }

    return handle;
}

bool AssetManager::Preload(AssetType type, const fs::path& path, std::type_index viewID) {
    switch (type) {
    case AssetType::Texture:
    case AssetType::FragmentShader:
    case AssetType::VertexShader:
    case AssetType::Font:
        if (!m_AssetsToLoad.contains(type))
            m_AssetsToLoad[type];
        m_AssetsToLoad.at(type) = path;
        return false;
    case AssetType::AudioStream:
        Load<AudioStream>(path);
        return true;
    case AssetType::Sound:
        Load<Sound>(path);
        return true;
    default:
        return false;
    }
}

void AssetManager::Init() {
    if (!m_AssetsToLoad.empty())
        Logger::AddDebug(typeid(AssetManager), "Some assets were requested to load when preloading. loading...");
    auto totalAssetLoaded = m_AssetsToLoad.size();
    for (const auto& [type, asset] : m_AssetsToLoad) {
        Logger::AddDebug(typeid(AssetManager), "Loading \"{}\"", asset.string());
        switch (type) {
        case AssetType::Texture:
            if (Load<Texture>(asset)) {
                totalAssetLoaded--;
            }
            break;
        case AssetType::FragmentShader:
            if (LoadShader(asset, Shader::Type::Fragment)) {
                totalAssetLoaded--;
            }
            break;
        case AssetType::VertexShader:
            if (LoadShader(asset, Shader::Type::Vertex)) {
                totalAssetLoaded--;
            }
            break;
        case AssetType::Font:
            if (Load<Font>(asset)) {
                totalAssetLoaded--;
            }
            break;
        default:
            break;
        }
    }
    Logger::AddDebug(typeid(AssetManager), "Loaded {} assets", totalAssetLoaded);
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
    ImGui::Begin("BeatEngine AssetManager Debug Window");
    ImGui::Text("Global Assets : %zu", m_GlobalAssets.size());
    ImGui::Text("View Assets: %zu", m_ViewAssets.size());
    ImGui::End();
}
