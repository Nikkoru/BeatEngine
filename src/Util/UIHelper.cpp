#include "BeatEngine/Util/UIHelper.h"

#include <cmath>

bool UIHelper::CheckCollisionRec(sf::Vector2i point, sf::RectangleShape rec) {
	auto recPos = rec.getPosition();
	auto recSize = rec.getSize();

	return ((point.x >= recPos.x) && (point.x < (recPos.x + recSize.x)) && (point.y >= recPos.y) && (point.y < (recPos.y + recSize.y)));
}

bool UIHelper::CheckCollisionRec(sf::Vector2f point, sf::RectangleShape rec) {
	auto recPos = rec.getPosition();
	auto recSize = rec.getSize();

	return ((point.x >= recPos.x) && (point.x < (recPos.x + recSize.x)) && (point.y >= recPos.y) && (point.y < (recPos.y + recSize.y)));
}

bool UIHelper::CheckCollisionRec(sf::Vector2u point, sf::RectangleShape rec) {
	auto recPos = rec.getPosition();
	auto recSize = rec.getSize();

	return ((point.x >= recPos.x) && (point.x < (recPos.x + recSize.x)) && (point.y >= recPos.y) && (point.y < (recPos.y + recSize.y)));
}

const float UIHelper::Pertentage2PixelsX(const float perc, const sf::Vector2f size) {
	return std::floor(static_cast<float>((size.x) * perc));
}

const float UIHelper::Pertentage2PixelsY(const float perc, const sf::Vector2f size) {
	return std::floor(static_cast<float>((size.y) * perc));
}
