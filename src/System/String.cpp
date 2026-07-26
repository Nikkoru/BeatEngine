#include "BeatEngine/System/String.hpp"

#include <utf8.h>
#include <string.h>

namespace {
// Helper functions
void CopyFromLatin1(std::u32string& data, const char* s, size_t length) {
	data.resize(length);

	for (size_t i = 0; i < length; i++) {
		data[i] = static_cast<unsigned char>(s[i]);
	}
}

void CopyFromUTF8(std::u32string& data, const char* s, size_t length) {
	data.resize(length);
	
	try {
		const String::Iterator dstEnd = utf8::utf8to32(s, s + length, data.begin());
		data.resize(dstEnd - data.begin());
	} 
	catch (const utf8::exception& e) {
		const String msg(e.what());
		data.clear();
	}
}

void CopyFromUTF16(std::u32string& data, const wchar_t* s, size_t length) {
    data.resize(length);

	try {
        if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
            data = std::u32string(reinterpret_cast<const char32_t*>(s));
        } 
        else {
            std::string utf8Str{};
            utf8Str.resize(length);
            const auto dstEnd = utf8::utf16to8(s, s + length, utf8Str.begin());
            utf8Str.resize(dstEnd - utf8Str.begin());

            CopyFromUTF8(data, utf8Str.data(), length);
        }
	} 
	catch (const utf8::exception& e) {
		const String msg(e.what());
		data.clear();
	}
}

std::wstring GetFromUTF32(std::u32string& data) {
    if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
        return std::wstring(data.begin(), data.end()); 
    }
    else {
        std::wstring result{};
        result.reserve(data.size());

        for (char32_t c : data) {
            if (c <= 0xFFFF) {
                result.push_back(static_cast<wchar_t>(c));
            }
            // this deconstructs the char32_t data to wchar_t pairs
            else if (c <= 0x10FFFFF) { 
                c -= 0x10000;
                result.push_back(static_cast<wchar_t>((c >> 10) + 0xD800));
                result.push_back(static_cast<wchar_t>((c >> 0x3FF) + 0xDC00));
            } 
            else {
                result.push_back('?');
            }
        }
    }
}
}

String::String() : m_Data(std::make_shared<StringData>()), m_Type(Type::UTF32) {}

String::String(const std::string& s, Type t) : m_Data(std::make_shared<StringData>()), m_Type(t) {
	switch (t) {
	case Latin1:
		CopyFromLatin1(m_Data->Data, s.data(), s.size());
		break;
	case UTF8:
		CopyFromUTF8(m_Data->Data, s.data(), s.size());
		break;
	default:
		m_Data->Data.clear();
		break;
	}
}

String::String(const std::wstring& s) : m_Data(std::make_shared<StringData>()), m_Type(UTF16) {
    CopyFromUTF16(m_Data->Data, s.data(), s.size());
}

String::String(const char* s, Type t) : m_Data(std::make_shared<StringData>()), m_Type(t) {
	switch (t) {
	case Latin1:
		CopyFromLatin1(m_Data->Data, s, strlen(s));
		break;
	case UTF8:
		CopyFromUTF8(m_Data->Data, s, strlen(s));
		break;
	default:
		m_Data->Data.clear();
		break;
	}
}
String::String(const std::u32string& s) : m_Data(std::make_shared<StringData>()), m_Type(UTF32) {
    m_Data->Data = s;
}

String::String(const wchar_t* s) : String(std::wstring(s)) {
}

String::String(char c, Type t) : m_Type(t) {
	switch (t) {
	case Latin1:
		CopyFromLatin1(m_Data->Data, &c, 1);
		break;
	case UTF8:
		CopyFromUTF8(m_Data->Data, &c, 1);
		break;
	default:
		m_Data->Data.clear();
		break;
	}
}

String::String(wchar_t c) : m_Data(std::make_shared<StringData>()), m_Type(UTF16) {
    CopyFromUTF16(m_Data->Data, &c, 1);
}


bool String::operator==(const String& s) const {
	return m_Data == s.m_Data;
}

bool String::operator!=(const String& s) const {
	return m_Data != s.m_Data;
}

bool String::operator==(const std::string& s) const {
    return *this == String{s};
}

bool String::operator!=(const std::string& s) const {
    return *this != String{s};
}

bool String::operator==(const char* s) const {
	if (!s)
		return IsEmpty();

	const char32_t* p = ToC32String();

	while (*p != L'\0' || *s != '\0') {
		if (*p++ != static_cast<unsigned char>(*s++))
			return false;
	}

	return true;
}

bool String::operator!=(const char* s) const {
	return !(*this == s);
}

bool String::operator==(const std::wstring& s) const {
    return *this == String{s};
}

bool String::operator!=(const std::wstring& s) const {
    return *this != String{s};
}

bool String::operator==(const wchar_t* s) const {
	if (!s)
		return IsEmpty();

    std::wstring wStr{s};
    std::u32string str32{};

    CopyFromUTF16(str32, wStr.data(), wStr.size());

	return m_Data->Data == str32;
}

bool String::operator!=(const wchar_t* s) const {
	return !(*this == s);
}

bool String::operator==(const std::u32string& s) const {
    return m_Data->Data == s;
}

