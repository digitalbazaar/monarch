# This file contains Makefile rules for building Monarch libraries, modules,
# and executables.
#
# The following should be defined before including this file:
#
# TOP_SRC_DIR
# TOP_BUILD_DIR (optional, defaults top TOP_SRC_DIR)
# CXX
# MO_CXX_FLAGS
# MO_LD_FLAGS
# INCLUDES
# MO_LIBS
# AR
# AS
# PLATFORM_NAME
# LIB_PREFIX
# DYNAMIC_LIB_EXT
# STATIC_LIB_EXT
# EXECUTABLE_EXT

TOP_BUILD_DIR ?= $(TOP_SRC_DIR)
MO_BUILD_DIR ?= $(TOP_BUILD_DIR)/build/$(subst $(TOP_BUILD_DIR)/cpp/,,$(CWD))
MO_DIST_DIR ?= $(TOP_BUILD_DIR)/dist
MO_LIB_DIR ?= $(MO_DIST_DIR)/lib
MO_MOD_DIR ?= $(MO_DIST_DIR)/modules
MO_BIN_DIR ?= $(MO_DIST_DIR)/bin

# Convert text into something appropriate for a var.
# for instance:
#   PROG = my-app-name
#   $(call mo_makevar,$(PROG))_LIBS = ...
# would result in:
#   my_app_name_LIBS = ...
#
# @param $(1) name to convert
# @return name suitable for a make variable name
define mo_makevar
$(subst .,_,$(subst -,_,$(1)))
endef

# Define rules for building a library or module.
#
# To setup a library for building:
# - add its base name to LIBRARIES or MODULES
# - convert name as with the mo_makevar macro
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
# Adds library to MO_LIBRARIES
#
# @param $(1) library name
# @param $(2) library var name
# @param $(3) library output filename
# @return make rules for the library
define _MO_LIBRARY_template
SOURCES += $$($(2)_SOURCES)
HEADERS += $$($(2)_HEADERS)
$(2)__SOURCES_OBJS += $$(patsubst %.cpp,$(MO_BUILD_DIR)/%-$(PLATFORM).o,$$($(2)_SOURCES))
$(2)__OBJS += $$($(2)__SOURCES_OBJS) $$($(2)_OBJS)
$(2)__DEPS += $$($(2)__OBJS)
$(3): $$($(2)__DEPS)
$(call mo_makevar,$(LIB_PREFIX)$(1).$(DYNAMIC_LIB_EXT)__LIBS) = $$($(2)_LIBS)
MO_LIBRARIES += $(3)
endef

# Build rules for LIBRARIES
define MO_LIBRARY_template
$(call _MO_LIBRARY_template,$(lib),$(call mo_makevar,$(1)),$(MO_LIB_DIR)/$(LIB_PREFIX)$(1).$(DYNAMIC_LIB_EXT))
endef
# Build rules for MODULES
define MO_MODULE_template
$(call _MO_LIBRARY_template,$(mod),$(call mo_makevar,$(1)),$(MO_MOD_DIR)/$$($(call mo_makevar,$(1))_MOD_DIR)/$(LIB_PREFIX)$(1).$(DYNAMIC_LIB_EXT))
endef

$(foreach lib,$(LIBRARIES),$(eval $(call MO_LIBRARY_template,$(lib))))
$(foreach mod,$(MODULES),$(eval $(call MO_MODULE_template,$(mod))))

