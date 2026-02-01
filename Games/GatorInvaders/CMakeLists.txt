cmake_minimum_required(VERSION 3.27)
project(GatorInvaders LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(GatorInvaders
        src/main.cpp
        src/GatorInvaders.cpp
        include/GatorInvaders.h
)

target_include_directories(GatorInvaders PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(GatorInvaders PRIVATE
        2DEngineLib
)

add_custom_command(TARGET GatorInvaders POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:GatorInvaders>/assets"
)
