cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(gltut2)

set(GLTUT2_CFLAGS "-march=native -ffast-math -pipe")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GLTUT2_CFLAGS} -Wl,-O1 -Wl,--as-needed")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GLTUT2_CFLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GLTUT2_CFLAGS}")

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_check_output)
if (ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO is not supported: ${ipo_check_output}")
endif ()

add_definitions(
        -DQT_DEPRECATED_WARNINGS
        -DQT_DISABLE_DEPRECATED_BEFORE=0x060000
        -DQT_NO_CAST_FROM_ASCII
        -DQT_NO_CAST_TO_ASCII
)
add_definitions(-DQ_ENABLE_OPENGL_FUNCTIONS_DEBUG)

find_package(Qt5 COMPONENTS Gui Widgets REQUIRED)

add_executable(gltut2
        main.cpp
        resources.qrc
        OpenGLWindow.cpp OpenGLWindow.h
        TutorialWindow.cpp TutorialWindow.h)
target_link_libraries(gltut2 Qt5::Gui Qt5::Widgets)
