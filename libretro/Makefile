DEBUG = 0
HAVE_EXCEPTIONS = 0

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

CXX			?= g++
CC			?= gcc
TARGET_NAME	= snes9x
LIBM		= -lm

# Unix
ifeq ($(platform), unix)
	TARGET := $(TARGET_NAME)_libretro.so
	fpic := -fPIC
	SHARED := -shared -Wl,--version-script=link.T
	ifneq ($(findstring Haiku,$(shell uname -a)),)
		LIBM :=
	endif

# OS X
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
	fpic += -mmacosx-version-min=10.1

# iOS
else ifneq (,$(findstring ios,$(platform)))
	TARGET := $(TARGET_NAME)_libretro_ios.dylib
	fpic := -fPIC
	SHARED := -dynamiclib

	ifeq ($(IOSSDK),)
		IOSSDK := $(shell xcodebuild -version -sdk iphoneos Path)
	endif

	CC = clang -arch armv7 -isysroot $(IOSSDK)
	CXX = clang++ -arch armv7 -isysroot $(IOSSDK)
	CXXFLAGS += -DIOS
	CXXFLAGS += -DARM
ifeq ($(platform),ios9)
	CC       += -miphoneos-version-min=8.0
	CXX      += -miphoneos-version-min=8.0
	CFLAGS   += -miphoneos-version-min=8.0
	CXXFLAGS += -miphoneos-version-min=8.0
else
	CC       += -miphoneos-version-min=5.0
	CXX      += -miphoneos-version-min=5.0
	CFLAGS   += -miphoneos-version-min=5.0
	CXXFLAGS += -miphoneos-version-min=5.0
endif
else ifeq ($(platform), theos_ios)
DEPLOYMENT_IOSVERSION = 5.0
TARGET = iphone:latest:$(DEPLOYMENT_IOSVERSION)
ARCHS = armv7 armv7s
TARGET_IPHONEOS_DEPLOYMENT_VERSION=$(DEPLOYMENT_IOSVERSION)
THEOS_BUILD_DIR := objs
include $(THEOS)/makefiles/common.mk

LIBRARY_NAME = $(TARGET_NAME)_libretro_ios

# QNX
else ifeq ($(platform), qnx)
	TARGET := $(TARGET_NAME)_libretro_$(platform).so
	fpic := -fPIC
   SHARED := -shared -Wl,--version-script=link.T
	CC = qcc -Vgcc_notarmv7le
	CXX = QCC -Vgcc_notarmv7le
	AR = QCC -Vgcc_ntoarmv7le
	CXXFLAGS += -D__BLACKBERRY_QNX__
	CXXFLAGS += -DARM
	HAVE_EXCEPTIONS = 1

# Vita
else ifeq ($(platform), vita)
	TARGET := $(TARGET_NAME)_libretro_$(platform).so
	fpic := -fPIC
	CC = arm-vita-eabi-gcc$(EXE_EXT)
	CXX = arm-vita-eabi-g++$(EXE_EXT)
	AR = arm-vita-eabi-ar$(EXE_EXT)
	CXXFLAGS += -DVITA
	HAVE_EXCEPTIONS = 1

# PS3
else ifeq ($(platform), ps3)
	TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CC = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-gcc.exe
	CXX = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-g++.exe
	AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
	CXXFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1

# sncps3
else ifeq ($(platform), sncps3)
	TARGET := $(TARGET_NAME)_libretro_ps3.a
	CC = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
	CXX = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
	AR = $(CELL_SDK)/host-win32/sn/bin/ps3snarl.exe
	CXXFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1

# Lightweight PS3 Homebrew SDK
else ifeq ($(platform), psl1ght)
	TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CC = $(PS3DEV)/ppu/bin/ppu-gcc$(EXE_EXT)
	CXX = $(PS3DEV)/ppu/bin/ppu-g++$(EXE_EXT)
	AR = $(PS3DEV)/ppu/bin/ppu-ar$(EXE_EXT)
	CXXFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1

