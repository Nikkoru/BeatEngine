#include "BeatEngine/Asset/Texture.h"

Texture& Texture::operator=(const Texture& other) {
    MakeCopy(other);
	return *this;
}
Texture& Texture::operator=(const Texture&& other) noexcept {
    MakeMove(std::move(other));
	return *this;
}
