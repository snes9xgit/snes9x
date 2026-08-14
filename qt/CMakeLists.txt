cmake_minimum_required(VERSION 3.20)

# Must be set before project(), which is where CMake bakes it into the
# compiler/linker probes.
#
# The floor is dictated by whichever Qt you link against: Homebrew's qt
# bottles are built for the host OS (currently macOS 14.0), so linking a
# lower target against them only produces "built for newer macOS version"
# warnings. Override on the command line when using a Qt that supports
# older systems, e.g. the official online-installer builds:
#   cmake -S qt -B qt/build -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
if(APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum macOS version")
endif()

project(snes9x-qt VERSION 1.63)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_GLOBAL_AUTOGEN_TARGET ON)

# AUTOMOC above applies to the FetchContent'd SDL3 too, and Ninja + PCH +
# AUTOMOC hit a CMake ordering bug where SDL3's mocs_compilation.cpp is
# scheduled against a cmake_pch.hxx.pch that nothing generates. The Linux
# AppImage scripts pass -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON for the same
# reason; do it by default on macOS, where Ninja is the usual generator.
if(APPLE AND NOT DEFINED CMAKE_DISABLE_PRECOMPILE_HEADERS)
    set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON CACHE BOOL "" FORCE)
endif()

set(DEFINES SNES9X_QT)
set(SNES9X_CORE_SOURCES
    ../fxinst.cpp
    ../fxemu.cpp
    ../fxdbg.cpp
    ../c4.cpp
    ../c4emu.cpp
    ../apu/apu.cpp
    ../apu/bapu/dsp/sdsp.cpp
    ../apu/bapu/smp/smp.cpp
    ../apu/bapu/smp/smp_state.cpp
    ../sgb/sgb.cpp
    ../sgb/sgb_packet.cpp
    ../sgb/sgb_state.cpp
    ../sgb/gb_cpu.cpp
    ../sgb/gb_ops.cpp
    ../sgb/gb_ops_cb.cpp
    ../sgb/gb_memory.cpp
    ../sgb/gb_mbc.cpp
    ../sgb/gb_ppu.cpp
    ../sgb/gb_apu.cpp
    ../sgb/gb_timer.cpp
    ../sgb/gb_joypad.cpp
    ../sgb/gb_cart.cpp
    ../msu1.cpp
    ../msu1.h
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
    ../sfcbox.cpp
    ../hd64180.cpp
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
    ../voicekun.cpp
    ../sa1cpu.cpp
    ../cheats.cpp
    ../cheats2.cpp
    ../sdd1emu.cpp
    ../netplay.cpp
    ../server.cpp
    ../loadzip.cpp
    ../fscompat.cpp
    ../unzip/unzip.c
    ../unzip/ioapi.c
    ../unzip/zip.c)
add_library(snes9x-core ${SNES9X_CORE_SOURCES})
target_include_directories(snes9x-core PRIVATE ../ ../unzip)
target_compile_definitions(snes9x-core PRIVATE ${DEFINES} ZLIB UNZIP_SUPPORT HAVE_STDINT_H HAVE_LIBPNG ALLOW_CPU_OVERCLOCK)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(PREBUILT_QT "${PROJECT_SOURCE_DIR}/../external/qt6-mingw-clang-bin")
    if (EXISTS ${PREBUILT_QT})
        message("Using existing Qt6 build at ${PREBUILT_QT}")
        set(CMAKE_PREFIX_PATH ${PREBUILT_QT})
    else()
	    message("Getting Qt6 binaries...")
        include(FetchContent)
        FetchContent_Declare(qt6-mingw-clang-bin
	        GIT_REPOSITORY https://github.com/bearoso/qt6-mingw-clang-bin.git
	        GIT_TAG main)
        FetchContent_MakeAvailable(qt6-mingw-clang-bin)
        set(CMAKE_PREFIX_PATH ${qt6-mingw-clang-bin_SOURCE_DIR})
    endif()
endif()
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
list(APPEND LIBS Qt6::Widgets Qt6::Gui)

# Must come before the first pkg_check_modules() call below. It used to sit
# after the libcurl lookup and only worked because something else in the
# Linux configure had already pulled FindPkgConfig in.
find_package(PkgConfig REQUIRED)

