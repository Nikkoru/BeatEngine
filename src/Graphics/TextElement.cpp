#include "BeatEngine/Graphics/TextElement.hpp"
#include "BeatEngine/Base/Asset.h"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/System/Angle.hpp"

#include <SheenBidi/SBCodepointSequence.h>
#include <SheenBidi/SBScriptLocator.h>
#include <SheenBidi/SBAlgorithm.h>

#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/Logger.h"

#include <hb-ft.h>
#include <mutex>

namespace {
// TODO: the line appending can be factorized to a single function to append the vertices as is just a rect vertex array
void addLineHorizontal(
    VertexArray& vertices,
    float lineLeft,
    float lineRight,
    float lineTop,
    LinearColor color,
    float offset,
    float thickness,
    float outlineThickness = 0
) {
    const float top = std::floor(lineTop + offset - (thickness / 2) + 0.5f);
    const float bottom = std::floor(thickness + 0.5f);

    // TODO: use triangle strip instead of triangle list
    vertices.EmplaceBack(Vertex{ {lineLeft + outlineThickness, top + outlineThickness},  { 1.0f, 1.0f }, color });
    vertices.EmplaceBack(Vertex{ {lineRight + outlineThickness, top + outlineThickness},  { 1.0f, 1.0f }, color });
    vertices.EmplaceBack(Vertex{ {lineLeft + outlineThickness, bottom + outlineThickness},  { 1.0f, 1.0f }, color });
    vertices.EmplaceBack(Vertex{ {lineLeft + outlineThickness, bottom + outlineThickness},  { 1.0f, 1.0f }, color });
    vertices.EmplaceBack(Vertex{ {lineRight + outlineThickness, top + outlineThickness},  { 1.0f, 1.0f }, color });
    vertices.EmplaceBack(Vertex{ {lineRight + outlineThickness, bottom + outlineThickness},  { 1.0f, 1.0f }, color });
}

void addLineVertical(
    VertexArray& vertices,
    float lineTop,
    float lineBottom,
    LinearColor color,
    float offset,
    float thickness,
    float outlineThickness = 0
) {
    const float left = std::floor(offset - (thickness / 2) + 0.5f);
    const float right = std::floor(thickness + 0.5f);

    // TODO: use triangle strip instead of triangle list
    vertices.EmplaceBack(Vertex{ {left + outlineThickness, lineTop + outlineThickness},  { 1.0f, 1.0f }, color });
    vertices.EmplaceBack(Vertex{ {right + outlineThickness, lineTop + outlineThickness},  { 1.0f, 1.0f }, color });
    vertices.EmplaceBack(Vertex{ {left + outlineThickness, lineBottom + outlineThickness},  { 1.0f, 1.0f }, color });
    vertices.EmplaceBack(Vertex{ {left + outlineThickness, lineBottom + outlineThickness},  { 1.0f, 1.0f }, color });
    vertices.EmplaceBack(Vertex{ {right + outlineThickness, lineTop + outlineThickness},  { 1.0f, 1.0f }, color });
    vertices.EmplaceBack(Vertex{ {right + outlineThickness, lineBottom + outlineThickness},  { 1.0f, 1.0f }, color });
}

void addGlyphQuad(
    VertexArray& vertices,
    Vector2f position,
    LinearColor color,
    const Glyph& glyph,
    Vector2u size,
    float italicShear
){
    const Vector2f padding{ 1.f, 1.f };

    const Vector2f p1 = glyph.Bounds.Position - padding;
    const Vector2f p2 = glyph.Bounds.Position + glyph.Bounds.Size + padding;

    const Vector2f uv1 = (Vector2f{glyph.TextureRect.Position} - padding) / Vector2f{ size };
    const Vector2f uv2 = (Vector2f{glyph.TextureRect.Position + glyph.TextureRect.Size} - padding) / Vector2f{ size };

    // TODO: use triangle strip instead of triangle list

    // Bottom Left 
    vertices.EmplaceBack(Vertex{ position + Vector2f{p1.X - italicShear * p1.Y, p1.Y}, { uv1.X, uv1.Y }, color });
    // Bottom Right
    vertices.EmplaceBack(Vertex{ position + Vector2f{p2.X - italicShear * p1.Y, p1.Y}, { uv2.X, uv1.Y }, color });
    // Top Left
    vertices.EmplaceBack(Vertex{ position + Vector2f{p1.X - italicShear * p2.Y, p2.Y}, { uv1.X, uv2.Y }, color });
    // Top Left
    vertices.EmplaceBack(Vertex{ position + Vector2f{p1.X - italicShear * p2.Y, p2.Y}, { uv1.X, uv2.Y }, color });
    // Bottom Right
    vertices.EmplaceBack(Vertex{ position + Vector2f{p2.X - italicShear * p1.Y, p1.Y}, { uv2.X, uv1.Y }, color });
    // Top Right
    vertices.EmplaceBack(Vertex{ position + Vector2f{p2.X - italicShear * p2.Y, p2.Y}, { uv2.X, uv2.Y }, color });
}
struct TextSegment
{
    size_t Offset{};
    size_t Length{};
    hb_script_t Script{};
    hb_direction_t Direction{};
};

std::vector<TextSegment> segmentString(const String& input) {
    std::vector<TextSegment> segments;

    const SBCodepointSequence codepointSequence{
        SBStringEncodingUTF32,
        static_cast<const void*>(input.ToC32String()),
        input.size()
    };
    auto* const scriptLocator = SBScriptLocatorCreate();
    const auto* const algorithm = SBAlgorithmCreate(&codepointSequence);
    SBUInteger paragraphOffset{};

    while (paragraphOffset < input.size()) {
        SBUInteger paragrahLength{};
        SBUInteger separatorLength{};
        SBAlgorithmGetParagraphBoundary(
            algorithm, 
            paragraphOffset, 
            std::numeric_limits<SBUInteger>::max(), 
            &paragrahLength, 
            &separatorLength
        );

        if (separatorLength < paragrahLength)
            paragrahLength -= separatorLength;

        const auto* const paragraph = SBAlgorithmCreateParagraph(algorithm, paragraphOffset, paragrahLength, SBLevelDefaultLTR);
        const auto* const line = SBParagraphCreateLine(paragraph, paragraphOffset, paragrahLength);
        const auto runCount = SBLineGetRunCount(line);
        const auto* runArray = SBLineGetRunsPtr(line);

        for (SBUInteger i = 0; i < runCount; i++) {
            const auto direction = (runArray[i].level % 2) ? HB_DIRECTION_RTL : HB_DIRECTION_LTR;

            const SBCodepointSequence codepointSubsequence{
                SBStringEncodingUTF32,
                static_cast<const void*>(input.ToC32String() + runArray[i].offset),
                runArray[i].length
            };

            SBScriptLocatorLoadCodepoints(scriptLocator, &codepointSubsequence);
            while (SBScriptLocatorMoveNext(scriptLocator)) {
                const auto* agent = SBScriptLocatorGetAgent(scriptLocator);
                const auto script = hb_script_from_iso15924_tag(SBScriptGetUnicodeTag(agent->script));

                segments.emplace_back(TextSegment{
                    .Offset = runArray[i].offset + agent->offset,
                    .Length = agent->length,
                    .Script = script,
                    .Direction = direction
                });
            }
            SBScriptLocatorReset(scriptLocator);
        }

        SBLineRelease(line);
        SBParagraphRelease(paragraph);

        paragraphOffset += paragrahLength;
    }
    
    SBAlgorithmRelease(algorithm);
    SBScriptLocatorRelease(scriptLocator);

    return segments;
}
}

