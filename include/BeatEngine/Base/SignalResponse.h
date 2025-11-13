#pragma once

#include <cstdint>
#include <string>

enum class SignalResult : uint8_t {
	OK,
	Failed
};

class SignalResponse {
public:
	const SignalResult Result;
	const std::string Message;
public:
	SignalResponse(SignalResult result, const std::string& message)
		: Result(result), Message(message) {}
	SignalResponse(SignalResult result)
		: Result(result), Message("") {
	}
};