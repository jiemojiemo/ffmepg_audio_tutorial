cmake_minimum_required(VERSION 3.10)

project(ffmepg_audio_tutorial)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(FindFFMPEG)

find_package(FFMPEG REQUIRED)
if(NOT FFMPEG_FOUND)
    message(ERROR "Cannot find ffmpeg")
else()
    message(STATUS "Found ffmpeg")
    message(STATUS "ffmpeg include dir is ${FFMPEG_INCLUDE_DIRS}")
    message(STATUS "ffmpeg libraries are ${FFMPEG_LIBRARIES}")
endif()


add_executable(hello_ffmpeg 0_hello_ffmpeg.cpp)
target_include_directories(hello_ffmpeg PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(hello_ffmpeg PRIVATE ${FFMPEG_LIBRARIES})