# Copyright (c) 2011 Daniel Kirchner
#
# This file is part of pentachoron.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.
#
find_package (PkgConfig)
pkg_check_modules (ASSIMP assimp)

if (WIN32)
set(CMAKE_EXE_LINKER_FLAGS "-static")
find_package (ZLIB REQUIRED)
endif ()

include_directories (${ASSIMP_INCLUDE_DIRS}
		     ${CMAKE_SOURCE_DIR}/libs/libpchm)
file (GLOB GENPATCHES_SOURCES *.cpp)

add_executable (genpatches ${GENPATCHES_SOURCES})
target_link_libraries (genpatches ${ASSIMP_LIBRARIES} pchm
                       ${ZLIB_LIBRARIES})

set_property (TARGET genpatches PROPERTY
	     COMPILE_FLAGS -std=c++0x)
