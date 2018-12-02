SUBDIRS = data po

bin_PROGRAMS = snes9x-gtk
snes9x_gtk_LDADD =
if DEBUG
else
snes9x_gtk_CXXFLAGS = -fno-exceptions -fno-rtti
endif
noinst_LIBRARIES =

AM_CPPFLAGS = -I../apu/bapu -I$(top_srcdir) -I.. -DDATADIR=\""$(snes9xdatadir)"\" -DSNES9XLOCALEDIR=\""$(snes9xlocaledir)"\"

CLEANFILES = \
    src/gtk_snes9x_ui.cpp \
    src/sourcify

snes9x_gtk_SOURCES = \
    src/gtk_sound_driver.h \
    ../filter/2xsai.cpp \
    ../filter/2xsai.h \
    ../filter/epx.cpp \
    ../filter/epx.h \
    src/filter_epx_unsafe.h \
    src/filter_epx_unsafe.cpp \
    src/gtk_binding.cpp \
    src/gtk_binding.h \
    src/gtk_cheat.cpp \
    src/gtk_cheat.h \
    src/gtk_config.cpp \
    src/gtk_config.h \
    src/gtk_control.cpp \
    src/gtk_control.h \
    src/gtk_display.cpp \
    src/gtk_display_driver_gtk.cpp \
    src/gtk_display_driver_gtk.h \
    src/gtk_display_driver.h \
    src/gtk_display.h \
    src/gtk_file.cpp \
    src/gtk_file.h \
    src/gtk_snes9x_ui.cpp \
    src/gtk_builder_window.cpp \
    src/gtk_builder_window.h \
    src/gtk_preferences.cpp \
    src/gtk_preferences.h \
    src/gtk_s9xcore.h \
    src/gtk_s9x.cpp \
    src/gtk_s9x.h \
    src/gtk_s9xwindow.cpp \
    src/gtk_s9xwindow.h \
    src/gtk_sound.cpp \
    src/gtk_sound.h \
    src/gtk_splash.cpp \
    ../filter/snes_ntsc_config.h \
    ../filter/snes_ntsc.h \
    ../filter/snes_ntsc_impl.h \
    ../filter/snes_ntsc.c \
    src/gtk_2_3_compat.h

if HQ2X
snes9x_gtk_SOURCES += \
    ../filter/hq2x.cpp \
    ../filter/hq2x.h
endif

if XBRZ
snes9x_gtk_SOURCES += \
    src/filter_xbrz.cpp\
    src/filter_xbrz.h\
    ../filter/xbrz.cpp \
    ../filter/xbrz.h
endif

if PORTAUDIO
snes9x_gtk_SOURCES += \
    src/gtk_sound_driver_portaudio.cpp \
    src/gtk_sound_driver_portaudio.h
endif

if PULSEAUDIO
snes9x_gtk_SOURCES += \
    src/gtk_sound_driver_pulse.cpp \
    src/gtk_sound_driver_pulse.h
endif

if OSS
snes9x_gtk_SOURCES += \
    src/gtk_sound_driver_oss.h \
    src/gtk_sound_driver_oss.cpp
endif

snes9x_gtk_SOURCES += \
    src/gtk_sound_driver_sdl.h \
    src/gtk_sound_driver_sdl.cpp

if ALSA
snes9x_gtk_SOURCES += \
    src/gtk_sound_driver_alsa.h \
    src/gtk_sound_driver_alsa.cpp
endif

snes9x_gtk_SOURCES += \
    ../fxinst.cpp \
    ../fxemu.cpp \
    ../fxdbg.cpp

snes9x_gtk_SOURCES += \
    ../c4.cpp \
    ../c4emu.cpp

if WAYLAND
snes9x_gtk_SOURCES += \
    src/gtk_wayland_egl_context.cpp
endif

# APU
snes9x_gtk_SOURCES += \
    ../apu/apu.cpp

