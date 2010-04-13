# This file contains Makefile rules for building libraries, 
# modules, and executables.
#
# The following should be defined before including this file:
#
# TOP_SRC_DIR
# TOP_BUILD_DIR (optional, defaults top TOP_SRC_DIR)
# CXX
# CXX_FLAGS
# LD_FLAGS
# INCLUDES
# LIBS
# AR
# AS
# PLATFORM_NAME
# LIB_PREFIX
# DYNAMIC_LIB_EXT
# STATIC_LIB_EXT
# EXECUTABLE_EXT

TOP_BUILD_DIR ?= $(TOP_SRC_DIR)
BUILD_DIR ?= $(TOP_BUILD_DIR)/build/$(subst $(TOP_BUILD_DIR)/cpp/,,$(CWD))
DIST_DIR ?= $(TOP_BUILD_DIR)/dist
LIB_DIR ?= $(DIST_DIR)/lib
MOD_DIR ?= $(DIST_DIR)/modules
BIN_DIR ?= $(DIST_DIR)/bin

# Convert text into something appropriate for a var.
# for instance:
#   PROG = my-app-name
#   $(call makevar,$(PROG))_LIBS = ...
# would result in:
#   my_app_name_LIBS = ...
#
# @param $(1) name to convert
# @return name suitable for a make variable name
define makevar
$(subst .,_,$(subst -,_,$(1)))
endef

# Define rules for building a library or module.
#
# To setup a library for building:
# - add its base name to LIBRARIES or MODULES
# - convert name as with the makevar macro
# - add lib specific values:
#   - sources to name_SOURCES
#   - headers to name_HEADERS
#   - extra objects to name_OBJS
#   - extra libs to name_LIBS
#   - for MODULES can define name_MOD_DIR to specify output dir
#     - ie, "name_MOD_DIR = tests" would output modules/tests/libname.so
#
# Creates name__SOURCES_OBJS out of name_SOURCES list.
# Creates name__OBJS from __SOURCES_OBJS and _OBJS
# Creates libname__LIBS from _LIBS
# Sets up dynamic lib dependencies on __OBJS and __LIBS
# Adds library to LIBRARIES
#
# @param $(1) library name
# @param $(2) library var name
# @param $(3) library output filename
# @return make rules for the library
define _LIBRARY_template
SOURCES += $$($(2)_SOURCES)
HEADERS += $$($(2)_HEADERS)
$(2)__SOURCES_OBJS += $$(patsubst %.cpp,$(BUILD_DIR)/%-$(PLATFORM).o,$$($(2)_SOURCES))
$(2)__OBJS += $$($(2)__SOURCES_OBJS) $$($(2)_OBJS)
$(2)__DEPS += $$($(2)__OBJS)
$(3): $$($(2)__DEPS)
$(call makevar,$(LIB_PREFIX)$(1).$(DYNAMIC_LIB_EXT)__LIBS) = $$($(2)_LIBS)
OUTPUT_LIBRARIES += $(3)
endef

# Build rules for LIBRARIES
define LIBRARY_template
$(call _LIBRARY_template,$(lib),$(call makevar,$(1)),$(LIB_DIR)/$(LIB_PREFIX)$(1).$(DYNAMIC_LIB_EXT))
endef
# Build rules for MODULES
define MODULE_template
$(call _LIBRARY_template,$(mod),$(call makevar,$(1)),$(MOD_DIR)/$$($(call makevar,$(1))_MOD_DIR)/$(LIB_PREFIX)$(1).$(DYNAMIC_LIB_EXT))
endef

$(foreach lib,$(LIBRARIES),$(eval $(call LIBRARY_template,$(lib))))
$(foreach mod,$(MODULES),$(eval $(call MODULE_template,$(mod))))

