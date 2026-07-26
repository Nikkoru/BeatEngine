#pragma once


#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Base/Asset.h"
#include "BeatEngine/Graphics/Glyph.hpp"
#include "BeatEngine/Graphics/Rect.hpp"
#include "BeatEngine/System/DataStream.hpp"
#include <freetype/freetype.h>
#include <freetype/ftstroke.h>
#include <freetype/ftsystem.h>
#include <memory>
#include <unordered_map>
#include <vector>

class GraphicsManager;
class Font : public Base::Asset {
private:
    friend class TextElement;
    friend class AssetManager;
public:
    struct Row {
        Row (unsigned int rowTop, unsigned int rowHeight) :
            Top(rowTop), Height(rowHeight) {}
        unsigned int Width{};
        unsigned int Top;
        unsigned int Height;
    };
    
    using GlyphTable = std::unordered_map<uint64_t, Glyph>;

    struct Page {
        explicit Page(GraphicsManager& mgr, bool smooth);

        GlyphTable Glyphs;
        std::shared_ptr<Texture> PageTexture;
        unsigned int NextRow{ 3 };
        std::vector<Row> Rows;
    };
    using PageTable = std::unordered_map<unsigned int, Page>;
private:
    using FontHandle = void*;

    FT_Library m_FTLibrary;
    FT_StreamRec m_FTStreamRec;
    FT_Face m_FTFace;
    FT_Stroker m_FTStroker;

    // the id is needed for the ShaperImpl, this because each Shaper is
    // binded to the FT_Face of a determined size or font, and it needs
    // to find a way to identify the font, is only requested internally
    // via TextElement
    uint64_t m_ID;
    std::string m_FamilyName;
    bool m_HasKerning{};
    bool m_HasVerticalMetrics{};

    bool m_IsSmooth{ true };
    bool m_StreamData{ true };
    mutable PageTable m_Pages{};
    mutable std::vector<uint8_t> m_PixelData{};
    DataStream m_Stream{};
public:
	Font() = default;
	Font(const Font& other) { *this = other; }
	Font(const Font&& other) noexcept { *this = std::move(other); }

	Font& operator=(const Font& other);
	Font& operator=(const Font&& other) noexcept;

    bool IsLoaded() const { return m_FTLibrary && m_FTStreamRec.descriptor.pointer && m_FTStroker && m_FTFace; }

    const Glyph& GetGlyphByID(GraphicsManager& mgr, uint32_t charID, unsigned int charSize, bool bold, float outlineThickness = 0) const;
    const Glyph& GetGlyph(GraphicsManager& mgr, char32_t codePoint, unsigned int charSize, bool bold, float outlineThickness = 0) const;

    const std::shared_ptr<Texture> GetTexture(GraphicsManager& mgr, unsigned int charSize) const { return LoadPage(mgr, charSize).PageTexture; }
    float GetUnderlinePosition(unsigned int charSize) const;
    float GetUnderlineThickness(unsigned int charSize) const;
    float GetLineSpacing(unsigned int charSize) const;
private:
    FontHandle GetFontHandle() { return m_FTFace; }

    Glyph LoadGlyph(GraphicsManager& mgr, uint32_t charID, unsigned int charSize, bool bold, float outlineThickness) const;
    Page& LoadPage(GraphicsManager& mgr, unsigned int charSize) const;
    IntRect FindGlyphRect(GraphicsManager& mgr, Page& page, Vector2u size) const; 
    bool SetFontSize(unsigned int size) const;
public:
    // virtual void MakeCopy(const Font& other) { (void)other; } 
    // virtual void MakeMove(const Font&& other) noexcept { (void)other; } 
};