if(APPLE)
    # The libcurl in the macOS SDK ships no pkg-config file, so go through
    # CMake's own finder rather than requiring a Homebrew curl.
    find_package(CURL REQUIRED)
    list(APPEND INCLUDES ${CURL_INCLUDE_DIRS})
    list(APPEND LIBS CURL::libcurl)
else()
    pkg_check_modules(CURL REQUIRED libcurl)
    list(APPEND INCLUDES ${CURL_INCLUDE_DIRS})
    list(APPEND LIBS ${CURL_LIBRARIES})
endif()
list(APPEND INCLUDES ${Qt6Gui_PRIVATE_INCLUDE_DIRS})

pkg_check_modules(ZLIB REQUIRED IMPORTED_TARGET zlib)
pkg_check_modules(PNG REQUIRED IMPORTED_TARGET libpng)
list(APPEND INCLUDES ${ZLIB_INCLUDE_DIRS})
list(APPEND FLAGS ${ZLIB_COMPILE_FLAGS})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message("Clang compiler, using libc++.")
        list(APPEND LIBS libc++.a)
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        message("GNU compiler, using libstdc++.")
        list(APPEND LIBS libstdc++.a)
    endif()

    list(APPEND LIBS libz.a libpng.a opengl32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32 dinput8)
    list(APPEND PLATFORM_SOURCES
        ../common/video/opengl/wgl_context.cpp
        ../external/glad/src/wgl.c
        src/resources/snes9x_win32.rc)
elseif(APPLE)

    enable_language(OBJCXX)

    # See the Windows note below: screenshot.cpp inside snes9x-core calls
    # libpng directly, so png/zlib have to hang off the core target to land
    # after the archive on the link line.
    target_link_libraries(snes9x-core PUBLIC PkgConfig::PNG PkgConfig::ZLIB)

    # OpenGL is deprecated on macOS but is the only 3D API available without
    # bringing in MoltenVK. It tops out at a 4.1 core profile.
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)
    list(APPEND LIBS ${COCOA_LIBRARY} ${OPENGL_FRAMEWORK})

    list(APPEND PLATFORM_SOURCES
        ../common/video/opengl/mac_opengl_context.mm
        ../common/video/opengl/mac_opengl_context.hpp)

    # The context implementation uses __bridge_retained/__bridge_transfer to
    # hand the NSOpenGLContext across the C++ boundary, which clang only
    # accepts under ARC. Nothing else in the project is Objective-C, so scope
    # the flag to this one file.
    set_source_files_properties(../common/video/opengl/mac_opengl_context.mm
        PROPERTIES COMPILE_OPTIONS "-fobjc-arc")

    # cubeb's CoreAudio backend covers macOS, so no PulseAudio equivalent is
    # needed here.

