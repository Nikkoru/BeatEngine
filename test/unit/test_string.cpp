#include <gtest/gtest.h>

#define private public
#define protected public
#include <BeatEngine/System/String.hpp>

TEST(StringTest, HandleEmpty) {
    String strFromNarrow{ "" };
	String strFromWide{ L"" };
	String strFromU32{ U"" };

    EXPECT_TRUE(strFromNarrow.IsType(String::Latin1));
    EXPECT_TRUE(strFromWide.IsType(String::Latin1));
    EXPECT_TRUE(strFromU32.IsType(String::Latin1));

	EXPECT_STREQ(strFromNarrow.ToCString(), "") << "Failed to transform to const char*";
	EXPECT_STREQ(strFromWide.ToCWString(), L"") << "Failed to transform to const wchar_t*";
	EXPECT_EQ(strFromNarrow.ToString(), std::string{ "" }) << "Failed to transform to std::string";
	EXPECT_EQ(strFromWide.ToWString(), std::wstring{ L"" }) << "Failed to transform to std::wstring";
	EXPECT_EQ(strFromU32.ToC32String(), std::u32string{ U"" }) << "Failed to transform to std::u32string";
}

TEST(StringTest, HandleEscapeCodes) {
    String strFromNarrowNewLine{ "\n" };
	String strFromWideNewLine{ L"\n" };
	String strFromU32NewLine{ U"\n" };

    String strFromNarrowColor{ "\033[0;31m" };
	String strFromWideColor{ L"\033[0;31m" };
	String strFromU32Color{ U"\033[0;31m" };

    EXPECT_TRUE(strFromNarrowNewLine.IsType(String::Latin1)) << "Check of real type failed from Narrow using Latin1 escape characters, reported type: " << String::TypeToString(strFromNarrowNewLine.m_Type);
    EXPECT_TRUE(strFromWideNewLine.IsType(String::Latin1)) << "Check of real type failed from Narrow using Latin1 escape characters, reported type: " << String::TypeToString(strFromWideNewLine.m_Type);;
    EXPECT_TRUE(strFromU32NewLine.IsType(String::Latin1)) << "Check of real type failed from UTF-32 using Latin1 escape characters, reported type: " << String::TypeToString(strFromU32NewLine.m_Type);;

    EXPECT_TRUE(strFromNarrowColor.IsType(String::Latin1)) << "Check of real type failed from Narrow using Latin1 escape characters, reported type: " << String::TypeToString(strFromNarrowColor.m_Type);
    EXPECT_TRUE(strFromWideColor.IsType(String::Latin1)) << "Check of real type failed from Narrow using Latin1 escape characters, reported type: " << String::TypeToString(strFromWideColor.m_Type);;
    EXPECT_TRUE(strFromU32Color.IsType(String::Latin1)) << "Check of real type failed from UTF-32 using Latin1 escape characters, reported type: " << String::TypeToString(strFromU32Color.m_Type);;

    EXPECT_EQ(strFromNarrowNewLine.ToString(), "\n") << "Failed to hold '\\n' escape code (Narrow)";
    EXPECT_EQ(strFromWideNewLine.ToString(), "\n") << "Failed to hold '\\n' escape code (Wide)";
    EXPECT_EQ(strFromU32NewLine.ToString(), "\n") << "Failed to hold '\\n' escape code (UTF-32)";

    EXPECT_EQ(strFromNarrowColor.ToString(), "\033[0;31m") << "Failed to hold '\\033' / '\\x1b' escape code (Narrow)";
    EXPECT_EQ(strFromWideColor.ToString(), "\033[0;31m") << "Failed to hold '\\033' / '\\x1b' escape code (Narrow)";
    EXPECT_EQ(strFromU32Color.ToString(), "\033[0;31m") << "Failed to hold '\\033' / '\\x1b' escape code (Narrow)";
}

