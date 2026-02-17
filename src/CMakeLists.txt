# CMakeList.txt : CMake project for BeatEngine, include source and define
# project specific logic here.
#

set(HEADERS
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/EventManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/EventManager.inl"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/GraphicsManager.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/GraphicsManager.inl"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/ViewManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/ViewManager.inl"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/SignalManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/SignalManager.inl"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/SettingsManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/SettingsManager.inl"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/EntityManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/AssetManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/AssetManager.inl"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/SystemManager.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Manager/SystemManager.inl"
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
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/Entity.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base/SignalResponse.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Base.h"

    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Graphics/BaseWindow.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Graphics/VSyncMode.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Graphics/Renderer.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Graphics/Vector2.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Graphics/Vector2.inl"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Graphics/Vector3.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Graphics/Vector3.inl"

    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Renderers/Vulkan.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Renderers/VulkanFrameData.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Renderers/VulkanWindow.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Renderers/VulkanBoilerplate.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/View/ViewLayer.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/View/ViewLayerStack.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/View/ViewLayerStack.inl"

    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Camera/Camera.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Camera/CameraMode.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Camera/ShakeParams.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Signals/ViewSignals.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Signals/AudioSignals.h"
	
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/System/Clock.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/System/Time.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/System/Time.inl"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Settings/GameSettings.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Events/ViewEvent.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Events/GameEvent.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Events/AudioEvent.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Asset/Texture.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Asset/Sound.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Asset/AudioStream.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Asset/Font.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Util/ChronoHelper.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Util/UID.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Util/Math.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Util/Exception.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Util/UIHelper.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Util/Helper.h"
	
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Enum/LogType.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Enum/AssetType.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Enum/Alignment.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Enum/GameFlags.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/UILayer.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/UILayer.inl"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/UIElement.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/UIClickeable.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/Alignment.h"

    "${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/Elements/UIPanel.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/Elements/Button.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/UI/Elements/ProgressBar.h"

	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Game.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/GameContext.h"
	"${PROJECT_SOURCE_DIR}/include/BeatEngine/Logger.h"
    "${PROJECT_SOURCE_DIR}/include/BeatEngine/Logger.inl"
)

set(SOURCES
	"Manager/ViewManager.cpp"
    "Manager/GraphicsManager.cpp"
	"Manager/EventManager.cpp"
	"Manager/SignalManager.cpp"
	"Manager/EntityManager.cpp"
	"Manager/AssetManager.cpp"
	"Manager/SystemManager.cpp"
	"Manager/SettingsManager.cpp"
	"Manager/AudioManager.cpp"
	"Manager/UIManager.cpp"	

	"Base/View.cpp"

    "Graphics/BaseWindow.cpp"
    "Graphics/Renderer.cpp"
	
    "Renderers/Vulkan.cpp"
    "Renderers/VulkanWindow.cpp"
    "Renderers/VulkanBoilerplate.cpp"

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
    "Util/Helper.cpp"

    "Camera/Camera.cpp"
    "Camera/ShakeParams.cpp"

	"Enum/LogType.cpp"
	
	"UI/UILayer.cpp"
	"UI/UIElement.cpp"
	"UI/UIClickeable.cpp"

    "UI/Elements/UIPanel.cpp"
	"UI/Elements/Button.cpp"
	"UI/Elements/ProgressBar.cpp"

	"Settings/GameSettings.cpp"
	
    "System/Clock.cpp"
    "System/Time.cpp"
 
	"Logger.cpp"
    "vma.cpp"
	
	"Game.cpp"
)
add_library(BeatEngine 
	${SOURCES}
	${HEADERS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

if (MAKE_TEST_BUILD)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BEATENGINE_TEST)
endif()
if (USE_VULKAN OR ON)
    find_package(Vulkan REQUIRED volk)

    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_NO_PROTOTYPES)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BEATENGINE_VULKAN)

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
else()
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        	sfml-graphics
		    sfml-network 
		    ImGui-SFML::ImGui-SFML
    )
    target_include_directories(${PROJECT_NAME} PRIVATE "${SFML_SOURCE_DIR}/include")
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
