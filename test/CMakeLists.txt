add_subdirectory(BeatEngineTest)

set(MAKE_TEST_BUILD ON PARENT_SCOPE)
enable_testing()
   
add_executable(StringTest
    test_string.cpp
)

target_link_libraries(StringTest
    PRIVATE
        GTest::gtest_main
        BeatEngine
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_link_options(StringTest PRIVATE "$<$<CONFIG:Debug>:/DEBUG>") 
endif()

set_property(TARGET StringTest PROPERTY CXX_STANDARD 23)
target_compile_features(StringTest PRIVATE cxx_std_23)

target_include_directories(StringTest PRIVATE "${HEADER_SOURCE_DIR}")

include(GoogleTest)
gtest_discover_tests(StringTest)
