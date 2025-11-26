#pragma once

#include <cstdint>

enum class AssetType : uint8_t {
	Texture = 0,
	Sound,
	AudioStream,
	Font
};