BINARYDIR := debug

# Tool chain
CC := gcc
CXX := g++
LD := $(CXX)
AR := ar
OBJCOPY := objcopy

# Additional flags
PREPROCESSOR_MACROS := DEBUG
INCLUDE_DIRS := ../ad9986_api/adi_inc ../ad9986_api/adi_utils/inc ../ad9986_api/ad9986/inc ../platform/ads9 ../platform/ads9/dma ../hmc7044/inc ../ads9_api ../ad7175/inc .
LIBRARY_DIRS := 
LIBRARY_NAMES := 
ADDITIONAL_LINKER_INPUTS := -lm
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
