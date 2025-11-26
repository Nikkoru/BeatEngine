#include "BeatEngine/Manager/AssetManager.h"

#include <SFML/Graphics.hpp>
#include <miniaudio.h>

#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Asset/Sound.h"
#include "BeatEngine/Asset/Font.h"

#include "BeatEngine/Logger.h"

#include "BeatEngine/Util/Exception.h"

template <> Base::AssetHandle<Texture> AssetManager::Load<Texture>(const fs::path& path, std::type_index viewID) {
	if (fs::exists(path)) {
		std::string name = path.stem().string();
		std::string fullpath = path.string();

		Base::AssetHandle<Texture> handle;

		bool global = viewID == typeid(nullptr);

		if (global) {
			if (!m_GlobalAssets.contains(name)) {
				sf::Texture sfTexture(fullpath);
				auto texture = std::make_shared<Texture>(sfTexture);

				handle = Base::AssetHandle<Texture>(texture);
				m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(texture) };
			}
			else {
				Logger::GetInstance()->AddError("Asset already exists: \"" + name + "\", returning existing asset", typeid(AssetManager));
				handle = Base::AssetHandle<Texture>::Cast(m_GlobalAssets[name].Handle);
			}
		}
		else {
			if (!m_ViewAssets.contains(viewID))
				m_ViewAssets[viewID];
			if (!m_ViewAssets.at(viewID).contains(name)) {
				sf::Texture sfTexture(fullpath);
				auto texture = std::make_shared<Texture>(sfTexture);

				handle = Base::AssetHandle<Texture>(texture);
				m_ViewAssets.at(viewID)[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(texture) };
			}
			else {
				Logger::GetInstance()->AddError("Asset already exists: \"" + name + "\", returning existing asset", typeid(AssetManager));
				handle = Base::AssetHandle<Texture>::Cast(m_ViewAssets.at(viewID)[name].Handle); 
			}
		}

		return handle;
	}
	else {
		Logger::GetInstance()->AddError("directory dosen't exists: \"" + path.string() + "\"", typeid(AssetManager));
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
				Logger::GetInstance()->AddError("Asset already exists: \"" + name + "\", returning existing asset", typeid(AssetManager));
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
				Logger::GetInstance()->AddError("Asset already exists: \"" + name + "\", returning existing asset", typeid(AssetManager));
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
				Logger::GetInstance()->AddCritical(msg, typeid(AssetManager));
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
				Logger::GetInstance()->AddCritical(msg, typeid(typeid(AssetManager)));
				THROW_RUNTIME_ERROR(msg);
			}

			auto sound = std::make_shared<Sound>(data, frameCount, m_AudioSampleRate);

			handle = Base::AssetHandle<Sound>(sound);

			if (global)
				m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(sound) };
			else
				m_ViewAssets.at(viewID)[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(sound) };
		}

		return handle;
	}
	else {
		Logger::GetInstance()->AddError("Directory dosen't exist: \"" + path.string() + "\"", typeid(AssetManager));
		return Base::AssetHandle<Sound>();
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
				sf::Font sfFont(fullpath);
				auto font = std::make_shared<Font>(sfFont);

				handle = Base::AssetHandle<Font>(font);
				m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(font) };
			}
			else {
				Logger::GetInstance()->AddError("Asset already exists: \"" + name + "\", returning existing asset", typeid(AssetManager));
				handle = Base::AssetHandle<Font>::Cast(m_ViewAssets.at(viewID)[name].Handle);
			}

		}
		else {
			if (!m_ViewAssets.contains(viewID))
				m_ViewAssets[viewID];
			if (!m_ViewAssets.at(viewID).contains(name)) {
				sf::Font sfFont(fullpath);
				auto font = std::make_shared<Font>(sfFont);

				handle = Base::AssetHandle<Font>(font);
				m_ViewAssets.at(viewID)[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(font) };
			}
			else {
				Logger::GetInstance()->AddError("Asset already exists: \"" + name + "\", returning existing asset", typeid(AssetManager));
				handle = Base::AssetHandle<Font>::Cast(m_ViewAssets.at(viewID)[name].Handle);
			}
		}
		return handle;
	}
	else {
		Logger::GetInstance()->AddError("Directory doesn't exist: \"" + path.string() + "\"", typeid(AssetManager));
		return Base::AssetHandle<Font>();
	}
}