else()

    pkg_check_modules(WAYLAND REQUIRED wayland-client wayland-egl)
    include(FindX11)
    if(NOT X11_FOUND)
        error()
    endif()

    list(APPEND INCLUDES ${WAYLAND_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
    list(APPEND LIBS ${WAYLAND_LIBRARIES} ${X11_LIBRARIES})
    list(APPEND FLAGS ${WAYLAND_CFLAGS})

    # screenshot.cpp (in snes9x-core) calls libpng directly; attach png/zlib to the
    # core target via pkg-config imported targets so CMake always emits them on the
    # link line, after the core archive, with full paths. Linking them only to the
    # final executable (as before) left the ordering up to the linker's defaults and
    # broke on stricter setups ("libpng16.so: DSO missing from command line").
    target_link_libraries(snes9x-core PUBLIC PkgConfig::PNG PkgConfig::ZLIB)

    pkg_check_modules(PULSEAUDIO libpulse)
    if(PULSEAUDIO_FOUND)
        list(APPEND LIBS ${PULSEAUDIO_LIBRARIES})
        list(APPEND INCLUDES ${PULSEAUDIO_INCLUDE_DIRS})
        list(APPEND DEFINES "USE_PULSEAUDIO")
        list(APPEND PLATFORM_SOURCES ../common/audio/s9x_sound_driver_pulse.cpp)
    endif()

    list(APPEND PLATFORM_SOURCES
    ../common/video/opengl/glx_context.cpp
    ../common/video/opengl/glx_context.hpp
    ../common/video/opengl/wayland_egl_context.cpp
    ../common/video/opengl/wayland_egl_context.hpp
    ../common/video/wayland/wayland_surface.cpp
    ../common/video/wayland/wayland_surface.hpp
    ../common/video/wayland/fractional-scale-v1.c
    ../common/video/wayland/fractional-scale-v1.h
    ../common/video/wayland/viewporter-client-protocol.c
    ../common/video/wayland/viewporter-client-protocol.h
    ../common/video/wayland/wayland-idle-inhibit-unstable-v1.c
    ../common/video/wayland/wayland-idle-inhibit-unstable-v1.h
    ../external/glad/src/glx.c
    ../external/glad/src/egl.c)
endif()

set(USE_SYSTEM_SDL3 CACHE BOOL OFF)

if (USE_SYSTEM_SDL3)
    find_package(SDL3 QUIET)
endif()

if (SDL3_FOUND)
    message("Using system SDL3.")
    list(APPEND LIBS SDL3::SDL3)
    list(APPEND INCLUDES ${SDL3_INCLUDE_DIRS})
else()
    include(FetchContent)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.8
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        EXCLUDE_FROM_ALL
    )
    message(STATUS "Using SDL3 via FetchContent")
    set(SDL_UNIX_CONSOLE_BUILD FORCE CACHE BOOL ON)
    set(SDL_GPU CACHE BOOL OFF)
    set(SDL_VIDEO CACHE BOOL OFF)
    set(SDL_RENDER CACHE BOOL OFF)
    set(SDL_SHARED BOOL FALSE)
    set(SDL_STATIC BOOL TRUE)
    FetchContent_MakeAvailable(SDL3)
    list(APPEND INCLUDES ${SDL3_SOURCE_DIR}/include)
    list(APPEND LIBS SDL3::SDL3-static)
endif()

list(APPEND DEFINES SDL_MAIN_HANDLED)

list(APPEND PLATFORM_SOURCES ../external/fmt/src/format.cc)
list(PREPEND INCLUDES ../external/fmt/include)

list(APPEND QT_GUI_SOURCES
    src/main.cpp
    src/EmuApplication.cpp
    src/EmuMainWindow.cpp
    src/Snes9xController.cpp
    src/EmuSettingsWindow.cpp
    src/EmuConfig.cpp
    src/EmuPoTranslator.cpp
    src/EmuBinding.cpp
    src/EmuCanvas.cpp
    src/BindingPanel.cpp
    src/ControllerPanel.cpp
    src/DisplayPanel.cpp
    src/SoundPanel.cpp
    src/EmulationPanel.cpp
    src/ShortcutsPanel.cpp
    src/GeneralPanel.cpp
    src/FoldersPanel.cpp
    src/SDLInputManager.cpp
    src/ShaderParametersDialog.cpp
    src/CheatsDialog.cpp
    src/SoftwareScalers.cpp
    src/SoftwareFilters.cpp
    src/SoftwareFilters.hpp
    src/EmuCanvasQt.cpp
    src/EmuCanvasOpenGL.cpp
    ../external/glad/src/gl.c
    ../common/audio/s9x_sound_driver_sdl3.cpp
    ../common/audio/s9x_sound_driver_sdl3.hpp
    ../common/audio/s9x_sound_driver_cubeb.cpp
    ../common/audio/s9x_sound_driver_cubeb.hpp
    ../filter/2xsai.cpp
    ../filter/2xsai.h
    ../filter/epx.cpp
    ../filter/epx.h
    ../filter/filter_epx_unsafe.cpp
    ../filter/filter_epx_unsafe.h
    ../filter/hq2x.cpp
    ../filter/hq2x.h
    ../filter/xbrz.cpp
    ../filter/xbrz.h
    ../filter/snes_ntsc_config.h
    ../filter/snes_ntsc.h
    ../filter/snes_ntsc_impl.h
    ../filter/snes_ntsc.c)

if(NOT APPLE)
    list(APPEND QT_GUI_SOURCES src/EmuCanvasVulkan.cpp)
endif()

# RetroAchievements support
list(APPEND DEFINES RETROACHIEVEMENTS_SUPPORT RC_DISABLE_LUA RC_CLIENT_SUPPORTS_HASH)
list(APPEND INCLUDES ../external/rcheevos/include)
list(APPEND QT_GUI_SOURCES
    src/RAIntegrationQt.cpp
    ../retroachievements.cpp
    ../external/rcheevos/src/rc_client.c
    ../external/rcheevos/src/rc_compat.c
    ../external/rcheevos/src/rc_util.c
    ../external/rcheevos/src/rc_version.c
    ../external/rcheevos/src/rcheevos/alloc.c
    ../external/rcheevos/src/rcheevos/condition.c
    ../external/rcheevos/src/rcheevos/condset.c
    ../external/rcheevos/src/rcheevos/consoleinfo.c
    ../external/rcheevos/src/rcheevos/format.c
    ../external/rcheevos/src/rcheevos/lboard.c
    ../external/rcheevos/src/rcheevos/memref.c
    ../external/rcheevos/src/rcheevos/operand.c
    ../external/rcheevos/src/rcheevos/rc_validate.c
    ../external/rcheevos/src/rcheevos/richpresence.c
    ../external/rcheevos/src/rcheevos/runtime.c
    ../external/rcheevos/src/rcheevos/runtime_progress.c
    ../external/rcheevos/src/rcheevos/trigger.c
    ../external/rcheevos/src/rcheevos/value.c
    ../external/rcheevos/src/rapi/rc_api_common.c
    ../external/rcheevos/src/rapi/rc_api_editor.c
    ../external/rcheevos/src/rapi/rc_api_info.c
    ../external/rcheevos/src/rapi/rc_api_runtime.c
    ../external/rcheevos/src/rapi/rc_api_user.c
    ../external/rcheevos/src/rhash/hash.c
    ../external/rcheevos/src/rhash/hash_disc.c
    ../external/rcheevos/src/rhash/hash_encrypted.c
    ../external/rcheevos/src/rhash/hash_rom.c
    ../external/rcheevos/src/rhash/hash_zip.c
    ../external/rcheevos/src/rhash/aes.c
    ../external/rcheevos/src/rhash/cdreader.c
    ../external/rcheevos/src/rhash/md5.c)

# Kaillera netplay support
list(APPEND DEFINES KAILLERA_SUPPORT)
list(APPEND QT_GUI_SOURCES
    src/KailleraIntegrationQt.cpp
    ../kaillera_client.cpp
    ../kaillera_server.cpp)

set(QT_UI_FILES
    src/GeneralPanel.ui
    src/ControllerPanel.ui
    src/EmuSettingsWindow.ui
    src/DisplayPanel.ui
    src/SoundPanel.ui
    src/EmulationPanel.ui
    src/ShortcutsPanel.ui
    src/FoldersPanel.ui)

set(USE_SANITIZERS CACHE BOOL OFF)
set(BUILD_TESTS CACHE BOOL OFF)
set(BUILD_TOOLS CACHE BOOL OFF)
set(BUNDLE_SPEEX FORCE BOOL ON)
add_subdirectory("../external/cubeb" "cubeb" EXCLUDE_FROM_ALL)
list(APPEND LIBS cubeb)
list(APPEND INCLUDES "../external/cubeb/include")

set(BUILD_TESTING CACHE BOOL OFF)
set(ENABLE_OPT 0)
add_subdirectory("../external/glslang" "glslang" EXCLUDE_FROM_ALL)
list(APPEND LIBS
    glslang
    OSDependent
    SPIRV
    glslang-default-resource-limits)
list(APPEND INCLUDES "../external/glslang")

set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS CACHE BOOL ON)
add_subdirectory("../external/SPIRV-Cross" "SPIRV-Cross" EXCLUDE_FROM_ALL)
list(APPEND LIBS
    spirv-cross-core
    spirv-cross-glsl
    spirv-cross-reflect
    spirv-cross-cpp)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    list(APPEND DEFINES "VK_USE_PLATFORM_WIN32_KHR")
