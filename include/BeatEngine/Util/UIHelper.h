#pragma once

#include "BeatEngine/Graphics/Vector2.h"
namespace UIHelper {
	// bool CheckCollisionRec(sf::Vector2i point, sf::RectangleShape rec);
	// bool CheckCollisionRec(sf::Vector2f point, sf::RectangleShape rec);
	// bool CheckCollisionRec(sf::Vector2u point, sf::RectangleShape rec);
	//
    float Pertentage2PixelsX(const float perc, const Vector2f size);
    float Pertentage2PixelsY(const float perc, const Vector2f size);
}
