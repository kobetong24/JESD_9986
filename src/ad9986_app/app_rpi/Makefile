TARGETNAME := ad9986_rpi_app

#TARGETTYPE can be APP, STATIC or SHARED
TARGETTYPE := APP

CONFIG ?= debug
CONFIGURATION_FLAGS_FILE := $(CONFIG).mak
include $(CONFIGURATION_FLAGS_FILE)

#LINKER_SCRIPT defined inside the configuration file (e.g. debug.mak)
CONFIGURATION_LINKER_SCRIPT := $(LINKER_SCRIPT)

include $(ADDITIONAL_MAKE_FILES)

ifneq ($(CONFIGURATION_LINKER_SCRIPT),)
LINKER_SCRIPT := $(CONFIGURATION_LINKER_SCRIPT)
endif

ifneq ($(LINKER_SCRIPT),)
LDFLAGS += -T$(LINKER_SCRIPT)
endif

ifeq ($(BINARYDIR),)
error:
	$(error Invalid configuration, please check your inputs)
endif

# Raspberry Pi 5 + Lattice FPGA platform: SPI control/configuration plane.
#   - JESD IP register access (adi_ads9) is re-routed over SPI0 CS0 by the
#     lattice platform (ads9_axi_reg_read32/write32).
#   - The ADS9 platform .c files and the Xilinx DMA stack are NOT compiled;
#     the lattice platform provides unsupported cdma_* stubs instead.
SOURCEFILES := $(wildcard ./*.c) \
               $(wildcard ../platform/lattice/*.c) \
               $(wildcard ../ad9986_api/ad9986/src/*.c) \
               $(wildcard ../ad9986_api/adi_utils/src/*.c) \
               $(wildcard ../hmc7044/src/*.c) \
               $(wildcard ../ads9_api/*.c)

EXTERNAL_LIBS :=
EXTERNAL_LIBS_COPIED := $(foreach lib, $(EXTERNAL_LIBS),$(BINARYDIR)/$(notdir $(lib)))

CFLAGS += $(addprefix -I,$(INCLUDE_DIRS))
CXXFLAGS += $(addprefix -I,$(INCLUDE_DIRS))

CFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS))
CXXFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS))
ASFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS))

LDFLAGS += $(addprefix -L,$(LIBRARY_DIRS))

ifeq ($(GENERATE_MAP_FILE),1)
LDFLAGS += -Wl,-Map=$(BINARYDIR)/$(basename $(TARGETNAME)).map
endif

LIBRARY_LDFLAGS = $(addprefix -l,$(LIBRARY_NAMES))

ifeq ($(IS_LINUX_PROJECT),1)
	RPATH_PREFIX := -Wl,--rpath='$$ORIGIN/../
	LIBRARY_LDFLAGS += $(EXTERNAL_LIBS)
	LIBRARY_LDFLAGS += -Wl,--rpath='$$ORIGIN'
	LIBRARY_LDFLAGS += $(addsuffix ',$(addprefix $(RPATH_PREFIX),$(dir $(EXTERNAL_LIBS))))

	ifeq ($(TARGETTYPE),SHARED)
		CFLAGS += -fPIC
		CXXFLAGS += -fPIC
		ASFLAGS += -fPIC
		LIBRARY_LDFLAGS += -Wl,-soname,$(TARGETNAME)
	endif

	ifneq ($(LINUX_PACKAGES),)
		PACKAGE_CFLAGS := $(foreach pkg,$(LINUX_PACKAGES),$(shell pkg-config --cflags $(pkg)))
		PACKAGE_LDFLAGS := $(foreach pkg,$(LINUX_PACKAGES),$(shell pkg-config --libs $(pkg)))
		CFLAGS += $(PACKAGE_CFLAGS)
		CXXFLAGS += $(PACKAGE_CFLAGS)
		LIBRARY_LDFLAGS += $(PACKAGE_LDFLAGS)
	endif
else
	LIBRARY_LDFLAGS += $(EXTERNAL_LIBS)
endif

LIBRARY_LDFLAGS += $(ADDITIONAL_LINKER_INPUTS)

all_make_files := $(firstword $(MAKEFILE_LIST)) $(CONFIGURATION_FLAGS_FILE) $(ADDITIONAL_MAKE_FILES)

ifeq ($(STARTUPFILES),)
	all_source_files := $(SOURCEFILES)
else
	all_source_files := $(STARTUPFILES) $(filter-out $(STARTUPFILES),$(SOURCEFILES))
endif

source_obj1 := $(all_source_files:.cpp=.o)
source_obj2 := $(source_obj1:.c=.o)
source_obj3 := $(source_obj2:.s=.o)
source_obj4 := $(source_obj3:.S=.o)
source_obj5 := $(source_obj4:.cc=.o)
source_objs := $(source_obj5:.cxx=.o)

all_objs := $(addprefix $(BINARYDIR)/, $(notdir $(source_objs)))

PRIMARY_OUTPUTS :=

ifeq ($(PRIMARY_OUTPUTS),)
PRIMARY_OUTPUTS := $(BINARYDIR)/$(TARGETNAME)
endif

all: $(PRIMARY_OUTPUTS)

ifeq ($(TARGETTYPE),APP)
$(BINARYDIR)/$(TARGETNAME): $(all_objs) $(EXTERNAL_LIBS)
	$(CC) $(COVERAGE_FLAGS) -o $@ $(LDFLAGS) $(START_GROUP) $(all_objs) $(LIBRARY_LDFLAGS) $(END_GROUP)
endif

ifeq ($(TARGETTYPE),SHARED)
$(BINARYDIR)/$(TARGETNAME): $(all_objs) $(EXTERNAL_LIBS)
	$(LD) -shared -o $@ $(LDFLAGS) $(START_GROUP) $(all_objs) $(LIBRARY_LDFLAGS) $(END_GROUP)
endif

ifeq ($(TARGETTYPE),STATIC)
$(BINARYDIR)/$(TARGETNAME): $(all_objs)
	$(AR) -r $@ $^
endif

-include $(all_objs:.o=.dep)

clean:
ifeq ($(USE_DEL_TO_CLEAN),1)
	cmd /C del /S /Q $(BINARYDIR)
else
	rm -rf $(BINARYDIR)
endif

$(BINARYDIR):
	mkdir $(BINARYDIR)

$(BINARYDIR)/%.o : ../ad9986_api/ad9986/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../ad9986_api/adi_utils/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../hmc7044/src/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../ads9_api/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ../platform/lattice/%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
$(BINARYDIR)/%.o : ./%.c $(all_make_files) |$(BINARYDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)