elseif(NOT APPLE)
    list(APPEND DEFINES
        "VK_USE_PLATFORM_XLIB_KHR"
        "VK_USE_PLATFORM_WAYLAND_KHR")
endif()

if(NOT APPLE)
    list(APPEND DEFINES
        "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1"
        "VULKAN_HPP_NO_NODISCARD_WARNINGS=1"
        "VULKAN_HPP_NO_EXCEPTIONS=1"
        "VMA_DYNAMIC_VULKAN_FUNCTIONS=1"
        "VMA_STATIC_VULKAN_FUNCTIONS=0")
    list(APPEND INCLUDES
        ../external/vulkan-headers/include
        ../external/VulkanMemoryAllocator-Hpp/include)
endif()

list(APPEND DEFINES "USE_SLANG")
list(APPEND INCLUDES
    ../external/stb
    "../external/glad/include")

# The slang_* files under common/video/vulkan/ are preset/shader *parsing*
# only -- they pull in glslang and SPIRV-Cross but no Vulkan API -- and the
# OpenGL shader stack (glsl.cpp, slang.cpp) depends on them. They are built
# on every platform; only the files that actually call Vulkan are skipped on
# macOS, which has no native Vulkan driver.
list(APPEND SOURCES
    ../common/video/vulkan/slang_shader.cpp
    ../common/video/vulkan/slang_shader.hpp
    ../common/video/vulkan/slang_preset.cpp
    ../common/video/vulkan/slang_preset.hpp
    ../common/video/std_chrono_throttle.cpp
    ../common/video/std_chrono_throttle.hpp
    ../common/video/vulkan/slang_helpers.cpp
    ../common/video/vulkan/slang_helpers.hpp
    ../common/video/vulkan/slang_preset_ini.cpp
    ../common/video/vulkan/slang_preset_ini.hpp
    ../external/stb/stb_image_implementation.cpp
    ../common/video/opengl/shaders/glsl.cpp
    ../common/video/opengl/shaders/slang.cpp
    ../common/video/opengl/shaders/shader_helpers.cpp)

