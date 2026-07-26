#include "BeatEngine/Manager/AssetManager.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <freetype/freetype.h>
#include <freetype/ftstroke.h>
#include <memory>
#include <miniaudio.h>
#include <sndfile.h>
#include <taglib/tag.h>
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

#include "BeatEngine/System/DataStream.hpp"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/Util/Profiler.h"
#include "imgui.h"

namespace {
unsigned long read(FT_Stream rec, unsigned long offset, unsigned char* buffer, unsigned long count) {
    auto* stream = static_cast<DataStream*>(rec->descriptor.pointer);
    if (auto streamPos = stream->Seek(offset).Value(); streamPos == offset) {
        if (count > 0)
            return static_cast<unsigned long>(stream->Read(reinterpret_cast<char*>(buffer), count).Value());

        return 0;
    }
    return count > 0 ? 0 : 1;
}
void close(FT_Stream) {}
}

AssetManager::AssetManager(GameContext* context, GameState* state)
    : m_Context(context), m_State(state) {}

AssetManager::~AssetManager() {
   m_GlobalAssets.clear();
   m_ViewAssets.clear();
}


void AssetManager::Uninit() {
    for (const auto& [viewID, assetMap] : m_ViewAssets) {
        for (const auto& [assetName, asset] : assetMap) {
            if (asset.Type == typeid(Texture)) {
                Logger::AddDebug(typeid(AssetManager), "Destroying texture \"{}\"", assetName);
                auto texture = std::dynamic_pointer_cast<Texture>(asset.Asset);
                m_State->GetGraphicsMgr().DestroyTexture(texture);
            }
            else if (asset.Type == typeid(Font)) {
                Logger::AddDebug(typeid(AssetManager), "Destroying font \"{}\"", assetName);
                auto font = std::dynamic_pointer_cast<Font>(asset.Asset);
                for (const auto& [charSize, page] : font->m_Pages) {
                    m_State->GetGraphicsMgr().DestroyTexture(page.PageTexture);
                }
            }
        }
    }

    for (const auto& [assetName, asset] : m_GlobalAssets) {
        if (asset.Type == typeid(Texture)) {
            Logger::AddDebug(typeid(AssetManager), "Destroying texture \"{}\"", assetName);
            auto texture = std::dynamic_pointer_cast<Texture>(asset.Asset);
            m_State->GetGraphicsMgr().DestroyTexture(texture);
        }
        else if (asset.Type == typeid(Font)) {
            Logger::AddDebug(typeid(AssetManager), "Destroying font \"{}\"", assetName);
            auto font = std::dynamic_pointer_cast<Font>(asset.Asset);
                for (const auto& [charSize, page] : font->m_Pages) {
                    m_State->GetGraphicsMgr().DestroyTexture(page.PageTexture);
                }
        }
    }
}


