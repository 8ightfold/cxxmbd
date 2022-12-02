cmake_minimum_required(VERSION 3.23)
project(cxxmbd CXX)

include(cmake/colorize.cmake)

if(NOT DEFINED CXXMBD_MODE)
    set(CXXMBD_MODE attach)
    cmessage(LIGHT_WARNING "cxxmbd: mode set to \"${CXXMBD_MODE}\" by default")
else()
    message(STATUS "cxxmbd: current mode set to ${Blue}${CXXMBD_MODE}${ColReset}")
endif()

if(NOT DEFINED CXXMBD_WINMAIN)
    set(CXXMBD_WINMAIN false)
endif()
if(NOT (${CXXMBD_WINMAIN} STREQUAL true OR ${CXXMBD_WINMAIN} STREQUAL false))
    message(FATAL_ERROR "cxxmbd: unrecognized WinMain option \"${CXXMBD_WINMAIN}\"")
endif()


add_library(${PROJECT_NAME} STATIC cli/cxxmbd/cxxmbd.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC cli)

# Compile modes
if(${CXXMBD_MODE} STREQUAL attach)
    #TODO: implement this and .xmbd
    message(STATUS "cxxmbd: WinMain is set to ${Blue}${CXXMBD_WINMAIN}${ColReset}")
    target_include_directories(${PROJECT_NAME} PUBLIC hotreload)

elseif(${CXXMBD_MODE} STREQUAL include)
elseif(${CXXMBD_MODE} STREQUAL standalone)
    message(STATUS "cxxmbd: WinMain is set to ${Blue}${CXXMBD_WINMAIN}${ColReset}")
    if(${CXXMBD_WINMAIN} STREQUAL true)
        set(ENV{MAIN_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/os_main/cli/windows_main.cpp)
    else()
        set(ENV{MAIN_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/os_main/cli/linux_main.cpp)
    endif()
else()
    message(FATAL_ERROR "cxxmbd: unrecognized compile mode \"${CXXMBD_MODE}\"")
endif()
