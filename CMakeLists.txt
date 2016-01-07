#
# Project:  Pulsewave lidar access
# Purpose:  CMake script
# Author:   Kyle Shannon <kyle at pobox dot com>
#
# The MIT License (MIT)
#
# Copyright (c) 2015 Boise State University
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

cmake_minimum_required(VERSION 2.6)

project(pw)

find_library(PULSEWAVES_LIB pulsewaves NO_DEFAULT_PATH)
find_path(PULSEWAVES_INC inc NO_DEFAULT_PATH)

add_subdirectory(src)

include(InstallRequiredSystemLibraries)

# *****************************************************************************
# CPack variables
# *****************************************************************************
if(WIN32)
    set(CPACK_NSIS_PACKAGE_NAME pwaves-reader)
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")

    set(CPACK_NSIS_INSTALL_ROOT "C:")
    cmake_policy(PUSH)
    cmake_policy(SET CMP0010 OLD)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "pwaves-reader")
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "pwaves-reader")
    cmake_policy(POP)
    set(CPACK_NSIS_DISPLAY_NAME "pwaves-reader")

    set(CPACK_NSIS_MODIFY_PATH ${RI_NAME})

    set(CPACK_PACKAGE_EXECUTABLES "pwaves" "pwaves")

endif(WIN32)
Include(CPack)

