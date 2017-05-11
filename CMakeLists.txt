cmake_minimum_required(VERSION 3.5)
SET(CMAKE_PREFIX_PATH $ENV{GST_PATH})
project(testMpegts)


set(CMAKE_C_STANDARD 99)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(FindPkgConfig)
include(PkgCheckVariable)

find_package(GLIB REQUIRED)

#check for dependencies
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-video-1.0 gstreamer-app-1.0
        gstreamer-base-1.0 gstreamer-check-1.0 gstreamer-codecparsers-1.0 gstreamer-mpegts-1.0
        gstreamer-player-1.0 gstreamer-rtsp-1.0 gstreamer-tag-1.0 gstreamer-video-1.0
        gstreamer-audio-1.0 gstreamer-pbutils-1.0)

pkg_check_variable(GST_PLUGIN_PATH gstreamer-1.0 pluginsdir)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.31.14)

set(SOURCE_FILES main.c)

add_executable(main ${SOURCE_FILES})

include_directories(
        ${GST_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIR}
)

target_include_directories(main PUBLIC ${GST_INCLUDE_DIRS})

target_link_libraries (main ${GST_LIBRARIES})


message("gst lib dir " ${GST_LIBRARY_DIRS})

message("glib lib dir " ${GLIB_INCLUDE_DIR})

message("gst lib dir " ${GLIB_LIBRARY_DIRS})
message("gst lib " ${GST_LIBRARIES})
