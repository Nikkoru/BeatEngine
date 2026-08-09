#include "BeatEngine/System/String.hpp"

#include <iterator>
#include <string>
#include <string_view>
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
	data.reserve(length);

	try {
        if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
            data = std::u32string(reinterpret_cast<const char32_t*>(s));
        } 
        else {
			const char16_t* start = reinterpret_cast<const char16_t*>(s);
			const char16_t* end = start + length;


			while (start < end) {
				char32_t cp = *start++;
				if (cp >= 0xD800 && cp <= 0xDBFF) {
					if (start < end) {
						char32_t low = *start;
						if (low >= 0xDC00 && low <= 0xDFFF) {
							cp = ((cp - 0xD800) << 10) + (low - 0xDC00) + 0x10000;
							start++;
						}
						else {
							data.clear();
							return;
						}
					}
					else {
						data.clear();
						return;
					}
				}
				data.push_back(cp);
			}
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
                result.push_back(static_cast<wchar_t>((c & 0x3FF) + 0xDC00));
            } 
            else {
                result.push_back('?');
            }
        }
		return result;
    }
}

String::Type CheckNextByteEncoding(std::u32string_view data, size_t startIndex) {
    size_t len = data.length();
    if (startIndex >= len) {
        return String::None;
    }

    bool detectedUTF16 = false;
    bool detectedUTF32 = false;

    for (size_t i = startIndex; i < len; ++i) {
        auto val = data[i];

        if (val > 0x10FFFF)
            return String::None;

        if (val > 0xFFFF)
            detectedUTF32 = true;

        if (val >= 0xD800 && val <= 0xDBFF) {
            if (i + 1 < len) {
                char32_t nextVal = data[i + 1];
                if (nextVal >= 0xDC00 && nextVal <= 0xDFFF) {
                    detectedUTF16 = true;
                    i++;
                }
            }
        }
    }

    if (detectedUTF16)
        return String::UTF16;
    if (detectedUTF32)
        return String::UTF32;

    return String::UTF8;
}

// keep in mind that this also fixes split UTF-8 bytes 
String::Type GetTypeFromString(std::u32string& data) {
	String::Type maxType{ String::Latin1 };
    size_t writeIndex = 0;
    size_t readIndex = 0;
    size_t len = data.length();

	while (readIndex < len) {
        auto curType = CheckNextByteEncoding(data, readIndex);
        auto c = static_cast<unsigned char>(data[readIndex] & 0xFF);

        if (c <= 0x7F && curType == String::UTF8) {
            if (maxType <= String::Latin1)
                maxType = String::Latin1;

            data[writeIndex++] = c;
            readIndex++;
            continue;
        }

        size_t expectedBytes{};
        char32_t codePoint{};
        if ((c & 0xE0) == 0xC0) {       // checks if the sequence is 110xxxxx
            expectedBytes = 2;
            codePoint = c & 0x1F;
        }
        else if ((c & 0xF0) == 0xE0) {  // checks if the sequence is 1110xxxx
            expectedBytes = 3;
            codePoint = c & 0x0F;
        }
        else if ((c & 0xF8) == 0xF0) {   // checks if the sequence is 11110xxx
            expectedBytes = 4;
            codePoint = c & 0x07;
        }
        else {
            if (maxType <= String::Latin1)
                maxType = String::Latin1;

            data[writeIndex++] = c;
            readIndex++;
            continue;
        }

        if (readIndex + expectedBytes > len) {
            while (readIndex < len) {
                data[writeIndex++] = data[readIndex++] & 0xFF;
            }
            break;
        }

        if (curType == String::UTF32) {
            data[writeIndex++] = data[readIndex];
            readIndex += sizeof(char32_t);
        }
        else if (curType == String::UTF16) {
            if (readIndex + 3 < len) {
                bool validSequence = true;
                auto b1 = c;
                auto b2 = static_cast<unsigned char>(data[readIndex + 1] & 0xFF);
                auto b3 = static_cast<unsigned char>(data[readIndex + 2] & 0xFF);
                auto b4 = static_cast<unsigned char>(data[readIndex + 3] & 0xFF);

                uint16_t high = 0;
                uint16_t low = 0;
                bool isPair = false;

                // LE pairs
                if ((b2 >= 0xD8 && b2 <= 0xDB) && (b4 >= 0xDC && b4 <= 0xDF)) {
                    high = (b2 << 8) | b1;
                    low = (b4 << 8) | b3;
                    isPair = true;
                }
                // BE pairs
                else if ((b1 >= 0xD8 && b1 <= 0xDB) && (b3 >= 0xDC && b3 <= 0xDF)) {
                    high = (b1 << 8) | b2;
                    low = (b3 << 8) | b4;
                    isPair = true;
                }

                if (isPair) {
                    codePoint = 0x10000 + ((high - 0xD800) << 10) + (low - 0xDC00);

                    data[writeIndex++] = codePoint;
                    readIndex += 4;
                    continue;
                }
            }

            if (readIndex + 1 < len) {
                auto b1 = c;
                auto b2 = static_cast<unsigned char>(data[readIndex + 1] & 0xFF);

                uint16_t c16 = (b2 << 8) | b1;

                if (!(c16 >= 0xD800 && c16 <= 0xDFFF)) {
                    if (b2 > 0x00) {
                        data[writeIndex++] = c16;
                        readIndex += 2;
                        continue;
                    }
                }
            }

            data[writeIndex++] = static_cast<char32_t>(c);
            readIndex++;
        }
        else if (curType == String::UTF8) {
            bool validSequence = true;

            for (size_t j = 1; j < expectedBytes; ++j) {
                auto nextByte = static_cast<unsigned char>(data[readIndex + j] & 0xFF);
                if ((nextByte & 0xC0) != 0x80) {
                    validSequence = false;
                    break;
                }
                if (codePoint != char32_t{})
                    codePoint = (c << 6) | (nextByte & 0x3F);
                else
                    codePoint = (codePoint << 6) | (nextByte & 0x3F);
            }

            if (validSequence) {
                if (maxType <= String::UTF8)
                    maxType = String::UTF8;
                data[writeIndex++] = codePoint;
                readIndex += expectedBytes;
            }
            else {
                if (maxType <= String::Latin1)
                    maxType = String::Latin1;

                data[writeIndex++] = c;
                readIndex++;
            }
        }

		// if (data[i] < static_cast<char32_t>(UCHAR_MAX) && data[i] > 0 && maxType <= String::Latin1)
		// 	maxType = String::Latin1;
		// else if (data[i] < static_cast<char32_t>(WCHAR_MAX) && data[i] > static_cast<char32_t>(WCHAR_MIN) && maxType <= String::UTF8)
		// 	maxType = String::UTF8;
		// else {
		// 	maxType = String::UTF16;
		// 	break;
		// }
	}

    data.resize(writeIndex);

    return maxType;
}