MO_CPP_SOURCES := $(filter %.cpp, $(SOURCES))
MO_AS_SOURCES := $(filter %.S, $(SOURCES))
MO_HEADER_FILES := $(patsubst %.h, $(TOP_BUILD_DIR)/$(HEADER_DIST_DIR)/%.h, $(HEADERS))
MO_LIBRARY_SOURCES := $(patsubst %.cpp, $(MO_BUILD_DIR)/%.cpp, $(MO_CPP_SOURCES)) $(patsubst %.S, $(MO_BUILD_DIR)/%.S, $(MO_AS_SOURCES))
MO_EXECUTABLE_SOURCES := $(patsubst %, $(MO_BUILD_DIR)/%.cpp, $(EXECUTABLES))
MO_LIBRARY_OBJECTS := $(patsubst %.cpp, $(MO_BUILD_DIR)/%-$(PLATFORM).o, $(MO_CPP_SOURCES)) $(patsubst %.S, $(MO_BUILD_DIR)/%-asm-$(PLATFORM).o, $(MO_AS_SOURCES))
MO_EXECUTABLE_OBJECTS := $(patsubst %, $(MO_BUILD_DIR)/%-$(PLATFORM).o, $(EXECUTABLES))
MO_DYNAMIC_LIBRARIES := $(DYNAMIC_LINK_LIBRARIES) $(MO_DYNAMIC_LIBRARIES)
MO_EXECUTABLES := $(patsubst %, $(MO_BIN_DIR)/%$(EXECUTABLE_EXT), $(EXECUTABLES))
MO_DEPENDENCIES := $(patsubst %.cpp, $(MO_BUILD_DIR)/%-$(PLATFORM).P, $(MO_CPP_SOURCES))
MO_DEPENDENCIES += $(patsubst %.cpp, %-$(PLATFORM).P, $(MO_EXECUTABLE_SOURCES))

ALL_HEADERS += $(MO_HEADER_FILES)
ALL_SOURCES += $(MO_LIBRARY_SOURCES) $(MO_EXECUTABLE_SOURCES)
ALL_OBJECTS += $(MO_LIBRARY_OBJECTS) $(MO_EXECUTABLE_OBJECTS)
ALL_LIBRARIES += $(MO_LIBRARIES)
ALL_EXECUTABLES += $(MO_EXECUTABLES)
ALL_DIRECTORIES += $(MO_BUILD_DIR) $(MO_DIST_DIR) $(MO_LIB_DIR) $(MO_BIN_DIR)

# Ignore dependencies if specified in the Makefile. This will ensure that the
# dependencies for all .cpp files are not pulled in and thus won't be built
ifndef IGNORE_DEPENDENCIES
-include $(MO_DEPENDENCIES)
endif

$(MO_HEADER_FILES): 
	$(PCMD) mkdir -p $(dir $@)
	$(PCMD) ln -sf $(subst $(TOP_BUILD_DIR)/$(HEADER_DIST_DIR)/,$(CWD)/,$@) $@
$(MO_LIBRARY_SOURCES):
	$(PCMD) mkdir -p $(dir $@)
	$(PCMD) ln -sf $(subst $(MO_BUILD_DIR)/,$(CWD)/,$@) $@
$(MO_EXECUTABLE_SOURCES):
	$(PCMD) mkdir -p $(dir $@)
	$(PCMD) ln -sf $(subst $(MO_BUILD_DIR)/,$(CWD)/,$@) $@

clean-objects:
	$(PCMD) rm -rf $(MO_LIBRARY_OBJECTS) $(MO_EXECUTABLE_OBJECTS)

clean-libraries:
	$(PCMD) rm -rf $(MO_LIBRARIES)

%-asm-$(PLATFORM).o: %.S
	@echo "Compiling build/$(subst .P,.o,$(subst $(TOP_BUILD_DIR)/build/,,$@))..."
	$(PCMD) $(CXX) $(MO_CXX_FLAGS) -c -MD -o $(basename $@).o $(INCLUDES) $(realpath $<)
	$(PCMD) cp $(basename $@).d $(basename $@).P; \
		sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $(basename $@).d >> $(basename $@).P; \
		rm -f $(basename $@).d

%-$(PLATFORM).o %-$(PLATFORM).P: %.cpp
	@echo "Compiling build/$(subst .P,.o,$(subst $(TOP_BUILD_DIR)/build/,,$@))..."
	$(PCMD) $(CXX) $(MO_CXX_FLAGS) -c -MD -o $(basename $@).o $(INCLUDES) $(realpath $<)
	$(PCMD) cp $(basename $@).d $(basename $@).P; \
		sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $(basename $@).d >> $(basename $@).P; \
		rm -f $(basename $@).d

ifeq ($(BUILD_FOR_MACOS),yes)

