# CMakeList.txt : CMake project for BeatEngine, include source and define
# project specific logic here.
#

set(HEADERS
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/EventManager.h" 
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/ViewManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/SignalManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/SettingsManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/EntityManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/AssetManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/SystemManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/AudioManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/UIManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/UIManager.inl"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Managers.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/Event.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/View.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/Signal.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/Settings.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/System.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/Asset.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/SignalResponse.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/View/ViewLayer.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/View/ViewLayerStack.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/View/ViewLayerStack.inl"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Signals/ViewSignals.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Signals/AudioSignals.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Settings/GameSettings.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Events/ViewEvent.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Events/GameEvent.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Asset/Texture.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Asset/Sound.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Asset/AudioStream.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Asset/Font.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Util/ChronoHelper.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Util/UID.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Util/Math.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Util/Exception.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Util/UIHelper.h"
	
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Enum/LogType.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Enum/AssetType.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Enum/Alignment.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/UILayer.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/UILayer.inl"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/UIElement.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/UIClickeable.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/UIPanel.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/Elements/Button.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/Elements/ProgressBar.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Game.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Logger.h"
)

set(SOURCES
	"Manager/ViewManager.cpp"	
	"Manager/EventManager.cpp"
	"Manager/SignalManager.cpp"
	"Manager/EntityManager.cpp"
	"Manager/AssetManager.cpp"
	"Manager/SystemManager.cpp"
	"Manager/SettingsManager.cpp"
	"Manager/AudioManager.cpp"
	"Manager/UIManager.cpp"	

	"Base/View.cpp"
	
	"View/ViewLayer.cpp"
	"View/ViewLayerStack.cpp"

	"Asset/Sound.cpp"
	"Asset/AudioStream.cpp"
	"Asset/Font.cpp" 
	"Asset/Texture.cpp"

	"Util/Exception.cpp"
	"Util/UIHelper.cpp"
	"Util/ChronoHelper.cpp"
	"Util/UID.cpp"

	"Enum/LogType.cpp"
	
	"UI/UILayer.cpp"
	"UI/UIElement.cpp"
	"UI/UIClickeable.cpp"
	"UI/UIPanel.cpp"

	"UI/Elements/Button.cpp"
	"UI/Elements/ProgressBar.cpp"

	"Settings/GameSettings.cpp"
 
	"Logger.cpp"
	
	"Game.cpp"
)
add_library(BeatEngine 
	${SOURCES}
	${HEADERS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_link_libraries(${PROJECT_NAME} 
	PRIVATE 
		sfml-graphics 
		sfml-network 
		ImGui-SFML::ImGui-SFML
		portaudio
		miniaudio
		sndfile
		samplerate
)

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${libsndfile_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${portaudio_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${samplerate_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${miniaudio_SOURCE_DIR}/")