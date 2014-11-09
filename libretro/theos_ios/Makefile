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

CXX_INCDIRS :=

ifeq ($(platform), osx)
_LATEST_SDK := $(shell xcrun -sdk iphoneos -show-sdk-version)
else
_SDK_DIR := $(THEOS)/sdks
_IOS_SDKS := $(sort $(patsubst $(_SDK_DIR)/iPhoneOS%.sdk,%,$(wildcard $(_SDK_DIR)/iPhoneOS*.sdk)))
_LATEST_SDK := $(word $(words $(_IOS_SDKS)),$(_IOS_SDKS))
CXX_INCDIRS += -I$(_SDK_DIR)/iPhoneOS$(_LATEST_SDK).sdk/usr/include/c++/4.2.1
endif

SDKVERSION := $(_LATEST_SDK)
ARCHS = armv7 armv7s
include $(THEOS)/makefiles/common.mk

LIBRARY_NAME = snes9x_libretro_ios
CORE_DIR := ../..

include ../Makefile.common

${LIBRARY_NAME}_FILES = $(SOURCES_CXX) $(SOURCES_C)

COMMON_FLAGS = -DIOS -DARM -D__LIBRETRO__ -DHAVE_STRINGS_H -DHAVE_STDINT_H -DRIGHTSHIFT_IS_SAR $(INCFLAGS) -I$(THEOS_INCLUDE_PATH) -Wno-error

$(LIBRARY_NAME)_CFLAGS += $(COMMON_FLAGS) $(CXX_INCDIRS)
$(LIBRARY_NAME)_CXXFLAGS += $(COMMON_FLAGS) $(CXX_INCDIRS)

include $(THEOS_MAKE_PATH)/library.mk
