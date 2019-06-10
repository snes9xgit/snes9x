AC_PREREQ([2.60])
AC_INIT([Snes9x], [1.60], [], [snes9x])
AC_REVISION([$Revision: 1.60 $])

AC_CONFIG_SRCDIR([unix.cpp])

AC_CANONICAL_TARGET

AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++])

S9XFLGS=""
S9XDEFS=""
S9XLIBS=""

AC_DEFUN([AC_S9X_COMPILER_FLAG],
[
	AC_MSG_CHECKING([whether g++ accepts $1])

	AC_CACHE_VAL([snes9x_cv_option_$2],
	[
		OLD_CXXFLAGS="[$]CXXFLAGS"
		CXXFLAGS="[$]OLD_CXXFLAGS $1"

		AC_TRY_RUN(
		[
			int	foo;

			int	main (int argc, char **argv)
			{
				/* The following code triggs gcc:s generation of aline opcodes,
				   which some versions of as does not support. */

				if (argc > 0)
					argc = 0;

				return (argc);
			}
		],
		[snes9x_cv_option_$2="yes"], [snes9x_cv_option_$2="no"])
	])

	CXXFLAGS="[$]OLD_CXXFLAGS"

	if test "x[$]snes9x_cv_option_$2" = "xyes"; then
		S9XFLGS="[$]S9XFLGS $1"
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
		$3
	fi
])

# *****************************
# *** Execution begins here ***
# *****************************

# Test what compiler flags we should use.

AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug],
		[leave debug information in the final binary (default: no)])],
	[], [enable_debug="no"])

if test "x$enable_debug" = "xyes"; then
	AC_S9X_COMPILER_FLAG([-g],  [g])
	AC_S9X_COMPILER_FLAG([-O0], [o0])
else
	AC_S9X_COMPILER_FLAG([-O3], [o3], [
		AC_S9X_COMPILER_FLAG([-O2], [o2], [
			AC_S9X_COMPILER_FLAG([-O1], [o1])])])
	AC_S9X_COMPILER_FLAG([-fomit-frame-pointer], [omit_frame_pointer])
fi

AC_ARG_ENABLE([mtune],
	[AS_HELP_STRING([--enable-mtune],
		[use the specified value for the -mtune/-mcpu flag (default: no)])],
	[], [enable_mtune="no"])

if test "x$enable_mtune" != "xno"; then
	AC_S9X_COMPILER_FLAG([-mtune="$enable_mtune"], [mtune],
	[
		AC_MSG_WARN([-mtune failed, trying -mcpu...])
		AC_S9X_COMPILER_FLAG([-mcpu="$enable_mtune"], [mcpu],
			[AC_MSG_ERROR([Please specify a working value for --enable-mtune.])])
	])
fi

AC_S9X_COMPILER_FLAG([-fno-exceptions],       [no_exceptions])
AC_S9X_COMPILER_FLAG([-fno-rtti],             [no_rtti])
AC_S9X_COMPILER_FLAG([-pedantic],             [pedantic])
AC_S9X_COMPILER_FLAG([-Wall],                 [Wall])
AC_S9X_COMPILER_FLAG([-W],                    [W])
AC_S9X_COMPILER_FLAG([-Wno-unused-parameter], [Wno_unused_parameter])

# Check if the OS is Linux.

AC_MSG_CHECKING([whether the OS is Linux])

AC_CACHE_VAL([snes9x_cv_linux_os],
[
	case "$target" in
		*-*-linux*)
			snes9x_cv_linux_os="yes"
			;;
		*)
			snes9x_cv_linux_os="no"
			;;
	esac
])

AC_MSG_RESULT([$snes9x_cv_linux_os])

# Enable gamepad support on Linux.

AC_ARG_ENABLE([gamepad],
	[AS_HELP_STRING([--enable-gamepad],
		[enable gamepad support if available (default: yes)])],
	[], [enable_gamepad="yes"])

if test "x$enable_gamepad" = "xyes"; then
	if test "x$snes9x_cv_linux_os" = "xyes"; then
		S9XDEFS="$S9XDEFS -DJOYSTICK_SUPPORT"
	else
		AC_MSG_WARN([Your OS is not Linux. Build without gamepad support.])
		enable_gamepad="no"
	fi
fi

# Enable debugger.

S9XDEBUGGER="#S9XDEBUGGER=1"

AC_ARG_ENABLE([debugger],
	[AS_HELP_STRING([--enable-debugger],
		[enable debugger (default: no)])],
	[], [enable_debugger="no"])

