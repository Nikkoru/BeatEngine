#include "BeatEngine/Util/UID.h"

#include <random>

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<uint32_t> dist;

UID::UID() : m_UID(dist(gen)) {}

UID::UID(int) : m_UID(0) {
}

UID& UID::operator=(const uint32_t id) {
	m_UID = id;
	return *this;
}

bool UID::operator==(const UID& other) {
	return other.m_UID == m_UID;
}

UID::operator uint32_t() {
	return m_UID;
}