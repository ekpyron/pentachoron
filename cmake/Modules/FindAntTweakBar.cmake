#
# Try to find AntTweakBar library and include path.
# Once done this will define
#
# ANTTWEAKBAR_FOUND
# ANTTWEAKBAR_INCLUDE_DIR
# ANTTWEAKBAR_LIBRARY
#

find_path (ANTTWEAKBAR_INCLUDE_DIR AntTweakBar.h)
find_library (ANTTWEAKBAR_LIBRARY NAMES AntTweakBar)

if (ANTTWEAKBAR_INCLUDE_DIR)
set (ANTTWEAKBAR_FOUND 1 CACHE STRING "Set to 1 if AntTweakBar is found, 0 otherwise")
else (ANTTWEAKBAR_INCLUDE_DIR)
set (ANTTWEAKBAR_FOUND 0 CACHE STRING "Set to 1 if AntTweakBar is found, 0 otherwise")
endif (ANTTWEAKBAR_INCLUDE_DIR)

mark_as_advanced (ANTTWEAKBAR_FOUND)
