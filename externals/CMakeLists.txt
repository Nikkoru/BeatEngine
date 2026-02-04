set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include (FetchContent)

# Fetch SFML (ON REMOVAL)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.x
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

# Fetch SDL
FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG a962f40bbba175e9716557a25d5d7965f134a3d3
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SDL3)

# Find Vulkan
find_package(slang CONFIG HINTS "$ENV{VULKAN_SDK}/lib/cmake" REQUIRED)

# Fetch ImGui
FetchContent_Declare(ImGui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG v1.91.1-docking
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(ImGui)
FetchContent_GetProperties(ImGui SOURCE_DIR IMGUI_DIR)

# Fetch ImGui-SFML
set(IMGUI_SFML_FIND_SFML OFF)
FetchContent_Declare(ImGui-SFML
        GIT_REPOSITORY https://github.com/SFML/imgui-sfml
        GIT_TAG v3.0
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(ImGui-SFML)

# Fetch miniaudio
FetchContent_Declare(miniaudio
        GIT_REPOSITORY https://github.com/mackron/miniaudio
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(miniaudio)

# Fetch PortAudio
FetchContent_Declare(portaudio
        GIT_REPOSITORY https://github.com/PortAudio/portaudio
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(portaudio)

# Making ImGuiMultiContextCompositor
# add_subdirectory(imgui-multicontext-compositor)

FetchContent_Declare(samplerate
    GIT_REPOSITORY https://github.com/libsndfile/libsamplerate
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(samplerate)

set(BUILD_TESTING OFF)
FetchContent_Declare(libsndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(libsndfile)

# Fetch TagLib

FetchContent_Declare(taglib
    GIT_REPOSITORY https://github.com/taglib/taglib.git
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(taglib)
