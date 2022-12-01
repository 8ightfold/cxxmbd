cmake_minimum_required(VERSION 3.23)
project(cxxmbd CXX)

include(cmake/colorize.cmake)

# $CXXMBD_MODE
# > attach
# > include
# > standalone

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
message(STATUS "cxxmbd: WinMain is set to ${Blue}${CXXMBD_WINMAIN}${ColReset}")

# Compile modes
if(${CXXMBD_MODE} STREQUAL attach)
    #TODO: implement this

elseif(${CXXMBD_MODE} STREQUAL include)
    add_library(${PROJECT_NAME} STATIC include/cxxmbd/cxxmbd.cpp)
    target_include_directories(${PROJECT_NAME} PUBLIC include)

elseif(${CXXMBD_MODE} STREQUAL standalone)
    if(${CXXMBD_WINMAIN} STREQUAL true)
        set(MAIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/os_main/windows_main.cpp)
    else()
        set(MAIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/os_main/linux_main.cpp)
    endif()
    add_library(${PROJECT_NAME} STATIC include/cxxmbd/cxxmbd.cpp)
    target_include_directories(${PROJECT_NAME} PUBLIC include)

else()
    message(FATAL_ERROR "cxxmbd: unrecognized compile mode \"${CXXMBD_MODE}\"")
endif()
