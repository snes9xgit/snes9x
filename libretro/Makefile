ifeq ($(platform),)
   platform = unix
   ifeq ($(shell uname -a),)
      platform = win
   else ifneq ($(findstring Darwin,$(shell uname -a)),)
      platform = osx
   else ifneq ($(findstring MINGW,$(shell uname -a)),)
      platform = win
   else ifneq ($(findstring win,$(shell uname -a)),)
      platform = win
   endif
endif

CXX        = g++
CC         = gcc
TARGET_NAME = snes9x

ifeq ($(platform), unix)
   TARGET := $(TARGET_NAME)_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=link.T
else ifeq ($(platform), osx)
   TARGET := $(TARGET_NAME)_libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
else ifeq ($(platform), ps3)
   TARGET := $(TARGET_NAME)_libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-gcc.exe
   CXX = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-g++.exe
   AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
   CFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
else ifeq ($(platform), sncps3)
   TARGET := $(TARGET_NAME)_libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   CXX = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   AR = $(CELL_SDK)/host-win32/sn/bin/ps3snarl.exe
   CFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
else ifeq ($(platform), psl1ght)
   TARGET := $(TARGET_NAME)_libretro_psl1ght.a
   CC = $(PS3DEV)/ppu/bin/ppu-gcc$(EXE_EXT)
   CXX = $(PS3DEV)/ppu/bin/ppu-g++$(EXE_EXT)
   AR = $(PS3DEV)/ppu/bin/ppu-ar$(EXE_EXT)
   CFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
else ifeq ($(platform), xenon)
   TARGET := $(TARGET_NAME)_libretro_xenon360.a
   CC = xenon-gcc$(EXE_EXT)
   CXX = xenon-g++$(EXE_EXT)
   AR = xenon-ar$(EXE_EXT)
   CFLAGS += -D__LIBXENON__ -m32 -D__ppc__
	STATIC_LINKING = 1
else ifeq ($(platform), wii)
   TARGET := $(TARGET_NAME)_libretro_wii.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   CFLAGS += -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
else
   TARGET := $(TARGET_NAME)_libretro.dll
   CC = gcc
   CXX = g++
   SHARED := -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=link.T
   CXXFLAGS += -D__WIN32__ -D__WIN32_LIBSNES__
endif

OBJECTS    = ../apu/apu.o ../apu/bapu/dsp/sdsp.o ../apu/bapu/dsp/SPC_DSP.o ../apu/bapu/smp/smp.o ../apu/bapu/smp/smp_state.o ../bsx.o ../c4.o ../c4emu.o ../cheats.o ../cheats2.o ../clip.o ../conffile.o ../controls.o ../cpu.o ../cpuexec.o ../cpuops.o ../crosshairs.o ../dma.o ../dsp.o ../dsp1.o ../dsp2.o ../dsp3.o ../dsp4.o ../fxinst.o ../fxemu.o ../gfx.o ../globals.o ../logger.o ../memmap.o ../movie.o ../obc1.o ../ppu.o ../stream.o ../sa1.o ../sa1cpu.o ../screenshot.o ../sdd1.o ../sdd1emu.o ../seta.o ../seta010.o ../seta011.o ../seta018.o ../snapshot.o ../snes9x.o ../spc7110.o ../srtc.o ../tile.o libretro.o

INCLUDES   = -I. -I.. -I../apu/ -I../apu/bapu

ifeq ($(platform), sncps3)
   WARNINGS_DEFINES =
   CODE_DEFINES =
else
   WARNINGS_DEFINES = -Wall -W -Wno-unused-parameter
   CODE_DEFINES = -fomit-frame-pointer
endif

CXXFLAGS    += -O3 $(CODE_DEFINES) -fno-exceptions -fno-rtti -pedantic $(WARNINGS_DEFINES) $(fpic)
CXXFLAGS    += -DHAVE_STRINGS_H -DHAVE_STDINT_H -DRIGHTSHIFT_IS_SAR -D__LIBRETRO__
CFLAGS     = $(CXXFLAGS)

all: $(TARGET)

$(TARGET): $(OBJECTS)
ifeq ($(STATIC_LINKING), 1)
	$(AR) rcs $@ $(OBJECTS)
else
	$(CXX) $(fpic) $(SHARED) $(INCLUDES) -o $@ $(OBJECTS) -lm
endif

%.o: %.cpp 
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