if test "x$enable_debugger" = "xyes"; then
	S9XDEBUGGER="S9XDEBUGGER=1"
	S9XDEFS="$S9XDEFS -DDEBUGGER"
fi

# Enable netplay support if requested.

S9XNETPLAY="#S9XNETPLAY=1"

AC_ARG_ENABLE([netplay],
	[AS_HELP_STRING([--enable-netplay],
		[enable netplay support (default: no)])],
	[], [enable_netplay="no"])

if test "x$enable_netplay" = "xyes"; then
	S9XNETPLAY="S9XNETPLAY=1"
	S9XDEFS="$S9XDEFS -DNETPLAY_SUPPORT"
fi

# Enable GZIP support through zlib.

AC_CACHE_VAL([snes9x_cv_zlib],
[
	AC_CHECK_HEADER([zlib.h],
		[AC_CHECK_LIB([z], [gzread], [snes9x_cv_zlib="yes"], [snes9x_cv_zlib="no"])],
		[snes9x_cv_zlib="no"])
])

AC_ARG_ENABLE([gzip],
	[AS_HELP_STRING([--enable-gzip],
		[enable GZIP support through zlib (default: yes)])],
	[], [enable_gzip="yes"])

if test "x$enable_gzip" = "xyes"; then
	if test "x$snes9x_cv_zlib" = "xyes"; then
		S9XDEFS="$S9XDEFS -DZLIB"
		S9XLIBS="$S9XLIBS -lz"
	else
		AC_MSG_WARN([zlib not found. Build without GZIP support.])
		enable_gzip="no"
	fi
fi

# Enable ZIP support through zlib.

S9XZIP="#S9XZIP=1"

AC_ARG_ENABLE([zip],
	[AS_HELP_STRING([--enable-zip],
		[enable ZIP support through zlib (default: yes)])],
	[], [enable_zip="yes"])

S9X_SYSTEM_ZIP="#SYSTEM_ZIP=1"

AC_ARG_WITH([system-zip],
	[AS_HELP_STRING([--with-system-zip],
		[Use system zip (default: check)])],
	[], [with_system_zip="check"])

if test "x$enable_zip" = "xyes"; then
	if test "x$with_system_zip" != "xno"; then
		PKG_CHECK_MODULES(
			SYSTEM_ZIP,
			minizip,
			S9XZIP="S9XZIP=1"
			S9XDEFS="$S9XDEFS -DUNZIP_SUPPORT"
			S9X_SYSTEM_ZIP="SYSTEM_ZIP=1"
			S9XLIBS="$S9XLIBS $SYSTEM_ZIP_LIBS"
			if test "x$enable_gzip" = "xno"; then
				S9XLIBS="$S9XLIBS -lz"
			fi
			S9XDEFS="$S9XDEFS -DSYSTEM_ZIP",
			if test "x${with_system_zip}" != "xcheck"; then
				AC_MSG_ERROR([--with-system-zip requested but no proper minizip lib found.])
			else
				AC_MSG_WARN([minizip not found. Build without SYSTEM_ZIP support.])
			fi
		)
	else
		if test "x$snes9x_cv_zlib" = "xyes"; then
			S9XZIP="S9XZIP=1"
			S9XDEFS="$S9XDEFS -DUNZIP_SUPPORT"
			if test "x$enable_gzip" = "xno"; then
				S9XLIBS="$S9XLIBS -lz"
			fi
		else
			AC_MSG_WARN([zlib not found. Build without ZIP support.])
			enable_zip="no"
		fi
	fi
fi

# Enable JMA support.

S9XJMA="#S9XJMA=1"

AC_ARG_ENABLE([jma],
	[AS_HELP_STRING([--enable-jma],
		[enable JMA support (default: yes)])],
	[], [enable_jma="yes"])

if test "x$enable_jma" = "xyes"; then
	S9XJMA="S9XJMA=1"
	S9XDEFS="$S9XDEFS -DJMA_SUPPORT"
fi

# Enable screenshot support through libpng.

AC_CACHE_VAL([snes9x_cv_libpng],
[
	AC_CHECK_HEADER([png.h],
		[AC_CHECK_LIB([png], [png_init_io], [snes9x_cv_libpng="yes"], [snes9x_cv_libpng="no"])],
		[snes9x_cv_libpng="no"])
])

AC_ARG_ENABLE([screenshot],
	[AS_HELP_STRING([--enable-screenshot],
		[enable screenshot support through libpng (default: yes)])],
	[], [enable_screenshot="yes"])