if(NOT APPLE)
    list(APPEND SOURCES
        ../common/video/vulkan/vulkan_hpp_storage.cpp
        ../common/video/vulkan/vk_mem_alloc_implementation.cpp
        ../common/video/vulkan/vulkan_context.cpp
        ../common/video/vulkan/vulkan_context.hpp
        ../common/video/vulkan/vulkan_common.cpp
        ../common/video/vulkan/vulkan_common.hpp
        ../common/video/vulkan/vulkan_texture.cpp
        ../common/video/vulkan/vulkan_texture.hpp
        ../common/video/vulkan/vulkan_swapchain.cpp
        ../common/video/vulkan/vulkan_swapchain.hpp
        ../common/video/vulkan/vulkan_slang_pipeline.cpp
        ../common/video/vulkan/vulkan_slang_pipeline.hpp
        ../common/video/vulkan/vulkan_pipeline_image.cpp
        ../common/video/vulkan/vulkan_pipeline_image.hpp
        ../common/video/vulkan/vulkan_shader_chain.cpp
        ../common/video/vulkan/vulkan_shader_chain.hpp
        ../common/video/vulkan/vulkan_simple_output.hpp
        ../common/video/vulkan/vulkan_simple_output.cpp)
endif()

list(APPEND SOURCES ../external/imgui/imgui.cpp
    ../external/imgui/imgui_demo.cpp
    ../external/imgui/imgui_draw.cpp
    ../external/imgui/imgui_tables.cpp
    ../external/imgui/imgui_widgets.cpp
    ../external/imgui/imgui_impl_opengl3.cpp
    ../external/imgui/snes9x_imgui.cpp)
if(NOT APPLE)
    list(APPEND DEFINES "IMGUI_IMPL_VULKAN_NO_PROTOTYPES")
    list(APPEND SOURCES ../external/imgui/imgui_impl_vulkan.cpp)
endif()
list(APPEND INCLUDES ../external/imgui)