# Byuu's APU
snes9x_gtk_SOURCES += \
    ../apu/bapu/dsp/sdsp.cpp \
    ../apu/bapu/smp/smp.cpp \
    ../apu/bapu/smp/smp_state.cpp

# MSU1
snes9x_gtk_SOURCES += \
    ../msu1.cpp \
    ../msu1.h

# DSP
snes9x_gtk_SOURCES += \
    ../dsp.cpp \
    ../dsp1.cpp \
    ../dsp2.cpp \
    ../dsp3.cpp \
    ../dsp4.cpp

# Base
snes9x_gtk_SOURCES += \
    ../spc7110.cpp \
    ../obc1.cpp \
    ../seta.cpp \
    ../seta010.cpp \
    ../seta011.cpp \
    ../seta018.cpp \
    ../controls.cpp \
    ../crosshairs.cpp \
    ../cpu.cpp \
    ../sa1.cpp \
    ../debug.cpp \
    ../sdd1.cpp \
    ../tile.cpp \
    ../srtc.cpp \
    ../gfx.cpp \
    ../memmap.cpp \
    ../clip.cpp \
    ../ppu.cpp \
    ../dma.cpp \
    ../snes9x.cpp \
    ../globals.cpp \
    ../stream.cpp \
    ../conffile.cpp \
    ../bsx.cpp \
    ../logger.cpp \
    ../snapshot.cpp \
    ../screenshot.cpp \
    ../movie.cpp \
    ../statemanager.cpp \
    ../sha256.cpp \
    ../bml.cpp

# ASMCPU Doesn't exist anymore.
snes9x_gtk_SOURCES += \
    ../cpuops.cpp \
    ../cpuexec.cpp \
    ../sa1cpu.cpp

snes9x_gtk_SOURCES += \
    ../cheats.cpp \
    ../cheats2.cpp

snes9x_gtk_SOURCES += ../sdd1emu.cpp

snes9x_gtk_SOURCES += \
    ../netplay.cpp \
    ../server.cpp


# Zip support is nonconfigurable.
snes9x_gtk_SOURCES += \
    ../loadzip.cpp
if ! SYSTEM_ZIP
snes9x_gtk_SOURCES += \
    ../unzip/unzip.c \
    ../unzip/ioapi.c \
    ../unzip/zip.c
endif
UNZIPDEFINES=-DUNZIP_SUPPORT

snes9x_gtk_LDADD += libjma.a
noinst_LIBRARIES += libjma.a
libjma_a_SOURCES = \
    ../jma/s9x-jma.cpp \
    ../jma/7zlzma.cpp \
    ../jma/crc32.cpp \
    ../jma/iiostrm.cpp \
    ../jma/inbyte.cpp \
    ../jma/jma.cpp \
    ../jma/lzma.cpp \
    ../jma/lzmadec.cpp \
    ../jma/winout.cpp
libjma_a_CPPFLAGS = -fexceptions

if OPENGL
snes9x_gtk_SOURCES += \
    src/gtk_display_driver_opengl.cpp \
    src/gtk_display_driver_opengl.h \
    src/gtk_glx_context.cpp \
    ../shaders/glsl.cpp \
    ../shaders/shader_helpers.cpp \
    src/gtk_shader_parameters.cpp 

endif

if XV
snes9x_gtk_SOURCES += \
    src/gtk_display_driver_xv.cpp \
    src/gtk_display_driver_xv.h
endif

snes9x_gtk_SOURCES += \
    src/gtk_netplay_dialog.cpp \
    src/gtk_netplay_dialog.h \
    src/gtk_netplay.cpp \
    src/gtk_netplay.h

src/sourcify: src/sourcify.c
	$(CC) src/sourcify.c -o src/sourcify

src/gtk_snes9x_ui.cpp: src/sourcify src/snes9x.ui
	src/sourcify src/snes9x.ui src/gtk_snes9x_ui.cpp snes9x_ui
