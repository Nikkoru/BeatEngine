#pragma once

#include <stdexcept>
#include <string>
#include <source_location>

namespace Base {
	class Exception : public std::runtime_error {
	private:
		mutable std::string m_Msg;

		const std::source_location m_Location;
	public:
		Exception(const std::string& msg, const std::source_location location);

		const char* what() const noexcept override;
	};
} // namespace Base

#define THROW_RUNTIME_ERROR(msg) throw Base::Exception((msg), std::source_location::current())