struct TextElement::ShaperImpl {
    ShaperImpl(void* fontHandle, uint64_t fontID, unsigned int charSize)
        : FontID(fontID), 
          CharacterSize(charSize), 
          Shaper(hb_ft_font_create(static_cast<FT_Face>(fontHandle), nullptr)),
          ShapingBuffer(hb_buffer_create())
    { hb_ft_font_set_funcs(Shaper.get()); }

    struct ShaperDeleter {
        void operator()(hb_font_t* ptr) const {
            hb_font_destroy(ptr);
        }
    };

    struct ShaperBufferDeleter {
        void operator()(hb_buffer_t* ptr) const {
            hb_buffer_destroy(ptr);
        }
    };

    static std::shared_ptr<ShaperImpl> GetShaper(void* fontHandle, uint64_t fontID, unsigned int charSize) {
        struct ShaperCache {
            std::mutex Mutex;
            std::vector<std::weak_ptr<ShaperImpl>> Cache;
        };

        static ShaperCache shaperCache;
        const std::lock_guard lock(shaperCache.Mutex);

        std::shared_ptr<ShaperImpl> result;

        for (auto it = shaperCache.Cache.begin(); it != shaperCache.Cache.end();) {
            if (auto shaper = it->lock(); !shaper) {
                it = shaperCache.Cache.erase(it);
            }
            else {
                if (shaper->FontID == fontID && shaper->CharacterSize == charSize) {
                    result = std::move(shaper);
                }

                ++it;
            }
        }

        if (!result) {
            result = std::make_shared<ShaperImpl>(fontHandle, fontID, charSize);
            shaperCache.Cache.emplace_back(result);
        }

        return result;

    }
    struct GlyphData {
        uint32_t ID{};
        uint32_t Cluster{};
        Vector2f Offset;
        Vector2f Advance;
        hb_direction_t Direction{};
    };

