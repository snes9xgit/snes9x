AC_INIT([snes9x-gtk], 1.55)

AC_CONFIG_MACRO_DIR([macros])

if test -z "$CFLAGS"; then
    CFLAGS="-Os -pipe"
fi
if test -z "$CXXFLAGS"; then
    CXXFLAGS="$CFLAGS"
fi

EXTRA_FLAGS="-Wall -W -pedantic -Wno-unused-parameter -Wno-unused-but-set-variable"
CFLAGS="$CFLAGS $EXTRA_FLAGS"
CXXFLAGS="$CXXFLAGS $EXTRA_FLAGS"

AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_LANG_CPLUSPLUS

AX_CXX_COMPILE_STDCXX([11], [noext], [optional])

AC_PROG_INSTALL
AM_INIT_AUTOMAKE([foreign subdir-objects])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_PROG_CC_C_O
IT_PROG_INTLTOOL([0.37.0])
GETTEXT_PACKAGE=[snes9x-gtk]
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE")

AM_GLIB_GNU_GETTEXT

snes9xlocaledir='${prefix}/${DATADIRNAME}/locale'
AC_SUBST(snes9xlocaledir)

AC_ARG_WITH(debug,
  [AS_HELP_STRING([--with(out)-debug],
    [Leave debug information in the final binary (default: without)])],
  [],
  [with_debug=no])

AC_ARG_WITH(opengl,
  [AS_HELP_STRING([--with(out)-opengl],
    [Use OpenGL for output (default: with)])],
  [],
  [with_opengl=yes])

AC_ARG_WITH(joystick, 
  [AS_HELP_STRING([--with(out)-joystick],
    [Enable joystick support if available (default: with)])],
  [],
  [with_joystick=yes])

AC_ARG_WITH(xv, 
  [AS_HELP_STRING([--with(out)-xv],
    [Enable XVideo output on GTK (default: with)])],
  [],
  [with_xv=yes])

AC_ARG_WITH(xrandr, 
  [AS_HELP_STRING([--with(out)-xrandr],
    [Enable XRandR support on GTK (default: with)])],
  [],
  [with_xrandr=yes])

AC_ARG_WITH(portaudio, 
  [AS_HELP_STRING([--with(out)-portaudio],
    [Enable PortAudio sound driver support (default: with)])],
  [],
  [with_portaudio=yes])

AC_ARG_WITH(oss, 
  [AS_HELP_STRING([--with(out)-oss],
    [Enable OSS sound driver support (default: with)])],
  [],
  [with_oss=yes])

AC_ARG_WITH(alsa,
  [AS_HELP_STRING([--with(out)-alsa],
     [Enable ALSA sound driver support (default: with)])],
  [],
  [with_alsa=yes])

AC_ARG_WITH(pulseaudio,
  [AS_HELP_STRING([--with(out)-pulseaudio],
    [Enable PulseAudio sound driver support (default: with)])],
  [],
  [with_pulseaudio=yes])

AC_ARG_WITH(debugger, 
  [AS_HELP_STRING([--with(out)-debugger],
    [Enable debugger (default: without)])],
  [],
  [with_debugger=no])

AC_ARG_WITH(sdd1-decomp,
  [AS_HELP_STRING([--with(out)-sdd1-decomp],
    [Use SDD1 decompression (default: with)])],
  [],
  [with_sdd1_decomp=yes])

AC_ARG_WITH(hq2x, 
  [AS_HELP_STRING([--with(out)-hq2x],
    [Enable support for the HQ2x family of filters (default: with)])],
  [],
  [with_hq2x=yes])

AC_ARG_WITH(xbrz, 
  [AS_HELP_STRING([--with(out)-xbrz],
    [Enable support for the xBRZ filter (default: with)])],
  [],
  [with_xbrz=yes])

AC_ARG_WITH(netplay,
  [AS_HELP_STRING([--with(out)-netplay],
    [Enable netplay support (default: without)])],
  [],
  [with_netplay=yes])

