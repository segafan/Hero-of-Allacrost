dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Hero of Allacrost, 0.1, bugs@allacrost.org, allacrost)
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
CXXFLAGS=

dnl Check for map editor and Qt.
AC_MSG_CHECKING(whether to enable usage of map editor)
editor_default="no"
AC_ARG_ENABLE(editor, [--enable-editor=[no/yes] will build the map editor [default=no]], , enable_editor=$editor_default)
if test "x$enable_editor" = "xyes"; then
    AC_MSG_RESULT(yes)
    gw_CHECK_QT
else
    AC_MSG_RESULT(no)
fi
AM_CONDITIONAL([COND_EDITOR], [test "$enable_editor" = yes])

dnl Check for libraries.
AC_CHECK_LIB([GL], [glEnable], [], \
	[echo "You need to install the GL library."
	 exit -1])
AC_CHECK_LIB([GLU], [gluBuild2DMipmaps], [], \
	[echo "You need to install the GLU library."
	 exit -1])
AC_CHECK_LIB([IL], [ilInit], [], \
	[echo "You need to install the IL library."
	 exit -1])
AC_CHECK_LIB([ILUT], [iluInit], [], \
	[echo "You need to install the ILU library."
	 exit -1])
AC_CHECK_LIB([SDL], [SDL_InitSubSystem], [], \
	[echo "You need to install the SDL library."
	 exit -1])
AC_CHECK_LIB([SDL_mixer], [Mix_OpenAudio], [], \
	[echo "You need to install the SDL_mixer library."
	 exit -1])
AC_CHECK_LIB([SDL_ttf], [TTF_Init], [], \
	[echo "You need to install the SDL_ttf library."
	 exit -1])
AC_CHECK_LIB([lua], [lua_open], [], \
	[echo "You need to install the lua library."
	 exit -1])
AC_CHECK_LIB([lualib], [luaopen_table], [], \
	[echo "You need to install the lualib library."
	 exit -1])
AC_CHECK_LIB([m], [log], [], \
	[echo "You need to install the math library."
	 exit -1])
#AC_CHECK_LIB([X11], [main], [], \
#	[echo
#	 echo "**********"
#	 echo "You need to install the X11 library (comes with XFree or Xorg)" \
#	 	  "in order to use the Map Editor."
#	 echo "**********"
#	 echo])
#AC_CHECK_LIB([Xext], [main], [], \
#	[echo
#	 echo "**********"
#	 echo "You need to install the Xext library in order to use the Map Editor."
#	 echo "**********"
#	 echo])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

dnl Checks for library functions.
AC_CHECK_FUNCS([atexit sqrt])

AC_CONFIG_FILES([Makefile])
dnl				 src/Makefile
dnl				 src/map_editor/Makefile])
										  
AC_OUTPUT

echo ""
echo "Now type 'make'."
echo ""
