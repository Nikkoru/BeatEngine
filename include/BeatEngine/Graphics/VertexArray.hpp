#pragma once

#include "BeatEngine/Graphics/PrimitiveType.hpp"
#include "BeatEngine/Graphics/Vertex.hpp"
#include <vector>

/// @brief Wrapper class to `std::vector<Vertex>` with some additional data
///
/// This class is essentially for redusing arguments for functions that also needs
/// `PrimitiveType` and additional data such as ID and initialized status required by the
/// renderer for retrieving needed data that the renderer holds for rendering the `VertexArray`
class VertexArray {
private:
    friend class Renderer;
    std::vector<Vertex> m_Vertices;
    PrimitiveType m_Type{ PrimitiveType::TriangleList };
    // this is meant to be used by the renderer, in the case of vulkan it needs to know what
    // element is this, so it can use the correct buffers binded to the id
    uint32_t m_ID{};
    // this is used for the highlights (border) of this vertex
    // if this id is the max of uint32_t that means that there is no highlights
    // if the VertexArray is a highlight, then this id is encoded, which that encode contains the source VertexArray ID
    // and the internal arrayID for the renderer
    uint32_t m_HighlightID{ (std::numeric_limits<uint32_t>::max)() };
    // also we need to know if it is already initialized so we can know if the requested id
    // needs to be freed or not, this overrides whatever it was in that id without questions
    bool m_Initialized{ false };
    bool m_IsHighlight{ false };
private:
    void SetHighlightID(uint32_t sourceID, uint32_t arrayID = 0) { m_IsHighlight ? m_HighlightID = ((sourceID << 22u) | (arrayID << 12u)) : m_HighlightID = sourceID; }
    uint32_t GetSourceID() { return m_HighlightID >> 22u; }
    uint32_t GetArrayID() { return (m_HighlightID >> 12u) & 0x3FFu; }
public:
    using Iterator = std::vector<Vertex>::iterator;
    using ConstIterator = std::vector<Vertex>::const_iterator;
    using ReverseIterator = std::vector<Vertex>::reverse_iterator;
    using ConstReverseIterator = std::vector<Vertex>::const_reverse_iterator;
public:
    Vertex& operator[] (size_t i) { return At(i); };
    const Vertex& operator[] (size_t i) const { return At(i); };

    Vertex& At(size_t i) { return m_Vertices.at(i); }
    const Vertex& At(size_t i) const { return m_Vertices.at(i); }
    void PushBack(Vertex vertex) { m_Vertices.push_back(vertex); }
    void EmplaceBack(Vertex vertex) { m_Vertices.emplace_back(vertex); }

    Iterator Erase(ConstIterator pos) { return m_Vertices.erase(pos); }
    Iterator Insert(ConstIterator pos, const Vertex& vertex) { return m_Vertices.insert(pos, vertex); }

    size_t GetSize() const { return m_Vertices.size(); }

    void Resize(size_t size) { m_Vertices.resize(size); }
    void Reserve(size_t size) { m_Vertices.reserve(size); }

    void Clear() { m_Vertices.clear(); }
    bool IsEmpty() { return m_Vertices.empty(); }

    void Replace(Vertex& vertex, size_t pos) { if (pos >= GetSize()) return; m_Vertices.at(pos) = vertex; }

    Vertex* GetData() { return m_Vertices.data(); }

    Iterator begin() { return m_Vertices.begin(); }
    ConstIterator cbegin() const { return m_Vertices.cbegin(); }
    ReverseIterator rbegin() { return m_Vertices.rbegin(); }
    ConstReverseIterator crbegin() { return m_Vertices.crbegin(); }

    Iterator end() { return m_Vertices.end(); }
    ConstIterator cend() const { return m_Vertices.cend(); }
    ReverseIterator rend() { return m_Vertices.rend(); }
    ConstReverseIterator crend() const { return m_Vertices.crend(); }
public:
    void SetType(PrimitiveType type) { m_Type = type; }

    PrimitiveType GetType() { return m_Type; }
};
