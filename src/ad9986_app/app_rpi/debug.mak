BINARYDIR := debug

# Tool chain
# For native Raspberry Pi 5 builds use the default gcc.
# For cross-compilation set CROSS_COMPILE, e.g.:
#   make CROSS_COMPILE=aarch64-linux-gnu-
CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD := $(CXX)
AR := $(CROSS_COMPILE)ar
OBJCOPY := $(CROSS_COMPILE)objcopy

# Additional flags
# Note: ../platform/ads9 and ../platform/ads9/dma are on the include path for
# the headers (ads9.h, cdmaapi.h) referenced by the reused adi_ads9 module.
# No ADS9 .c files are compiled for this target.
PREPROCESSOR_MACROS := DEBUG
INCLUDE_DIRS := ../ad9986_api/adi_inc ../ad9986_api/adi_utils/inc ../ad9986_api/ad9986/inc ../platform/lattice ../platform/ads9 ../platform/ads9/dma ../hmc7044/inc ../ads9_api .
LIBRARY_DIRS :=
LIBRARY_NAMES :=
ADDITIONAL_LINKER_INPUTS := -lm -lpthread
MACOS_FRAMEWORKS :=
LINUX_PACKAGES :=

CFLAGS := -ggdb -ffunction-sections -O0 -pedantic -std=gnu99 -Wall -fprofile-arcs -ftest-coverage
CXXFLAGS := -ggdb -ffunction-sections -O0 -pedantic -std=gnu99 -Wall -fprofile-arcs -ftest-coverage
ASFLAGS :=
LDFLAGS := -Wl,-gc-sections
COMMONFLAGS :=
LINKER_SCRIPT :=

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

# Additional options detected from testing the tool chain
IS_LINUX_PROJECT := 1

# gcov instrumentation (debug only); release.mak leaves this empty
COVERAGE_FLAGS := -fprofile-arcs -ftest-coverage
