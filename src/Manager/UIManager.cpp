#include "BeatEngine/Manager/UIManager.h"

#include "BeatEngine/GameContext.h"
#include "BeatEngine/GameState.h"
#include "BeatEngine/Graphics/Color.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/VertexArray.hpp"
#include "BeatEngine/UI/UIElement.h"
#include "BeatEngine/UI/UILayer.h"
#include "BeatEngine/Logger.h"
#include "imgui.h"
#include <format>
#include <memory>

UIManager::UIManager(GameContext* context, GameState* state)
    : m_Context(context), m_State(state) {}

void UIManager::OnEvent(Optional<Base::Event> event) {
	for (const auto& [name, layer] : m_GlobalLayers) {
		layer->OnEvent(event);
	}

	for (const auto& [name, layer] : m_Layers[m_Context->ActiveView]) {
		layer->OnEvent(event);
	}
}

std::shared_ptr<UILayer> UIManager::AddLayer(const std::string layerName, bool global) {
	auto layer = std::make_shared<UILayer>();
	
	if (global) {
        if (m_GlobalLayers.contains(layerName)) {
            Logger::AddWarning(typeid(UIManager), "Layer named \"{}\" already exists, returning existing one", layerName);
            layer = m_GlobalLayers.at(layerName);
        }
        else 
		    m_GlobalLayers[layerName] = layer;
    }
	else {
        if (!m_Layers.contains(m_Context->ActiveView)) {
            Logger::AddDebug(typeid(UIManager), "View \"{}\" doesn't have a entry. Creating and adding layer \"{}\"", m_Context->ActiveView.name(), layerName);
            m_Layers[m_Context->ActiveView].try_emplace(layerName, layer);
        }
        else {
            if (!m_Layers.at(m_Context->ActiveView).contains(layerName)) {
                m_Layers.at(m_Context->ActiveView)[layerName] = layer;
            }
            else {
                Logger::AddWarning(typeid(UIManager), "Layer named \"{}\" already exists, returning existing one", layerName);
                layer = m_Layers.at(m_Context->ActiveView)[layerName];
            }
        }
    }

	return layer;
}

void UIManager::RemoveLayer(const std::string layerName, bool global) {
    if (global) {
        m_GlobalLayers[layerName]->UninitGraphics(m_State->GetGraphicsMgr());
        m_GlobalLayers.erase(layerName);
    }
    else {
        m_Layers[m_Context->ActiveView][layerName]->UninitGraphics(m_State->GetGraphicsMgr());
        m_Layers[m_Context->ActiveView].erase(layerName);
    }
}

void UIManager::RemoveViewLayers(const std::type_index viewID) {
    for (const auto& layer : m_Layers.at(viewID)) {
        layer.second->UninitGraphics(m_State->GetGraphicsMgr());
    }
	m_Layers.erase(viewID);
}

void UIManager::RemoveGlobalLayers() {
    for (const auto& layer : m_GlobalLayers) {
        layer.second->UninitGraphics(m_State->GetGraphicsMgr());
    }
	m_GlobalLayers.clear();
}

void UIManager::RemoveAllLayers() {
    RemoveGlobalLayers();

    for (const auto& [viewID, layerMap] : m_Layers) {
        for (const auto& layer : layerMap) {
            layer.second->UninitGraphics(m_State->GetGraphicsMgr());
        }
    }
	m_Layers.clear();
}

void UIManager::OnDraw() {
	for (const auto& [name, layer] : m_Layers[m_Context->ActiveView]) {
        layer->Draw(m_State->GetGraphicsMgr());
	}
	for (const auto& [name, layer] : m_GlobalLayers) {
        layer->Draw(m_State->GetGraphicsMgr());
	}
}

void UIManager::Update(float dt) {
	for (const auto& [name, layer] : m_GlobalLayers) {
		layer->Update(dt);
	}
	for (const auto& [name, layer] : m_Layers[m_Context->ActiveView]) {
		layer->Update(dt);
	}
}