bool String::operator!=(const std::u32string& s) const {
    return m_Data->Data != s;
}

bool String::operator==(const char32_t* s) const {
    return m_Data->Data == s;
}

bool String::operator!=(const char32_t* s) const {
    return m_Data->Data != s;
}

String& String::operator+=(const String& s) {
	m_Data->Data += s.m_Data->Data;
	return *this;
}

String& String::operator+=(const char32_t* s) {
    m_Data->Data += s;
    return *this;
}

String& String::operator+=(const wchar_t* s) {
	if (s) {
        std::wstring wStr{s};
        std::u32string str32{};

        CopyFromUTF16(str32, wStr.data(), wStr.size());

		m_Data->Data += str32;
    }
	return *this;
}

String& String::operator+=(const char* s) {
	if (s) {
		for (int i = 0; s[i] != 0; i++) {
			m_Data->Data += static_cast<unsigned char>(s[i]);
		}
	}
	return *this;
}

String& String::operator+=(char32_t c) {
	m_Data->Data += c;
	return *this;
}

String& String::operator+=(wchar_t c) {
    std::u32string str32{};
    CopyFromUTF16(str32, &c, 1);

	m_Data->Data += str32;
	return *this;
}

String& String::operator+=(char c) {
	m_Data->Data += static_cast<unsigned char>(c);
	return *this;
}

String String::operator+(const String& s) {
	return m_Data->Data + s.m_Data->Data;
}

String String::operator+(const wchar_t* s) {
	if (s) {
        std::wstring wStr{s};
        std::u32string str32{};

        CopyFromUTF16(str32, wStr.data(), wStr.size());

		return m_Data->Data + str32;
    }

	return *this;
}

String String::operator+(const char* s) {
	String str = m_Data->Data;
	if (s) {
		for (int i = 0; s[i] != 0; i++) {
			str.m_Data->Data += static_cast<unsigned char>(s[i]);
		}
	}
	return str;
}

String String::operator+(wchar_t c) {
    std::u32string str32{};
    CopyFromUTF16(str32, &c, 1);
	return m_Data->Data + str32;
}

String String::operator+(char c) {
	String str = m_Data->Data;
	str.m_Data->Data += static_cast<unsigned char>(c);
	return str;
}

String& String::operator=(const std::string& s) {
	String(s).Swap(*this);
	return *this;
}

String& String::operator=(const std::wstring& s) {
	String(s).Swap(*this);
	return *this;
}

String& String::operator=(const wchar_t* s) {
	String(s).Swap(*this);
	return *this;
}

String& String::operator=(const char* s) {
	String(s).Swap(*this);
	return *this;
}

String& String::operator=(wchar_t c) {
	String(c).Swap(*this);
	return *this;
}

String& String::operator=(char c) {
	String(c).Swap(*this);
	return *this;
}

String::operator std::string() {
	return ToString(true);
}

String::operator std::wstring() {
	return ToWString();
}

String::operator std::u32string() {
    return m_Data->Data;
}

String::operator const char*() {
	return ToCString(true);
}

String::operator const wchar_t*() {
	return ToCWString();
}

String::operator const char32_t*() {
    return ToC32String();
}

std::string String::ToString(bool unicode) const {
	if (m_Data->HoldStringCasting.empty()) {
		const auto v = GetByteData(unicode ? UTF8 : Latin1);
		m_Data->HoldStringCasting = std::string(v.data(), v.size());
	}
	return m_Data->HoldStringCasting;
}

std::wstring String::ToWString() const {
    if (m_Data->HoldWStringCasting.empty()) {
        m_Data->HoldWStringCasting = GetFromUTF32(m_Data->Data);
    }
    return m_Data->HoldWStringCasting;
}

const char* String::ToCString(bool unicode) const {
	if (m_Data->HoldStringCasting.empty())
		m_Data->HoldStringCasting = ToString(unicode);
	return m_Data->HoldStringCasting.c_str();
}

const wchar_t* String::ToCWString() const {
    if (m_Data->HoldWStringCasting.empty())
        m_Data->HoldWStringCasting = ToWString();
    return m_Data->HoldWStringCasting.c_str();
}

std::vector<char> String::GetByteData(Type t) const {
	std::vector<char> v;
	char* p = nullptr;
	switch (t) {
	case Latin1:
		v.resize(size());
		p = v.data();

		for (char32_t c : *this) {
			*p++ = static_cast<char>(c);
		}
		break;
	case UTF8:
		v.resize(size() * 4);
		try {
			const auto dstEnd = utf8::utf32to8(begin(), end(), v.begin());
			v.resize(static_cast<unsigned int>(dstEnd - v.begin()));
		}
		catch(const utf8::exception &e) {
			const String msg(e.what());
			v.clear();
		}
		break;
	case UTF16:
		v.resize(size() * 2);
		p = v.data();

		*p++ = '\xff';
		*p++ = '\xfe';

		for (char32_t c : *this) {
			*p++ = static_cast<char>(c & 0xff);
			*p++ = static_cast<char>(c >> 8);
		}
		break;
	default:
		break;
	}
	return v;
}

String operator+(const std::string& lhs, const String& rhs) {
	return String(lhs) + rhs;
}

String operator+(const char* lhs, const String& rhs) {
	return String(lhs) + rhs;
}