CPP_SOURCES := $(filter %.cpp, $(SOURCES))
AS_SOURCES := $(filter %.S, $(SOURCES))
HEADER_FILES := $(patsubst %.h, $(TOP_BUILD_DIR)/$(HEADER_DIST_DIR)/%.h, $(HEADERS))
LIBRARY_SOURCES := $(patsubst %.cpp, $(BUILD_DIR)/%.cpp, $(CPP_SOURCES)) $(patsubst %.S, $(BUILD_DIR)/%.S, $(AS_SOURCES))
EXECUTABLE_SOURCES := $(patsubst %, $(BUILD_DIR)/%.cpp, $(EXECUTABLES))
LIBRARY_OBJECTS := $(patsubst %.cpp, $(BUILD_DIR)/%-$(PLATFORM).o, $(CPP_SOURCES)) $(patsubst %.S, $(BUILD_DIR)/%-asm-$(PLATFORM).o, $(AS_SOURCES))
EXECUTABLE_OBJECTS := $(patsubst %, $(BUILD_DIR)/%-$(PLATFORM).o, $(EXECUTABLES))
DYNAMIC_LIBRARIES := $(DYNAMIC_LINK_LIBRARIES) $(DYNAMIC_LIBRARIES)
EXECUTABLES := $(patsubst %, $(BIN_DIR)/%$(EXECUTABLE_EXT), $(EXECUTABLES))
DEPENDENCIES := $(patsubst %.cpp, $(BUILD_DIR)/%-$(PLATFORM).P, $(CPP_SOURCES))
DEPENDENCIES += $(patsubst %.cpp, %-$(PLATFORM).P, $(EXECUTABLE_SOURCES))

ALL_HEADERS += $(HEADER_FILES)
ALL_SOURCES += $(LIBRARY_SOURCES) $(EXECUTABLE_SOURCES)
ALL_OBJECTS += $(LIBRARY_OBJECTS) $(EXECUTABLE_OBJECTS)
ALL_LIBRARIES += $(OUTPUT_LIBRARIES)
ALL_EXECUTABLES += $(EXECUTABLES)
ALL_DIRECTORIES += $(BUILD_DIR) $(DIST_DIR) $(LIB_DIR) $(BIN_DIR)

# Ignore dependencies if specified in the Makefile. This will ensure that the
# dependencies for all .cpp files are not pulled in and thus won't be built
ifndef IGNORE_DEPENDENCIES
-include $(DEPENDENCIES)
endif

# Whether or not to print commands as they are being executed, helpful for
# debugging the build system.
ifdef PRINT_COMMANDS
PCMD=
else
PCMD=@
endif

$(HEADER_FILES): 
	$(PCMD) mkdir -p $(dir $@)
	$(PCMD) ln -sf $(subst $(TOP_BUILD_DIR)/$(HEADER_DIST_DIR)/,$(CWD)/,$@) $@
$(LIBRARY_SOURCES):
	$(PCMD) mkdir -p $(dir $@)
	$(PCMD) ln -sf $(subst $(BUILD_DIR)/,$(CWD)/,$@) $@
$(EXECUTABLE_SOURCES):
	$(PCMD) mkdir -p $(dir $@)
	$(PCMD) ln -sf $(subst $(BUILD_DIR)/,$(CWD)/,$@) $@

clean-objects:
	$(PCMD) rm -rf $(LIBRARY_OBJECTS) $(EXECUTABLE_OBJECTS)

clean-libraries:
	$(PCMD) rm -rf $(OUTPUT_LIBRARIES)

%-asm-$(PLATFORM).o: %.S
	@echo "Compiling build/$(subst .P,.o,$(subst $(TOP_BUILD_DIR)/build/,,$@))..."
	$(PCMD) $(CXX) $(CXX_FLAGS) -c -MD -o $(basename $@).o $(INCLUDES) $(realpath $<)
	$(PCMD) cp $(basename $@).d $(basename $@).P; \
		sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $(basename $@).d >> $(basename $@).P; \
		rm -f $(basename $@).d

ifndef OVERRIDE_OP_BUILDRULE
%-$(PLATFORM).o %-$(PLATFORM).P: %.cpp
	@echo "Compiling build/$(subst .P,.o,$(subst $(TOP_BUILD_DIR)/build/,,$@))..."
	$(PCMD) $(CXX) $(CXX_FLAGS) -c -MD -o $(basename $@).o $(INCLUDES) $(realpath $<)
	$(PCMD) cp $(basename $@).d $(basename $@).P; \
		sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $(basename $@).d >> $(basename $@).P; \
		rm -f $(basename $@).d
