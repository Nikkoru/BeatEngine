#pragma once

#include "BeatEngine/Graphics/RectShape.hpp"
#include "BeatEngine/Graphics/Vector2.h"
namespace UIHelper {
	bool CheckCollisionRec(Vector2i point, const RectShape& rec);
	bool CheckCollisionRec(Vector2f point, const RectShape& rec);
	bool CheckCollisionRec(Vector2u point, const RectShape& rec);
	//
    float Pertentage2PixelsX(const float perc, const Vector2f size);
    float Pertentage2PixelsY(const float perc, const Vector2f size);
}
