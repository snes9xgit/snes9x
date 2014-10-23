ifeq ($(platform),)
   platform = unix
   ifeq ($(shell uname -a),)
      platform = win
   else ifneq ($(findstring Darwin,$(shell uname -a)),)
      platform = osx
	arch = intel
   ifeq ($(shell uname -p),powerpc)
	   arch = ppc
	endif
   else ifneq ($(findstring MINGW,$(shell uname -a)),)
      platform = win
   else ifneq ($(findstring win,$(shell uname -a)),)
      platform = win
   endif
endif

CXX        = g++
CC         = gcc
TARGET_NAME = snes9x
LIBM       = -lm

ifeq ($(platform), unix)
   TARGET := $(TARGET_NAME)_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=link.T
ifneq ($(findstring Haiku,$(shell uname -a)),)
   LIBM :=
endif
else ifeq ($(platform), osx)
   TARGET := $(TARGET_NAME)_libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib

   arch = intel
ifeq ($(shell uname -p),powerpc)
   arch = ppc
endif

ifeq ($(arch),ppc)
   CXXFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
endif
   OSXVER = `sw_vers -productVersion | cut -d. -f 2`
   OSX_LT_MAVERICKS = `(( $(OSXVER) <= 9)) && echo "YES"`
ifeq ($(OSX_LT_MAVERICKS),"YES")
   fpic += -mmacosx-version-min=10.6
endif
else ifeq ($(platform), ios)
   TARGET := $(TARGET_NAME)_libretro_ios.dylib
   fpic := -fPIC
   SHARED := -dynamiclib

ifeq ($(IOSSDK),)
   IOSSDK := $(shell xcrun -sdk iphoneos -show-sdk-path)
endif

	CC = clang -arch armv7 -isysroot $(IOSSDK)
	CXX = clang++ -arch armv7 -isysroot $(IOSSDK)
	CXXFLAGS += -DIOS
   CXXFLAGS += -DARM
   OSXVER = `sw_vers -productVersion | cut -d. -f 2`
   OSX_LT_MAVERICKS = `(( $(OSXVER) <= 9)) && echo "YES"`
ifeq ($(OSX_LT_MAVERICKS),"YES")
   CC += -miphoneos-version-min=5.0
   CXX += -miphoneos-version-min=5.0
   CFLAGS += -miphoneos-version-min=5.0
   CXXFLAGS += -miphoneos-version-min=5.0
endif
else ifeq ($(platform), qnx)
	TARGET := $(TARGET_NAME)_libretro_qnx.so
	fpic := -fPIC
   SHARED := -shared -Wl,--version-script=link.T
	CC = qcc -Vgcc_notarmv7le
	CC = QCC -Vgcc_notarmv7le_cpp
	AR = QCC -Vgcc_ntoarmv7le
	CXXFLAGS += -D__BLACKBERRY_QNX__
   CXXFLAGS += -DARM
else ifeq ($(platform), ps3)
   TARGET := $(TARGET_NAME)_libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-gcc.exe
   CXX = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-g++.exe
   AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
   CXXFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
else ifeq ($(platform), sncps3)
   TARGET := $(TARGET_NAME)_libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   CXX = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   AR = $(CELL_SDK)/host-win32/sn/bin/ps3snarl.exe
   CXXFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
else ifeq ($(platform), psl1ght)
   TARGET := $(TARGET_NAME)_libretro_psl1ght.a
   CC = $(PS3DEV)/ppu/bin/ppu-gcc$(EXE_EXT)
   CXX = $(PS3DEV)/ppu/bin/ppu-g++$(EXE_EXT)
   AR = $(PS3DEV)/ppu/bin/ppu-ar$(EXE_EXT)
   CXXFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
else ifeq ($(platform), xenon)
   TARGET := $(TARGET_NAME)_libretro_xenon360.a
   CC = xenon-gcc$(EXE_EXT)
   CXX = xenon-g++$(EXE_EXT)
   AR = xenon-ar$(EXE_EXT)
   CXXFLAGS += -D__LIBXENON__ -m32 -D__ppc__
	STATIC_LINKING = 1
else ifeq ($(platform), wii)
   TARGET := $(TARGET_NAME)_libretro_wii.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   CXXFLAGS += -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
else ifneq (,$(findstring armv,$(platform)))
   TARGET := $(TARGET_NAME)_libretro.so
   SHARED := -shared -Wl,--no-undefined
   fpic := -fPIC
   CC = gcc
   CXX = g++
ifneq (,$(findstring cortexa8,$(platform)))
   CXXFLAGS += -marm -mcpu=cortex-a8
else ifneq (,$(findstring cortexa9,$(platform)))
   CXXFLAGS += -marm -mcpu=cortex-a9
endif
   CXXFLAGS += -marm
ifneq (,$(findstring neon,$(platform)))
   CXXFLAGS += -mfpu=neon
   HAVE_NEON = 1
endif
ifneq (,$(findstring softfloat,$(platform)))
   CXXFLAGS += -mfloat-abi=softfp
else ifneq (,$(findstring hardfloat,$(platform)))
   CXXFLAGS += -mfloat-abi=hard
endif
   CXXFLAGS += -DARM
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
	$(CXX) $(fpic) $(SHARED) $(INCLUDES) -o $@ $(OBJECTS) $(LIBM)
endif

%.o: %.cpp 
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

