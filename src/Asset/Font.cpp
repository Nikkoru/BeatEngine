#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Graphics/Glyph.hpp"
#include "BeatEngine/Graphics/Rect.hpp"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Manager/GraphicsManager.h"

#include <freetype/config/integer-types.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftimage.h>
#include <freetype/ftoutln.h>
#include <freetype/ftstroke.h>
#include <freetype/ftsystem.h>
#include <freetype/ftbitmap.h>

namespace {
template <typename T, typename U>
inline T reinterpret(const U& input) {
    T output;
    memcpy(&output, &input, sizeof(U));
    return output;
}
uint64_t combine(float outlineThickness, bool bold, uint32_t index) {
    return (uint64_t{ reinterpret<uint32_t>(outlineThickness)} << 32) | (uint64_t{bold} << 31) | index;
}
}

Font& Font::operator=(const Font& other) {
    this->m_HasVerticalMetrics = other.m_HasVerticalMetrics;
    this->m_HasKerning = other.m_HasKerning;
    this->m_FamilyName = other.m_FamilyName;
    this->m_FTStroker = other.m_FTStroker;
    this->m_FTLibrary = other.m_FTLibrary;
    this->m_FTStreamRec = other.m_FTStreamRec;
    this->m_FTFace = other.m_FTFace;
	return *this;
}

Font& Font::operator=(const Font&& other) noexcept {
    (void)other;
	return *this;
}

float Font::GetUnderlinePosition(unsigned int charSize) const {
    auto face = m_FTFace;

    if (face && SetFontSize(charSize)) {
        if (!FT_IS_SCALABLE(face))
            return static_cast<float>(charSize) / 10.f;

        return -static_cast<float>(FT_MulFix(face->underline_position, face->size->metrics.y_scale)) / float{1 << 6};
    }
    return 0.f;
}

float Font::GetUnderlineThickness(unsigned int charSize) const {
    auto face = m_FTFace;

    if (face && SetFontSize(charSize)) {
        if (!FT_IS_SCALABLE(face))
            return static_cast<float>(charSize) / 14.f;

        return -static_cast<float>(FT_MulFix(face->underline_thickness, face->size->metrics.y_scale)) / float{1 << 6};
    }
    return 0.f;
}

float Font::GetLineSpacing(unsigned int charSize) const {
    FT_Face face = m_FTFace;

    if (face && SetFontSize(charSize)) {
        return static_cast<float>(face->size->metrics.height) / float{1 << 6};
    }
    return 0.1;
}

const Glyph& Font::GetGlyphByID(GraphicsManager& mgr, uint32_t charID, unsigned int charSize, bool bold, float outlineThickness) const {
    GlyphTable& glyphs = LoadPage(mgr, charSize).Glyphs;

    const uint64_t key = combine(outlineThickness, bold, charID);
    
    if (const auto it = glyphs.find(key); it != glyphs.end())
        return it->second;

    const Glyph glyph = LoadGlyph(mgr, charID, charSize, bold, outlineThickness);
    return glyphs.try_emplace(key, glyph).first->second;
}

const Glyph& Font::GetGlyph(GraphicsManager& mgr, char32_t codePoint, unsigned int charSize, bool bold, float outlineThickness) const {
    return GetGlyphByID(mgr, FT_Get_Char_Index(m_FTFace, codePoint), charSize, bold, outlineThickness);
}