    std::vector<GlyphData> Shape(
        const String& input,
        const std::vector<uint32_t>& indices,
        hb_script_t script,
        hb_direction_t direction,
        TextOrientation orientation,
        ClusterGrouping ClusterGrouping,
        float outlineThickness,
        uint32_t style
    ) {
        assert(input.size() == indices.size() && "Input string length does not match indices count");

        if (orientation == TextOrientation::TopToBottom) {
            direction = HB_DIRECTION_TTB;
        }
        else if (orientation == TextOrientation::BottomToTop) {
            direction = HB_DIRECTION_BTT;
        }

        const std::lock_guard lock(Mutex);
        auto* buffer = ShapingBuffer.get();

        hb_buffer_clear_contents(buffer);
        hb_buffer_pre_allocate(buffer, static_cast<unsigned int>(input.size()));

        for (auto i = 0u; i < input.size(); i++) {
            hb_buffer_add(buffer, input[i], indices[i]);
        }

        hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);

        hb_buffer_set_script(buffer, script);
        hb_buffer_set_direction(buffer, direction);

        hb_buffer_guess_segment_properties(buffer);

        switch (ClusterGrouping) {
        case ClusterGrouping::Grapheme:
            hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
            break;
        case ClusterGrouping::Character:
            hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
            break;
        case ClusterGrouping::None:
            hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_CHARACTERS);
            break;
        }

        FT_Int32 flags = FT_LOAD_TARGET_NORMAL;
        if (outlineThickness != 0)
            flags |= FT_LOAD_NO_BITMAP;

        hb_ft_font_set_load_flags(Shaper.get(), flags);

        hb_shape(Shaper.get(), buffer, nullptr, 0);

        const auto glyphCount = hb_buffer_get_length(buffer);
        const auto* glyphInfo = hb_buffer_get_glyph_infos(buffer, nullptr);
        const auto* glyphPositions = hb_buffer_get_glyph_positions(buffer, nullptr);

        std::vector<GlyphData> output;
        output.reserve(glyphCount);

        Vector2i scale{};
        hb_font_get_scale(Shaper.get(), &scale.X, &scale.Y);
        const Vector2f divisor{
            static_cast<float>(scale.X) / static_cast<float>(CharacterSize),
            static_cast<float>(scale.Y) / -static_cast<float>(CharacterSize)
        };

        for (auto i = 0u; i < glyphCount; i++) {
            auto& glyphData = output.emplace_back(GlyphData{
                .ID = glyphInfo[i].codepoint,
                .Cluster = glyphInfo->cluster,
                .Offset = { std::round(static_cast<float>((glyphPositions[i].x_offset) / divisor.X)),
                            std::round(static_cast<float>((glyphPositions[i].y_offset) / divisor.Y)) },
                .Advance = { std::round(static_cast<float>((glyphPositions[i].x_advance) / divisor.X)),
                             std::round(static_cast<float>((glyphPositions[i].y_advance) / divisor.Y)), },
                .Direction = hb_buffer_get_direction(buffer)
            });

            if ((style & Bold) != 0) {
                glyphData.Advance.X += (glyphData.Advance.X != 0.f) ? ((glyphData.Advance.X >= 0.f) ? 0.8f : -0.8f) : 0.f;
                glyphData.Advance.Y += (glyphData.Advance.Y != 0.f) ? ((glyphData.Advance.Y >= 0.f) ? 0.8f : -0.8f) : 0.f;
            }
        }

        return output;
    }

    std::mutex Mutex;
    const uint64_t FontID;
    const unsigned int CharacterSize;
    std::unique_ptr<hb_font_t, ShaperDeleter> Shaper;
    std::unique_ptr<hb_buffer_t, ShaperBufferDeleter> ShapingBuffer;
};

