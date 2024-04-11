cmake_minimum_required(VERSION 3.18)
project(snes9x-gtk VERSION 1.62.3)

option(USE_SLANG "Build support for Vulkan output and .slangp shaders" ON)
option(USE_XV "Build support for XVideo output" ON)
option(USE_PORTAUDIO "Build PortAudio sound driver" ON)
option(USE_ALSA "Build ALSA sound driver" ON)
option(USE_PULSEAUDIO "Use PulseAudio" ON)
option(USE_OSS "Build support for OSS" ON)
option(DEBUGGER "Enable Snes9x Debugger" ON)
option(USE_HQ2X "Build the HQ2x family of filters" ON)
option(USE_XBRZ "Build the XBRZ family of filters" ON)
option(USE_SYSTEMZIP "Force use of system minizip" ON)
option(USE_WAYLAND "Build support for Wayland" ON)
option(DANGEROUS_HACKS "Allow dangerous hacks to be used" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "One of: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

string(APPEND DATADIR ${CMAKE_INSTALL_PREFIX} "/" ${CMAKE_INSTALL_DATAROOTDIR} "/snes9x")
string(APPEND LOCALEDIR ${CMAKE_INSTALL_PREFIX} "/" ${CMAKE_INSTALL_DATAROOTDIR} "/locale")
add_compile_definitions(HAVE_LIBPNG
                        ZLIB SNES9X_GTK
                        NETPLAY_SUPPORT
                        JMA_SUPPORT
                        UNZIP_SUPPORT
                        HAVE_MKSTEMP
                        HAVE_STRINGS_H
                        HAVE_STDINT_H
                        RIGHTSHIFT_IS_SAR
                        "GETTEXT_PACKAGE=\"${CMAKE_PROJECT_NAME}\""
                        DATADIR=\"${DATADIR}\"
                        SNES9XLOCALEDIR=\"${LOCALEDIR}\")
set(INCLUDES ../apu/bapu ../ src)
set(SOURCES)
set(ARGS -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-nullability-completeness)
set(LIBS)
set(DEFINES)

include(CheckIncludeFile)

include(FindGettext)

# Add any new language to this foreach loop
foreach(lang de es fr_FR ja pl pt_BR ru sr@latin uk zh_CN)
    GETTEXT_PROCESS_PO_FILES(${lang} ALL PO_FILES po/${lang}.po)
    install(FILES ${CMAKE_BINARY_DIR}/${lang}.gmo
            DESTINATION ${LOCALEDIR}/${lang}/LC_MESSAGES
            RENAME ${CMAKE_PROJECT_NAME}.mo
            COMPONENT translations)
endforeach()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(GTK REQUIRED gtkmm-3.0 gthread-2.0 libpng)
pkg_check_modules(XRANDR REQUIRED xrandr)

find_library(X11 X11 REQUIRED)
find_library(XEXT Xext REQUIRED)
list(APPEND ARGS ${SDL2_CFLAGS} ${GTK_CFLAGS} ${XRANDR_CFLAGS})
list(APPEND LIBS ${X11} ${XEXT} ${CMAKE_DL_LIBS} ${SDL2_LIBRARIES} ${GTK_LIBRARIES} ${XRANDR_LIBRARIES})

list(APPEND SOURCES src/gtk_display_driver_opengl.cpp
                    ../common/video/glx_context.cpp
                    ../shaders/glsl.cpp
                    ../shaders/shader_helpers.cpp
                    ../vulkan/slang_helpers.cpp
                    ../vulkan/slang_helpers.hpp
                    ../vulkan/slang_preset_ini.cpp
                    ../vulkan/slang_preset_ini.hpp
                    ../external/glad/src/glx.c
                    ../external/glad/src/egl.c
                    ../external/glad/src/gl.c
                    src/gtk_shader_parameters.cpp
                    ../vulkan/std_chrono_throttle.cpp
                    ../vulkan/std_chrono_throttle.hpp)
list(APPEND INCLUDES ../external/glad/include)

if(USE_SLANG)
    list(APPEND SOURCES ../shaders/slang.cpp)
    list(APPEND INCLUDES ../external/glslang)

    set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS CACHE BOOL ON)
    set(BUILD_TESTING CACHE BOOL OFF)
    add_subdirectory("../external/glslang" "glslang" EXCLUDE_FROM_ALL)
    add_subdirectory("../external/SPIRV-Cross" "SPIRV-Cross" EXCLUDE_FROM_ALL)
    list(APPEND LIBS glslang
                     OGLCompiler
                     HLSL
                     OSDependent
                     SPIRV
                     glslang-default-resource-limits)
    list(APPEND LIBS spirv-cross-core
                     spirv-cross-glsl
                     spirv-cross-reflect
                     spirv-cross-cpp)
    list(APPEND DEFINES "USE_SLANG")
    list(APPEND INCLUDES "../external/glslang")

    list(APPEND DEFINES "VK_USE_PLATFORM_XLIB_KHR"
                        "VK_USE_PLATFORM_WAYLAND_KHR"
                        "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1"
                        "VMA_DYNAMIC_VULKAN_FUNCTIONS=1"
                        "VMA_STATIC_VULKAN_FUNCTIONS=0"
                        "IMGUI_IMPL_VULKAN_NO_PROTOTYPES")
    list(APPEND INCLUDES ../external/vulkan-headers/include)
    list(APPEND INCLUDES ../external/VulkanMemoryAllocator-Hpp/include)
    list(APPEND INCLUDES ../external/stb)
    list(APPEND SOURCES ../external/stb/stb_image_implementation.cpp)
    list(APPEND SOURCES ../vulkan/slang_shader.cpp
                        ../vulkan/slang_shader.hpp
                        ../vulkan/slang_preset.cpp
                        ../vulkan/slang_preset.hpp
                        ../vulkan/vulkan_hpp_storage.cpp
                        ../vulkan/vk_mem_alloc_implementation.cpp
                        ../vulkan/vulkan_context.cpp
                        ../vulkan/vulkan_context.hpp
                        ../vulkan/vulkan_texture.cpp
                        ../vulkan/vulkan_texture.hpp
                        ../vulkan/vulkan_swapchain.cpp
                        ../vulkan/vulkan_swapchain.hpp
                        ../vulkan/vulkan_slang_pipeline.cpp
                        ../vulkan/vulkan_slang_pipeline.hpp
                        ../vulkan/vulkan_pipeline_image.cpp
                        ../vulkan/vulkan_pipeline_image.hpp
                        ../vulkan/vulkan_shader_chain.cpp
                        ../vulkan/vulkan_shader_chain.hpp
                        ../vulkan/vulkan_simple_output.hpp
                        ../vulkan/vulkan_simple_output.cpp
                        src/gtk_display_driver_vulkan.cpp
                        src/gtk_display_driver_vulkan.h
                        ../external/imgui/imgui_impl_vulkan.cpp)
endif()


list(APPEND SOURCES ../external/imgui/imgui.cpp
../external/imgui/imgui_demo.cpp
../external/imgui/imgui_draw.cpp
../external/imgui/imgui_impl_opengl3.cpp
../external/imgui/imgui_tables.cpp
../external/imgui/imgui_widgets.cpp
../external/imgui/snes9x_imgui.cpp)
list(APPEND INCLUDES ../external/imgui)

if(USE_WAYLAND)
    pkg_check_modules(WAYLAND REQUIRED wayland-client wayland-egl)
    list(APPEND DEFINES "USE_WAYLAND")
    list(APPEND SOURCES ../common/video/wayland_egl_context.cpp
                        ../common/video/wayland_egl_context.hpp
                        ../common/video/wayland_surface.cpp
                        ../common/video/wayland_surface.hpp
                        ../common/video/wayland-idle-inhibit-unstable-v1.c
                        ../common/video/viewporter-client-protocol.c
                        ../common/video/fractional-scale-v1.c)
    list(APPEND ARGS ${WAYLAND_CFLAGS})
    list(APPEND LIBS ${WAYLAND_LIBRARIES})
endif()

if(USE_XV)
    pkg_check_modules(XV REQUIRED xv)
    list(APPEND DEFINES "USE_XV")
    list(APPEND SOURCES src/gtk_display_driver_xv.cpp
                        src/gtk_display_driver_xv.h)
    list(APPEND ARGS ${XV_CFLAGS})
    list(APPEND LIBS ${XV_LIBRARIES})
endif()

if(USE_PULSEAUDIO)
    pkg_check_modules(PULSEAUDIO REQUIRED libpulse)
    list(APPEND DEFINES "USE_PULSEAUDIO")
    list(APPEND SOURCES ../common/audio/s9x_sound_driver_pulse.cpp)
    list(APPEND ARGS ${PULSEAUDIO_CFLAGS})
    list(APPEND LIBS ${PULSEAUDIO_LIBRARIES})
endif()

if(USE_PORTAUDIO)
    pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
    list(APPEND DEFINES "USE_PORTAUDIO")
    list(APPEND SOURCES ../common/audio/s9x_sound_driver_portaudio.cpp)
    list(APPEND ARGS ${PORTAUDIO_CFLAGS})
    list(APPEND LIBS ${PORTAUDIO_LIBRARIES})
endif()

if(USE_ALSA)
    pkg_check_modules(ALSA REQUIRED alsa)
    list(APPEND DEFINES "USE_ALSA")
    list(APPEND SOURCES ../common/audio/s9x_sound_driver_alsa.cpp)
    list(APPEND ARGS ${ALSA_CFLAGS})
    list(APPEND LIBS ${ALSA_LIBRARIES})
endif()

if(USE_OSS)
    CHECK_INCLUDE_FILE("sys/soundcard.h" OSS)
    if(OSS)
        list(APPEND SOURCES ../common/audio/s9x_sound_driver_oss.cpp)
        list(APPEND DEFINES "USE_OSS")
    endif()
endif()

pkg_check_modules(LIBPNG REQUIRED "libpng")
list(APPEND ARGS ${LIBPNG_CFLAGS})
list(APPEND LIBS ${LIBPNG_LIBRARIES})

pkg_check_modules(ZLIB REQUIRED "zlib")
list(APPEND ARGS ${ZLIB_CFLAGS})
list(APPEND LIBS ${ZLIB_LIBRARIES})

pkg_check_modules(MINIZIP "minizip")
if(USE_SYSTEMZIP AND MINIZIP_FOUND)
    list(APPEND DEFINES "SYSTEM_ZIP")
    list(APPEND ARGS ${MINIZIP_CFLAGS})
    list(APPEND LIBS ${MINIZIP_LIBRARIES})
else()
    list(APPEND INCLUDES ../unzip)
    list(APPEND SOURCES ../unzip/unzip.c ../unzip/ioapi.c ../unzip/zip.c)
endif()

if(USE_HQ2X)
    list(APPEND DEFINES "USE_HQ2X")
    list(APPEND SOURCES ../filter/hq2x.cpp)
endif()

if(USE_XBRZ)
    list(APPEND DEFINES "USE_XBRZ")
    list(APPEND SOURCES ../filter/xbrz.cpp ../filter/xbrz.h src/filter_xbrz.cpp)
endif()

if(DANGEROUS_HACKS)
    list(APPEND DEFINES ALLOW_CPU_OVERCLOCK)
    message("Dangerous hacks are enabled. Use at your own risk.")
endif()

list(APPEND SOURCES
    ../common/audio/s9x_sound_driver.hpp
    ../filter/2xsai.cpp
    ../filter/2xsai.h
    ../filter/epx.cpp
    ../filter/epx.h
    src/filter_epx_unsafe.h
    src/filter_epx_unsafe.cpp
    src/gtk_binding.cpp
    src/gtk_binding.h
    src/gtk_cheat.cpp
    src/gtk_cheat.h
    src/gtk_config.cpp
    src/gtk_config.h
    src/gtk_control.cpp
    src/gtk_control.h
    src/gtk_display.cpp
    src/gtk_display_driver_gtk.cpp
    src/gtk_display_driver_gtk.h
    src/gtk_display_driver.h
    src/gtk_display.h
    src/threadpool.cpp
    src/threadpool.h
    src/gtk_file.cpp
    src/gtk_file.h
    src/gtk_builder_window.cpp
    src/gtk_builder_window.h
    src/gtk_preferences.cpp
    src/gtk_preferences.h
    src/gtk_s9xcore.h
    src/gtk_s9x.cpp
    src/gtk_s9x.h
    src/gtk_s9xwindow.cpp
    src/gtk_s9xwindow.h
    src/gtk_sound.cpp
    src/gtk_sound.h
    src/gtk_splash.cpp
    ../filter/snes_ntsc_config.h
    ../filter/snes_ntsc.h
    ../filter/snes_ntsc_impl.h
    ../filter/snes_ntsc.c
    src/gtk_compat.h
    ../common/audio/s9x_sound_driver_sdl.hpp
    ../common/audio/s9x_sound_driver_sdl.cpp
    ../fxinst.cpp
    ../fxemu.cpp
    ../fxdbg.cpp
    ../c4.cpp
    ../c4emu.cpp
    ../apu/apu.cpp
    ../apu/bapu/dsp/sdsp.cpp
    ../apu/bapu/smp/smp.cpp
    ../apu/bapu/smp/smp_state.cpp
    ../msu1.cpp
    ../msu1.h
    ../vorbis.cpp
    ../vorbis.h
    ../dsp.cpp
    ../dsp1.cpp
    ../dsp2.cpp
    ../dsp3.cpp
    ../dsp4.cpp
    ../spc7110.cpp
    ../obc1.cpp
    ../seta.cpp
    ../seta010.cpp
    ../seta011.cpp
    ../seta018.cpp
    ../controls.cpp
    ../crosshairs.cpp
    ../cpu.cpp
    ../sa1.cpp
    ../debug.cpp
    ../sdd1.cpp
    ../tile.cpp
    ../tileimpl-n1x1.cpp
    ../tileimpl-n2x1.cpp
    ../tileimpl-h2x1.cpp
    ../srtc.cpp
    ../gfx.cpp
    ../memmap.cpp
    ../clip.cpp
    ../ppu.cpp
    ../dma.cpp
    ../snes9x.cpp
    ../globals.cpp
    ../stream.cpp
    ../conffile.cpp
    ../bsx.cpp
    ../snapshot.cpp
    ../screenshot.cpp
    ../movie.cpp
    ../statemanager.cpp
    ../sha256.cpp
    ../bml.cpp
    ../cpuops.cpp
    ../cpuexec.cpp
    ../sa1cpu.cpp
    ../cheats.cpp
    ../cheats2.cpp
    ../sdd1emu.cpp
    ../netplay.cpp
    ../server.cpp
    ../loadzip.cpp
    ../fscompat.cpp
    src/gtk_netplay_dialog.cpp
    src/gtk_netplay_dialog.h
    src/gtk_netplay.cpp
    src/gtk_netplay.h
    src/background_particles.cpp
    src/background_particles.h)

list(APPEND SOURCES ../external/fmt/src/format.cc)
list(APPEND INCLUDES ../external/fmt/include)

set(LIBJMA_SOURCES
    ../jma/s9x-jma.cpp
    ../jma/7zlzma.cpp
    ../jma/crc32.cpp
    ../jma/iiostrm.cpp
    ../jma/inbyte.cpp
    ../jma/jma.cpp
    ../jma/lzma.cpp
    ../jma/lzmadec.cpp
    ../jma/winout.cpp)

add_library(jma ${LIBJMA_SOURCES})
target_include_directories(jma PRIVATE ${INCLUDES})
target_compile_options(jma PUBLIC ${ARGS})
list(APPEND LIBS jma)

add_executable(sourcify src/sourcify.c)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gtk_snes9x_ui.cpp
    COMMAND sourcify ${CMAKE_CURRENT_SOURCE_DIR}/src/snes9x.ui ${CMAKE_CURRENT_BINARY_DIR}/gtk_snes9x_ui.cpp snes9x_ui
    DEPENDS sourcify)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mini_icon.cpp
    COMMAND sourcify ${CMAKE_CURRENT_SOURCE_DIR}/data/mini_icon.png ${CMAKE_CURRENT_BINARY_DIR}/mini_icon.cpp mini_icon
    DEPENDS sourcify)