TEST(StringTest, HandleRealType) {
    {
        String strFromNarrow{ "abc123" };
        String strFromWide{ L"abc123" };

        EXPECT_TRUE(strFromNarrow.IsType(String::Latin1)) << "Check of real type failed from Narrow using Latin1 characters, reported type: " << String::TypeToString(strFromNarrow.m_Type);
        EXPECT_TRUE(strFromWide.IsType(String::Latin1)) << "Check of real type failed from Wide using Latin1 characters, reported type: " << String::TypeToString(strFromNarrow.m_Type);
    }

    {
        String strFromNarrow{ "日本語",};
        String strFromWide{ L"日本語" };

        EXPECT_TRUE(strFromNarrow.IsType(String::UTF8)) << "Check of real type failed from Latin1 using UTF-8 characters, reported type: " << String::TypeToString(strFromNarrow.m_Type);
        EXPECT_TRUE(strFromWide.IsType(String::UTF8)) << "Check of real type failed from UTF-16 using UTF-8 characters, reported type: " << String::TypeToString(strFromWide.m_Type);
    }
}

TEST(StringTest, HandleLatin) {
    String strFromNarrow{ "abc123" };
    String strFromWide{ L"abc123" };

    String strWithGrave{ "Pokémon" };

    EXPECT_TRUE(strFromNarrow.IsType(String::Latin1));
    EXPECT_TRUE(strFromWide.IsType(String::Latin1));
    EXPECT_TRUE(strWithGrave.IsType(String::Latin1));

    EXPECT_STREQ(strFromNarrow.ToCString(), "abc123") << "Failed to transform to const char* (Narrow to Narrow)";
    EXPECT_STREQ(strFromWide.ToCString(), "abc123") << "Failed to transform to const char* (Wide to Narrow)";
    EXPECT_STREQ(strWithGrave.ToCString(), "Pokémon");
    EXPECT_EQ(strFromNarrow.ToString(), std::string{ "abc123" }) << "Failed to transform to std::string (Narrow to Narrow)";
    EXPECT_EQ(strFromWide.ToString(), std::string{ "abc123" }) << "Failed to transform to std::string (Wide to Narrow)";
    EXPECT_EQ(strWithGrave.ToString(), std::string{ "Pokémon" });
}

TEST(StringTest, HandleUTF8) {
    String strFromNarrowLA{ "abc123" };
    String strFromNarrowUN{ "日本語" };
	String strWithGraveNarrow{ "é" };
    String strFromWideLA{ L"abc123" };
    String strFromWideUN{ L"日本語" };
	String strWithGraveWide{ L"é" };

    EXPECT_TRUE(strFromNarrowLA.IsType(String::Latin1)) << "strFromNarrowLA is not Latin1, reported type: " << String::TypeToString(strFromNarrowLA.m_Type);
    EXPECT_TRUE(strFromNarrowUN.IsType(String::UTF8)) << "strFromNarrowUN is not UTF8, reported type: " << String::TypeToString(strFromNarrowUN.m_Type);
    EXPECT_TRUE(strWithGraveNarrow.IsType(String::Latin1)) << "strWithGraveNarrow is not Latin1, reported type: " << String::TypeToString(strWithGraveNarrow.m_Type);
    EXPECT_TRUE(strFromWideLA.IsType(String::Latin1)) << "strFromWideLA is not Latin1, reported type: " << String::TypeToString(strFromWideLA.m_Type);
    EXPECT_TRUE(strFromWideUN.IsType(String::UTF8)) << "strFromWideUN is not UTF8, reported type: " << String::TypeToString(strFromWideUN.m_Type);
    EXPECT_TRUE(strWithGraveWide.IsType(String::Latin1)) << "strWithGraveWide is not Latin1, reported type: " << String::TypeToString(strWithGraveWide.m_Type);

    EXPECT_STREQ(strFromNarrowLA.ToCString(true), "abc123") << "Failed to transform to const char* using Latin1 (ASCII) characters (Narrow to UTF-8 Wide)";
    EXPECT_EQ(strFromNarrowLA.ToString(true), std::string{ "abc123" }) << "Failed to transform to std::string using Latin1 (ASCII) characters (Narrow to UTF-8 Wide)";
    EXPECT_STREQ(strWithGraveNarrow.ToCString(true), "é") << "Failed to transform to const char* using Latin1 (ASCII) characters (Narrow to UTF-8 Wide)";
    EXPECT_EQ(strWithGraveNarrow.ToString(true), std::string{ "é" }) << "Failed to transform to std::string using Latin1 (ASCII) characters (Narrow to UTF-8 Wide)";

    EXPECT_STREQ(strFromNarrowUN.ToCString(true), "日本語") << "Failed to transform to const char* using UTF-8 (wide) characters (Narrow to UTF-8 Wide)";
    EXPECT_EQ(strFromNarrowUN.ToString(true), std::string{ "日本語" }) << "Failed to transform to std::string using UTF-8 (wide) characters (Narrow to UTF-8 Wide)";

    EXPECT_STREQ(strFromWideLA.ToCString(true), "abc123") << "Failed to transform to const char* using Latin1 (ASCII) characters (Wide to UTF-8 Wide)";
    EXPECT_EQ(strFromWideLA.ToString(true), std::string{ "abc123" }) << "Failed to transform to std::string using Latin1 (ASCII) characters (Wide to UTF-8 Wide)";
    EXPECT_STREQ(strFromWideUN.ToCString(true), "日本語") << "Failed to transform to const char* using UTF-8 (wide) characters (Wide to UTF-8 Wide)";
    EXPECT_EQ(strFromWideUN.ToString(true), std::string{ "日本語" }) << "Failed to transform to std::string using UTF-8 (wide) characters (Wide to UTF-8 Wide)";
    EXPECT_STREQ(strWithGraveWide.ToCString(true), "é") << "Failed to transform to const char* using Latin1 (ASCII) characters (Wide to UTF-8 Wide)";
    EXPECT_EQ(strWithGraveWide.ToString(true), std::string{ "é" }) << "Failed to transform to std::string using Latin1 (ASCII) characters (Wide to UTF-8 Wide)";
}