# Xbox 360
else ifeq ($(platform), xenon)
	TARGET := $(TARGET_NAME)_libretro_xenon360.a
	CC = xenon-gcc$(EXE_EXT)
	CXX = xenon-g++$(EXE_EXT)
	AR = xenon-ar$(EXE_EXT)
	CXXFLAGS += -D__LIBXENON__ -m32 -D__ppc__
	STATIC_LINKING = 1

# Nintendo Wii
else ifeq ($(platform), wii)
	TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
	CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
	AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
	CXXFLAGS += -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1

else ifeq ($(platform), emscripten)
	TARGET := $(TARGET_NAME)_libretro_$(platform).bc
	STATIC_LINKING = 1

# ARM
else ifneq (,$(findstring armv,$(platform)))
	TARGET := $(TARGET_NAME)_libretro.so
	SHARED := -shared -Wl,--no-undefined
	fpic := -fPIC
	CC ?= gcc
	CXX ?= g++
	ifneq (,$(findstring cortexa8,$(platform)))
		CXXFLAGS += -marm -mcpu=cortex-a8
	else ifneq (,$(findstring cortexa9,$(platform)))
		CXXFLAGS += -marm -mcpu=cortex-a9
	else ifneq (,$(findstring cortexa53,$(platform)))
		CXXFLAGS += -marm -mcpu=cortex-a53
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

# Android
else ifneq (,$(findstring android,$(platform)))
	TARGET := $(TARGET_NAME)_libretro_android.so
	SHARED := -shared -Wl,--no-undefined -march=armv7-a -Wl,--fix-cortex-a8
	fpic := -fPIC
	CXXFLAGS += -marm -march=armv7-a -mfloat-abi=softfp -mfpu=neon -DANDROID -DARM
	HAVE_NEON = 1

# Windows
else
	TARGET := $(TARGET_NAME)_libretro.dll
	CC = gcc
	CXX = g++
	SHARED := -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=link.T
	CXXFLAGS += -D__WIN32__ -D__WIN32_LIBSNES__

endif

CORE_DIR := ..

ifeq ($(DEBUG), 1)
   CXXFLAGS += -O0 -g
   CFLAGS += -O0 -g
else
   CXXFLAGS += -O3 -DNDEBUG
   CFLAGS += -O3 -DNDEBUG
endif

include Makefile.common

OBJECTS := $(SOURCES_CXX:.cpp=.o) $(SOURCES_C:.c=.o)

ifeq ($(platform), sncps3)
	WARNINGS_DEFINES =
	CODE_DEFINES =
else
	WARNINGS_DEFINES = -Wall -W -Wno-unused-parameter
	CODE_DEFINES = -fomit-frame-pointer
endif

ifneq ($(HAVE_EXCEPTIONS), 1)
CXXFLAGS += -fno-exceptions
endif

CXXFLAGS	+= $(CODE_DEFINES) -fno-rtti -pedantic $(WARNINGS_DEFINES) $(fpic)
CXXFLAGS	+= -DHAVE_STRINGS_H -DHAVE_STDINT_H -DRIGHTSHIFT_IS_SAR -D__LIBRETRO__
CFLAGS		= $(CXXFLAGS)

ifeq ($(platform), theos_ios)
COMMON_FLAGS := -DIOS -DARM $(COMMON_DEFINES) $(INCFLAGS) -I$(THEOS_INCLUDE_PATH) -Wno-error
$(LIBRARY_NAME)_CFLAGS += $(CFLAGS) $(COMMON_FLAGS)
$(LIBRARY_NAME)_CXXFLAGS += $(CXXFLAGS) $(COMMON_FLAGS)
${LIBRARY_NAME}_FILES = $(SOURCES_CXX) $(SOURCES_C)
include $(THEOS_MAKE_PATH)/library.mk
else
all: $(TARGET)

$(TARGET): $(OBJECTS)
ifeq ($(STATIC_LINKING), 1)
	$(AR) rcs $@ $(OBJECTS)
else
	$(CXX) $(fpic) $(SHARED) $(INCFLAGS) -o $@ $(OBJECTS) $(LIBM)
endif

%.o: %.cpp 
	$(CXX) $(INCFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(INCFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
endif
