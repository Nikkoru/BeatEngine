#pragma once

#include "BeatEngine/Graphics/RectShape.hpp"
#include "BeatEngine/Graphics/Vector2.h"
namespace UIHelper {
	bool CheckCollisionRec(Vector2i point, RectShape rec);
	bool CheckCollisionRec(Vector2f point, RectShape rec);
	bool CheckCollisionRec(Vector2u point, RectShape rec);
	//
    float Pertentage2PixelsX(const float perc, const Vector2f size);
    float Pertentage2PixelsY(const float perc, const Vector2f size);
}