template <> Base::AssetHandle<Texture> AssetManager::Load<Texture>(const fs::path& path, std::type_index viewID) {
	if (fs::exists(path)) {
		std::string name = path.stem().string();

		Base::AssetHandle<Texture> handle;

		bool global = viewID == typeid(nullptr);

		if (global) {
			if (!m_GlobalAssets.contains(name)) {
                auto texture = m_State->GetGraphicsMgr().CreateTexture(path);

				handle = Base::AssetHandle<Texture>(texture, typeid(Texture));
				m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(texture), typeid(Texture) };
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

				handle = Base::AssetHandle<Texture>(texture, typeid(Texture));
				m_ViewAssets.at(viewID)[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(texture), typeid(Texture) };
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

			handle = Base::AssetHandle<Sound>(sound, typeid(Sound));

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
                seconds = static_cast<float>(totalFrames) / sfInfo.samplerate;
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

			handle = Base::AssetHandle<AudioStream>(stream, typeid(AudioStream));
            if (global)
                m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(stream), typeid(AudioStream) };
            else
                m_ViewAssets.at(viewID)[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(stream) };
		}

        Logger::AddDebug(typeid(AudioManager), "Asset {} created", name);
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

        auto font = std::make_shared<Font>();

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
            if (FT_Init_FreeType(&font->m_FTLibrary) != 0) {
                Logger::AddCritical("Failed to load font \"{}\": FreeType failed to init", name);
                THROW_RUNTIME_ERROR("Failed to load font");
            }

            font->m_Stream.StartReadForFile(path);

            font->m_FTStreamRec.base = nullptr;
            font->m_FTStreamRec.size = static_cast<unsigned long>(font->m_Stream.GetSize());
            font->m_FTStreamRec.pos = 0;
            font->m_FTStreamRec.descriptor.pointer = &font->m_Stream;
            font->m_FTStreamRec.read = &read;
            font->m_FTStreamRec.close = &close;

            FT_Open_Args args{};
            args.flags = FT_OPEN_STREAM;
            args.stream = &font->m_FTStreamRec;
            args.driver = nullptr;

            if (auto result = FT_Open_Face(font->m_FTLibrary, &args, 0, &font->m_FTFace); result != 0) {
                auto errStringC = FT_Error_String(result);
                auto errStr = errStringC == nullptr ? "error in the error thats amazing" : errStringC; 
                Logger::AddCritical("", "Failed to load font \"{}\": FreeType failed to create the font face, reason: {}", name, errStr);
                THROW_RUNTIME_ERROR("Failed to load font");
            }

            if (FT_Stroker_New(font->m_FTLibrary, &font->m_FTStroker) != 0) {
                Logger::AddCritical("Failed to load font \"{}\": FreeType failed to create the stroker", name);
                THROW_RUNTIME_ERROR("Failed to load font");
            }

            if (FT_Select_Charmap(font->m_FTFace, FT_ENCODING_UNICODE) != 0) {
                Logger::AddCritical("Failed to load font \"{}\": FreeType failed set unicode character set", name);
                THROW_RUNTIME_ERROR("Failed to load font");
            }

            font->m_FamilyName = font->m_FTFace->family_name ? font->m_FTFace->family_name : "";
            font->m_HasKerning = FT_HAS_KERNING(font->m_FTFace);
            font->m_HasVerticalMetrics = FT_HAS_VERTICAL(font->m_FTFace);
        }

        handle = Base::AssetHandle<Font>(font, typeid(Font));

        // the handle id is sufficient
        font->m_ID = handle.GetID();

		if (global) {
			if (!m_GlobalAssets.contains(name)) {
				m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(font), typeid(Font) };
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
				m_ViewAssets.at(viewID)[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(font), typeid(Font) };
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

            m_GlobalAssets[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(shader), typeid(Shader) };
        }
    }
    else {
        if (!m_ViewAssets.contains(viewID))
            m_ViewAssets[viewID];
        if (!m_ViewAssets.at(viewID).contains(name)) {
            auto shader = m_State->GetGraphicsMgr().CreateShader(path, type);
            handle = Base::AssetHandle<Shader>(shader, typeid(Shader));
            m_ViewAssets.at(viewID)[name] = { static_cast<Base::AssetHandle<void>>(handle), std::static_pointer_cast<Base::Asset>(shader), typeid(Shader) };
        }
        else {
            Logger::AddWarning(typeid(AssetManager), "Asset \"{}\" already exists, returning existing asset", name);
            handle = Base::AssetHandle<Shader>::Cast(m_ViewAssets.at(viewID)[name].Handle);
        }
    }
    Logger::AddDebug(typeid(AssetManager), "Loaded Shader \"{}\"", name);

    return handle;
}

