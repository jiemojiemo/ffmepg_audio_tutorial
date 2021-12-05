cmake_minimum_required(VERSION 3.10)

project(ffmepg_audio_tutorial)

set(CMAKE_CXX_STANDARD 14)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(FFMPEG REQUIRED COMPONENTS AVCODEC AVFORMAT AVUTIL SWRESAMPLE)
if(NOT FFMPEG_FOUND)
    message(ERROR "Cannot find ffmpeg")
else()
    message(STATUS "Found ffmpeg")
    message(STATUS "ffmpeg include dir is ${FFMPEG_INCLUDE_DIRS}")
    message(STATUS "ffmpeg libraries are ${FFMPEG_LIBRARIES}")
endif()

add_library(ffmpeg_dep INTERFACE)
target_link_libraries(ffmpeg_dep INTERFACE ${FFMPEG_LIBRARIES})
target_include_directories(ffmpeg_dep INTERFACE ${FFMPEG_INCLUDE_DIRS})


add_executable(hello_ffmpeg 0_hello_ffmpeg.cpp)
target_link_libraries(hello_ffmpeg ffmpeg_dep)

add_executable(decode_audio 1_decode_audio.cpp)
target_link_libraries(decode_audio ffmpeg_dep)

add_executable(encoder_audio 2_encode_audio.cpp)
target_link_libraries(encoder_audio ffmpeg_dep)

add_executable(decode_from_memory 3_decode_with_callback.cpp)
target_link_libraries(decode_from_memory ffmpeg_dep)

add_executable(4_print_codec_and_formats 4_print_codec_and_formats.cpp)
target_link_libraries(4_print_codec_and_formats ffmpeg_dep)

add_executable(5_encode_audio_with_callback 5_encode_audio_with_callback.cpp)
target_link_libraries(5_encode_audio_with_callback ffmpeg_dep)