# Special LD_FLAGS for Mac OS X linking
#
# @param $(1) library base name (via $(@F))
# @return extra CC dynamic lib linking options
define mo_extra_ld_flags
-dynamiclib -install_name @loader_path/$(1)
endef

endif

%.$(DYNAMIC_LIB_EXT):
	@echo "Linking $(subst $(TOP_BUILD_DIR)/,,$@)..."
	$(PCMD) mkdir -p $(@D)
	$(PCMD) $(CXX) $(MO_LD_FLAGS) $(MO_LIB_LD_FLAGS) $(MO_LIBS) -shared $(call mo_extra_ld_flags,$(@F)) -o $@ $^ $(MO_DYNAMIC_LIBRARIES:%=-l%) $($(call mo_makevar,$(@F)__LIBS):%=-l%)

%.$(STATIC_LIB_EXT): $(MO_LIBRARY_OBJECTS)
	@echo "Creating archive $(subst $(TOP_BUILD_DIR)/,,$@)..."
	$(PCMD) $(AR) $(AR_FLAGS) $@ $^

ifdef EXECUTABLES
$(MO_EXECUTABLES): $(MO_BUILD_DIR)/$$(@F)-$(PLATFORM).o $(MO_STATIC_EXECUTABLE_LIBRARIES) $$(foreach source,$$($$(subst -,_,$$(@F))_EXTRA_SOURCES),$(MO_BUILD_DIR)/$$(source)-$(PLATFORM).o)
	@echo "Linking dist/bin/$(@F)..."
	$(PCMD) mkdir -p $(@D)
	$(PCMD) $(CXX) $(MO_CXX_FLAGS) $(MO_LD_FLAGS) $(MO_LIBS) \
		-o $@ $^ \
		$(MO_DYNAMIC_LIBRARIES:%=-l%) \
		$(MO_DYNAMIC_EXECUTABLE_LIBRARIES:%=-l%)
endif

dump-variables:
	@echo BUILD_FOR_LINUX=$(BUILD_FOR_LINUX)
	@echo BUILD_FOR_WINDOWS=$(BUILD_FOR_WINDOWS)
	@echo BUILD_FOR_MACOS=$(BUILD_FOR_MACOS)
	@echo TOP_SRC_DIR=$(TOP_SRC_DIR)
	@echo TOP_BUILD_DIR=$(TOP_BUILD_DIR)
	@echo CWD=$(CWD)

	@echo CXX=$(CXX)
	@echo MO_CXX_FLAGS=$(MO_CXX_FLAGS)
	@echo INCLUDES=$(INCLUDES)
	@echo MO_LIB_DIR=$(MO_LIB_DIR)
	@echo MO_LIBS=$(MO_LIBS)
	@echo AR=$(AR)
	@echo AR_FLAGS=$(AR_FLAGS)
	@echo AS=$(AS)
	@echo MO_BUILD_DIR=$(MO_BUILD_DIR)
	@echo MO_DIST_DIR=$(MO_DIST_DIR)
	@echo MO_DYNAMIC_LIBRARIES=$(MO_DYNAMIC_LIBRARIES)
	@echo MO_LIBRARY_SOURCES=$(MO_LIBRARY_SOURCES)
	@echo MO_EXECUTABLE_SOURCES=$(MO_EXECUTABLE_SOURCES)
	@echo MO_LIBRARY_OBJECTS=$(MO_LIBRARY_OBJECTS)
	@echo MO_EXECUTABLE_OBJECTS=$(MO_EXECUTABLE_OBJECTS)
	@echo MO_EXECUTABLES=$(MO_EXECUTABLES)
	@echo MO_DEPENDENCIES=$(MO_DEPENDENCIES)
	@echo MO_LIBRARIES=$(MO_LIBRARIES)

	@echo ALL_SOURCES=$(ALL_SOURCES)
	@echo ALL_OBJECTS=$(ALL_OBJECTS)
	@echo ALL_LIBRARIES=$(ALL_LIBRARIES)
	@echo ALL_EXECUTABLES=$(ALL_EXECUTABLES)
