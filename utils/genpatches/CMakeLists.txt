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
find_package (PkgConfig)
pkg_check_modules (GLFW libglfw)
pkg_check_modules (ASSIMP assimp)
pkg_check_modules (GLEW glew)

if (WIN32)
set(CMAKE_EXE_LINKER_FLAGS "-static")
endif ()

include_directories (${GLFW_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS}
		     ${GLEW_INCLUDE_DIRS})
file (GLOB GENPATCHES_SOURCES *.cpp)

add_executable (genpatches ${GENPATCHES_SOURCES})
target_link_libraries (genpatches ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES}
		       ${ASSIMP_LIBRARIES} ${GLEW_LIBRARIES})

set_property (TARGET genpatches PROPERTY
	     COMPILE_FLAGS -std=c++0x)