AC_ARG_WITH(jma-decomp, 
  [AS_HELP_STRING([--with(out)-jma-decomp],
    [Enable JMA archive decompression (default: with)])],
  [],
  [with_jma_decomp=yes])

AC_ARG_WITH(newblend,
  [AS_HELP_STRING([--with(out)-newblend],
    [New colour blender (default: with)])],
  [],
  [with_newblend=yes])

AC_ARG_WITH(zlib,
  [AS_HELP_STRING([--with(out)-zlib],
    [Support gzip decompression])],
  [],
  [with_zlib=yes])

AC_ARG_WITH(system-zip,
  [AS_HELP_STRING([--with(out)-system-zip],
    [Use system zip])],
  [],
  [system_zip=check])

AC_ARG_WITH(screenshot,
  [AS_HELP_STRING([--with(out)-screenshot],
    [Screenshot support through libpng if available (default: with)])],
  [],
  [with_screenshot=yes])

AC_ARG_WITH(gtk3,
  [AS_HELP_STRING([--with(out)-gtk3],
    [Build with GTK+ 3 if available (default: without)])],
  [],
  [with_gtk3=no])

if test yes = "$with_debug" ; then
  CFLAGS="$CFLAGS -g"
else
  CFLAGS="$CFLAGS -fomit-frame-pointer"
fi

LIBS="$LIBS -lX11 -ldl -lXext"

ZSNESFX='#ZSNESFX=yes'
ZSNESC4='#ZSNESC4=yes'
ASMCPU=''
SDD1_DECOMP='#SDD1_DECOMP=yes'
JMA='#JMA=yes'
SPC700ASM=''
I386SPC=''
CPUINC=''
NETPLAY='#NETPLAY=yes'

SYSLIBS=''
NASM='not_found'

OPENGL=0

if test yes = "$with_opengl" ; then
  OPENGL=yes
fi

GTK=yes
snes9x_info_display="GTK"

GTK3_WARNING=no
if test yes = "$with_gtk3" ; then
    GTK_VERSION="gtk+-3.0"
    GTK3_WARNING=yes
    CFLAGS="$CFLAGS -DUSE_GTK3"
else
    GTK_VERSION="gtk+-2.0 >= 2.16"
fi

PKG_CHECK_MODULES([GTK], ["$GTK_VERSION"])
PKG_CHECK_MODULES([GLIB], [gthread-2.0 >= 2.6 gobject-2.0 >= 2.6])
PKG_CHECK_MODULES([LIBXML], [libxml-2.0 >= 2.0])

if test yes = "$with_opengl" ; then

    AC_CHECK_LIB(GL,glGetString,[
        OPENGL=yes
        CFLAGS="$CFLAGS -DUSE_OPENGL"
        LIBS="$LIBS -lGL"
    ],[
        OPENGL=0
    ])
fi

GTKJOY=0
SDL_CFLAGS=""
SDL_LIBS=""
if test yes = "$with_joystick"; then
    PKG_CHECK_MODULES([SDL],[sdl],[
        GTKJOY=yes
        CFLAGS="$CFLAGS -DUSE_JOYSTICK"
    ],[
        AC_CHECK_PROG(SDLCONFIGFOUND,[sdl-config],yes,no)
        
        if test yes = "$SDLCONFIGFOUND" ; then
            SDL_CFLAGS="-DUSE_JOYSTICK `sdl-config --cflags`"
            SDL_LIBS="`sdl-config --libs`"
            GTKJOY=yes
        else
            echo "Cannot find SDL. Disabling joystick support."
        fi
    ])
fi

XV=0
XV_CFLAGS=""
XV_LIBS=""
if test yes = "$with_xv" ; then
    PKG_CHECK_MODULES([XV],[xv],[
        XV=yes
        CFLAGS="$CFLAGS -DUSE_XV"
    ],[
        echo "Cannot find libXv. Make sure the X11 development headers are installed."
        echo "--> Disabling Xv support"
    ])
