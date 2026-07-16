# GetGitCommit.cmake
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

# Fallback if git is not installed or not a repo
if(NOT GIT_COMMIT_HASH)
    set(GIT_COMMIT_HASH "unknown")
endif()

# Content of the file to generate
set(HEADER_CONTENT "// Generated automatically at build time\n#define BEATENGINE_COMMIT_ID_BUILD \"${GIT_COMMIT_HASH}\"\n")

# Check if file exists and has changed to prevent unnecessary rebuilds
set(TARGET_FILE "${BINARY_DIR}/version.h")
if(EXISTS "${TARGET_FILE}")
    file(READ "${TARGET_FILE}" EXISTING_CONTENT)
else()
    set(EXISTING_CONTENT "")
endif()

# Only overwrite if the hash changed
if(NOT "${HEADER_CONTENT}" STREQUAL "${EXISTING_CONTENT}")
    file(WRITE "${TARGET_FILE}" "${HEADER_CONTENT}")
endif()
