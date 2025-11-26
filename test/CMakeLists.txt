cmake_minimum_required(VERSION 3.10)

project("BeatEngineTest")

add_executable(BeatEngineTest 
	"main.cpp"
 	"view/view.cpp"			"view/view.h"
	"system/system.cpp"		"system/system.h"
)
target_link_libraries(BeatEngineTest
	PUBLIC
	BeatEngine
	sfml-graphics 
	sfml-network 
	ImGui-SFML::ImGui-SFML
	miniaudio
	portaudio
)

target_link_options(BeatEngineTest PRIVATE "$<$<CONFIG:Debug>:/DEBUG>") 

set_property(TARGET BeatEngineTest PROPERTY CXX_STANDARD 23)
target_compile_features(BeatEngineTest PRIVATE cxx_std_23)

target_include_directories(BeatEngineTest PRIVATE "${HEADER_SOURCE_DIR}")