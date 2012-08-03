#
# Try to find OGLP library and include path.
# Once done this will define
#
# OGLP_FOUND
# OGLP_INCLUDE_DIR
# OGLP_LIBRARY
#

find_path (OGLP_INCLUDE_DIR oglp/oglp.h)
find_library (OGLP_LIBRARY NAMES oglp)

if (GLEW_INCLUDE_DIR)
set (GLEW_FOUND 1 CACHE STRING "Set to 1 if GLEW is found, 0 otherwise")
else (GLEW_INCLUDE_DIR)
set (GLEW_FOUND 0 CACHE STRING "Set to 1 if GLEW is found, 0 otherwise")
endif (GLEW_INCLUDE_DIR)

mark_as_advanced (GLEW_FOUND)
