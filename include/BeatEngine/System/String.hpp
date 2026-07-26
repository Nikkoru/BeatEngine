#pragma once

#include <string>
#include <vector>
#include <format>
#include <algorithm>
#include <sstream>
#include <memory>


class String {
public:
	using Iterator = std::u32string::iterator;
	using ConstIterator = std::u32string::const_iterator;

	enum Type {
		None,
		Latin1,
		UTF8,
		UTF16,
        UTF32
	};
private:
    struct StringData {
        std::u32string Data{};
        std::wstring HoldWStringCasting{};
        std::string HoldStringCasting{};
    };
	std::shared_ptr<StringData> m_Data{};
	Type m_Type{ None };
public:
	String();
	String(const String& s) = default;
	String(const std::string& s, Type t = UTF8);
	String(const std::wstring& s);
    String(const std::u32string& s);
	String(const char* s, Type t = UTF8);
	String(const wchar_t* s);
    String(const char32_t* s);
	String(char c, Type t = UTF8);
	String(wchar_t c);
    String(char32_t s);

	~String() = default;
public:
    constexpr char32_t& operator[](int i) {
        return m_Data->Data[i];
    }

    constexpr const char32_t& operator[](int i) const {
        return m_Data->Data[i];
    }
	bool operator==(const String& s) const;
	bool operator!=(const String& s) const;
    bool operator==(const std::string& s) const;
    bool operator!=(const std::string& s) const;
	bool operator==(const char* s) const;
	bool operator!=(const char* s) const;
    bool operator==(const std::wstring& s) const;
    bool operator!=(const std::wstring& s) const;
	bool operator==(const wchar_t* s) const;
	bool operator!=(const wchar_t* s) const;
    bool operator==(const std::u32string& s) const;
    bool operator!=(const std::u32string& s) const;
	bool operator==(const char32_t* s) const;
	bool operator!=(const char32_t* s) const;

	String& operator+=(const String& s);
    String& operator+=(const char32_t* s);
	String& operator+=(const wchar_t* s);
	String& operator+=(const char* s);
    String& operator+=(char32_t c);
	String& operator+=(wchar_t c);
	String& operator+=(char c);

	String operator+(const String& s);
	String operator+(const wchar_t* s);
	String operator+(const char* s);
	String operator+(wchar_t c);
	String operator+(char c);

    String& operator=(const String& s) = default;
    String& operator=(const std::string& s);
    String& operator=(const std::wstring& s);
    String& operator=(const std::u32string& s);
    String& operator=(const char32_t* s);
    String& operator=(const wchar_t* s);
    String& operator=(const char* s);
    String& operator=(wchar_t c);
    String& operator=(char c);

	explicit operator std::string();
	explicit operator std::wstring();
    explicit operator std::u32string();
	explicit operator const char*();
	explicit operator const wchar_t*();
    explicit operator const char32_t*();
public:
	std::string ToString(bool unicode = false) const;
	std::wstring ToWString() const;
    std::u32string ToU32String() const { return m_Data->Data; }
	const char* ToCString(bool unicode = false) const;
	const wchar_t* ToCWString() const;
    const char32_t* ToC32String() const { return m_Data->Data.c_str(); }

	std::vector<char> GetByteData(Type t) const;

	constexpr bool IsEmpty() const noexcept { return m_Data->Data.empty(); }
	constexpr bool IsType(Type t) const noexcept { return m_Type == t; }
	constexpr void Clear() noexcept { m_Data->Data.clear(); }
	void Swap(String& s) noexcept { m_Data->Data.swap(s.m_Data->Data); }
	String& Append(const String& s) noexcept { m_Data->Data.append(s.m_Data->Data); return *this; }
public:
	constexpr int find(const String& s, int offset) const noexcept { 
        return static_cast<int>(m_Data->Data.find(s.m_Data->Data, offset)); 
    }
	constexpr int rfind(const String& s, int offset) const noexcept {
        return static_cast<int>(m_Data->Data.rfind(s.m_Data->Data, offset));
    };
    constexpr size_t size() const noexcept { return m_Data->Data.size(); }
    constexpr Iterator begin() noexcept { return m_Data->Data.begin(); }
    constexpr ConstIterator begin() const noexcept { return m_Data->Data.begin(); }
    constexpr ConstIterator cbegin() const noexcept { return m_Data->Data.cbegin(); }
    constexpr Iterator end() noexcept { return m_Data->Data.end(); }
    constexpr ConstIterator end() const noexcept { return m_Data->Data.end(); }
    constexpr ConstIterator cend() const noexcept { return m_Data->Data.cend(); }
};

String operator+(const std::string& lhs, const String& rhs);
String operator+(const char* lhs, const String& rhs);

inline bool operator==(const std::string& lhs, const String& rhs) { return rhs == lhs; };
inline bool operator!=(const std::string& lhs, const String& rhs) { return rhs != lhs; }
inline bool operator==(const char* lhs, const String& rhs) { return rhs == lhs; }
inline bool operator!=(const char* lhs, const String& rhs) { return rhs != lhs; }
inline bool operator==(const std::wstring& lhs, const String& rhs) { return rhs == lhs; }
inline bool operator!=(const std::wstring& lhs, const String& rhs) { return rhs != lhs; }
inline bool operator==(const wchar_t* lhs, const String& rhs) { return rhs == lhs; }
inline bool operator!=(const wchar_t* lhs, const String& rhs) { return rhs != lhs; }
inline bool operator==(const std::u32string& lhs, const String& rhs) { return rhs == lhs; }
inline bool operator!=(const std::u32string& lhs, const String& rhs) { return rhs != lhs; }
inline bool operator==(const char32_t* lhs, const String& rhs) { return rhs == lhs; }
inline bool operator!=(const char32_t* lhs, const String& rhs) { return rhs != lhs; }

namespace std {
	template <>
	struct hash<String> {
		std::size_t operator()(const String& s) const noexcept {
			return std::hash<std::wstring>{}(s.ToWString());
		}
	};

	template <>
	struct formatter<String, char> {
		template<class ParseContext>
		constexpr ParseContext::iterator parse(ParseContext& context) {
			auto it = context.begin();
			if (it != context.end() && *it != '}') {
				throw std::format_error("Invalid format specifier for String");
			}
			return it;
		}

		template<typename FormatContext>
		auto format(const String& s, FormatContext& context) const {
			std::ostringstream out;
			out << s.ToString(true);

			return std::ranges::copy(std::move(out).str(), context.out()).out;
		}
	};
}
