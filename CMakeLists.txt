cmake_minimum_required(VERSION 3.23)
project(cxxmbd CXX)

set(ENV{CXXMBD_VER} "2.0.0")
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

if(NOT DEFINED DISABLE_ANSI)
    set(DISABLE_ANSI false)
endif()


add_library(${PROJECT_NAME} STATIC cli/cxxmbd/cxxmbd.cpp reload/cxxmbd/os.cpp reload/cxxmbd/core.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC cli)
target_compile_definitions(${PROJECT_NAME} PUBLIC
        EMBED_POINT= EMBED_START= EMBED_END=
        CXXMBD_VER="$ENV{CXXMBD_VER}" PLATFORM_NAME="${CMAKE_SYSTEM_NAME}")
if(${DISABLE_ANSI} STREQUAL true)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DISABLE_ANSI=)
endif()
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ON_WINDOWS=PLATFORM_NAME)
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ON_UNIX=PLATFORM_NAME)
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC ON_UNKNOWN=PLATFORM_NAME)
endif()

# Compile modes
if(${CXXMBD_MODE} STREQUAL attach)
    set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")
    message(STATUS "cxxmbd: WinMain is set to ${Blue}${CXXMBD_WINMAIN}${ColReset}")
    if(${CXXMBD_WINMAIN} STREQUAL true)
        set(ENV{MAIN_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/os_main/reload/windows_main.cpp)
    else()
        set(ENV{MAIN_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/os_main/reload/linux_main.cpp)
    endif()

    target_include_directories(${PROJECT_NAME} PUBLIC reload)
    add_executable(mbdcore $ENV{MAIN_PATH})
    target_link_libraries(mbdcore ${PROJECT_NAME})
    add_custom_target(mbdrun
            COMMAND mbdcore ${CMAKE_SOURCE_DIR}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

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