add_executable(snes9x-gtk ${SOURCES} ${jma} ${CMAKE_CURRENT_BINARY_DIR}/gtk_snes9x_ui.cpp ${CMAKE_CURRENT_BINARY_DIR}/mini_icon.cpp)
target_include_directories(snes9x-gtk PRIVATE ${INCLUDES})
target_compile_options(snes9x-gtk PRIVATE ${ARGS})
target_link_libraries(snes9x-gtk PRIVATE ${LIBS})
target_compile_definitions(snes9x-gtk PRIVATE ${DEFINES})

if(USE_SLANG)
    add_executable(slang_test EXCLUDE_FROM_ALL
        ../vulkan/slang_helpers.cpp
        ../vulkan/slang_helpers.hpp
        ../vulkan/slang_shader.cpp
        ../vulkan/slang_shader.hpp
        ../vulkan/slang_preset.cpp
        ../vulkan/slang_preset.hpp
        ../vulkan/slang_preset_ini.cpp
        ../vulkan/slang_preset_ini.hpp
        ../vulkan/vulkan_hpp_storage.cpp
        ../vulkan/slang_preset_test.cpp
        ../conffile.cpp
        ../stream.cpp)

    target_include_directories(slang_test PRIVATE ${INCLUDES})
    target_compile_options(slang_test PRIVATE ${ARGS})
    target_compile_definitions(slang_test PRIVATE ${DEFINES})
    target_link_libraries(slang_test PRIVATE ${LIBS})
endif()

install(TARGETS snes9x-gtk)
install(FILES ../data/cheats.bml DESTINATION ${DATADIR})
install(FILES data/snes9x-gtk.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(FILES data/snes9x_16x16.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/16x16/apps RENAME snes9x.png)
install(FILES data/snes9x_24x24.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/24x24/apps RENAME snes9x.png)
install(FILES data/snes9x_32x32.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps RENAME snes9x.png)
install(FILES data/snes9x_64x64.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps RENAME snes9x.png)
install(FILES data/snes9x_128x128.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps RENAME snes9x.png)
install(FILES data/snes9x_256x256.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps RENAME snes9x.png)
install(FILES data/snes9x.svg DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps)
