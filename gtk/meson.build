project('snes9x-gtk', ['c', 'cpp'], version: '1.58', default_options: ['cpp_std=c++11'])

subdir('po')
subdir('data')

args = [ '-DSNES9X_GTK', '-DUNZIP_SUPPORT', '-DNETPLAY_SUPPORT', '-DJMA_SUPPORT', '-Wall', '-W', '-Wno-unused-parameter' ]
srcs = []
deps = []
includes = include_directories('../apu/bapu', '../', 'src')

prefix = get_option('prefix')
localedir = join_paths(prefix, get_option('localedir'))
datadir = join_paths(prefix, get_option('datadir'), 'snes9x')
args += [ '-DDATADIR="' + datadir + '"', '-DSNES9XLOCALEDIR="' + localedir + '"' ]

glib_dep = dependency('glib-2.0', version: '> 2.28')
gthread_dep = dependency('gthread-2.0', version: '>= 2.6')
gobject_dep = dependency('gobject-2.0', version: '>= 2.6')
sdl2_dep = dependency('sdl2')

deps += [ glib_dep, gthread_dep, gobject_dep, sdl2_dep ]

c_compiler = meson.get_compiler('c')

if get_option('gtk3') and not get_option('gtk2')
  message('Building with GTK+-3.0')
  gtk_dep = dependency('gtk+-3.0', version: '>= 3.22')
  gtk_ver = 3
else
  message('Building with GTK+-2.0')
  gtk_dep = dependency('gtk+-2.0', version: '>= 2.16')
  gtk_ver = 2
endif

deps += gtk_dep

x11_dep = c_compiler.find_library('X11')
xext_dep = c_compiler.find_library('Xext')
dl_dep = c_compiler.find_library('dl')
deps += x11_dep
deps += xext_dep
deps += dl_dep

xrandr_dep = dependency('xrandr')
deps += xrandr_dep

if get_option('opengl')
  opengl_dep = dependency('epoxy', required: false)

  if opengl_dep.found() 
    args += '-DUSE_OPENGL'
    srcs += [ 'src/gtk_display_driver_opengl.cpp',
              'src/gtk_display_driver_opengl.h',
              'src/gtk_glx_context.cpp',
              'src/gtk_glx_context.h',
              '../shaders/glsl.cpp',
              '../shaders/shader_helpers.cpp',
              'src/gtk_shader_parameters.cpp' ]
    deps += opengl_dep
  endif
endif

if get_option('wayland') and gtk_ver == 3
  wayland_dep = dependency('wayland-egl', required: false)
  
  if wayland_dep.found()
    args += '-DUSE_WAYLAND'
    srcs += [ 'src/gtk_wayland_egl_context.cpp', 'src/gtk_wayland_egl_context.h' ]
    deps += wayland_dep
  else
    message('Wayland not found. Disabling.')
  endif
endif

if get_option('xv')
  xv_dep = dependency('xv', required: false)
  
  if xv_dep.found()
    args += '-DUSE_XV'
    srcs += [ 'src/gtk_display_driver_xv.cpp', 'src/gtk_display_driver_xv.h' ]
    deps += xv_dep
  else
    message('XV not found. Disabling.')
  endif
endif

if get_option('portaudio')
  portaudio_dep = dependency('portaudio-2.0', version: '>= 10', required: false)
  
  if portaudio_dep.found()
    args += '-DUSE_PORTAUDIO'
    srcs += [ 'src/gtk_sound_driver_portaudio.cpp', 'src/gtk_sound_driver_portaudio.h' ]
    deps += portaudio_dep
  else
    message('PortAudio not found. Disabling.')
  endif
endif

if get_option('oss')
  if c_compiler.has_header('sys/soundcard.h')
    args += '-DUSE_OSS'
    srcs += [ 'src/gtk_sound_driver_oss.cpp', 'src/gtk_sound_driver_oss.h' ]
  else
    message('OSS not found. Disabling.')
  endif
endif

