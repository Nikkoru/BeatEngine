# CMakeList.txt : CMake project for BeatEngine, include source and define
# project specific logic here.
#

set(HEADERS
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/EventManager.h" 
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/ViewManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/SignalManager.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/Events/ViewEvent.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/Signals/ViewSignals.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/Event.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/View.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/Signal.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/SignalResponse.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Util/ChronoHelper.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Enum/Commands.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Enum/LogType.h"
)

set(SOURCES
	"./Manager/ViewManager.cpp"	
	"./Manager/EventManager.cpp"
	"./Manager/SignalManager.cpp"
	
	"./Base/View.cpp"
	
	"./Util/ChronoHelper.cpp"
	"./Util/LogType.cpp"
	"./Util/Commands.cpp"
	
	"Logger.cpp"
)
 add_library(BeatEngine 
	${SOURCES}
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