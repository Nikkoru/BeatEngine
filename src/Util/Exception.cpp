#include "BeatEngine/Util/Exception.h"

#include <sstream>
#include <stdexcept>

Base::Exception::Exception(const std::string& msg, const std::source_location location) :
	std::runtime_error(msg), m_Location(location)
{}

const char* Base::Exception::what() const noexcept {
	std::ostringstream oss;
	oss << "\033[1;31m[Runtime Error]\033[0m " << std::runtime_error::what() << "\n  \033[1;33mLocation:\033[0m "
		<< m_Location.file_name() << ":" << m_Location.line() << ":" << m_Location.column() << "\n  \033[1;36mFunction:\033[0m " << m_Location.function_name() << "\n";

	m_Msg = oss.str();

	return m_Msg.c_str();
}