if test "x$enable_screenshot" = "xyes"; then
	if test "x$snes9x_cv_libpng" = "xyes"; then
		S9XDEFS="$S9XDEFS -DHAVE_LIBPNG"
		S9XLIBS="$S9XLIBS -lpng"
	else
		AC_MSG_WARN([libpng not found. Build without screenshot support.])
		enable_screenshot="no"
	fi
fi

# Check for functions

AC_CHECK_FUNC([mkstemp],
[
	S9XDEFS="$S9XDEFS -DHAVE_MKSTEMP"
])

# Check X11

AC_PATH_XTRA
if test "x$no_x" = "xyes"; then
	AC_MSG_ERROR([X11 is required.])
else
	S9XFLGS="$S9XFLGS $X_CFLAGS"
	S9XLIBS="$S9XLIBS $X_PRE_LIBS -lX11 -lXext $X_LIBS $X_EXTRA_LIBS"
fi

# Check for headers

snes9x_have_stdint_h="";

AC_CHECK_HEADER([strings.h],
[
	S9XDEFS="$S9XDEFS -DHAVE_STRINGS_H"
])

AC_CHECK_HEADER([sys/ioctl.h],
[
	S9XDEFS="$S9XDEFS -DHAVE_SYS_IOCTL_H"
])

AC_CHECK_HEADER([stdint.h],
[
	S9XDEFS="$S9XDEFS -DHAVE_STDINT_H"
	snes9x_have_stdint_h="-DHAVE_STDINT_H"
])

AC_CHECK_HEADERS([unistd.h sys/socket.h])

# Check whether the size of pointer is int.

if test "x$snes9x_have_stdint_h" = "x"; then
	AC_MSG_CHECKING([whether the size of pointer is int])
	
	AC_TRY_RUN(
	[
		int main (void)
		{
			return (!(sizeof(void *) == sizeof(int)));
		}
	],
	[snes9x_ptr_is_int="yes"], [snes9x_ptr_is_int="no"])
	
	if test "x$snes9x_ptr_is_int" = "xyes"; then
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
		S9XDEFS="$S9XDEFS -DPTR_NOT_INT"
	fi
fi

# Check whether right shift is arithmetic or not

AC_DEFUN([AC_S9X_CHECK_SAR],
[
	AC_MSG_CHECKING([whether right shift $1 is arithmetic])

	OLD_CXXFLAGS="[$]CXXFLAGS"
	CXXFLAGS="[$]OLD_CXXFLAGS $snes9x_have_stdint_h"

	AC_TRY_RUN(
	[
		#ifdef HAVE_STDINT_H
		#include <stdint.h>
		typedef int8_t			int8;
		typedef int16_t			int16;
		typedef int32_t			int32;
		typedef int64_t			int64;
		#else 
		typedef signed char		int8;
		typedef signed short	int16;
		typedef signed int		int32;
		#ifdef __GNUC__
		__extension__
		#endif
		typedef long long		int64;
		#endif

		int main (void)
		{
			$1	i;

			i = -1;
			i >>= 1;

			return (i < 0 ? 0 : 1);
		}
	],
	[snes9x_sar_$1="yes"], [snes9x_sar_$1="no"])

	CXXFLAGS="[$]OLD_CXXFLAGS"

	if test "x$snes9x_sar_$1" = "xno"; then
		AC_MSG_RESULT([no])
	else
		S9XDEFS="$S9XDEFS -DRIGHTSHIFT_$1_IS_SAR"
		AC_MSG_RESULT([yes])
	fi
])

AC_S9X_CHECK_SAR([int8])
AC_S9X_CHECK_SAR([int16])
AC_S9X_CHECK_SAR([int32])
AC_S9X_CHECK_SAR([int64])

if test "x$snes9x_sar_int8" = "xyes" -a "x$snes9x_sar_int16" = "xyes" -a "x$snes9x_sar_int32" = "xyes" -a "x$snes9x_sar_int64" = "xyes"; then
	S9XDEFS="`echo \"$S9XDEFS\" | sed -e 's/-DRIGHTSHIFT_int8_IS_SAR//'`"
	S9XDEFS="`echo \"$S9XDEFS\" | sed -e 's/-DRIGHTSHIFT_int16_IS_SAR//'`"
	S9XDEFS="`echo \"$S9XDEFS\" | sed -e 's/-DRIGHTSHIFT_int32_IS_SAR//'`"
	S9XDEFS="`echo \"$S9XDEFS\" | sed -e 's/-DRIGHTSHIFT_int64_IS_SAR//'`"
	S9XDEFS="$S9XDEFS -DRIGHTSHIFT_IS_SAR"