String::Type GetTypeFromString(const std::wstring& data) {
    std::u32string str32{};
    CopyFromUTF16(str32, data.data(), data.size());
    return GetTypeFromString(str32);
}

String::Type GetTypeFromString(const std::string& data) {
    std::u32string str32{};
    CopyFromUTF8(str32, data.data(), data.size());
    return GetTypeFromString(str32);
}
}

String String::TypeToString(Type type) {
    switch (type) {
    default:
    case None:
        return "None";
    case Latin1:
        return "Latin1";
    case UTF8:
        return "UTF-8";
    case UTF16:
        return "UTF-16";
    case UTF32:
        return "UTF-32";
    }
}

String::String() : m_Data(std::make_shared<StringData>()), m_Type(Type::UTF32) {}

String::String(const String& s) : m_Data(std::make_shared<StringData>(*(s.m_Data))), m_Type(s.m_Type) {
}

String::String(std::string_view s) : String(s.data()) {
}

String::String(std::wstring_view s) : String(s.data()) {
}

String::String(std::u32string_view s) : String(s.data()) {
}

String::String(std::string s) : m_Data(std::make_shared<StringData>()), m_Type(GetTypeFromString(s)) {
	switch (m_Type) {
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

String::String(std::wstring s) : m_Data(std::make_shared<StringData>()), m_Type(GetTypeFromString(s)) {
    CopyFromUTF16(m_Data->Data, s.data(), s.size());
}

String::String(const char* s) : m_Data(std::make_shared<StringData>()), m_Type(GetTypeFromString(s)) {
	switch (m_Type) {
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
String::String(std::u32string s) : m_Data(std::make_shared<StringData>()), m_Type(GetTypeFromString(s)) {
    m_Data->Data = s;
}

String::String(const wchar_t* s) : String(std::wstring(s)) {
}

String::String(const char32_t* s) : m_Data(std::make_shared<StringData>()), m_Type() {
	if (s) {
		m_Data->Data = std::u32string(s);
	}
    m_Type = GetTypeFromString(m_Data->Data);
}

String::String(char c) : m_Type(GetTypeFromString(std::string{c})) {
	switch (m_Type) {
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

String::String(wchar_t c) : m_Data(std::make_shared<StringData>()), m_Type(GetTypeFromString(std::wstring{c})) {
    CopyFromUTF16(m_Data->Data, &c, 1);
}

String::String(char32_t c) : m_Data(std::make_shared<StringData>()), m_Type() {
	m_Data->Data = std::u32string(1, c);
    m_Type = GetTypeFromString(m_Data->Data);
}

bool String::operator==(const String& s) const {
	return m_Data->Data == s.m_Data->Data;
}

bool String::operator!=(const String& s) const {
	return m_Data->Data != s.m_Data->Data;
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

String& String::operator=(const std::string_view& s) {
	String(s).Swap(*this);
	return *this;
}

String& String::operator=(const std::wstring_view& s) {
	String(s).Swap(*this);
	return *this;
}

String& String::operator=(const std::u32string_view& s) {
    String(s).Swap(*this);
    return *this;
}

String& String::operator=(const std::string& s) {
	String(s).Swap(*this);
	return *this;
}

String& String::operator=(const std::wstring& s) {
	String(s).Swap(*this);
	return *this;
}

String& String::operator=(const std::u32string& s) {
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
        // auto data = GetByteData(unicode ? UTF8 : Latin1);
        // m_Data->HoldStringCasting = std::string{ data.data(), data.size() };
        utf8::utf32to8(begin(), end(), std::back_inserter(m_Data->HoldStringCasting));
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
			const auto dstEnd = utf8::utf32to8(begin(), end(), std::back_inserter(v));
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

void String::CheckRealType() noexcept {
	m_Type = GetTypeFromString(m_Data->Data);
}

String operator+(const std::string& lhs, const String& rhs) {
	return String(lhs) + rhs;
}

String operator+(const char* lhs, const String& rhs) {
	return String(lhs) + rhs;
}

std::ostream& operator<<(std::ostream& os, const String& str) {
    os << str.ToString();
    return os;
}

std::wostream& operator<<(std::wostream& os, const String& str) {
    os << str.ToWString();
    return os;
}
