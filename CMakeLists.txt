cmake_minimum_required(VERSION 3.30)

# Enable Hot Reload for MSVC compilers if supported.
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if (POLICY CMP0141)
        cmake_policy(SET CMP0141 NEW)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
    endif()
endif()

project("BeatEngine")

option(BUILD_TESTS "Build & include tests" ON)
option(MAKE_TEST_BUILD "Makes this project a BeatEngine Test Build" OFF)
option(USE_SDL_WINDOW "Include and make the default BeatEngine window SDL" ON)
option(USE_GLFW_WINDOW "Include and make the default BeatEngine window GLFW" OFF)
option(USE_OPENGL_RENDERER "Include and make the default BeatEngine renderer OpenGL" OFF)
option(USE_VULKAN_RENDERER "Include and make the default BeatEngine renderer Vulkan" ON)
option(USE_SFML "Includes and makes the SFML library as default backend for graphics and window for BeatEngine" OFF)
option(BUILD_SHARED_LIBS, "build shared libraries" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(HEADER_SOURCE_DIR "${PROJECT_SOURCE_DIR}/include")
set(HEADER_DEST_DIR "${PROJECT_SOURCE_DIR}/include")

# Include sub-projects.
add_subdirectory("externals")
add_subdirectory("src")
add_subdirectory("include")

add_library(BeatEngine 
	${SOURCES}
	${HEADERS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

if (MAKE_TEST_BUILD)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BEATENGINE_TEST)
endif()

if(USE_SDL_WINDOW AND NOT USE_GLFW_WINDOW)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BEATENGINE_SDL_WINDOW)
    message(STATUS "Using SDL backend.")
elseif(USE_GLFW_WINDOW AND NOT USE_SDL_WINDOW)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BEATENGINE_GLFW_WINDOW)
    message(STATUS "Using GLFW backend.")
elseif(NOT USE_GLFW_WINDOW AND NOT USE_SDL_WINDOW)
    message(STATUS "Not including any window API backend.")
else()
    message(FATAL_ERROR "Conflict between the window API backends. Check what window API backend you want to use and enable it, then disable all the other ones to proceed. You can also disable")
endif()

if(USE_OPENGL_RENDERER AND NOT USE_VULKAN_RENDERER)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BEATENGINE_OPENGL_RENDERER)

    find_package(OpenGL REQUIRED)
    
    target_include_directories(${PROJECT_NAME} PRIVATE ${OPENGL_INCLUDE_DIRS})
    target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_INCLUDE_DIR}")
    target_include_directories(${PROJECT_NAME} PRIVATE "${ImGui_SOURCE_DIR}")

    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            ${OPENGL_LIBRARIES}
            GLAD
            ImGui
    )

    message(STATUS "Using OpenGL APIs")
elseif(USE_VULKAN_RENDERER AND NOT USE_OPENGL_RENDERER OR ON)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BEATENGINE_VULKAN_RENDERER)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_NO_PROTOTYPES)

    find_package(Vulkan REQUIRED volk)

    target_include_directories(${PROJECT_NAME} PRIVATE "${SDL3_SOURCE_DIR}/include")
    target_include_directories(${PROJECT_NAME} PRIVATE "${VMA_SOURCE_DIR}/include")
    target_include_directories(${PROJECT_NAME} PRIVATE "${Vulkan_INCLUDE_DIRS}")
    target_include_directories(${PROJECT_NAME} PRIVATE "${ImGui_SOURCE_DIR}")

    target_link_libraries(${PROJECT_NAME}
        PUBLIC 
            SDL3
            Vulkan::Vulkan
            Vulkan::volk
            VulkanMemoryAllocator
            ImGui
    )
    if(WIN32)
        target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)
    elseif(APPLE)
        target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_MACOS_MVK)
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Metal" "-framework MetalKit" "-framework Cocoa")
    elseif(UNIX AND NOT APPLE)
        target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_XCB_KHR)
    endif()

    message(STATUS "Using Vulkan APIs")
elseif(NOT USE_VULKAN_RENDERER AND NOT USE_OPENGL_RENDERER)
    message(STATUS "Not using Graphics APIs. Remember to implement your own")
else()
    message(FATAL_ERROR "Conflict between the renderer API backends. Check what API renderer you want to use and enable it, then disable all the other ones to proceed.")
endif()

if (USE_SFML AND NOT USE_OPENGL_RENDERER AND NOT USE_VULKAN_RENDERER AND NOT USE_SDL_WINDOW AND NOT USE_GLFW_WINDOW)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        	sfml-graphics
		    sfml-network 
		    ImGui-SFML::ImGui-SFML
    )
    target_include_directories(${PROJECT_NAME} PRIVATE "${SFML_SOURCE_DIR}/include")
elseif(USE_SFML)
    message(FATAL_ERROR "SFML already implements renderer and window. No need to set defaults")
endif()

target_link_libraries(${PROJECT_NAME} 
	PRIVATE 
		portaudio
		miniaudio
		sndfile
		samplerate
        tag
)

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${libsndfile_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${portaudio_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${samplerate_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${miniaudio_SOURCE_DIR}/")

if (BUILD_TESTS)
    add_subdirectory("test")
endif()
