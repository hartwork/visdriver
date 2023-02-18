# This file is part of the visdriver project.
#
# Copyright (c) 2023 Sebastian Pipping <sebastian@pipping.org>
#
# visdriver is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# visdriver is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with visdriver. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.0)

project(visdriver)

if (NOT MSVC)
    if (NOT CMAKE_TOOLCHAIN_FILE)
        message(SEND_ERROR "Please pass -DCMAKE_TOOLCHAIN_FILE=[..]"
                " for use of a MinGW compiler"
                ", e.g. \"-DCMAKE_TOOLCHAIN_FILE=cmake/mingw-toolchain.cmake\""
                "; we cannot do that from within CMakeLists.txt, thank you.")
    endif ()
    message(STATUS "Using toolchain file \"${CMAKE_TOOLCHAIN_FILE}\".")
endif ()

add_executable(visdriver
        src/audio_dsp.c
        src/input_plugin.c
        src/log.c
        src/main.c
        src/main_window.c
        src/output_plugin.c
        src/vis_plugin.c
        src/visualization.c
        )

if (MSVC)
    target_compile_definitions(visdriver PRIVATE _CRT_SECURE_NO_WARNINGS)
endif ()

# Request Windows >=Vista
# https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
target_compile_definitions(visdriver PRIVATE WINVER=0x0600 _WIN32_WINNT=0x0600)

target_include_directories(visdriver PRIVATE "${CMAKE_SOURCE_DIR}/src/thirdparty")
