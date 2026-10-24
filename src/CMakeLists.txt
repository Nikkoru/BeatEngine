file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

list(APPEND SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

list(APPEND HEADERS
    ${IMGUI_DIR}/imconfig.h
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imstb_rectpack.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imstb_truetype.h
)

if(USE_OPENGL_RENDERER AND NOT USE_VULKAN_RENDERER)
    list(APPEND SOURCES
        "${IMGUI_DIR}/backends/imgui_impl_opengl2.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
    )
    list(APPEND HEADERS
        "${IMGUI_DIR}/backends/imgui_impl_opengl2.h"
        "${IMGUI_DIR}/backends/imgui_impl_opengl3.h"
        "${IMGUI_DIR}/backends/imgui_impl_opengl3_loader.h"
    )
elseif (USE_VULKAN_RENDERER AND NOT USE_OPENGL_RENDERER)
    list(APPEND SOURCES
        "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
    )
    list(APPEND HEADERS
        "${IMGUI_DIR}/backends/imgui_impl_vulkan.h"
    )
endif()

if (USE_SDL_WINDOW AND NOT USE_GLFW_WINDOW)
    list(APPEND SOURCES
        "${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
    )
    list(APPEND HEADERS
        "${IMGUI_DIR}/backends/imgui_impl_sdl3.h"
    )
endif()

list(APPEND SHADERS
    "${PROJECT_SOURCE_DIR}/src/shaders/base.frag"
    "${PROJECT_SOURCE_DIR}/src/shaders/base.vert"
)

set(SOURCES ${SOURCES} PARENT_SCOPE)
set(SHADERS ${SHADERS} PARENT_SCOPE)
