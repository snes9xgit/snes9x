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

ifeq ($(platform), unix)
   TARGET := libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=link.T
else ifeq ($(platform), osx)
   TARGET := libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
else
   TARGET := libretro.dll
   CC = gcc
   CXX = g++
   SHARED := -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=link.T
   CXXFLAGS += -D__WIN32__ -D__WIN32_LIBSNES__
endif

OBJECTS    = ../apu/apu.o ../apu/bapu/dsp/sdsp.o ../apu/bapu/dsp/SPC_DSP.o ../apu/bapu/smp/smp.o ../apu/bapu/smp/smp_state.o ../bsx.o ../c4.o ../c4emu.o ../cheats.o ../cheats2.o ../clip.o ../conffile.o ../controls.o ../cpu.o ../cpuexec.o ../cpuops.o ../crosshairs.o ../dma.o ../dsp.o ../dsp1.o ../dsp2.o ../dsp3.o ../dsp4.o ../fxinst.o ../fxemu.o ../gfx.o ../globals.o ../logger.o ../memmap.o ../movie.o ../obc1.o ../ppu.o ../stream.o ../sa1.o ../sa1cpu.o ../screenshot.o ../sdd1.o ../sdd1emu.o ../seta.o ../seta010.o ../seta011.o ../seta018.o ../snapshot.o ../snes9x.o ../spc7110.o ../srtc.o ../tile.o libretro.o

CXX        = g++
CC         = gcc
INCLUDES   = -I. -I.. -I../apu/ -I../apu/bapu

CXXFLAGS    += -O3 -fomit-frame-pointer -fno-exceptions -fno-rtti -pedantic -Wall -W -Wno-unused-parameter $(fpic)
CXXFLAGS    += -DHAVE_STRINGS_H -DHAVE_STDINT_H -DRIGHTSHIFT_IS_SAR
CFLAGS     = $(CXXFLAGS)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(fpic) $(SHARED) $(INCLUDES) -o $@ $(OBJECTS) -lm

%.o: %.cpp 
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

