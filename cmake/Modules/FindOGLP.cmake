#
# Try to find OGLP library and include path.
# Once done this will define
#
# OGLP_FOUND
# OGLP_INCLUDE_DIR
# OGLP_LIBRARIES
#

find_path (OGLP_INCLUDE_DIR oglp/oglp.h)

if (OGLP_INCLUDE_DIR)
set (OGLP_FOUND 1 CACHE STRING "Set to 1 if OGLP is found, 0 otherwise")
else (OGLP_INCLUDE_DIR)
set (OGLP_FOUND 0 CACHE STRING "Set to 1 if OGLP is found, 0 otherwise")
endif (OGLP_INCLUDE_DIR)

mark_as_advanced (OGLP_FOUND)
