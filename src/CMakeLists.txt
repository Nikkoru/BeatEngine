# CMakeList.txt : CMake project for BeatEngine, include source and define
# project specific logic here.
#

add_library(BeatEngine 
	"src/Logger.cpp"				"include/Logger.h"

	"src/Util/LogType.cpp"			"include/Enum/LogType.h"
	"src/Util/Commands.cpp"			"include/Enum/Commands.h"

	"src/Base/View.cpp"				"include/Base/View.h"
									"include/Base/Event.h"
									"include/Base.h"

	"src/Manager/ViewManager.cpp"	"include/Manager/ViewManager.h"
	"src/Manager/EventManager.cpp"	"include/Manager/EventManager.h" 
 "include/Base/Events/ViewEvent.h")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_link_libraries(${PROJECT_NAME} 
	PRIVATE 
		sfml-graphics 
		sfml-network 
		ImGui-SFML::ImGui-SFML
)

target_include_directories(${PROJECT_NAME} PRIVATE "include")