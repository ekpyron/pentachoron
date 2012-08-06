# Copyright (c) 2011 Daniel Kirchner
#
# This file is part of pentachoron.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.
#
find_package (OpenGL)
find_package (FLTK REQUIRED)
find_package (PkgConfig)
pkg_check_modules (ASSIMP assimp)
pkg_check_modules (GLEW glew)

if (WIN32)
set(CMAKE_EXE_LINKER_FLAGS "-static")
endif ()

include_directories (${CMAKE_SOURCE_DIR}/libs/libpchm
		     ${ASSIMP_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS}
		     ${FLTK_INCLUDE_DIR} )
file (GLOB CONV2PCHM_SOURCES *.cpp)

add_executable (conv2pchm ${CONV2PCHM_SOURCES})
target_link_libraries (conv2pchm ${OPENGL_LIBRARIES} ${ASSIMP_LIBRARIES}
		      		 ${GLEW_LIBRARIES} ${FLTK_LIBRARIES} pchm)

set_property (TARGET conv2pchm PROPERTY
	     COMPILE_FLAGS -std=c++0x)
