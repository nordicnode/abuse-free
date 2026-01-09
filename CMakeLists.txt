cmake_minimum_required(VERSION 3.10)
project(Abuse VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find PkgConfig
find_package(PkgConfig REQUIRED)

# Find SDL2
find_package(SDL2)
if(NOT SDL2_FOUND)
    pkg_check_modules(SDL2 REQUIRED sdl2)
endif()

# Find SDL2_mixer (Optional)
pkg_check_modules(SDL2_MIXER SDL2_mixer)
if(SDL2_MIXER_FOUND)
    add_definitions(-DHAVE_SDL_MIXER)
else()
    message(WARNING "SDL2_mixer not found. Sound will be disabled.")
endif()

# Include directories
include_directories(
    src
    src/imlib
    src/lisp
    src/net
    src/sdlport
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
)

# Common flags
add_definitions(-DNO_CHECK -DASSETDIR="./data" -DPACKAGE_NAME="Abuse" -DPACKAGE_VERSION="1.0")

# --- Libraries ---

# liblisp
file(GLOB LISP_SOURCES src/lisp/*.cpp src/lisp/*.c)
add_library(lisp STATIC ${LISP_SOURCES})

# libimlib
file(GLOB IMLIB_SOURCES src/imlib/*.cpp)
add_library(imlib STATIC ${IMLIB_SOURCES})

# libnet
file(GLOB NET_SOURCES src/net/*.cpp)
add_library(net STATIC ${NET_SOURCES})

# libsdlport
file(GLOB SDLPORT_SOURCES 
    src/sdlport/video.cpp 
    src/sdlport/mouse.cpp 
    src/sdlport/event.cpp 
    src/sdlport/sound.cpp 
    src/sdlport/timing.cpp 
    src/sdlport/jdir.cpp 
    src/sdlport/joystick.cpp 
    src/sdlport/setup.cpp 
    src/sdlport/hmi.cpp
)
add_library(sdlport STATIC ${SDLPORT_SOURCES})

# --- Main Executable ---

file(GLOB ABUSE_SOURCES
    src/*.cpp
    src/ui/*.cpp
    src/lol/*.cpp
)

# Remove tool files from main executable sources if necessary
list(REMOVE_ITEM ABUSE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/tool/abuse-tool.cpp")

add_executable(abuse ${ABUSE_SOURCES})
target_link_libraries(abuse
    sdlport
    imlib
    net
    lisp
    ${SDL2_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    m
    pthread
)

# --- Tool ---
add_executable(abuse-tool src/tool/abuse-tool.cpp src/crc.cpp)
target_link_libraries(abuse-tool imlib)