TextElement::TextElement(Base::AssetHandle<Font> font, String str, unsigned int charSize)
 : m_Text(str), m_Font(font), m_CharacterSize(charSize) {
    m_PrimitiveType = PrimitiveType::TriangleList;
}

void TextElement::SetString(const String str) {
    if (str == m_Text) return;

    m_Text = str;
    m_UpdateGeometry = true;
}

void TextElement::SetFont(const Base::AssetHandle<Font>& font) {
    if (m_Font && (m_Font.Get()->m_ID == font.Get()->m_ID)) return;

    m_Font = font;
    m_UpdateGeometry = true;
}

void TextElement::SetCharacterSize(unsigned int charSize) {
    if (charSize == m_CharacterSize) return;
        
    m_CharacterSize = charSize;
    m_UpdateGeometry = true;
}

void TextElement::SetLetterSpacing(float spacingFactor) {
    if (spacingFactor == m_LetterSpacingFactor) return;

    m_LetterSpacingFactor = spacingFactor;
    m_UpdateGeometry = true;
}

void TextElement::SetStyle(uint32_t style) {
    if (style == m_Style) return;

    m_Style = style;
    m_UpdateGeometry = true;
}

void TextElement::SetColor(LinearColor color) {
    if (color == m_Color) return;
    m_Color = color;

    if (m_UpdateGeometry) return;
    for (auto& vertex : m_Vertices) {
        vertex.Color = m_Color;
    }
}

void TextElement::SetOutlineColor(LinearColor color) {
    if (color == m_OutlineColor) return;

    m_OutlineColor = color;

    if (m_UpdateGeometry) return;

    for (auto& vertex : m_OutlineVertices) {
        vertex.Color = m_OutlineColor;
    }
}

void TextElement::SetLineAlignment(LineAlignment alignment) {
    if (alignment == m_LineAlignment) return;

    m_LineAlignment = alignment;
    m_UpdateGeometry = true;
}

void TextElement::Draw(GraphicsManager& mgr, RenderState state) {
    if (!m_Font) return;

    UpdateGeometryIfNeed(mgr);

    DrawCommand cmd{
        .projection = mgr.GetMainCamera()->GetProjection(),
        .transform = GetTransform().GetMatrix(),
        .padding = m_Padding.ToGLMVec2(),
        .textureID = m_FontTextureID,
        .shaderID = 0
    };

    state._DrawCommand = std::make_shared<DrawCommand>(cmd);
    state.DrawCommandSize = sizeof(DrawCommand);

    m_OutlineVertices.SetType(PrimitiveType::TriangleList);
    m_Vertices.SetType(PrimitiveType::TriangleList);

    if (m_OutlineVertices.GetSize() > 0)
        mgr.DrawVertices(m_OutlineVertices, state);

    mgr.DrawVertices(m_Vertices, state);
}