if get_option('alsa')
  alsa_dep = dependency('alsa', required: false)
  
  if alsa_dep.found()
    args += '-DUSE_ALSA'
    srcs += [ 'src/gtk_sound_driver_alsa.cpp', 'src/gtk_sound_driver_alsa.h' ]
    deps += alsa_dep
  else
    message('Alsa not found. Disabling.')
  endif
endif

if get_option('pulseaudio')
  pulseaudio_dep = dependency('libpulse', required: false)
  
  if pulseaudio_dep.found()
    args += '-DUSE_PULSEAUDIO'
    srcs += [ 'src/gtk_sound_driver_pulse.cpp', 'src/gtk_sound_driver_pulse.h' ]
    deps += pulseaudio_dep
  else
    message('PulseAudio not found. Disabling.')
  endif
endif

if get_option('screenshot')
  screenshot_dep = dependency('libpng', required: false)

  if screenshot_dep.found()
    args += '-DHAVE_LIBPNG'
    deps += screenshot_dep
  else
    message('libpng not found. Disabling screenshot support.')
  endif
endif

if get_option('system-zip')
  systemzip_dep = dependency('minizip', required: false)

  if systemzip_dep.found()
    args += '-DSYSTEM_ZIP'
    deps += systemzip_dep
  else
    message('minizip not found. Using built-in minizip.')
    srcs += [ '../unzip/unzip.c', '../unzip/ioapi.c', '../unzip/zip.c' ]
  endif
endif
        
if get_option('zlib')
  zlib_dep = dependency('zlib', required: false)

  if zlib_dep.found()
    args += '-DZLIB'
    deps += zlib_dep
  else
    message('zlib not found. Disabling.')
  endif
endif

if get_option('dangerous-hacks')
  args += '-DALLOW_CPU_OVERCLOCK'
endif

if get_option('hq2x')
  args += '-DUSE_HQ2X'
  srcs += [ '../filter/hq2x.cpp', '../filter/hq2x.h' ]
endif

if get_option('xbrz')
  args += '-DUSE_XBRZ'
  srcs += [ '../filter/xbrz.cpp', '../filter/xbrz.h', 'src/filter_xbrz.cpp', 'src/filter_xbrz.h' ]
endif

if c_compiler.has_function('mkstemp')
  args += '-DHAVE_MKSTEMP'
endif

if c_compiler.has_header('strings.h')
  args += '-DHAVE_STRINGS_H'
endif

if c_compiler.has_header('stdint.h')
  args += '-DHAVE_STDINT_H'
endif

results = c_compiler.run('int main(int argc, char **argv) { int i; i = -1; i >>= 1; return (i < 0 ? 0 : 1); }')

if results.returncode() == 0
  args += '-DRIGHTSHIFT_IS_SAR'
endif

results = c_compiler.run('int main(int argc, char **argv) { return (!(sizeof(void *) == sizeof(int))); }')

if results.returncode() != 0
  args += '-DPTR_NOT_INT'
endif

