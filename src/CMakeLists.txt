# CMakeList.txt : CMake project for BeatEngine, include source and define
# project specific logic here.
#

set(HEADERS
	"${PROJECT_SOURCE_DIR}/include/Base/Events/ViewEvent.h"
	"${PROJECT_SOURCE_DIR}/include/Manager/EventManager.h" 
	"${PROJECT_SOURCE_DIR}/include/Manager/ViewManager.h"
	"${PROJECT_SOURCE_DIR}/include/Base.h"
	"${PROJECT_SOURCE_DIR}/include/Base/Event.h"
	"${PROJECT_SOURCE_DIR}/include/Base/View.h"
	"${PROJECT_SOURCE_DIR}/include/Enum/Commands.h"
	"${PROJECT_SOURCE_DIR}/include/Enum/LogType.h"
)
add_library(BeatEngine 
	"Logger.cpp"
	"./Util/LogType.cpp"
	"./Util/Commands.cpp"
	"./Base/View.cpp"
	"./Manager/ViewManager.cpp"	
	"./Manager/EventManager.cpp"
	${HEADERS}
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_link_libraries(${PROJECT_NAME} 
	PRIVATE 
		sfml-graphics 
		sfml-network 
		ImGui-SFML::ImGui-SFML
)

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include")