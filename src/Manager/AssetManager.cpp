#include "BeatEngine/Manager/AssetManager.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <freetype/freetype.h>
#include <freetype/ftstroke.h>
#include <memory>
#include <miniaudio.h>
#include <extras/decoders/libopus/miniaudio_libopus.h>
#include <extras/decoders/libvorbis/miniaudio_libvorbis.h>
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

#include "BeatEngine/AppContext.hpp"
#include "BeatEngine/AppState.hpp"
#include "BeatEngine/Logger.h"

#include "BeatEngine/Util/Profiler.h"
#include "imgui.h"

AssetManager::AssetManager(AppContext* context, AppState* state)
    : m_Context(context), m_State(state) {}

AssetManager::~AssetManager() {
    for (const auto& [viewID, assetMap] : m_ViewAssets) {
        for (const auto& [assetName, asset] : assetMap) {
            if (m_UnloadCallbacks.contains(asset.Type))
                m_UnloadCallbacks.at(asset.Type)(asset.Handle);
        }
    }
    m_ViewAssets.clear();
    for (auto& [assetName, asset] : m_GlobalAssets) {
        if (m_UnloadCallbacks.contains(asset.Type))
            m_UnloadCallbacks.at(asset.Type)(asset.Handle);
    }
    m_GlobalAssets.clear();
}

void AssetManager::BulkLoad(const Assets& assets, const std::type_index& viewID) {
    if (assets.empty()) return; 
    size_t assetsCount{};
    size_t loadedAssets{};
    for (const auto& [type, vecPath] : assets) {
        assetsCount += vecPath.size();
        loadedAssets += vecPath.size();
        for (const auto& path : vecPath) {
            auto handle = _DoLoad(type, path, viewID);
            if (!handle)
                loadedAssets--;
        }
    }

    Logger::AddDebug(typeid(AssetManager), "Loaded {}/{} assets", loadedAssets, assetsCount);
}

bool AssetManager::Has(const String& name, const std::type_index viewID) {
	bool global = viewID == typeid(nullptr);

	if (global)
		return m_GlobalAssets.contains(name);
	else {
        return m_ViewAssets.contains(viewID) && m_ViewAssets.at(viewID).contains(name);
	}
}

void AssetManager::ShowImGuiDebugWindow(Renderer* const renderer) {
    if (!m_Context->ContainsAFlags(AppFlags_ImGui)) return;

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
                // LoadShader(buf, Shader::Type::Vertex);
                break;
            case AssetType::FragmentShader:
                // LoadShader(buf, Shader::Type::Fragment);
                break;
            case AssetType::ComputeShader:
                // LoadShader(buf, Shader::Type::Compute);
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
                // LoadShader(buf, Shader::Type::Vertex, activeView);
                break;
            case AssetType::FragmentShader:
                // LoadShader(buf, Shader::Type::Fragment, activeView);
                break;
            case AssetType::ComputeShader:
                // LoadShader(buf, Shader::Type::Compute, activeView);
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
            for (auto it = m_ViewAssets.begin(); it != m_ViewAssets.end();) {
                auto map = it->second;
                for (auto innerIt = map.begin(); innerIt != map.end();) {
                    innerIt->second.Asset.reset();
                    innerIt = map.erase(innerIt);
                }

                it = m_ViewAssets.erase(it);
            }
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
            for (auto it = m_GlobalAssets.begin(); it != m_GlobalAssets.end();) {
                it->second.Asset.reset();
                it = m_GlobalAssets.erase(it);
            }
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
        ShowAssetBrowser(renderer);
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

Base::AssetHandle<void> AssetManager::_DoLoad(std::type_index assetType, const fs::path& path, std::type_index viewID) {
    if (!fs::exists(path)) {
        Logger::AddError(typeid(AssetManager), "Path \"{}\" doesn't exists", path.string());
        return {};
    }
    if (!fs::is_regular_file(path)) {
        Logger::AddError(typeid(AssetManager), "Path \"{}\" is not a regular file", path.string());
        return {};
    }

    auto global = viewID == typeid(nullptr);
    std::string assetTypeName = assetType.name();
    auto assetName = path.stem().string();

    if (global && m_GlobalAssets.contains(assetName)) {
        Logger::AddWarning(typeid(AssetManager), "Asset \"{}\" is already loaded. Retuning existing one", assetName);
        return m_GlobalAssets.at(assetName).Handle;
    }
    else if (!global && !m_ViewAssets.contains(viewID))
        m_ViewAssets[viewID];
    else if (!global && m_ViewAssets.at(viewID).contains(assetName)) {
        Logger::AddWarning(typeid(AssetManager), "Asset \"{}\" is already loaded. Retuning existing one", assetName);
        return m_ViewAssets.at(viewID).at(assetName).Handle;
    }

    if (!m_LoadCallbacks.contains(assetType)) {
        auto msg = std::format("Load failed for file in \"{}\": Theres no callback available for this type -> {}", path.string(), assetTypeName);
        Logger::AddCritical(typeid(AssetManager), msg);
        THROW_RUNTIME_ERROR(msg);
    }
    auto [handle, ptr] = m_LoadCallbacks.at(assetType)(path);
    auto slot = Slot{ handle, ptr, assetType };
    if (global)
        m_GlobalAssets[assetName] = slot;
    else
        m_ViewAssets.at(viewID)[assetName] = slot;
    return handle;
}

void AssetManager::ShowAssetBrowser(Renderer* const renderer) {
    Profiler::StartProfile({ typeid(AssetManager), "ShowAssetBrowser" }, IM_COL32(150, 0, 100, 255));
    std::vector<Slot> totalAssets;
    std::vector<String> assetNames;
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
                    const bool displayLabel = (size.x >= ImGui::CalcTextSize(assetName.ToCString(true)).x);
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
                            drawList->AddImage(texture->GetImGuiTexture(renderer), padBoxMin, padBoxMax);
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
                            drawList->AddText(ImVec2(boxMin.x, boxMax.y - ImGui::GetFontSize()), labelCol, assetName.ToCString(true));
                        }
                        else {
                            auto trunName = assetName.SubString(0, 4);
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