void TextElement::UpdateGeometryIfNeed(GraphicsManager& mgr) {
    auto font = m_Font.Get();

    if (!m_UpdateGeometry && font->GetTexture(mgr, m_CharacterSize)->m_CacheID == m_FontTextureID) return;

    m_FontTextureID = font->GetTexture(mgr, m_CharacterSize)->m_CacheID;

    m_UpdateGeometry = false;

    m_Vertices.Clear();
    m_OutlineVertices.Clear();
    m_Glyphs.clear();
    m_Bounds = FloatRect{};

    if (m_Text.IsEmpty()) return;

    const Vector2u textureSize = font->GetTexture(mgr, m_CharacterSize)->m_Size;

    const bool isBold = m_Style & Bold;
    const bool isUnderlined = m_Style & Underlined;
    const bool isStrikeThrough = m_Style & StrikeThrough;

    const float underlineOffset = font->GetUnderlinePosition(m_CharacterSize);
    const float underlineThickness = font->GetUnderlineThickness(m_CharacterSize);

    const float strikeThroughOffset = font->GetGlyph(mgr, U'x', m_CharacterSize, isBold).Bounds.GetCenter().Y;

    const float whitespaceWidth = font->GetGlyph(mgr, U' ', m_CharacterSize, isBold).Advance;
    const float letterSpacing = (whitespaceWidth / 3.0f) * (m_LetterSpacingFactor - 1.0f);
    const float lineSpacing = font->GetLineSpacing(m_CharacterSize) * m_LineSpacingFactor;
    float x = 0.f;
    float y = (m_TextOrientation == TextOrientation::Default) ? static_cast<float>(m_CharacterSize) : 0.f;

    auto minX = static_cast<float>(m_CharacterSize);
    auto minY = static_cast<float>(m_CharacterSize);
    auto maxX = 0.f;
    auto maxY = 0.f;

    const auto fontID = font->m_ID;
    auto* const fontHandle = font->GetFontHandle();

    assert(fontID && fontHandle && "Font not usable for shaping text");

    if (!font->SetFontSize(m_CharacterSize)) {
        THROW_RUNTIME_ERROR("Failed to set font size");
    }

    String currentLine{};
    std::vector<uint32_t> currentLineIndices;
    std::vector<uint32_t> currentLineTabIndices;
    hb_script_t currentScript{};
    hb_direction_t currentDirection{};

    const auto outputLine = [&] {
        if (!m_Shaper || m_Shaper->FontID != fontID || m_Shaper->CharacterSize != m_CharacterSize) {
            m_Shaper = ShaperImpl::GetShaper(fontHandle, fontID, m_CharacterSize);
        }

        const auto shapeOutput = m_Shaper->Shape(
            currentLine, 
            currentLineIndices, 
            currentScript, 
            currentDirection, 
            m_TextOrientation, 
            m_ClusterGrouping, 
            m_OutlineThickness, 
            m_Style & Bold
        );

        auto lineMinX = static_cast<float>(m_CharacterSize);
        auto lineMinY = static_cast<float>(m_CharacterSize);
        auto lineMaxX = 0.f;
        auto lineMaxY = 0.f;

        struct VerticalUnderlineEntry {
            Vector2f Position;
            float Width{};
        };

        std::vector<VerticalUnderlineEntry> verticalUnderlineData;

        if (m_TextOrientation != TextOrientation::Default)
            verticalUnderlineData.reserve(shapeOutput.size());

        auto glyphsToSkip = 0;

        for (const auto& shapeGlyph : shapeOutput) {
            if (glyphsToSkip > 0) {
                glyphsToSkip--;
                continue;
            }

            const Glyph& glyph = font->GetGlyphByID(mgr, shapeGlyph.ID, m_CharacterSize, isBold);

            auto& glyphEntry = m_Glyphs.emplace_back(ShapedGlyph{glyph, {}, {}, {}, {}, {}, {}});
            glyphEntry.Cluster = shapeGlyph.Cluster;
            glyphEntry.Position = Vector2f{x, y} + shapeGlyph.Offset;
            auto isHorizontal = false;
            auto isVertical = false;

            switch (shapeGlyph.Direction) {
            case HB_DIRECTION_LTR:
                glyphEntry.Direction = TextDirection::LeftToRight;
                glyphEntry._Glyph.Advance = shapeGlyph.Advance.X;
                glyphEntry.Baseline = y;
                isHorizontal = true;
                break;
            case HB_DIRECTION_RTL:
                glyphEntry.Direction = TextDirection::RightToLeft;
                glyphEntry._Glyph.Advance = shapeGlyph.Advance.X;
                glyphEntry.Baseline = y;
                isHorizontal = true;
                break;
            case HB_DIRECTION_TTB:
                glyphEntry.Direction = TextDirection::TopToBottom;
                glyphEntry._Glyph.Advance = shapeGlyph.Advance.Y;
                glyphEntry.Baseline = x;
                isVertical = true;
                break;
            case HB_DIRECTION_BTT:
                glyphEntry.Direction = TextDirection::BottomToTop;
                glyphEntry._Glyph.Advance = shapeGlyph.Advance.Y;
                glyphEntry.Baseline = x;
                isVertical = true;
                break;
            default:
                glyphEntry.Direction = TextDirection::Unspecified;
                break;
            }

            uint32_t style = m_Style;
            LinearColor fillColor = m_Color;
            LinearColor outlineColor = m_OutlineColor;
            float outlineThickness = m_OutlineThickness;
            float italicShear = 0.f;

            if (glyph.TextureRect.Size.X != 0 && glyph.TextureRect.Size.Y != 0) {
               if (m_GlyphPrePrecessor)
                   m_GlyphPrePrecessor(glyphEntry, style, fillColor, outlineColor, outlineThickness);

               italicShear = (style & Italic) ? Angle::FromDegrees(12).AsRadians() : 0.f;

               if (outlineThickness != 0) {
                   const Glyph& outlineGlyph = font->GetGlyphByID(mgr, shapeGlyph.ID, m_CharacterSize, style & Bold, outlineThickness);

                   addGlyphQuad(m_OutlineVertices, glyphEntry.Position, outlineColor, outlineGlyph, textureSize, italicShear);
               }

               glyphEntry.VertexOffset = m_Vertices.GetSize();
               const Glyph& fillGlyph = font->GetGlyphByID(mgr, shapeGlyph.ID, m_CharacterSize, style & Bold);
               addGlyphQuad(m_Vertices, glyphEntry.Position, fillColor, fillGlyph, textureSize, italicShear);

               glyphEntry.VertexCount = m_Vertices.GetSize() - glyphEntry.VertexOffset;
            }
            else {
                if ((shapeGlyph.Direction == HB_DIRECTION_TTB) || (shapeGlyph.Direction == HB_DIRECTION_BTT))
                    glyphEntry.Position.X -= shapeGlyph.Offset.X;

                if (std::find(currentLineTabIndices.begin(), currentLineTabIndices.end(), shapeGlyph.Cluster) != currentLineTabIndices.end()) {
                    glyphEntry._Glyph.Advance *= 4.f;

                    if (isHorizontal)
                        glyphEntry._Glyph.Bounds.Size.X = glyphEntry._Glyph.Advance;
                    else if (isVertical)
                        glyphEntry._Glyph.Bounds.Size.Y = glyphEntry._Glyph.Advance;

                    glyphsToSkip = 3;
                }
                else {
                    if (isHorizontal)
                        glyphEntry._Glyph.Bounds.Size = { shapeGlyph.Advance.X + letterSpacing, 0.f };
                    else if (isVertical)
                        glyphEntry._Glyph.Bounds.Size = { 0.f, shapeGlyph.Advance.Y + letterSpacing };
                }
                
                if (m_GlyphPrePrecessor)
                    m_GlyphPrePrecessor(glyphEntry, style, fillColor, outlineColor, outlineThickness);

                italicShear = (style & Italic) ? Angle::FromDegrees(12).AsRadians() : 0.f;
            }

            const Vector2f p1 = glyph.Bounds.Position + shapeGlyph.Offset;
            const Vector2f p2 = p1 + glyphEntry._Glyph.Bounds.Size;

            lineMinX = std::min(lineMinX, x + p1.X - italicShear * p2.Y);
            lineMaxX = std::max(lineMaxX, x + p2.X - italicShear * p1.Y);
            lineMinY = std::min(lineMinY, y + p1.Y);
            lineMaxY = std::max(lineMaxY, y + p2.Y);

            const auto hasAdvance = glyphEntry._Glyph.Advance > 0.f;

            if (isHorizontal)
                x += glyphEntry._Glyph.Advance + (hasAdvance ? letterSpacing : 0.f);
            else if (isVertical) {
                if (isUnderlined && hasAdvance)
                    verticalUnderlineData.emplace_back(
                        VerticalUnderlineEntry{
                            glyphEntry.Position + Vector2f{glyphEntry._Glyph.Bounds.Position.X, 0.f},
                            glyphEntry._Glyph.Bounds.Size.X
                        }
                    );

                y += glyphEntry._Glyph.Advance + (hasAdvance ? letterSpacing : 0.f);
            }
        }

        minX = std::min(minX, lineMinX);
        maxX = std::max(maxX, lineMaxX);
        minY = std::min(minY, lineMinY);
        maxY = std::max(maxY, lineMaxY);

        currentLine.Clear();
        currentLineIndices.clear();
        currentLineTabIndices.clear();

        if (isUnderlined) {
            if (m_TextOrientation == TextOrientation::Default) {
                addLineHorizontal(m_Vertices, 0.f, x, y, m_Color, underlineOffset, underlineThickness);

                if (m_OutlineThickness != 0)
                    addLineHorizontal(m_OutlineVertices, 0.f, x, y, m_OutlineColor, underlineThickness, underlineThickness, m_OutlineThickness);
            }
            else {
                for (const auto& entry : verticalUnderlineData) {
                    addLineHorizontal(
                        m_Vertices,
                        entry.Position.X,
                        entry.Position.X + entry.Width,
                        entry.Position.Y,
                        m_Color,
                        underlineOffset,
                        underlineThickness
                    );

                    if (m_OutlineThickness != 0)
                        addLineHorizontal(
                            m_OutlineVertices, 
                            entry.Position.X,
                            entry.Position.X + entry.Width,
                            entry.Position.Y,
                            m_OutlineColor,
                            underlineOffset,
                            underlineThickness,
                            m_OutlineThickness
                        );
                }
            }
        }

        if (isStrikeThrough) {
            if (m_TextOrientation == TextOrientation::Default) {
                addLineHorizontal(m_Vertices, 0.f, x, y, m_Color, strikeThroughOffset, underlineThickness);

                if (m_OutlineThickness != 0)
                    addLineHorizontal(m_OutlineVertices, 0.f, x, y, m_OutlineColor, strikeThroughOffset, underlineThickness, m_OutlineThickness);
            }
            else {
                addLineVertical(
                    m_Vertices,
                    lineMinY,
                    lineMaxY,
                    m_Color,
                    (m_Style & Italic) ? (lineMaxX - lineMinX) * 0.1f : 0.f,
                    underlineThickness
                );

                if (m_OutlineThickness != 0)
                    addLineVertical(
                        m_OutlineVertices, 
                        lineMinY,
                        lineMaxY,
                        m_OutlineColor,
                        (m_Style & Italic) ? (lineMaxX - lineMinX) * 0.1f : 0.f,
                        underlineThickness,
                        m_OutlineThickness
                    );
            }
        }
    };

    const auto segments = segmentString(m_Text);

    struct LineRecord {
        size_t GlyphsStart{};
        size_t GlyphsCount{};
        size_t VerticesStart{};
        size_t VerticesCount{};
        size_t OutlineVerticesStart{};
        size_t OutlineVerticesCount{};
        size_t FirstCodepointOffset = std::numeric_limits<size_t>::max();
        hb_direction_t Direction{};
        float LineWidth;
    };

    std::vector<LineRecord> lines;

    const auto beginLineRecord = [&] {
        auto& lineRecord = lines.emplace_back(); 
        lineRecord.GlyphsStart = m_Glyphs.size();
        lineRecord.VerticesStart = m_Vertices.GetSize();
        lineRecord.OutlineVerticesStart = m_OutlineVertices.GetSize();
    };

    const auto endLineRecord = [&] {
        auto& lineRecord = lines.back();
        lineRecord.GlyphsCount = m_Glyphs.size() - lineRecord.GlyphsStart;
        lineRecord.VerticesCount = m_Vertices.GetSize() - lineRecord.VerticesStart;
        lineRecord.OutlineVerticesCount = m_OutlineVertices.GetSize() - lineRecord.OutlineVerticesStart;
        lineRecord.LineWidth = x;
    };

    if (!segments.empty())
        beginLineRecord();

    for (const auto& segment : segments) {
        currentScript = segment.Script;
        currentDirection = segment.Direction;

        if (segment.Offset < lines.back().FirstCodepointOffset) {
            lines.back().FirstCodepointOffset = segment.Offset;
            lines.back().Direction = currentDirection;
        }

        for (auto index = static_cast<uint32_t>(segment.Offset); index < static_cast<uint32_t>(segment.Offset + segment.Length); index++) {
            const auto& curChar = m_Text[index];

            if (curChar == U'\n') {
                if (!currentLine.IsEmpty())
                    outputLine();

                auto& glyph = m_Glyphs.emplace_back(
                    ShapedGlyph{font->GetGlyph(mgr, '\n', m_CharacterSize, isBold), {}, {}, {}, {}, {}, {}}
                );

                glyph._Glyph.Bounds.Size = {};
                glyph.Baseline = y;

                if (m_Glyphs.size() == 1) {
                    glyph.Position = { x, y };
                }
                else {
                    const auto& highestClusterGlyph = *std::max_element(
                        m_Glyphs.begin() + static_cast<int>(lines.back().GlyphsStart), m_Glyphs.end(), [](const ShapedGlyph& left, const ShapedGlyph& right) { return left.Cluster < right.Cluster; }
                    );
                    glyph.Position = {
                        highestClusterGlyph.Position.X + (highestClusterGlyph.Direction == TextDirection::RightToLeft ? 0.f : highestClusterGlyph._Glyph.Advance),
                        y
                    };
                }

                glyph.Cluster = index;

                endLineRecord();
                beginLineRecord();

                minX = std::min(minX, x);
                minY = std::min(minY, y);

                y += lineSpacing;
                x = 0;

                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);

                continue;
            }

            if (curChar == U'\t') {
                currentLine += "    ";
                currentLineIndices.resize(currentLineIndices.size() + 4u, index);
                currentLineTabIndices.emplace_back(index);
                continue;
            }

            if ((curChar < 0x80) && std::iscntrl(static_cast<int>(curChar))) {
                continue;
            }

            currentLine += curChar;
            currentLineIndices.emplace_back(index);
        }

        if (!currentLine.IsEmpty())
            outputLine();
    }

    if (!segments.empty())
        endLineRecord();

    std::sort(
        m_Glyphs.begin(),
        m_Glyphs.end(),
        [](const ShapedGlyph& left, const ShapedGlyph& right) { return left.Cluster < right.Cluster; }
    );

    if (m_OutlineThickness != 0) {
        const float outline = std::abs(std::ceil(m_OutlineThickness));
        minX -= outline;
        maxX += outline;
        minY -= outline;
        maxY += outline;
    }

    m_Bounds.Position = { minX, minY };
    m_Bounds.Size = Vector2f{ maxX, maxY } - Vector2f{ minX, minY };

    if (!lines.empty()) {
        const auto maxWidth = std::max_element(
            lines.begin(), lines.end(),
            [](const LineRecord& left, const LineRecord& right) { return left.LineWidth < right.LineWidth; }
        )->LineWidth;

        for (auto& line : lines) {
            auto shift = 0.f;

            if (m_LineAlignment == LineAlignment::Center)
                shift = line.LineWidth / -2.f;
            else if (m_LineAlignment == LineAlignment::Right)
                shift = -line.LineWidth;
            else if ((m_LineAlignment == LineAlignment::Default) && (line.Direction == HB_DIRECTION_RTL))
                shift = maxWidth - line.LineWidth;
            else
                continue;

            for (auto i = line.GlyphsStart; i < line.GlyphsStart + line.GlyphsCount; i++)
                m_Glyphs[i].Position.X += shift;
            for (auto i = line.VerticesStart; i < line.VerticesStart + line.VerticesCount; i++)
                m_Vertices[i].Position.X += shift;
            for (auto i = line.OutlineVerticesStart; i < line.OutlineVerticesStart + line.OutlineVerticesCount; i++)
                m_OutlineVertices[i].Position.X += shift;
        }

        if (m_LineAlignment == LineAlignment::Center)
            m_Bounds.Position.X -= m_Bounds.Size.X / 2.f;
        else if (m_LineAlignment == LineAlignment::Right)
            m_Bounds.Position.X -= m_Bounds.Size.X;
    }
}

