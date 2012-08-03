# Copyright (c) 2011 Daniel Kirchner
#
# This file is part of pentachoron.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.
#
find_package (Freetype)
find_package (ZLIB)
if (WIN32)
find_package (OpenGL)
endif ()
find_package (PkgConfig)
pkg_check_modules (GLFW libglfw)
pkg_check_modules (OGLP oglp)
pkg_check_modules (YAML_CPP yaml-cpp)

if (WIN32)
set(CMAKE_EXE_LINKER_FLAGS "-static")
endif ()

include_directories (${CMAKE_SOURCE_DIR}/include/ ${GLFW_INCLUDE_DIRS}
		     ${FREETYPE_INCLUDE_DIRS} ${OGLP_INCLUDE_DIRS}
		     ${YAML_CPP_INCLUDE_DIRS})
file (GLOB PENTACHORON_SOURCES *.cpp model/*.cpp font/*.cpp)

add_executable (pentachoron ${PENTACHORON_SOURCES})
target_link_libraries (pentachoron ${GLFW_LIBRARIES} ${FREETYPE_LIBRARIES}
		       ${OGLP_LIBRARIES} ${YAML_CPP_LIBRARIES} ${OPENGL_LIBRARIES}
		       ${ZLIB_LIBRARIES})

set_property (TARGET pentachoron PROPERTY
	     COMPILE_FLAGS -std=c++0x)
