# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if (POLICY CMP0141)
        cmake_policy(SET CMP0141 NEW)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
    endif()
endif()

project("BeatEngine")

option(BUILD_TESTS "Build / include tests" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(HEADER_SOURCE_DIR "${PROJECT_SOURCE_DIR}/include")
set(HEADER_DEST_DIR "${PROJECT_SOURCE_DIR}/include")

# Include sub-projects.
add_subdirectory("externals")
add_subdirectory("src")

if (BUILD_TESTS)
    add_subdirectory("test")
endif()