TEST(StringTest, HandleUTF16) {
    String strFromNarrowLA{ "abc123" };
    String strFromNarrowUN{ "日本語" };
    String strFromWideLA{ L"abc123" };
    String strFromWideUN{ L"日本語" };

    EXPECT_TRUE(strFromNarrowLA.IsType(String::Latin1)) << "strFromNarrowLA is not Latin1, reported type: " << String::TypeToString(strFromNarrowLA.m_Type);
    EXPECT_TRUE(strFromNarrowUN.IsType(String::UTF8)) << "strFromNarrowUN is not UTF8, reported type: " << String::TypeToString(strFromNarrowUN.m_Type);
    EXPECT_TRUE(strFromWideLA.IsType(String::Latin1)) << "strFromWideLA is not Latin1, reported type: " << String::TypeToString(strFromWideLA.m_Type);
    EXPECT_TRUE(strFromNarrowUN.IsType(String::UTF8)) << "strFromWideUN is not UTF8, reported type: " << String::TypeToString(strFromWideUN.m_Type);

    EXPECT_STREQ(strFromNarrowLA.ToCWString(), L"abc123") << "Failed to transform to const wchar_t* using Latin1 (ASCII) characters (Narrow to Wide)";
    EXPECT_EQ(strFromNarrowLA.ToWString(), std::wstring{ L"abc123" }) << "Failed to transform to std::wstring using Latin1 (ASCII) characters (Narrow to Wide)";
    EXPECT_STREQ(strFromNarrowUN.ToCWString(), L"日本語") << "Failed to transform to const wchar_t* using UTF-8 (wide) characters (Narrow to Wide)";
    EXPECT_EQ(strFromNarrowUN.ToWString(), std::wstring{ L"日本語" }) << "Failed to transform to std::wstring using UTF-8 (wide) characters (Narrow to Wide)";

    EXPECT_STREQ(strFromWideLA.ToCWString(), L"abc123") << "Failed to transform to const wchar_t* using Latin1 (ASCII) characters (Wide to Wide)";
    EXPECT_EQ(strFromWideLA.ToWString(), std::wstring{ L"abc123" }) << "Failed to transform to std::wstring using Latin1 (ASCII) characters (Wide to Wide)";
    EXPECT_STREQ(strFromWideUN.ToCWString(), L"日本語") << "Failed to transform to const wchar_t* using UTF-8 (wide) characters (Wide to Wide)";
    EXPECT_EQ(strFromWideUN.ToWString(), std::wstring{ L"日本語" }) << "Failed to transform to std::wstring using UTF-8 (wide) characters (Wide to Wide)";
}
