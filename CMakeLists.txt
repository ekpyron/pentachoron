# Copyright (c) 2011 Daniel Kirchner
#
# This file is part of pentachoron.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.
#

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_SOURCE_DIR}/cmake/Modules/")
cmake_minimum_required (VERSION 2.8)
project (pentachoron)

add_subdirectory (src)
add_subdirectory (utils/genpatches)
add_subdirectory (utils/conv2pchm)
add_subdirectory (libs/libpchm)
