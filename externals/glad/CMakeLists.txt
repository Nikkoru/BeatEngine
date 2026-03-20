add_library(GLAD STATIC
    include/glad/glad.h
    include/KHR/khrplatform.h
    src/glad.c
)

set(GLAD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