bool AssetManager::Preload(AssetType type, const fs::path& path, std::type_index viewID) {
    switch (type) {
    case AssetType::Texture:
    case AssetType::FragmentShader:
    case AssetType::VertexShader:
    case AssetType::ComputeShader:
    case AssetType::Font:
        if (!m_AssetsToLoad.contains(type))
            m_AssetsToLoad[type];
        m_AssetsToLoad.at(type) = path;
        return false;
    case AssetType::AudioStream:
        Load<AudioStream>(path, viewID);
        return true;
    case AssetType::Sound:
        Load<Sound>(path, viewID);
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
        case AssetType::ComputeShader:
            if (LoadShader(asset, Shader::Type::Compute)) {
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
        return m_ViewAssets.contains(viewID) && m_ViewAssets.at(viewID).contains(name);
	}
}

void AssetManager::ShowImGuiDebugWindow() {
    if (!(m_Context->GFlags & GameFlags_ImGui)) return;

    ImGui::Begin("AssetManager Debug");
    ImGui::Text("Global Assets : %zu", m_GlobalAssets.size());
    ImGui::Text("View Assets: %zu", m_ViewAssets.size());
    if (ImGui::Button("Asset Browser")) {
        m_ShowAssetBrowser = true;
    }
    
    static char buf[100];

    ImGui::InputTextWithHint("Path", "asset.mp3", buf, 100);

    static AssetType selectedType = AssetType::AudioStream;
    static uint8_t index{};
    static uint8_t selectedIndex{}; 

    if (ImGui::BeginCombo("Type", AssetTypeUtils::TypeToString(selectedType).c_str(), ImGuiComboFlags_WidthFitPreview | ImGuiComboFlags_PopupAlignLeft)) {
        index = 0;
        for (const auto& [type, typeStr] : AssetTypeUtils::GetMap()) {
            if (type == AssetType::None)
                continue;
            const bool selected = (index == selectedIndex);
            if (ImGui::Selectable(typeStr.c_str(), selected)) {
                selectedType = AssetTypeUtils::StringToType(typeStr);
                selectedIndex = index;
            }
            index++;
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load For Global")) {
        switch (selectedType) {
            case AssetType::AudioStream:
                Load<AudioStream>(buf);
                break;
            case AssetType::Sound:
                Load<Sound>(buf);
                break;
            case AssetType::Texture:
                Load<Texture>(buf);
                break;
            case AssetType::Font:
                Load<Font>(buf);
                break;
            case AssetType::VertexShader:
                LoadShader(buf, Shader::Type::Vertex);
                break;
            case AssetType::FragmentShader:
                LoadShader(buf, Shader::Type::Fragment);
                break;
            case AssetType::ComputeShader:
                LoadShader(buf, Shader::Type::Compute);
                break;
            case AssetType::None:
                break;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Load For Active View")) {
        auto activeView = m_Context->ActiveView;
        switch (selectedType) {
            case AssetType::AudioStream:
                Load<AudioStream>(buf, activeView);
                break;
            case AssetType::Sound:
                Load<Sound>(buf, activeView);
                break;
            case AssetType::Texture:
                Load<Texture>(buf, activeView);
                break;
            case AssetType::Font:
                Load<Font>(buf, activeView);
                break;
            case AssetType::VertexShader:
                LoadShader(buf, Shader::Type::Vertex, activeView);
                break;
            case AssetType::FragmentShader:
                LoadShader(buf, Shader::Type::Fragment, activeView);
                break;
            case AssetType::ComputeShader:
                LoadShader(buf, Shader::Type::Compute, activeView);
                break;
            case AssetType::None:
                break;
        }
    }
    if (ImGui::Button("Clear View Assets")) {
        ImGui::OpenPopup("View Sure?");
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Global Assets")) {
        ImGui::OpenPopup("Global Sure?");
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear ALL Assets")) {
        ImGui::OpenPopup("Sure?");
    }
    auto center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("View Sure?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("This action will delete all assets in a view!\nIf the view tries to get a previously loaded asset WILL throw a runtime error!\nAre you sure to continue?");
        ImGui::Separator();
        if (ImGui::Button("Yes")) {
            ImGui::CloseCurrentPopup();
            m_ViewAssets.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("Global Sure?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("This action will delete all global assets!\nIf something tries to get a previously loaded asset WILL throw a runtime error!\nAre you sure to continue?");
        ImGui::Separator();
        if (ImGui::Button("Yes")) {
            ImGui::CloseCurrentPopup();
            m_GlobalAssets.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("Sure?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("This action will delete ALL assets available!\nAre you sure to continue?");
        ImGui::Separator();
        if (ImGui::Button("Yes")) {
            ImGui::CloseCurrentPopup();
            m_ViewAssets.clear();
            m_GlobalAssets.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::End();

    if (m_ShowAssetBrowser) {
        ShowAssetBrowser();
    }
}

void AssetManager::ApplySelections(ImGuiMultiSelectIO* io, std::vector<UID>& ids, std::vector<Slot>& totalAssets) {
    for (const auto& req : io->Requests) {
        if (req.Type == ImGuiSelectionRequestType_SetAll) {
            ids.clear();
            if (req.Selected)
                for (int i = 0; i < io->ItemsCount; i++) {
                    auto handle = totalAssets[i].Handle;
                    ids.emplace_back(handle.GetID());
                }
        }
        else if (req.Type == ImGuiSelectionRequestType_SetRange) {
            const size_t selectionChanges = req.RangeLastItem - req.RangeFirstItem + 1;
            
            if (selectionChanges == 1 || (selectionChanges < ids.size() / 100)) {
                for (int i = req.RangeFirstItem; i <= req.RangeLastItem; i++) {
                    auto id = totalAssets[i].Handle.GetID();
                    if (req.Selected)
                        ids.emplace_back(id);
                    else {
                        if (auto it = std::find(ids.begin(), ids.end(), id); it != ids.end()) {
                            ids.erase(it);
                        }
                    }
                }
            }
            else {
                int selectionOrder = ((req.RangeDirection < 0) ? selectionChanges - 1 : 0);

                for (int i = (int)req.RangeFirstItem; i <= (int)req.RangeLastItem; i++) {
                    auto id = totalAssets[i + selectionOrder].Handle.GetID();
                    if (req.Selected)
                        ids.emplace_back(id);
                    else {
                        if (auto it = std::find(ids.begin(), ids.end(), id); it != ids.end())
                            ids.erase(it);
                    }
                }
            }
        }
    }
}

void AssetManager::ShowAssetBrowser() {
    Profiler::StartProfile({ typeid(AssetManager), "ShowAssetBrowser" }, IM_COL32(150, 0, 100, 255));
    std::vector<Slot> totalAssets;
    std::vector<std::string> assetNames;
    static std::vector<UID> selectedIds;
    static std::vector<int> assetDetail;

    for (const auto& [name, slot] : m_GlobalAssets) {
        totalAssets.emplace_back(slot);
        assetNames.emplace_back(name);
    }

    for (const auto& [_, map] : m_ViewAssets)
        for (const auto& [name, slot] : map) {
            totalAssets.emplace_back(slot);
            assetNames.emplace_back(name);
        }

    assetDetail.resize(totalAssets.size());

    static UID deleteAsset{ 0 };
    ImGui::Begin("Asset List", &m_ShowAssetBrowser, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Delete")) {
            
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Layout")) {
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
 
    if (ImGui::BeginChild("Assets", { .0f, -ImGui::GetTextLineHeightWithSpacing() }, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoMove)) {
        auto size = ImVec2{ 50, 50 };
        auto spacing = 5;

        auto drawList = ImGui::GetWindowDrawList();
        float availWidth = ImGui::GetContentRegionAvail().x;

        auto startPos = ImGui::GetCursorScreenPos();

        auto columnCount = std::max(static_cast<int>(availWidth / (size.x + spacing)), 1);
        spacing = std::floor(availWidth - size.x * columnCount) / columnCount;
        int totalLines = (totalAssets.size() + columnCount - 1) / columnCount;

        auto itemStep = ImVec2{ size.x + spacing, size.y + spacing };

        auto multiFlags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid;
        multiFlags |= ImGuiMultiSelectFlags_NavWrapX;
        multiFlags |= ImGuiMultiSelectFlags_BoxSelect2d;

        auto multiIO = ImGui::BeginMultiSelect(multiFlags, selectedIds.size(), totalAssets.size());
        ApplySelections(multiIO, selectedIds, totalAssets);

        // const bool wantDelete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat));

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));

        const ImU32 iconBgColor = ImGui::GetColorU32(IM_COL32(35, 35, 35, 220));

        ImGuiListClipper clipper;
        clipper.Begin(totalLines, itemStep.y);

        while(clipper.Step()) {
            for (int lineIndex = clipper.DisplayStart; lineIndex < clipper.DisplayEnd; lineIndex++) {
                const int minItemIndex = lineIndex * columnCount;
                const int maxItemIndex = std::min((lineIndex + 1) * columnCount, static_cast<int>(totalAssets.size()));
                for (int itemIndex = minItemIndex; itemIndex < maxItemIndex; itemIndex++) {
                    auto& showDetails = assetDetail[itemIndex];
                    auto assetData = totalAssets[itemIndex].Handle;
                    auto assetType = totalAssets[itemIndex].Type;
                    auto assetName = assetNames[itemIndex];
                    const bool displayLabel = (size.x >= ImGui::CalcTextSize(assetName.c_str()).x);
                    ImGui::PushID(assetData.GetID());

                    auto pos = ImVec2{startPos.x + (itemIndex % columnCount) * itemStep.x, startPos.y + lineIndex * itemStep.y };
                    ImGui::SetCursorScreenPos(pos);

                    ImGui::SetNextItemSelectionUserData(itemIndex);
                    bool selected{ false };
                    bool visible = ImGui::IsRectVisible(size);

                    for (auto& id : selectedIds) {
                        if (id == assetData.GetID()) {
                            selected = true;
                            break;
                        }
                    }
                    ImGui::Selectable("", selected, ImGuiSelectableFlags_AllowDoubleClick, size);
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && selected) {
                        showDetails = 1;
                    }

                    if (ImGui::IsItemToggledSelection())
                        selected = !selected;

                    if (visible) {
                        ImU32 labelCol = ImGui::GetColorU32(selected ? ImGuiCol_Text : ImGuiCol_TextDisabled);
                        auto boxMin = ImVec2{ pos.x - 1, pos.y - 1 };
                        auto boxMax = ImVec2{ boxMin.x + size.x + 2, boxMin.y + size.y + 2 };

                        drawList->AddRectFilled(boxMin, boxMax, iconBgColor);

                        if (assetType == typeid(Texture)) {
                            auto texture = Base::AssetHandle<Texture>::Cast(assetData).Get();
                            ImVec2 padBoxMin = { boxMin.x - 3, boxMin.y - 3 };
                            ImVec2 padBoxMax = { boxMax.x - 3, boxMax.y - 3 };
                            drawList->AddImage(texture->GetImGuiTexture(m_State->GetGraphicsMgr()), padBoxMin, padBoxMax);
                        }

                        std::string typeLabel;

                        if (assetType == typeid(AudioStream))
                            typeLabel = "A.S.";
                        else if (assetType == typeid(Sound))
                            typeLabel = "Sound";
                        else if (assetType == typeid(Font))
                            typeLabel = "Font";
                        else if (assetType == typeid(Texture))
                            typeLabel = "Texture";
                        else if (assetType == typeid(Shader)) {
                            auto asset = Base::AssetHandle<Shader>::Cast(assetData).Get();
                            auto type = asset->GetType();
                            char shaderType;
                            switch (type) {
                                case Shader::Type::Compute:
                                    shaderType = 'C';
                                    break;
                                case Shader::Type::Fragment:
                                    shaderType = 'F';
                                    break;
                                case Shader::Type::Vertex:
                                    shaderType = 'V';
                                    break;
                            }
                            typeLabel = std::format("Shader{}", shaderType);
                        }
                        else {
                            typeLabel = "Unknown";
                        }

                        drawList->AddText({ boxMax.x - ImGui::CalcTextSize(typeLabel.c_str()).x, boxMin.y }, labelCol, typeLabel.c_str());

                        if (displayLabel) {
                            drawList->AddText(ImVec2(boxMin.x, boxMax.y - ImGui::GetFontSize()), labelCol, assetName.c_str());
                        }
                        else {
                            auto trunName = assetName.substr(0, 4);
                            drawList->AddText(ImVec2(boxMin.x, boxMax.y - ImGui::GetFontSize()), labelCol, std::format("{}...", trunName).c_str());
                        }

                        if (showDetails == 1) {
                            auto col = ImGui::ColorConvertU32ToFloat4(labelCol);
                            drawList->AddRectFilled({ boxMin.x + 8, boxMin.y + 8 }, { boxMin.x + 4, boxMin.y + 4 }, ImGui::ColorConvertFloat4ToU32({ .0f, .0f, 1.0f, col.w }));
                        }
                    }
                    
                    ImGui::PopID();
                }
            }
        }
        clipper.End();
        ImGui::PopStyleVar();

        if (ImGui::BeginPopupContextWindow()) {
            ImGui::Text("Selection: %zu assets", selectedIds.size());
            ImGui::EndPopup();
        }

        multiIO = ImGui::EndMultiSelect();

        ApplySelections(multiIO, selectedIds, totalAssets);
    }
    ImGui::EndChild();

    ImGui::Text("Selected: %zu/%zu items", selectedIds.size(), totalAssets.size());
    
    auto i = 0;
    for (auto& slot : totalAssets) {
        if (assetDetail[i] == 1) {
            bool open = true;
            slot.Asset->ShowImGuiDetails(&open);
            if (!open)
                assetDetail[i] = 0;
        }
        i++;
    }
    
    ImGui::End();
    Profiler::EndProfile({ typeid(AssetManager), "ShowAssetBrowser" });
}
