AC_INIT([pentachoron], [0.0], [daniel@ekpyron.org])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
CXXFLAGS="$CXXFLAGS -std=c++0x"
AC_PROG_CXX
AC_LANG([C++])
AC_LANG_CPLUSPLUS

AC_ARG_WITH([glm],
	[AS_HELP_STRING([--with-glm=DIR],
			[location of the glm headers])],
	[CPPFLAGS="-I$withval ${CPPFLAGS}"])
AC_CHECK_HEADERS(glm/glm.hpp, [], AC_MSG_FAILURE([glm headers missing]))

PKG_CHECK_MODULES([DEPS], [oglp libpng freetype2 libglfw yaml-cpp])

DX_INIT_DOXYGEN($PACKAGE_NAME, doxygen.cfg)

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
