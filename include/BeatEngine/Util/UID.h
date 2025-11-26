#pragma once
#include <cstdint>
	
class UID {
public:
	UID();
	UID(int);

	UID& operator=(const uint32_t id);

	bool operator==(const UID& other);

	operator uint32_t();
private:
	uint32_t m_UID;

};