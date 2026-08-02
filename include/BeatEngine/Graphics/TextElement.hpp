#pragma once

#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Base/Asset.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/Glyph.hpp"
#include "BeatEngine/Graphics/Rect.hpp"
#include "BeatEngine/Graphics/Transformable.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/System/String.hpp"

#include <memory>
class TextElement : public GraphicalElement, public Transformable {
public:
    enum Style {
        Regular         = 0,
        Bold            = 1 << 0,
        Italic          = 1 << 1,
        Underlined      = 1 << 2,
        StrikeThrough   = 1 << 3
    };

    enum class LineAlignment {
        Default = 0,
        Left,
        Center,
        Right
    };

    enum class ClusterGrouping {
        Grapheme = 0,
        Character,
        None
    };

    enum class TextDirection {
        Unspecified = 0,
        LeftToRight,
        RightToLeft,
        TopToBottom,
        BottomToTop
    };

    enum class TextOrientation {
        Default,
        TopToBottom,
        BottomToTop
    };

    struct ShapedGlyph {
        Glyph _Glyph;

        uint32_t Cluster;
        Vector2f Position;
        TextDirection Direction;
        float Baseline;

        size_t VertexOffset;
        size_t VertexCount;
    };

    using GlyphPreProcessor = std::function<
        void(const ShapedGlyph& shapedGlyph, std::uint32_t& style, LinearColor& fillColor, LinearColor& outlineColor, float& outlineThickness)>;
private:
    struct ShaperImpl;
private:
    String m_Text{};
    Base::AssetHandle<Font> m_Font{ nullptr };
    uint32_t m_FontTextureID{};
    unsigned int m_CharacterSize{ 30 };
    float m_LetterSpacingFactor{ 1.f };
    float m_LineSpacingFactor{ 1.f };
    float m_OutlineThickness{};
    uint32_t m_Style{ Regular };
    LinearColor m_Color{ LinearColor::White() };
    LinearColor m_OutlineColor{};
    LineAlignment m_LineAlignment{ LineAlignment::Default };
    TextOrientation m_TextOrientation{ TextOrientation::Default };
    ClusterGrouping m_ClusterGrouping{ ClusterGrouping::Character };
    GlyphPreProcessor m_GlyphPrePrecessor;
    mutable FloatRect m_Bounds{};
    mutable bool m_UpdateGeometry{ false };
    mutable std::vector<ShapedGlyph> m_Glyphs;
    mutable std::shared_ptr<ShaperImpl> m_Shaper;
    mutable VertexArray m_OutlineVertices;
public:
    TextElement() : TextElement(Base::AssetHandle<Font>{}) {}
    TextElement(Base::AssetHandle<Font> font, String str = "", unsigned int charSize = 30);
    ~TextElement() override = default;

    void SetString(const String str);
    void SetFont(const Base::AssetHandle<Font>& font);
    void SetCharacterSize(unsigned int charSize);
    void SetLetterSpacing(float spacingFactor);
    void SetStyle(uint32_t style);
    void SetColor(LinearColor color);
    void SetOutlineColor(LinearColor color);
    void SetLineAlignment(LineAlignment alignment);

    String GetText() { return m_Text; }
    Base::AssetHandle<Font>& GetFont() { return m_Font; }
    unsigned int GetCharacterSize() { return m_CharacterSize; }
    float GetLetterSpacing() { return m_LetterSpacingFactor; }
    LinearColor GetColor() { return m_Color; }
    LinearColor GetOutlineColor() { return m_OutlineColor; }
    LineAlignment GetLineAlignment() { return m_LineAlignment; }

    void Draw(GraphicsManager& mgr, RenderState state = RenderState::Default) override;
    // void UninitGraphics(GraphicsManager& mgr) override;
private:
    void UpdateGeometryIfNeed(GraphicsManager& mgr);
};