fi

# Check if we can build with Xvideo acceleration support
AC_ARG_ENABLE([xvideo],
	[AS_HELP_STRING([--enable-xvideo],
		[enable Xvideo if available (default: yes)])],
	[], [enable_xvideo="yes"])

if test "x$enable_xvideo" = "xyes"; then
	enable_xvideo="no"
	AC_CHECK_HEADER([X11/extensions/Xv.h],
	[
		enable_xvideo="yes"
		S9XLIBS="$S9XLIBS -lXv"
		S9XDEFS="$S9XDEFS -DUSE_XVIDEO"
	])
fi

# Check if we can build with Xinerama multi-monitor support
AC_ARG_ENABLE([xinerama],
	[AS_HELP_STRING([--enable-xinerama],
		[enable Xinerama if available (default: yes)])],
	[], [enable_xinerama="yes"])

if test "x$enable_xinerama" = "xyes"; then
	enable_xinerama="no"
	AC_CHECK_HEADER([X11/extensions/Xinerama.h],
	[
		enable_xinerama="yes"
		S9XLIBS="$S9XLIBS -lXinerama"
		S9XDEFS="$S9XDEFS -DUSE_XINERAMA"
	])
fi

# Check if we have sound code for this platform.

AC_ARG_ENABLE([sound],
	[AS_HELP_STRING([--enable-sound],
		[enable sound if available (default: yes)])],
	[], [enable_sound="yes"])

if test "x$enable_sound" = "xyes"; then
	AC_MSG_CHECKING([whether sound is supported on this platform])
	if test "x$snes9x_cv_linux_os" = "xyes"; then
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
		AC_MSG_WARN([Your OS is not Linux. Build without sound support.])
		enable_sound="no"
	fi
fi

if test "x$enable_sound" = "xyes"; then
	AC_CHECK_HEADER([pthread.h],
	[
		S9XDEFS="$S9XDEFS -DUSE_THREADS"
		S9XLIBS="$S9XLIBS -lpthread"
	])
else
	S9XDEFS="$S9XDEFS -DNOSOUND"
fi

# Output.

S9XFLGS="$S9XFLGS $CXXFLAGS $CPPFLAGS $LDFLAGS"
S9XLIBS="$LIBS $S9XLIBS"

S9XFLGS="`echo \"$S9XFLGS\" | sed -e 's/  */ /g'`"
S9XDEFS="`echo \"$S9XDEFS\" | sed -e 's/  */ /g'`"
S9XLIBS="`echo \"$S9XLIBS\" | sed -e 's/  */ /g'`"
S9X_SYSTEM_ZIP="`echo \"$S9X_SYSTEM_ZIP\" | sed -e 's/  */ /g'`"
S9XFLGS="`echo \"$S9XFLGS\" | sed -e 's/^  *//'`"
S9XDEFS="`echo \"$S9XDEFS\" | sed -e 's/^  *//'`"
S9XLIBS="`echo \"$S9XLIBS\" | sed -e 's/^  *//'`"
S9X_SYSTEM_ZIP="`echo \"$S9X_SYSTEM_ZIP\" | sed -e 's/^  *//'`"

AC_SUBST(S9XFLGS)
AC_SUBST(S9XDEFS)
AC_SUBST(S9XLIBS)
AC_SUBST(S9XXVIDEO)
AC_SUBST(S9XDEBUGGER)
AC_SUBST(S9XNETPLAY)
AC_SUBST(S9XZIP)
AC_SUBST(S9XJMA)
AC_SUBST(S9X_SYSTEM_ZIP)

rm config.info 2>/dev/null

cat >config.info <<EOF

build information:
cc...............,,,. $CC
c++.................. $CXX
options.............. $S9XFLGS
defines.............. $S9XDEFS
libs................. $S9XLIBS

features:
Xvideo support....... $enable_xvideo
Xinerama support..... $enable_xinerama
sound support........ $enable_sound
screenshot support... $enable_screenshot
netplay support...... $enable_netplay
gamepad support...... $enable_gamepad
GZIP support......... $enable_gzip
ZIP support.......... $enable_zip
SYSTEM_ZIP........... $with_system_zip
JMA support.......... $enable_jma
debugger............. $enable_debugger

EOF

cat config.info

AC_OUTPUT(Makefile)
