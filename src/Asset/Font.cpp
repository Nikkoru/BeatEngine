#include "BeatEngine/Asset/Font.h"

Font::operator bool() const {
    return IsValid();
}

Font& Font::operator=(const Font& other) {
    MakeCopy(other);
	return *this;
}

Font& Font::operator=(const Font&& other) noexcept {
    MakeMove(std::move(other));
	return *this;
}
