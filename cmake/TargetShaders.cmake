find_program(GLSLANG glslang REQUIRED
    HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/bin/ $ENV{VULKAN_SDK}/bin32/
)

# find_program(GLSLVALIDATOR glslangValidator
#     HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/bin/ $ENV{VULKAN_SDK}/bin32/
# )

function (target_shaders target SHADERS)
    set(SHADERS_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin/shaders")
	file(MAKE_DIRECTORY "${SHADERS_BUILD_DIR}")

    list(LENGTH SHADERS SHADERS_LEN)
    if (SHADERS_LEN EQUAL 1)
        message(FATAL_ERROR someting)
    elseif(SHADER_LEN LESS 1)
        message(FATAL_ERROR bro and the shaders)
    endif()

    foreach(SHADER_PATH ${SHADERS})
        get_filename_component(SHADER_FILENAME "${SHADER_PATH}" NAME)
        set(SHADER_SPRIV_PATH "${SHADERS_BUILD_DIR}/${SHADER_FILENAME}.spv")
        set(DEPFILE "${SHADER_SPRIV_PATH}.d")
        add_custom_command(
            COMMENT "Building shader ${SHADER_FILENAME}"
            OUTPUT  "${SHADER_SPRIV_PATH}"
            COMMAND ${GLSLANG} -V "${SHADER_PATH}" -o "${SHADER_SPRIV_PATH}" --depfile ${DEPFILE} -gVS
            DEPENDS "${SHADER_PATH}"
            DEPFILE "${DEPFILE}"
        )
        list(APPEND SPRIV_BINARY_FILES ${SHADER_SPRIV_PATH})
    endforeach()

    set(shaders_target_name "${target}_build_shaders")
    add_custom_target(${shaders_target_name}
        DEPENDS ${SPRIV_BINARY_FILES}
    )

    add_dependencies(${target} ${shaders_target_name})

    message(STATUS "SPRIV_BINARY_FILES = ${SPRIV_BINARY_FILES}")

    # message(FATAL_ERROR "the close thing i have to cmake breakpoints")
endfunction()