Glyph Font::LoadGlyph(GraphicsManager& mgr, uint32_t charID, unsigned int charSize, bool bold, float outlineThickness) const {
    Glyph glyph;

    if (!IsLoaded())
        return glyph;

    auto face = m_FTFace;
    if (!face)
        return glyph;

    if (!SetFontSize(charSize))
        return glyph;

    FT_Int32 flags = FT_LOAD_TARGET_NORMAL;
    if (outlineThickness != 0)
        flags |= FT_LOAD_NO_BITMAP;

    if (FT_Load_Glyph(face, charID, flags) != 0)
        return glyph;
    
    FT_Glyph ftGlyph = nullptr;
    if (FT_Get_Glyph(face->glyph, &ftGlyph) != 0)
        return glyph;

    const FT_Pos weight = 1 << 6;
    const bool outline = (ftGlyph->format == FT_GLYPH_FORMAT_OUTLINE);
    if (outline) {
        if (bold) {
            auto* outlineGlyph = reinterpret_cast<FT_OutlineGlyph>(ftGlyph);
            FT_Outline_Embolden(&outlineGlyph->outline, weight);
        }

        if (outlineThickness != 0) {
            auto stroker = m_FTStroker;
            FT_Stroker_Set(
                stroker, 
                static_cast<FT_Fixed>(outlineThickness * float{ 1 << 6 }),
                FT_STROKER_LINECAP_ROUND,
                FT_STROKER_LINEJOIN_ROUND,
                0
            );
            FT_Glyph_Stroke(&ftGlyph, stroker, true);
        }
    }

    FT_Glyph_To_Bitmap(&ftGlyph, FT_RENDER_MODE_NORMAL, nullptr, 1);
    auto* bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(ftGlyph);
    auto& bitmap = bitmapGlyph->bitmap;

    if (!outline) {
        if (bold)
            FT_Bitmap_Embolden(m_FTLibrary, &bitmap, weight, weight);

        if (outlineThickness != 0) {
            Logger::AddWarning("", "Failed to outline glyph");
        }
    }

    glyph.Advance = static_cast<float>(bitmapGlyph->root.advance.x >> 16);
    if (bold)
        glyph.Advance += static_cast<float>(weight) / float{ 1 << 6 };

    glyph.lbsDelta = static_cast<int>(face->glyph->lsb_delta);
    glyph.rsbDelta = static_cast<int>(face->glyph->rsb_delta);

    Vector2u size{ bitmap.width, bitmap.rows };

    if ((size.X > 0) && (size.Y > 0)) {
        const unsigned int padding = 2;

        size += 2u * Vector2u(padding, padding);

        auto& page = LoadPage(mgr, charSize);

        glyph.TextureRect = FindGlyphRect(mgr, page, size);

        glyph.TextureRect.Position += Vector2i{padding, padding}; 
        glyph.TextureRect.Size -= 2 * Vector2i{padding, padding};
        
        glyph.Bounds.Position = Vector2f(Vector2i(bitmapGlyph->left, -bitmapGlyph->top));
        glyph.Bounds.Size = Vector2f(Vector2u(bitmap.width, bitmap.rows));

        m_PixelData.resize(size_t{size.X} * size_t{size.Y} * 4);

        uint8_t* current = m_PixelData.data();
        uint8_t* end = current + size.X * size.Y * 4;

        while (current != end) {
            (*current++) = 255;
            (*current++) = 255;
            (*current++) = 255;
            (*current++) = 0;
        }

        const uint8_t* pixels = bitmap.buffer;
        if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
            for (unsigned int y = padding; y < size.Y - padding; ++y) {
                for (unsigned int x = padding; x < size.X - padding; ++x) {
                    const size_t index = x + y * size.X;
                    m_PixelData[index * 4 + 3] = ((pixels[(x - padding) / 8]) & (1 << (7 - ((x - padding) % 8)))) ? 255 : 0;
                }
                pixels += bitmap.pitch;
            }
        }
        else {
            for (unsigned int y = padding; y < size.Y - padding; ++y) {
                for (unsigned int x = padding; x < size.X - padding; ++x) {
                    const size_t index = (x + y * size.X) * 4;

                    auto ftX = x - padding;
                    auto ftY = y - padding;

                    // The +3 is for accessing the Alpha channel. The pixel data is a 1D array that every 4 bytes corresponds to a pixel.
                    // Each byte is a channel and its order is RGBA. So, for accessing the Alpha channel we just need to offset the index to 3
                    m_PixelData[index + 3] = pixels[ftX + ftY * bitmap.pitch];
                }
            }
        }

        const auto dest = Vector2u(glyph.TextureRect.Position) - Vector2u(padding, padding);
        const auto updateSize = Vector2u(glyph.TextureRect.Size) + 2u * Vector2u(padding, padding);

        mgr.UpdateTexture(page.PageTexture, m_PixelData.data(), updateSize, dest);
    }

    FT_Done_Glyph(ftGlyph);

    return glyph;
}

Font::Page& Font::LoadPage(GraphicsManager& mgr, unsigned int charSize) const {
    return m_Pages.try_emplace(charSize, mgr, m_IsSmooth).first->second;
}

IntRect Font::FindGlyphRect(GraphicsManager& mgr, Page& page, Vector2u size) const {
    Row* row = nullptr;
    float bestRatio = 0;
    for (auto it = page.Rows.begin(); it != page.Rows.end(); it++) {
        const float ratio = static_cast<float>(size.Y) / static_cast<float>(it->Height);
        
        if (ratio < 0.7f || ratio > 1.f)
            continue;
        if (size.X > page.PageTexture->GetSize().X - it->Width)
            continue;
        if (ratio < bestRatio)
            continue;

        row = &*it;
        bestRatio = ratio;
    }

    if (!row) {
        const unsigned int rowHeight = size.Y + size.Y / 10;
        while ((page.NextRow + rowHeight >= page.PageTexture->GetSize().Y) || (size.X >= page.PageTexture->GetSize().X)) {
            // Current size of the texture isn't sufficient, extent its size by 2
            Vector2u textureSize = page.PageTexture->GetSize();

            if ((textureSize.X * 2 <= mgr.GetMaxTextureSize()) && (textureSize.Y * 2 <= mgr.GetMaxTextureSize())) {
                auto texture = mgr.CreateEmptyTexture(textureSize * 2u);
                mgr.UpdateTexture(texture, page.PageTexture);
                mgr.DestroyTexture(page.PageTexture);
                page.PageTexture = texture;
            }
        }

        page.Rows.emplace_back(page.NextRow, rowHeight);
        page.NextRow += rowHeight;
        row = &page.Rows.back();
    }

    IntRect rect{Rect<unsigned int>({row->Width, row->Top}, size)};

    row->Width += size.X;

    return rect;
}

bool Font::SetFontSize(unsigned int size) const {
    FT_Face face = m_FTFace;
    const FT_UShort currentSize = face->size->metrics.x_ppem;

    if (currentSize != size) {
        const FT_Error result = FT_Set_Pixel_Sizes(face, 0, size);

        if (result == FT_Err_Invalid_Pixel_Size) {
            if (!FT_IS_SCALABLE(face)) {
                Logger::AddWarning("", "Failed to resize font, font is not scalable and the requested size isn't available");
                Logger::AddInfo("Available sizes:");
                for (int i = 0; i < face->num_fixed_sizes; ++i)
                {
                    const long availSize = (face->available_sizes[i].y_ppem + 32) >> 6;
                    Logger::AddInfo("", "   Size {}px", availSize);
                }
            }
        }

        return result == FT_Err_Ok;
    }

    return true;
}

Font::Page::Page(GraphicsManager& mgr, bool smooth) {
    std::array<uint32_t, 4> underlineReserve{ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
    PageTexture = mgr.CreateEmptyTexture({128, 128});

    mgr.UpdateTexture(PageTexture, underlineReserve.data(), { 2, 2 }, { 0, 0 });
}