void UIManager::ShowImGuiDebugWindow() {
    if (!(m_Context->GFlags & GameFlags_ImGui)) return;

    ImGui::Begin("UIManager Debug");
    if (ImGui::BeginTabBar("uiManagerTabBar")) {
        if (ImGui::BeginTabItem("Global layers")) {
            for (const auto& [layerName, layer] : m_GlobalLayers) {
                if (ImGui::TreeNode(layerName.c_str())) {
                    ImGui::Text("Root");
                    auto root = layer->GetRootElement<UIElement>();
                    DrawDebugElement(*root);
                    ImGui::TreePop();
                }
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("View layers")) {
            for (const auto& [viewID, layerMap] : m_Layers) {
                if (ImGui::TreeNode(viewID.name())) {
                    for (const auto& [layerName, layer] : layerMap) {
                        if (ImGui::TreeNode(layerName.c_str())) {
                            ImGui::Text("Root");
                            auto root = layer->GetRootElement<UIElement>();
                            DrawDebugElement(*root);
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();
    ImGui::End();
}

void UIManager::Uninit() {
    if (m_GlobalLayers.empty() && m_Layers.empty()) return;

    // for (const auto& [viewID, layerMap] : m_Layers) {
    //     for (const auto& [layerName, layer] : layerMap) {
    //         Logger::AddDebug(typeid(UIManager), "Uninitializing layer \"{}\" from {}", layerName, viewID.name());
    //         layer->UninitGraphics(m_State->GetGraphicsMgr());
    //     }
    // }
    //
    // for (const auto& [layerName, layer] : m_GlobalLayers) {
    //     Logger::AddDebug(typeid(UIManager), "Uninitializing global layer \"{}\"", layerName);
    //     layer->UninitGraphics(m_State->GetGraphicsMgr());
    // }
}

namespace {
void DrawVertexArrayImGui(VertexArray& array) {
    // ImGui::
} 
}

void UIManager::DrawDebugElement(UIElement& element) {
    ImGui::Text("Type: %s", element.m_ID.name());
    ImGui::Text("Textures: %zu", element.m_Textures.size());
    ImGui::Text("Position: (X: %f Y: %f)", element.m_Position.X, element.m_Position.Y);
    ImGui::Text("Size: (X: %f Y: %f)", element.m_Size.X, element.m_Size.Y);
    ImGui::Text("Is Active:");
    ImGui::SameLine();
    element.m_Active ? ImGui::TextColored({ 0, 255, 0, 255 }, "Yes") : ImGui::TextColored({ 255, 0, 0, 255 }, "No");

    ImGui::Text("Is Hidden:");
    ImGui::SameLine();
    element.m_Hidden ? ImGui::TextColored({ 0, 255, 0, 255 }, "Yes") : ImGui::TextColored({ 255, 0, 0, 255 }, "No");

    if (ImGui::TreeNode("Layout Rect")) {
        auto rect = element.m_LayoutRect;
        ImGui::Text("Position: (X: %f, Y: %f)", rect.GetPosition().X, rect.GetPosition().Y);
        ImGui::Text("Size: (X: %f, Y: %f)", rect.GetSize().X, rect.GetSize().Y);
        auto color = element.m_LayoutRect.GetColor();
        ImGui::Text("Color:");
        ImGui::SameLine();
        ImGui::ColorButton("##smt", { color.R, color.G, color.B, color.A });
        // ImGui::Text("Color: (R: %f, G: %f, B: %f, A: %f)", color.R, color.G, color.B, color.A);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Vertex")) {
        for (size_t i = 0; i < element.m_Vertices.GetSize(); i++) {
            const auto& vertex = element.m_Vertices[i];
            ImGui::Text("Color:");
            ImGui::SameLine();
            auto desc = std::format("Vertex{}", i);
            ImGui::ColorButton(desc.c_str(), { vertex.Color.R, vertex.Color.G, vertex.Color.B, vertex.Color.A });
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Draw data")) {
        element.DrawImGuiDrawData();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Specific from type")) {
        element.SpecificImGuiDebug();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Childs")) {
        for (const auto& [childName, child] : element.m_Childs) {
            if (ImGui::TreeNode(childName.c_str())) {
                DrawDebugElement(*child);
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}
