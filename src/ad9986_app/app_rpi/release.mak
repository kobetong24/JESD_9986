BINARYDIR := release

# Tool chain
# Cross-compile for Raspberry Pi 5 (aarch64) by setting CROSS_COMPILE, e.g.:
#   make CONFIG=release CROSS_COMPILE=aarch64-linux-gnu-
# Build natively on the Pi by leaving CROSS_COMPILE empty.
CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD := $(CXX)
AR := $(CROSS_COMPILE)ar
OBJCOPY := $(CROSS_COMPILE)objcopy

# Additional flags
# Release configuration: optimized, no gcov instrumentation, so the binary is
# self-contained and suitable for deployment on the target.
PREPROCESSOR_MACROS := NDEBUG
INCLUDE_DIRS := ../ad9986_api/adi_inc ../ad9986_api/adi_utils/inc ../ad9986_api/ad9986/inc ../platform/lattice ../platform/ads9 ../platform/ads9/dma ../hmc7044/inc ../ads9_api .
LIBRARY_DIRS :=
LIBRARY_NAMES :=
ADDITIONAL_LINKER_INPUTS := -lm
MACOS_FRAMEWORKS :=
LINUX_PACKAGES :=

CFLAGS := -ffunction-sections -O2 -std=gnu99 -Wall
CXXFLAGS := -ffunction-sections -O2 -std=gnu99 -Wall
ASFLAGS :=
LDFLAGS := -Wl,-gc-sections
COMMONFLAGS :=
LINKER_SCRIPT :=

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

IS_LINUX_PROJECT := 1
