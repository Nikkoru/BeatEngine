list(APPEND SOURCES
    "${PROJECT_SOURCE_DIR}/src/Manager/ViewManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/Manager/GraphicsManager.cpp"
	"${PROJECT_SOURCE_DIR}/src/Manager/EventManager.cpp"
	"${PROJECT_SOURCE_DIR}/src/Manager/SignalManager.cpp"
	"${PROJECT_SOURCE_DIR}/src/Manager/EntityManager.cpp"
	"${PROJECT_SOURCE_DIR}/src/Manager/AssetManager.cpp"
	"${PROJECT_SOURCE_DIR}/src/Manager/SystemManager.cpp"
	"${PROJECT_SOURCE_DIR}/src/Manager/SettingsManager.cpp"
	"${PROJECT_SOURCE_DIR}/src/Manager/AudioManager.cpp"
	"${PROJECT_SOURCE_DIR}/src/Manager/UIManager.cpp"	

	"${PROJECT_SOURCE_DIR}/src/Base/View.cpp"

    "${PROJECT_SOURCE_DIR}/src/Graphics/BaseWindow.cpp"
    "${PROJECT_SOURCE_DIR}/src/Graphics/Transform.cpp"
    "${PROJECT_SOURCE_DIR}/src/Graphics/Transformable.cpp"
    "${PROJECT_SOURCE_DIR}/src/Graphics/RenderState.cpp"
    "${PROJECT_SOURCE_DIR}/src/Graphics/GraphicalElement.cpp"
    "${PROJECT_SOURCE_DIR}/src/Graphics/TextElement.cpp"
    "${PROJECT_SOURCE_DIR}/src/Graphics/Renderer.cpp"
    "${PROJECT_SOURCE_DIR}/src/Graphics/Shape.cpp"
    "${PROJECT_SOURCE_DIR}/src/Graphics/RectShape.cpp"

    "${PROJECT_SOURCE_DIR}/src/Renderers/Vulkan/Core.cpp"
    "${PROJECT_SOURCE_DIR}/src/Renderers/Vulkan/ImageData.cpp"
    "${PROJECT_SOURCE_DIR}/src/Renderers/Vulkan/ImageCache.cpp"
    "${PROJECT_SOURCE_DIR}/src/Renderers/Vulkan/BindlessSetManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/Renderers/Vulkan/Instance.cpp"
    "${PROJECT_SOURCE_DIR}/src/Renderers/Vulkan/ImmediateExecutor.cpp"
    "${PROJECT_SOURCE_DIR}/src/Renderers/Vulkan/Swapchain.cpp"
    "${PROJECT_SOURCE_DIR}/src/Renderers/Vulkan/Renderer.cpp"
    "${PROJECT_SOURCE_DIR}/src/Renderers/Vulkan/Boilerplate.cpp"
    "${PROJECT_SOURCE_DIR}/src/Renderers/Vulkan/UninitQueue.cpp"
    "${PROJECT_SOURCE_DIR}/src/Renderers/Vulkan/Assets/Texture.cpp"
    "${PROJECT_SOURCE_DIR}/src/Renderers/Vulkan/Assets/Shader.cpp"

    "${PROJECT_SOURCE_DIR}/src/Renderers/OpenGL/Renderer.cpp"

    "${PROJECT_SOURCE_DIR}/src/Windows/SDL/Window.cpp"

	"${PROJECT_SOURCE_DIR}/src/View/ViewLayer.cpp"
	"${PROJECT_SOURCE_DIR}/src/View/ViewLayerStack.cpp"

	"${PROJECT_SOURCE_DIR}/src/Asset/Sound.cpp"
	"${PROJECT_SOURCE_DIR}/src/Asset/AudioStream.cpp"
    "${PROJECT_SOURCE_DIR}/src/Asset/Shader.cpp"
	"${PROJECT_SOURCE_DIR}/src/Asset/Font.cpp" 
	"${PROJECT_SOURCE_DIR}/src/Asset/Texture.cpp"

	"${PROJECT_SOURCE_DIR}/src/Util/Exception.cpp"
	"${PROJECT_SOURCE_DIR}/src/Util/Graphics.cpp"
	"${PROJECT_SOURCE_DIR}/src/Util/UIHelper.cpp"
	"${PROJECT_SOURCE_DIR}/src/Util/ChronoHelper.cpp"
	"${PROJECT_SOURCE_DIR}/src/Util/Profiler.cpp"
    "${PROJECT_SOURCE_DIR}/src/Util/Humanize.cpp"
	"${PROJECT_SOURCE_DIR}/src/Util/UID.cpp"
    "${PROJECT_SOURCE_DIR}/src/Util/Helper.cpp"

    "${PROJECT_SOURCE_DIR}/src/Camera/Camera.cpp"
    "${PROJECT_SOURCE_DIR}/src/Camera/ShakeParams.cpp"

	"${PROJECT_SOURCE_DIR}/src/Enum/LogType.cpp"
    "${PROJECT_SOURCE_DIR}/src/Enum/AssetType.cpp"

	"${PROJECT_SOURCE_DIR}/src/UI/UILayer.cpp"
	"${PROJECT_SOURCE_DIR}/src/UI/UIElement.cpp"
	"${PROJECT_SOURCE_DIR}/src/UI/UIClickeable.cpp"

    "${PROJECT_SOURCE_DIR}/src/UI/Elements/UIPanel.cpp"
	"${PROJECT_SOURCE_DIR}/src/UI/Elements/Button.cpp"
	"${PROJECT_SOURCE_DIR}/src/UI/Elements/ProgressBar.cpp"

	"${PROJECT_SOURCE_DIR}/src/Settings/GameSettings.cpp"
    "${PROJECT_SOURCE_DIR}/src/Settings/GameDebugSettings.cpp"

    "${PROJECT_SOURCE_DIR}/src/System/Clock.cpp"
    "${PROJECT_SOURCE_DIR}/src/System/DataStream.cpp"
    "${PROJECT_SOURCE_DIR}/src/System/String.cpp"
    "${PROJECT_SOURCE_DIR}/src/System/Time.cpp"
 
	"${PROJECT_SOURCE_DIR}/src/Logger.cpp"
    "${PROJECT_SOURCE_DIR}/src/vma.cpp"

    "${PROJECT_SOURCE_DIR}/src/GameState.cpp"
	"${PROJECT_SOURCE_DIR}/src/Game.cpp"

    "${PROJECT_SOURCE_DIR}/src/ImGui/MultiPlot.cpp"
)

list(APPEND SHADERS
    "${PROJECT_SOURCE_DIR}/src/shaders/base.frag"
    "${PROJECT_SOURCE_DIR}/src/shaders/base.vert"
)

set(SOURCES ${SOURCES} PARENT_SCOPE)
set(SHADERS ${SHADERS} PARENT_SCOPE)
