TARGET     := main
TARGET_MCU ?= CH32V003
CH32FUN := ../ch32fun/ch32fun

# --- Build configuration ---
# CFLAGS uses ?= in ch32fun.mk, so setting it here takes precedence.
COMMON_CFLAGS := -ffunction-sections -fdata-sections -fmessage-length=0 -msmall-data-limit=8

ifeq ($(BUILD),debug)
  # Debug: optimized for debugging, no LTO, full debug symbols + macro defs
  CFLAGS := $(COMMON_CFLAGS) -Og -g3
  DEBUG  := 1
else
  # Release (default): size-optimized with LTO, minimal debug info for addr2line
  CFLAGS := $(COMMON_CFLAGS) -Os -g -flto
endif

include $(CH32FUN)/ch32fun.mk

# --- Targets ---
all : flash

flash : cv_flash
clean : cv_clean

# Build-only (no flash) profiles
debug :
	$(MAKE) BUILD=debug build

release :
	$(MAKE) build

# Flash a debug build
flash-debug :
	$(MAKE) BUILD=debug flash

.PHONY : all flash clean debug release flash-debug
