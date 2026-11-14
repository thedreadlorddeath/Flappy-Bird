cmake_minimum_required(VERSION 3.20)
project(FlappyBird LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Automatically fetch SFML 2.6.2 if not already present
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.2
    GIT_SHALLOW TRUE
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
set(SFML_BUILD_GRAPHICS ON CACHE BOOL "" FORCE)
set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SFML)

# Project Source Files
file(GLOB_RECURSE SOURCES "src/*.cpp")

if(ANDROID)
    # Android uses NativeActivity which loads libFlappyBird.so
    add_library(FlappyBird SHARED ${SOURCES})
    target_include_directories(FlappyBird PRIVATE include)
    target_link_libraries(FlappyBird PRIVATE
        sfml-main
        sfml-activity
        sfml-graphics
        sfml-audio
        sfml-window
        sfml-system
        android
        log
    )
else()
    # Desktop Executable (Windows / Linux / macOS)
    add_executable(FlappyBird ${SOURCES})
    target_include_directories(FlappyBird PRIVATE include)
    target_link_libraries(FlappyBird PRIVATE
        sfml-graphics
        sfml-audio
        sfml-window
        sfml-system
    )

    if(WIN32)
        target_link_libraries(FlappyBird PRIVATE opengl32 winmm gdi32)
        
        # Automatically copy openal32.dll next to executable on Windows
        add_custom_command(TARGET FlappyBird POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll"
                $<TARGET_FILE_DIR:FlappyBird>
            COMMENT "Copying openal32.dll to target directory"
        )
    endif()
endif()