if(APPLE)
    # Build the .icns from the icon the fork already ships, so the bundle
    # never depends on a checked-in binary that can drift from the Windows
    # and Linux artwork. sips/iconutil are part of the base system.
    set(MACOS_ICONSET "${CMAKE_CURRENT_BINARY_DIR}/super-snes9x-qt.iconset")
    set(MACOS_ICNS "${CMAKE_CURRENT_BINARY_DIR}/super-snes9x-qt.icns")
    set(MACOS_ICON_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/snes9x.ico")

    # The source art is 256x256, so the largest honest entry is
    # icon_128x128@2x. Anything above that would just be an upscale.
    add_custom_command(
        OUTPUT ${MACOS_ICNS}
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${MACOS_ICONSET}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MACOS_ICONSET}
        COMMAND sips -s format png ${MACOS_ICON_SOURCE} --out ${MACOS_ICONSET}/base.png > /dev/null
        COMMAND sips -z 16 16     ${MACOS_ICONSET}/base.png --out ${MACOS_ICONSET}/icon_16x16.png > /dev/null
        COMMAND sips -z 32 32     ${MACOS_ICONSET}/base.png --out ${MACOS_ICONSET}/icon_16x16@2x.png > /dev/null
        COMMAND sips -z 32 32     ${MACOS_ICONSET}/base.png --out ${MACOS_ICONSET}/icon_32x32.png > /dev/null
        COMMAND sips -z 64 64     ${MACOS_ICONSET}/base.png --out ${MACOS_ICONSET}/icon_32x32@2x.png > /dev/null
        COMMAND sips -z 128 128   ${MACOS_ICONSET}/base.png --out ${MACOS_ICONSET}/icon_128x128.png > /dev/null
        COMMAND sips -z 256 256   ${MACOS_ICONSET}/base.png --out ${MACOS_ICONSET}/icon_128x128@2x.png > /dev/null
        COMMAND ${CMAKE_COMMAND} -E rm -f ${MACOS_ICONSET}/base.png
        COMMAND iconutil -c icns ${MACOS_ICONSET} -o ${MACOS_ICNS}
        DEPENDS ${MACOS_ICON_SOURCE}
        COMMENT "Generating super-snes9x-qt.icns"
        VERBATIM)

    # Listing the .icns as a source with MACOSX_PACKAGE_LOCATION is what gets
    # it copied into Contents/Resources.
    set_source_files_properties(${MACOS_ICNS} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
    list(APPEND PLATFORM_SOURCES ${MACOS_ICNS})

    # Ship the cheat database inside the bundle as well -- an .app has no
    # /usr/share to fall back on.
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/../data/cheats.bml"
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources" HEADER_FILE_ONLY ON)
    list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../data/cheats.bml")
endif()

add_executable(snes9x-qt ${QT_GUI_SOURCES} ${SOURCES} ${PLATFORM_SOURCES} src/resources/snes9x.qrc)
set_target_properties(snes9x-qt PROPERTIES OUTPUT_NAME super-snes9x-qt)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set_target_properties(snes9x-qt PROPERTIES WIN32_EXECUTABLE True)
endif()

if(APPLE)
    set_target_properties(snes9x-qt PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/Info.plist.in"
        MACOSX_BUNDLE_EXECUTABLE_NAME "super-snes9x-qt"
        MACOSX_BUNDLE_BUNDLE_NAME "Super Snes9x"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.snes9x.super-snes9x-qt"
        MACOSX_BUNDLE_ICON_FILE "super-snes9x-qt.icns"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Licensed under the Snes9x License.")
endif()
target_link_libraries(snes9x-qt PRIVATE snes9x-core ${LIBS})
target_compile_definitions(snes9x-qt PRIVATE ${DEFINES})
target_compile_options(snes9x-qt PRIVATE ${FLAGS})
target_include_directories(snes9x-qt PRIVATE "../" ${INCLUDES})

if(APPLE)
    # cheats.bml rides inside the bundle (CheatsDialog looks in
    # Contents/Resources); there is no shared datadir to install it to.
    install(TARGETS snes9x-qt BUNDLE DESTINATION .)
else()
    install(TARGETS snes9x-qt)
    install(FILES ../data/cheats.bml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/snes9x)
endif()
if(UNIX AND NOT APPLE)
    install(FILES src/resources/super-snes9x-qt.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    install(FILES src/resources/snes9x.svg DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps)
endif()