fi

PORTAUDIO=0
PORTAUDIO_CFLAGS=""
PORTAUDIO_LIBS=""
if test yes = "$with_portaudio" ; then
    PKG_CHECK_MODULES([PORTAUDIO], [portaudio-2.0 >= 10],[
        PORTAUDIO=yes
        CFLAGS="$CFLAGS -DUSE_PORTAUDIO"
    ],[
        echo "Cannot find PortAudio."
        echo "--> Disabling PortAudio support"
    ])
fi

OSS=0
if test yes = "$with_oss" ; then

    AC_CHECK_HEADER(sys/soundcard.h, [
      CFLAGS="$CFLAGS -DUSE_OSS"
      OSS=yes
    ],[
        echo "Cannot find OSS headers."
        echo "--> Disabling OSS support"
    ])    
fi

ALSA=0
ALSA_CFLAGS=""
ALSA_LIBS=""
if test yes = "$with_alsa" ; then

    PKG_CHECK_MODULES([ALSA], [alsa],[
        ALSA=yes
        CFLAGS="$CFLAGS -DUSE_ALSA"
    ],[
        echo "Cannot find ALSA development support."
        echo "--> Disabling ALSA"
    ])
fi

PULSEAUDIO=0
PULSEAUDIO_CFLAGS=""
PULSEAUDIO_LIBS=""
if test yes = "$with_pulseaudio" ; then
    PKG_CHECK_MODULES([PULSEAUDIO], [libpulse-simple],[
        PULSEAUDIO=yes
        CFLAGS="$CFLAGS -DUSE_PULSEAUDIO"
    ],[
        echo "Cannot find PulseAudio library."
        echo "--> Disabling PulseAudio"
    ])
fi

XRANDR=0
XRANDR_CFLAGS=""
XRANDR_LIBS=""
if test yes = "$with_xrandr" ; then
    PKG_CHECK_MODULES([XRANDR],[xrandr],[
        XRANDR=yes
        CFLAGS="$CFLAGS -DUSE_XRANDR"
    ],[
        echo "Cannot find libXrandr. Make sure the X11 development headers are installed."
        echo "--> Disabling Xrandr support."
    ])
fi

LIBPNG_CFLAGS=""
LIBPNG_LIBS=""
if test yes = "$with_screenshot"; then
    PKG_CHECK_MODULES([LIBPNG],[libpng],[
        CFLAGS="$CFLAGS -DHAVE_LIBPNG"
    ],[
        echo "Cannot find libpng."
        echo "--> Disabling screenshot support."
    ])
fi

SYSTEM_ZIP=0
SYSTEM_ZIP_CFLAGS=""
SYSTEM_ZIP_LIBS=""
ZIP_CFLAGS="-I../unzip"
if test no != "$with_system_zip" && test yes = "$with_zlib" ; then
    PKG_CHECK_MODULES([SYSTEM_ZIP],[minizip],[
	ZIP_CFLAGS=""
	SYSTEM_ZIP=yes
	CFLAGS="$CFLAGS $SYSTEM_ZIP_CFLAGS -DSYSTEM_ZIP"
	LIBS="$LIBS $SYSTEM_ZIP_LIBS"
    ],[
	if test check = "$with_system_zip"; then
	    AC_MSG_WARN(Cannot find system minizip library)
	else
	    AC_MSG_ERROR(--with-system-zip given but cannot find system minizip library)
	fi
    ])
fi

if test yes = "$with_hq2x" ; then
    HQ2X=yes
    CFLAGS="$CFLAGS -DUSE_HQ2X"
else
    HQ2X=0
fi

if test yes = "$with_xbrz" ; then
    XBRZ=yes
    CFLAGS="$CFLAGS -DUSE_XBRZ"
else
    XBRZ=0
fi

