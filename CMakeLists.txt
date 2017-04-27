cmake_minimum_required(VERSION 3.5)
SET(CMAKE_PREFIX_PATH $ENV{GST_PATH})


project(testMpegts)


list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(FindPkgConfig)
include(PkgCheckVariable)

find_package(GLIB REQUIRED)

#check for dependencies
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-1.0 gstreamer-app-1.0
        gstreamer-base-1.0 gstreamer-check-1.0 gstreamer-codecparsers-1.0 gstreamer-mpegts-1.0
        gstreamer-player-1.0 gstreamer-rtsp-1.0 gstreamer-tag-1.0 gstreamer-video-1.0
        gstreamer-audio-1.0 gstreamer-pbutils-1.0)

pkg_check_variable(GST_PLUGIN_PATH gstreamer-1.0 pluginsdir)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.31.14)


include_directories(
        ${GST_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIR}
)

link_directories(
        ${GST_LIBRARY_DIRS}
        ${GLIB_LIBRARY_DIRS}
)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c )

add_executable(testMpegts ${SOURCE_FILES} ${CMAKE_INCLUDE_CURRENT_DIR})

message("gst lib dir " ${GST_LIBRARY_DIRS})

message("glib lib dir " ${GLIB_INCLUDE_DIR})

message("gst lib dir " ${GST_LIBRARY_DIRS})