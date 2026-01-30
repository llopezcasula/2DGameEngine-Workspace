cmake_minimum_required(VERSION 3.27)
project(2DEngineLib LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE ENGINE_CPP
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

set(GLAD_C "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c")
if(NOT EXISTS "${GLAD_C}")
    message(FATAL_ERROR "Missing glad.c at: ${GLAD_C}")
endif()

# Force glad.c to compile as C
set_source_files_properties(${GLAD_C} PROPERTIES LANGUAGE C)

add_library(2DEngineLib STATIC
        ${ENGINE_CPP}
        ${GLAD_C}
)

target_include_directories(2DEngineLib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/external/miniaudio
)

set(GLFW_LIB "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/lib-vc2022/glfw3.lib")
if(NOT EXISTS "${GLFW_LIB}")
    message(FATAL_ERROR "GLFW library not found at: ${GLFW_LIB}")
endif()

target_link_libraries(2DEngineLib PUBLIC
        "${GLFW_LIB}"
        opengl32
)
