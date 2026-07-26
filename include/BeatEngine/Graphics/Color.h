#pragma once

#include <cstdint>
struct RGBColor {
    uint8_t R{};
    uint8_t G{};
    uint8_t B{};
    uint8_t A{ 255 };
    RGBColor(uint8_t r, uint8_t g,uint8_t b ,uint8_t a = 255) :
        R(r), G(g), B(b), A(a) {}
    static RGBColor White() { return { 255, 255, 255 }; }
    static RGBColor Black() { return { 0, 0, 0}; }
};

struct LinearColor {
    float R{};
    float G{};
    float B{};
    float A{ 1.f };

    LinearColor(float r, float g, float b , float a = 1.f) :
        R(r), G(g), B(b), A(a) {}
    LinearColor(RGBColor color) : 
        R(static_cast<float>(color.R) / 255), G(static_cast<float>(color.G) / 255), B(static_cast<float>(color.B) / 255), A(static_cast<float>(color.A) / 255) {}
    LinearColor() : LinearColor(0, 0, 0) {}
    static LinearColor FromRGB(RGBColor color); 

    static LinearColor White() { return { 1.f, 1.f, 1.f }; }
    static LinearColor Black() { return { .0f, .0f, .0f}; }

    bool operator<=>(const LinearColor& other) const = default;
};
