include(GoogleTest)

function(target_test name source)
    enable_testing()
    add_executable(${name} ${source})

    target_link_libraries(${name}
        PRIVATE
            GTest::gtest_main
            BeatEngine
    )


    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_link_options(${name} PRIVATE "$<$<CONFIG:Debug>:/DEBUG>") 
    endif()

    set_property(TARGET ${name} PROPERTY CXX_STANDARD 23)
    target_compile_features(${name} PRIVATE cxx_std_23)

    target_include_directories(${name} PRIVATE "${HEADER_SOURCE_DIR}")

    gtest_discover_tests(${name})
endfunction()
