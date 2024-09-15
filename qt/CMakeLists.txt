cmake_minimum_required(VERSION 3.20)
project(snes9x-qt VERSION 1.63)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_GLOBAL_AUTOGEN_TARGET ON)

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
    ../fscompat.cpp)
add_library(snes9x-core ${SNES9X_CORE_SOURCES})
target_include_directories(snes9x-core PRIVATE ../)
target_compile_definitions(snes9x-core PRIVATE ${DEFINES} ZLIB HAVE_STDINT_H HAVE_LIBPNG ALLOW_CPU_OVERCLOCK)

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
list(APPEND INCLUDES ${Qt6Gui_PRIVATE_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL REQUIRED sdl2)
pkg_check_modules(ZLIB REQUIRED zlib)
pkg_check_modules(PNG REQUIRED libpng)
list(APPEND INCLUDES ${SDL_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
list(APPEND FLAGS ${SDL_COMPILE_FLAGS} ${ZLIB_COMPILE_FLAGS})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message("Clang compiler, using libc++.")
        list(APPEND LIBS libc++.a)
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        message("GNU compiler, using libstdc++.")
        list(APPEND LIBS libstdc++.a)
    endif()

    list(APPEND LIBS libSDL2.a libz.a libpng.a opengl32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32 dinput8)
    list(APPEND DEFINES SDL_MAIN_HANDLED)
    list(APPEND PLATFORM_SOURCES
        ../common/video/opengl/wgl_context.cpp
        ../external/glad/src/wgl.c
        src/resources/snes9x_win32.rc)
else()

    pkg_check_modules(WAYLAND REQUIRED wayland-client wayland-egl)
    include(FindX11)
    if(NOT X11_FOUND)
        error()
    endif()

    list(APPEND INCLUDES ${WAYLAND_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
    list(APPEND LIBS ${WAYLAND_LIBRARIES} ${PNG_LIBRARIES} ${X11_LIBRARIES} ${ZLIB_LIBRARIES} ${SDL_LIBRARIES})
    list(APPEND FLAGS ${WAYLAND_CFLAGS})

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

list(APPEND PLATFORM_SOURCES ../external/fmt/src/format.cc)
list(APPEND INCLUDES ../external/fmt/include)

list(APPEND QT_GUI_SOURCES
    src/main.cpp
    src/EmuApplication.cpp
    src/EmuMainWindow.cpp
    src/Snes9xController.cpp
    src/EmuSettingsWindow.cpp
    src/EmuConfig.cpp
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
    src/EmuCanvasQt.cpp
    src/EmuCanvasOpenGL.cpp
    src/EmuCanvasVulkan.cpp
    ../external/glad/src/gl.c
    ../common/audio/s9x_sound_driver_sdl.cpp
    ../common/audio/s9x_sound_driver_sdl.hpp
    ../common/audio/s9x_sound_driver_cubeb.cpp
    ../common/audio/s9x_sound_driver_cubeb.hpp
    ../filter/2xsai.cpp
    ../filter/2xsai.h
    ../filter/epx.cpp
    ../filter/epx.h
    ../filter/snes_ntsc_config.h
    ../filter/snes_ntsc.h
    ../filter/snes_ntsc_impl.h
    ../filter/snes_ntsc.c)

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
add_subdirectory("../external/glslang" "glslang" EXCLUDE_FROM_ALL)
list(APPEND LIBS
    glslang
    OGLCompiler
    HLSL
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
else()
    list(APPEND DEFINES
        "VK_USE_PLATFORM_XLIB_KHR"
        "VK_USE_PLATFORM_WAYLAND_KHR")
endif()

list(APPEND DEFINES
    "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1"
    "VULKAN_HPP_NO_NODISCARD_WARNINGS=1"
    "VULKAN_HPP_NO_EXCEPTIONS=1"
    "VMA_DYNAMIC_VULKAN_FUNCTIONS=1"
    "VMA_STATIC_VULKAN_FUNCTIONS=0"
    "USE_SLANG")
list(APPEND INCLUDES
    ../external/vulkan-headers/include
    ../external/VulkanMemoryAllocator-Hpp/include
    ../external/stb
    "../external/glad/include")
list(APPEND SOURCES
    ../common/video/vulkan/slang_shader.cpp
    ../common/video/vulkan/slang_shader.hpp
    ../common/video/vulkan/slang_preset.cpp
    ../common/video/vulkan/slang_preset.hpp
    ../common/video/vulkan/vulkan_hpp_storage.cpp
    ../common/video/vulkan/vk_mem_alloc_implementation.cpp
    ../common/video/vulkan/vulkan_context.cpp
    ../common/video/vulkan/vulkan_context.hpp
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
    ../common/video/vulkan/vulkan_simple_output.cpp
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

list(APPEND DEFINES "IMGUI_IMPL_VULKAN_NO_PROTOTYPES")
list(APPEND SOURCES ../external/imgui/imgui.cpp
    ../external/imgui/imgui_demo.cpp
    ../external/imgui/imgui_draw.cpp
    ../external/imgui/imgui_tables.cpp
    ../external/imgui/imgui_widgets.cpp
    ../external/imgui/imgui_impl_opengl3.cpp
    ../external/imgui/imgui_impl_vulkan.cpp
    ../external/imgui/snes9x_imgui.cpp)
list(APPEND INCLUDES ../external/imgui)

add_executable(snes9x-qt ${QT_GUI_SOURCES} ${SOURCES} ${PLATFORM_SOURCES} src/resources/snes9x.qrc)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set_target_properties(snes9x-qt PROPERTIES WIN32_EXECUTABLE True)
endif()
target_link_libraries(snes9x-qt snes9x-core ${LIBS})
target_compile_definitions(snes9x-qt PRIVATE ${DEFINES})
target_compile_options(snes9x-qt PRIVATE ${FLAGS})
target_include_directories(snes9x-qt PRIVATE "../" ${INCLUDES})

install(TARGETS snes9x-qt)
install(FILES ../data/cheats.bml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/snes9x)
