#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>

namespace UIHelper {
	bool CheckCollisionRec(sf::Vector2i point, sf::RectangleShape rec);
	bool CheckCollisionRec(sf::Vector2f point, sf::RectangleShape rec);
	bool CheckCollisionRec(sf::Vector2u point, sf::RectangleShape rec);

	const float Pertentage2PixelsX(const float perc, const sf::Vector2f size);
	const float Pertentage2PixelsY(const float perc, const sf::Vector2f size);
}