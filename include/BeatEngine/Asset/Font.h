#pragma once


#include "BeatEngine/Base/Asset.h"

class Font : public Base::Asset {
public:
	Font() = default;
	Font(const Font& other) { *this = other; }
	Font(const Font&& other) noexcept { *this = std::move(other); }

	explicit operator bool() const;

	Font& operator=(const Font& other);
	Font& operator=(const Font&& other) noexcept;

    virtual bool IsValid() const { return false; }
public:
    virtual void MakeCopy(const Font& other) { (void)other; } 
    virtual void MakeMove(const Font&& other) noexcept { (void)other; } 
};
