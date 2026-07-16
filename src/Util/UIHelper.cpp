#include "BeatEngine/Util/UIHelper.h"

#include <cmath>

bool UIHelper::CheckCollisionRec(Vector2i point, RectShape rec) {
	auto recPos = rec.GetPosition();
	auto recSize = rec.GetSize();

	return ((point.X >= recPos.X) && (point.X < (recPos.X + recSize.X)) && (point.Y >= recPos.Y) && (point.Y < (recPos.Y + recSize.Y)));
}

bool UIHelper::CheckCollisionRec(Vector2f point, RectShape rec) {
	auto recPos = rec.GetPosition();
	auto recSize = rec.GetSize();

	return ((point.X >= recPos.X) && (point.X < (recPos.X + recSize.X)) && (point.Y >= recPos.Y) && (point.Y < (recPos.Y + recSize.Y)));
}

bool UIHelper::CheckCollisionRec(Vector2u point, RectShape rec) {
	auto recPos = rec.GetPosition();
	auto recSize = rec.GetSize();

	return ((point.X >= recPos.X) && (point.X < (recPos.X + recSize.X)) && (point.Y >= recPos.Y) && (point.Y < (recPos.Y + recSize.Y)));
}

float UIHelper::Pertentage2PixelsX(const float perc, const Vector2f size) {
	return std::floor(static_cast<float>((size.X) * perc));
}

float UIHelper::Pertentage2PixelsY(const float perc, const Vector2f size) {
	return std::floor(static_cast<float>((size.Y) * perc));
}