CFLAGS="$CFLAGS $GTK_CFLAGS $GLIB_CFLAGS $LIBXML_CFLAGS $PORTAUDIO_CFLAGS $LIBPNG_CFLAGS $SDL_CFLAGS $XV_CFLAGS $XRANDR_CFLAGS $ALSA_CFLAGS $PULSEAUDIO_CFLAGS"
LIBS="$LIBS $GTK_LIBS $GLIB_LIBS $LIBXML_LIBS $PORTAUDIO_LIBS $LIBPNG_LIBS $SDL_LIBS $XV_LIBS $XRANDR_LIBS $ALSA_LIBS $PULSEAUDIO_LIBS"

NASM="not_found"
ZSNESC4=no
ZSNESFX=no

CFLAGS="$CFLAGS -DCORRECT_VRAM_READS"
CORRECT_VRAM_READS=1

if test yes = "$with_debugger"; then
   CFLAGS="$CFLAGS -DDEBUGGER"
fi

dnl Enable SDD1 decompression if requested
if test yes = "$with_sdd1_decomp"; then
   CFLAGS="$CFLAGS -DSDD1_DECOMP"
   SDD1_DECOMP=yes
fi

dnl Enable Netplay support if requested
if test yes = "$with_netplay"; then
   CFLAGS="$CFLAGS -DNETPLAY_SUPPORT"
   NETPLAY=yes
fi

dnl Enable jma decompression (nachs own format)
if test yes = "$with_jma_decomp"; then
   CFLAGS="$CFLAGS -DJMA_SUPPORT"
   JMA=yes
fi

if test yes = "$with_zlib"; then
  AC_CHECK_HEADERS(zlib.h)
  AC_CHECK_LIB(z, gzread, [
        CFLAGS="$CFLAGS -DZLIB"
        LIBS="$LIBS -lz"
      ], ,)
fi

dnl Check for functions
AC_CHECK_FUNC(mkstemp, [
  CFLAGS="$CFLAGS -DHAVE_MKSTEMP"
])

dnl Check for headers

AC_CHECK_HEADER(strings.h, [
  CFLAGS="$CFLAGS -DHAVE_STRINGS_H"
])

AC_CHECK_HEADER(sys/ioctl.h, [
  CFLAGS="$CFLAGS -DHAVE_SYS_IOCTL_H"
])

AC_CHECK_HEADER(stdint.h, [
  CFLAGS="$CFLAGS -DHAVE_STDINT_H"
])


dnl Check SAR

AC_DEFUN([AC_MY_CHECK_SAR],
[
AC_MSG_CHECKING([if rightshift $1 is arithmetic])
AC_CACHE_VAL(ac_cv_my_sar_$1,
[
AC_TRY_RUN([
#include "../port.h"

int main(void){
    $1 i;

    i=-1;
    i>>=1;
    return(i<0?0:1);
}
],ac_cv_my_sar_$1=yes,ac_cv_my_sar_$1=no,ac_cv_my_sar_%1=no)
])

if test $ac_cv_my_sar_$1 = no; then
  AC_MSG_RESULT(no)
else
  CFLAGS="$CFLAGS -DRIGHTSHIFT_$1_IS_SAR"
  AC_MSG_RESULT(yes)
fi
])

AC_MSG_CHECKING(if sizeof(void *) == sizeof(int))
AC_TRY_RUN([
  int main()
  {
   return(!(sizeof(void *) == sizeof(int)));
  }
], snes9x_ptr_is_int=yes)
if test x$snes9x_ptr_is_int = xyes; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  CFLAGS="$CFLAGS -DPTR_NOT_INT"
fi

AC_MY_CHECK_SAR(int8)
AC_MY_CHECK_SAR(int16)
AC_MY_CHECK_SAR(int32)
AC_MY_CHECK_SAR(int64)
if test $ac_cv_my_sar_int8 = yes && \
   test $ac_cv_my_sar_int16 = yes && \
   test $ac_cv_my_sar_int32 = yes && \
   test $ac_cv_my_sar_int64 = yes ; then
  CFLAGS="$CFLAGS -DRIGHTSHIFT_IS_SAR"
