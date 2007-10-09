# Makefile to compile the Digital Bazaar C++ libraries

# Clean does not create a target file so this will
# make sure it always runs when typing "make clean"
.PHONY: clean

# base path
BASE_DIR = e:/work/dbcpp/dbcore/trunk

# windows include path
INCLUDES = \
	-IE:/OpenSSL/include \
	-I$(BASE_DIR)/rt/cpp/pthread \
	-I$(BASE_DIR)/data/cpp/db/data/xml/expat

# windows libs
WIN_LIBS = \
	$(BASE_DIR)/rt/cpp/pthread/libpthreadGCE2.a \
	E:/OpenSSL/lib/MinGW/ssleay32.a \
	E:/OpenSSL/lib/MinGW/libeay32.a \
	-lws2_32 \
	$(BASE_DIR)/data/cpp/db/data/xml/expat/libexpat.a \
	$(BASE_DIR)/data/cpp/db/data/xml/expat/libexpat.dll
	#$(BASE_DIR)/sql/cpp/sqlite/sqlite3.dll \
	

# Compiler
CC = g++

# Compiler flags:
# -g	include debug information
CFLAGS = -g -Wall $(INCLUDES)
PYTHON_INCLUDE = -I/usr/include/python2.4

# Archive builder
AR = ar
ARFLAGS = cr

# Library path
LIBS_DIR = $(BASE_DIR)/libs
LIBS = -L$(LIBS_DIR)

# Linker flags:
LDFLAGS = $(LIBS)

# H files
FIND_H = $(wildcard $(dir)/*.h)

# CPP files
FIND_CPP = $(wildcard $(dir)/*.cpp)

# Prefix for all MODULES
MODGROUP = db

# All modules
MODULES = rt modest util io crypto net data sql logging

# All executables
EXES = test

# Module specific build rules

modules_MODLIBS = rt

util_MODLIBS = rt

io_MODLIBS = util rt

crypto_MODLIBS = io util rt

net_MODLIBS = crypto io util modest rt
net_SUBDIRS = http

#util_SUBDIRS = regex

data_SUBDIRS = xml mpeg id3v2
data_MODLIBS = io util rt

#sql_SUBDIRS = sqlite3 mysql util
#sql_MODLIBS = rt

# test.exe binary rules
test_MODLIBS = rt modest util io crypto net data logging
test_SOURCES = main

#
# Generic rules

# Builds all modules and binaries
#all: $(MODULES:%=lib$(MODGROUP)%) $(EXES)
#	@echo Make all finished.
all: all2

# Cleans all build and dist files
clean:
	@echo Cleaning all DB object and library files...
	rm -rf $(BUILD) $(DIST) $(LIBS_DIR)
	@echo Make clean finished.

define MODULE_template
# Include path
INCLUDES += -I$$(BASE_DIR)/$(1)/cpp

# The DB build directories
BUILD += $$(BASE_DIR)/$(1)/cpp/build

# The DB dist directories
DIST += $$(BASE_DIR)/$(1)/cpp/dist

# Individual DB libraries as make targets
# This will need to be changed for a windows build
$$(MODGROUP)_$(1)_LIB = $$(BASE_DIR)/$(1)/cpp/dist/lib$$(MODGROUP)$(1).a

# H files
$$(MODGROUP)_$(1)_H = $$(foreach dir,$$(BASE_DIR)/$(1)/cpp/$$(MODGROUP)/$(1) $$(foreach subdir,$$($(1)_SUBDIRS),$$(BASE_DIR)/$(1)/cpp/$$(MODGROUP)/$(1)/$$(subdir)),$$(FIND_H))
ALL_H += $$($$(MODGROUP)_$(1)_H)

# CPP files
$$(MODGROUP)_$(1)_CPP = $$(foreach dir,$$(BASE_DIR)/$(1)/cpp/$$(MODGROUP)/$(1) $$(foreach subdir,$$($(1)_SUBDIRS),$$(BASE_DIR)/$(1)/cpp/$$(MODGROUP)/$(1)/$$(subdir)),$$(FIND_CPP))
ALL_CPP += $$($$(MODGROUP)_$(1)_CPP)

# Object files
$$(MODGROUP)_$(1)_OBJS = $$($$(MODGROUP)_$(1)_CPP:$$(BASE_DIR)/$(1)/cpp/$$(MODGROUP)/$(1)/%.cpp=$$(BASE_DIR)/$(1)/cpp/build/%.o)

$$(BASE_DIR)/$(1)/cpp/dist/lib$$(MODGROUP)$(1).a: $$($$(MODGROUP)_$(1)_OBJS)
	$$(AR) $$(ARFLAGS) $$@ $$^

$$(LIBS_DIR)/lib$$(MODGROUP)$(1).a: $$(BASE_DIR)/$(1)/cpp/dist/lib$$(MODGROUP)$(1).a
	@mkdir -p $$(LIBS_DIR)
	@cp $$< $$@

lib$$(MODGROUP)$(1): $$($(1)_MODLIBS:%=lib$$(MODGROUP)%) \
	$$(BASE_DIR)/$(1)/cpp/dist/lib$$(MODGROUP)$(1).a \
	$$(LIBS_DIR)/lib$$(MODGROUP)$(1).a

# Builds object files
$$(BASE_DIR)/$(1)/cpp/build/%.o: $$(BASE_DIR)/$(1)/cpp/$$(MODGROUP)/$(1)/%.cpp $$($$(MODGROUP)_$(1)_H)
	@mkdir -p $$(BASE_DIR)/$(1)/cpp/build \
		$$($(1)_SUBDIRS:%=$$(BASE_DIR)/$(1)/cpp/build/%) \
		$$(BASE_DIR)/$(1)/cpp/dist
	$$(CC) $$(CFLAGS) -o $$@ -c $$< $$($(1)_CFLAGS)
endef

$(foreach mod,$(MODULES),$(eval $(call MODULE_template,$(mod))))


define EXE_template
# The DB build directories
BUILD += $$(BASE_DIR)/$(1)/cpp/build

# The DB dist directories
DIST += $$(BASE_DIR)/$(1)/cpp/dist

# CPP files
ALL_CPP += $$($(1)_SOURCES:%=$$(BASE_DIR)/$(1)/cpp/%.cpp)

$(1)_exe: $$(BASE_DIR)/$(1)/cpp/dist/$(1).exe

# Builds the binary
$$(BASE_DIR)/$(1)/cpp/dist/$(1).exe: $$($(1)_SOURCES:%=$$(BASE_DIR)/$(1)/cpp/build/%.o) $$($(1)_MODLIBS:%=lib$$(MODGROUP)%) $$($(1)_OBJS)
	$$(CC) $$(CFLAGS) -o $$@ $$< $$(foreach mod,$$($(1)_MODLIBS),$$($$(MODGROUP)_$$(mod)_LIB)) $(WIN_LIBS)

# Builds object files
$$(BASE_DIR)/$(1)/cpp/build/%.o: $$(BASE_DIR)/$(1)/cpp/%.cpp $$(ALL_H)
	@mkdir -p $$(BASE_DIR)/$(1)/cpp/build \
		$$(BASE_DIR)/$(1)/cpp/dist
	$$(CC) $$(CFLAGS) -o $$@ -c $$< $$($(1)_CFLAGS)
endef

$(foreach exe,$(EXES),$(eval $(call EXE_template,$(exe))))

TAGS: $(ALL_H) $(ALL_CPP)
	etags $^

all2: $(MODULES:%=lib$(MODGROUP)%) $(EXES:%=%_exe)
	@echo Make all finished.

