LOCAL_PATH := $(call my-dir)

CORE_DIR := $(LOCAL_PATH)/../..

include $(CORE_DIR)/libretro/Makefile.common

COREFLAGS := -DANDROID -D__LIBRETRO__ -DHAVE_STRINGS_H -DRIGHTSHIFT_IS_SAR $(INCFLAGS)

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
  COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

include $(CLEAR_VARS)
LOCAL_MODULE    := retro
LOCAL_SRC_FILES := $(SOURCES_C) $(SOURCES_CXX)
LOCAL_CXXFLAGS  := $(COREFLAGS)
LOCAL_LDFLAGS   := -Wl,-version-script=$(CORE_DIR)/libretro/link.T
include $(BUILD_SHARED_LIBRARY)