srcs += [
 'src/gtk_sound_driver.h',
 '../filter/2xsai.cpp',
 '../filter/2xsai.h',
 '../filter/epx.cpp',
 '../filter/epx.h',
 'src/filter_epx_unsafe.h',
 'src/filter_epx_unsafe.cpp',
 'src/gtk_binding.cpp',
 'src/gtk_binding.h',
 'src/gtk_cheat.cpp',
 'src/gtk_cheat.h',
 'src/gtk_config.cpp',
 'src/gtk_config.h',
 'src/gtk_control.cpp',
 'src/gtk_control.h',
 'src/gtk_display.cpp',
 'src/gtk_display_driver_gtk.cpp',
 'src/gtk_display_driver_gtk.h',
 'src/gtk_display_driver.h',
 'src/gtk_display.h',
 'src/gtk_file.cpp',
 'src/gtk_file.h',
 'src/gtk_builder_window.cpp',
 'src/gtk_builder_window.h',
 'src/gtk_preferences.cpp',
 'src/gtk_preferences.h',
 'src/gtk_s9xcore.h',
 'src/gtk_s9x.cpp',
 'src/gtk_s9x.h',
 'src/gtk_s9xwindow.cpp',
 'src/gtk_s9xwindow.h',
 'src/gtk_sound.cpp',
 'src/gtk_sound.h',
 'src/gtk_splash.cpp',
 '../filter/snes_ntsc_config.h',
 '../filter/snes_ntsc.h',
 '../filter/snes_ntsc_impl.h',
 '../filter/snes_ntsc.c',
 'src/gtk_2_3_compat.h',
 'src/gtk_sound_driver_sdl.h',
 'src/gtk_sound_driver_sdl.cpp',
 '../fxinst.cpp',
 '../fxemu.cpp',
 '../fxdbg.cpp',
 '../c4.cpp',
 '../c4emu.cpp',
 '../apu/apu.cpp',
 '../apu/bapu/dsp/sdsp.cpp',
 '../apu/bapu/smp/smp.cpp',
 '../apu/bapu/smp/smp_state.cpp',
 '../msu1.cpp',
 '../msu1.h',
 '../dsp.cpp',
 '../dsp1.cpp',
 '../dsp2.cpp',
 '../dsp3.cpp',
 '../dsp4.cpp',
 '../spc7110.cpp',
 '../obc1.cpp',
 '../seta.cpp',
 '../seta010.cpp',
 '../seta011.cpp',
 '../seta018.cpp',
 '../controls.cpp',
 '../crosshairs.cpp',
 '../cpu.cpp',
 '../sa1.cpp',
 '../debug.cpp',
 '../sdd1.cpp',
 '../tile.cpp',
 '../srtc.cpp',
 '../gfx.cpp',
 '../memmap.cpp',
 '../clip.cpp',
 '../ppu.cpp',
 '../dma.cpp',
 '../snes9x.cpp',
 '../globals.cpp',
 '../stream.cpp',
 '../conffile.cpp',
 '../bsx.cpp',
 '../logger.cpp',
 '../snapshot.cpp',
 '../screenshot.cpp',
 '../movie.cpp',
 '../statemanager.cpp',
 '../sha256.cpp',
 '../bml.cpp',
 '../cpuops.cpp',
 '../cpuexec.cpp',
 '../sa1cpu.cpp',
 '../cheats.cpp',
 '../cheats2.cpp',
 '../sdd1emu.cpp',
 '../netplay.cpp',
 '../server.cpp',
 '../loadzip.cpp',
 'src/gtk_netplay_dialog.cpp',
 'src/gtk_netplay_dialog.h',
 'src/gtk_netplay.cpp',
 'src/gtk_netplay.h'
]

libjma_srcs = [
 '../jma/s9x-jma.cpp',
 '../jma/7zlzma.cpp',
 '../jma/crc32.cpp',
 '../jma/iiostrm.cpp',
 '../jma/inbyte.cpp',
 '../jma/jma.cpp',
 '../jma/lzma.cpp',
 '../jma/lzmadec.cpp',
 '../jma/winout.cpp'
]

libjma = static_library('jma', 
                        libjma_srcs, 
                        c_args: args, 
                        cpp_args: [args, '-fexceptions'],
                        include_directories: includes)

sourcify = executable('sourcify', 'src/sourcify.c')
gtk_snes9x_ui_cpp = custom_target('sourcify', 
                                  input: 'src/snes9x.ui', 
                                  output: 'gtk_snes9x_ui.cpp',
                                  command: [ sourcify, '@INPUT@', '@OUTPUT@', 'snes9x_ui'])

snes9x_gtk = executable('snes9x-gtk', 
                        srcs, 
                        gtk_snes9x_ui_cpp,
                        c_args: args, 
                        cpp_args: [args, '-fno-exceptions', '-fno-rtti'],
                        dependencies: deps,
                        include_directories: includes,
                        link_with: libjma,
                        install: true)

