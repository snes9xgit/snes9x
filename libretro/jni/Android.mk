LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS += -DANDROID_ARM
LOCAL_ARM_MODE := arm
endif

ifeq ($(TARGET_ARCH),x86)
LOCAL_CFLAGS +=  -DANDROID_X86
endif

ifeq ($(TARGET_ARCH),mips)
LOCAL_CFLAGS += -DANDROID_MIPS
endif

LOCAL_MODULE    := libretro
LOCAL_SRC_FILES    =  ../../apu/apu.cpp ../../apu/bapu/dsp/sdsp.cpp  ../../apu/bapu/dsp/SPC_DSP.cpp ../../apu/bapu/smp/smp.cpp ../../apu/bapu/smp/smp_state.cpp  ../../bsx.cpp ../../c4.cpp ../../c4emu.cpp ../../cheats.cpp ../../cheats2.cpp ../../clip.cpp ../../conffile.cpp ../../controls.cpp ../../cpu.cpp ../../cpuexec.cpp ../../cpuops.cpp ../../crosshairs.cpp ../../dma.cpp ../../dsp.cpp ../../dsp1.cpp ../../dsp2.cpp ../../dsp3.cpp ../../dsp4.cpp ../../fxinst.cpp ../../fxemu.cpp ../../gfx.cpp ../../globals.cpp ../../logger.cpp ../../memmap.cpp ../../movie.cpp ../../obc1.cpp ../../ppu.cpp ../../stream.cpp ../../sa1.cpp ../../sa1cpu.cpp ../../screenshot.cpp ../../sdd1.cpp ../../sdd1emu.cpp ../../seta.cpp ../../seta010.cpp ../../seta011.cpp ../../seta018.cpp ../../snapshot.cpp ../../snes9x.cpp ../../spc7110.cpp ../../srtc.cpp ../../tile.cpp ../libretro.cpp
LOCAL_CXXFLAGS = -DANDROID -DARM -D__LIBRETRO__ -DHAVE_STRINGS_H -DHAVE_STDINT_H -DRIGHTSHIFT_IS_SAR
LOCAL_C_INCLUDES = ../../ ../../apu/bapu/

include $(BUILD_SHARED_LIBRARY)
