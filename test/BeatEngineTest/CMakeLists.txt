# set(MAKE_TEST_BUILD ON PARENT_SCOPE)

add_executable(BeatEngineTest 
	"main.cpp"
 	"view/view.cpp"			"view/view.h"
    "view/gameView.cpp"     "view/gameView.h"
	"system/system.cpp"		"system/system.h"
 "layer/globalLayer.h" "layer/globalLayer.cpp")

target_link_libraries(BeatEngineTest
    PRIVATE
        BeatEngine
)


if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_link_options(BeatEngineTest PRIVATE "$<$<CONFIG:Debug>:/DEBUG>") 
endif()

file(
    COPY "${CMAKE_SOURCE_DIR}/assets" 
    DESTINATION "${PROJECT_BINARY_DIR}/${CMAKE_PRESET_NAME}/bin"
)

set_property(TARGET BeatEngineTest PROPERTY CXX_STANDARD 23)
target_compile_features(BeatEngineTest PRIVATE cxx_std_23)

target_include_directories(BeatEngineTest PRIVATE "${HEADER_SOURCE_DIR}")
target_include_directories(BeatEngineTest PRIVATE "${libsndfile_SOURCE_DIR}/include")
target_include_directories(BeatEngineTest PRIVATE "${IMGUI_DIR}")
target_include_directories(BeatEngineTest PRIVATE "${portaudio_SOURCE_DIR}/include")
target_include_directories(BeatEngineTest PRIVATE "${samplerate_SOURCE_DIR}/include")
target_include_directories(BeatEngineTest PRIVATE "${miniaudio_SOURCE_DIR}")