fi

CFLAGS="$CFLAGS -DUNZIP_SUPPORT -DSPC700_C -I. -I.. $ZIP_CFLAGS"

CXXFLAGS="$CFLAGS"

CCASFLAGS="$CFLAGS -I.. -I../i386"

AM_CONDITIONAL(ZSNESFX, [test yes = "$ZSNESFX"])
AM_CONDITIONAL(ZSNESC4, [test yes = "$ZSNESC4"])
AM_CONDITIONAL(ASMCPU, [test yes = "$ASMCPU"])
AM_CONDITIONAL(SDD1_DECOMP, [test yes = "$SDD1_DECOMP"])
AM_CONDITIONAL(NETPLAY, [test yes = "$NETPLAY"])
AM_CONDITIONAL(JMA, [test yes = "$JMA"])
AM_CONDITIONAL(OPENGL, [test yes = "$OPENGL"])
AM_CONDITIONAL(XV, [test yes = "$XV"])
AM_CONDITIONAL(PORTAUDIO, [test yes = "$PORTAUDIO"])
AM_CONDITIONAL(OSS, [test yes = "$OSS"])
AM_CONDITIONAL(DEBUG, [test yes = "$with_debug" ])
AM_CONDITIONAL(SDL, [test yes = "$GTKJOY"])
AM_CONDITIONAL(ALSA, [test yes = "$ALSA"])
AM_CONDITIONAL(PULSEAUDIO, [test yes = "$PULSEAUDIO"])
AM_CONDITIONAL(HQ2X, [test yes = "$HQ2X"])
AM_CONDITIONAL(XBRZ, [test yes = "$XBRZ"])
AM_CONDITIONAL(SYSTEM_ZIP, [test yes = "$SYSTEM_ZIP"])

AC_SUBST(NASM)
AC_SUBST(NASM_FLAGS)

AC_OUTPUT(Makefile data/Makefile po/Makefile.in)

echo ""
echo "Snes9x will build with support for the following:"
echo ""

if test yes = "$GTK3_WARNING" ; then
    echo "    GTK+ 3.0 (experimental)"
else
    echo "    GTK+ 2.0"
fi

if test yes = "$ALSA" ; then
    echo "    ALSA"
fi

if test yes = "$PULSEAUDIO" ; then
    echo "    PulseAudio"
fi

if test yes = "$OPENGL" ; then
    echo "    OpenGL"
else
    if test yes = "$with_opengl" ; then
        echo "!!! NOT able to use OpenGL support"
    fi
fi

if test yes = "$XV" ; then
    echo "    XVideo"
else
    if test yes = "$with_xv" ; then
        echo "!!! NOT able to use XVideo support"
        echo "--- Needs recent X11 libraries and headers"
    fi
fi

if test yes = "$XRANDR" ; then
    echo "    XRandR"
else
    if test yes = "$with_xrandr" ; then
        echo "!!! NOT able to use XRandR support"
        echo "--- Needs recent X11 libraries and headers"
    fi
fi

if test yes = "$GTKJOY" ; then
    echo "    Joysticks"
else
    if test yes = "$with_joystick" ; then
        echo "!!! NOT able to use joystick support"
        echo "--- Needs SDL libraries and headers"
    fi
fi

if test yes = "$NETPLAY" ; then
    echo "    NetPlay"
else
    if test yes = "$with_netplay" ; then
        echo "!!! NOT able to build NetPlay support"

    fi
fi

echo ""

if ! test yes = "$PORTAUDIO" ; then
    if ! test yes = "$OSS" ; then
        if ! test yes = "$SDL"; then
            if ! test yes = "$ALSA"; then
                echo "ERROR: No audio driver is selected or available. Snes9x will probably not build."
            fi
        fi
    fi
fi

if ! test yes = "$HQ2X"; then
    echo "HQ2x filter compilation is disabled."
fi

if ! test yes = "$XBRZ"; then
    echo "xBRZ filter compilation is disabled."
fi