endif

ifeq ($(BUILD_FOR_MACOS),yes)

# Special LD_FLAGS for Mac OS X linking
#
# @param $(1) library base name (via $(@F))
# @return extra CC dynamic lib linking options
define extra_ld_flags
-dynamiclib -install_name @loader_path/$(1)
endef

endif

%.$(DYNAMIC_LIB_EXT):
	@echo "Linking $(subst $(TOP_BUILD_DIR)/,,$@)..."
	$(PCMD) mkdir -p $(@D)
	$(PCMD) $(CXX) $(LD_FLAGS) $(LIB_LD_FLAGS) $(LIBS) -shared $(call extra_ld_flags,$(@F)) -o $@ $^ $(DYNAMIC_LIBRARIES:%=-l%) $($(call makevar,$(@F)__LIBS):%=-l%)

%.$(STATIC_LIB_EXT): $(LIBRARY_OBJECTS)
	@echo "Creating archive $(subst $(TOP_BUILD_DIR)/,,$@)..."
	$(PCMD) $(AR) $(AR_FLAGS) $@ $^

ifdef EXECUTABLES
$(EXECUTABLES): $(BUILD_DIR)/$$(basename $$(@F))-$(PLATFORM).o $(STATIC_EXECUTABLE_LIBRARIES) $$(foreach source,$$($$(subst -,_,$$(@F))_EXTRA_SOURCES),$(BUILD_DIR)/$$(source)-$(PLATFORM).o)
	@echo "Linking dist/bin/$(@F)..."
	$(PCMD) mkdir -p $(@D)
	$(PCMD) $(CXX) $(CXX_FLAGS) $(LD_FLAGS) $(LIBS) \
		-o $@ $^ \
		$(DYNAMIC_LIBRARIES:%=-l%) \
		$(DYNAMIC_EXECUTABLE_LIBRARIES:%=-l%)
endif

dump-variables:
	@echo BUILD_FOR_LINUX=$(BUILD_FOR_LINUX)
	@echo BUILD_FOR_WINDOWS=$(BUILD_FOR_WINDOWS)
	@echo BUILD_FOR_MACOS=$(BUILD_FOR_MACOS)
	@echo TOP_SRC_DIR=$(TOP_SRC_DIR)
	@echo TOP_BUILD_DIR=$(TOP_BUILD_DIR)
	@echo CWD=$(CWD)

	@echo CXX=$(CXX)
	@echo CXX_FLAGS=$(CXX_FLAGS)
	@echo INCLUDES=$(INCLUDES)
	@echo LIB_DIR=$(LIB_DIR)
	@echo LIBS=$(LIBS)
	@echo AR=$(AR)
	@echo AR_FLAGS=$(AR_FLAGS)
	@echo AS=$(AS)
	@echo BUILD_DIR=$(BUILD_DIR)
	@echo DIST_DIR=$(DIST_DIR)
	@echo DYNAMIC_LIBRARIES=$(DYNAMIC_LIBRARIES)
	@echo LIBRARY_SOURCES=$(LIBRARY_SOURCES)
	@echo EXECUTABLE_SOURCES=$(EXECUTABLE_SOURCES)
	@echo LIBRARY_OBJECTS=$(LIBRARY_OBJECTS)
	@echo EXECUTABLE_OBJECTS=$(EXECUTABLE_OBJECTS)
	@echo EXECUTABLES=$(EXECUTABLES)
	@echo DEPENDENCIES=$(DEPENDENCIES)
	@echo LIBRARIES=$(LIBRARIES)
	@echo OUTPUT_LIBRARIES=$(OUTPUT_LIBRARIES)

	@echo ALL_SOURCES=$(ALL_SOURCES)
	@echo ALL_OBJECTS=$(ALL_OBJECTS)
	@echo ALL_LIBRARIES=$(ALL_LIBRARIES)
	@echo ALL_EXECUTABLES=$(ALL_EXECUTABLES)
