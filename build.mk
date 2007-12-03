# Makefile to compile the Digital Bazaar C++ libraries

# Clean does not create a target file so this will
# make sure it always runs when typing "make clean"
.PHONY: clean

# Platform detection
ifneq ($(findstring Linux,$(shell uname)),)
UNIX = true
endif
ifneq ($(findstring CYGWIN,$(shell uname)),)
WIN32 = true
endif

# platform specific variables
ifdef UNIX
BASE_DIR ?= /work/src/dbcpp/dbcore/trunk
PICFLAGS = -fPIC
SOEXT = so
LDPATH = 
else ifdef WIN32
WINDRIVE ?= c
CYGDRIVE ?= /cygdrive/$(WINDRIVE)
BASE_DIR ?= $(WINDRIVE):/work/src/dbcpp/dbcore/trunk
OPENSSL_DIR ?= $(WINDRIVE):/work/OpenSSL
PICFLAGS =
SOEXT = a
LDPATH = $(OPENSSL_DIR:$(WINDRIVE):%=$(CYGDRIVE)%)/lib/MinGW:$(BASE_DIR:$(WINDRIVE):%=$(CYGDRIVE)%)/data/cpp/db/data/xml/expat:$(BASE_DIR:$(WINDRIVE):%=$(CYGDRIVE)%)/sql/cpp/sqlite3:$(BASE_DIR:$(WINDRIVE):%=$(CYGDRIVE)%)/sql/cpp/mysql/lib

# windows include path
INCLUDES = \
	-I$(OPENSSL_DIR)/include \
	-I$(BASE_DIR)/rt/cpp/pthread \
	-I$(BASE_DIR)/data/cpp/db/data/xml/expat \
	-I$(BASE_DIR)/sql/cpp/sqlite3 \
	-I$(BASE_DIR)/sql/cpp/mysql

# windows libs
WIN_LIBS = \
	$(BASE_DIR)/rt/cpp/pthread/libpthreadGCE2.a \
	$(OPENSSL_DIR)/lib/MinGW/ssleay32.a \
	$(OPENSSL_DIR)/lib/MinGW/libeay32.a \
	-lws2_32 \
	$(BASE_DIR)/data/cpp/db/data/xml/expat/libexpat.a \
	$(BASE_DIR)/data/cpp/db/data/xml/expat/libexpat.dll \
	$(BASE_DIR)/sql/cpp/sqlite3/sqlite3.dll \
	$(BASE_DIR)/sql/cpp/mysql/lib/libmysql.a \
	$(BASE_DIR)/sql/cpp/mysql/lib/libmysql.dll
endif

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
MODULES = \
	config \
	crypto \
	data \
	event \
	io \
	mail \
	modest \
	net \
	rt \
	sql \
	test \
	util
	#logging

# All executables
EXES = maintest


# Module specific build rules

modules_MODLIBS = rt

ifdef UNIX
modest_CFLAGS = -DMODEST_API_EXPORT
endif

util_MODLIBS = rt

io_MODLIBS = util

crypto_MODLIBS = io
ifndef WIN32
crypto_EXTRADEPS = $(BASE_DIR)/crypto/python/cppwrapper/_dbcrypto.so
endif

net_MODLIBS = modest crypto
net_SUBDIRS = http

ifndef WIN32
util_SUBDIRS = regex
endif

data_SUBDIRS = xml mpeg id3v2 json
ifndef WIN32
data_LIBS = expat
endif
data_MODLIBS = io util rt

sql_SUBDIRS = sqlite3 mysql util
ifndef WIN32
sql_LIBS = sqlite3 mysqlclient
endif

event_MODLIBS = modest

config_MODLIBS = rt util io

mail_MODLIBS = net util

test_MODLIBS = rt sql


# exe rules

maintest_DIR = test
maintest_EXE = run-unit-tests.exe
maintest_MODLIBS = rt modest util io crypto net data sql event mail test \
	config #logging
maintest_SOURCES = run-unit-tests
ifndef WIN32
maintest_LIBS = pthread crypto ssl expat sqlite3 mysqlclient
endif


#
# Generic rules

# Builds all modules and binaries
#all: $(MODULES:%=lib$(MODGROUP)%) $(EXES)
#	@echo Make all finished.
all: all2

# Cleans all build and dist files
clean:
	@echo Cleaning all DB object and library files...
	rm -rf $(BUILD) $(DIST) $(LIBS_DIR) $(CLEANFILES)
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
ifdef UNIX
$$(MODGROUP)_$(1)_LIBS = $$($(1)_LIBS:%=/usr/lib/lib%.a)
endif

$$(BASE_DIR)/$(1)/cpp/dist/lib$$(MODGROUP)$(1).a: $$($$(MODGROUP)_$(1)_OBJS)
	$$(AR) $$(ARFLAGS) $$@ $$^ $$($$(MODGROUP)_$(1)_LIBS)

ifndef WIN32
$$(BASE_DIR)/$(1)/cpp/dist/lib$$(MODGROUP)$(1).so: $$($$(MODGROUP)_$(1)_OBJS)
	$$(CC) $$(LIBS) -shared -o $$@ $$^ $$($(1)_MODLIBS:%=-l$$(MODGROUP)%) $$($(1)_LIBS:%=-l%)
endif

$$(LIBS_DIR)/lib$$(MODGROUP)$(1).$$(SOEXT): $$(BASE_DIR)/$(1)/cpp/dist/lib$$(MODGROUP)$(1).$$(SOEXT)
	@mkdir -p $$(LIBS_DIR)
	@cp $$< $$@

ifdef UNIX
SODEP = $$(BASE_DIR)/$(1)/cpp/dist/lib$$(MODGROUP)$(1).$$(SOEXT)
endif
lib$$(MODGROUP)$(1): $$($(1)_MODLIBS:%=lib$$(MODGROUP)%) \
	$$(BASE_DIR)/$(1)/cpp/dist/lib$$(MODGROUP)$(1).a \
	$$(SODEP) \
	$$(LIBS_DIR)/lib$$(MODGROUP)$(1).$$(SOEXT) \
	$$($(1)_EXTRADEPS)

# Builds object files
$$(BASE_DIR)/$(1)/cpp/build/%.o: $$(BASE_DIR)/$(1)/cpp/$$(MODGROUP)/$(1)/%.cpp $$($$(MODGROUP)_$(1)_H)
	@mkdir -p $$(BASE_DIR)/$(1)/cpp/build \
		$$($(1)_SUBDIRS:%=$$(BASE_DIR)/$(1)/cpp/build/%) \
		$$(BASE_DIR)/$(1)/cpp/dist
	$$(CC) $$(CFLAGS) $(PICFLAGS) -o $$@ -c $$< $$($(1)_CFLAGS)
endef

$(foreach mod,$(MODULES),$(eval $(call MODULE_template,$(mod))))


define EXE_template
# The DB build directories
BUILD += $$(BASE_DIR)/$$($(1)_DIR)/cpp/build

# The DB dist directories
DIST += $$(BASE_DIR)/$$($(1)_DIR)/cpp/dist

# CPP files
ALL_CPP += $$($(1)_SOURCES:%=$$(BASE_DIR)/$$($(1)_DIR)/cpp/%.cpp)

$(1)_exe: $$(BASE_DIR)/$$($(1)_DIR)/cpp/dist/$$($(1)_EXE)

CHECK_EXES += $$(BASE_DIR)/$$($(1)_DIR)/cpp/dist/$$($(1)_EXE)

# Builds the binary
ifdef WIN32
BINLIBS=$(WIN_LIBS)
else
BINLIBS=$$($(1)_LIBS:%=-l%)
endif
$$(BASE_DIR)/$$($(1)_DIR)/cpp/dist/$$($(1)_EXE): $$($(1)_SOURCES:%=$$(BASE_DIR)/$$($(1)_DIR)/cpp/build/%.o) $$($(1)_MODLIBS:%=lib$$(MODGROUP)%) $$($(1)_OBJS)
	$$(CC) $$(CFLAGS) -o $$@ $$< $$(foreach mod,$$($(1)_MODLIBS),$$($$(MODGROUP)_$$(mod)_LIB)) $$(BINLIBS)

# Builds object files
$$(BASE_DIR)/$$($(1)_DIR)/cpp/build/%.o: $$(BASE_DIR)/$$($(1)_DIR)/cpp/%.cpp $$(ALL_H)
	@mkdir -p $$(BASE_DIR)/$$($(1)_DIR)/cpp/build \
		$$(BASE_DIR)/$$($(1)_DIR)/cpp/dist
	$$(CC) $$(CFLAGS) -o $$@ -c $$< $$($(1)_CFLAGS)
endef

$(foreach exe,$(EXES),$(eval $(call EXE_template,$(exe))))


ifdef WIN32
check: all2
	@for exe in $(CHECK_EXES); do \
		PATH=$(LDPATH) $$exe; \
	done
else
check: all2
	@for exe in $(CHECK_EXES); do \
		$$exe; \
	done
endif

#
# Extra build rules

ifndef WIN32
# Build DB cryptography wrapper for python
$(BASE_DIR)/crypto/python/cppwrapper/_dbcrypto.so: $(BASE_DIR)/crypto/python/cppwrapper/dbcryptoWrapper.o $(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.o $(db_crypto_LIB) $(db_io_LIB) $(db_rt_LIB) $(db_util_LIB)
	$(CC) $(LIBS) -shared -o $@ $^ -lpthread -lcrypto -lssl

$(BASE_DIR)/crypto/python/cppwrapper/dbcryptoWrapper.o: $(BASE_DIR)/crypto/python/cppwrapper/dbcryptoWrapper.cpp
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Build DB cryptography swig wrapper for python
$(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.o: $(BASE_DIR)/crypto/python/cppwrapper/dbcrypto.i
	swig -c++ -python -o $(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.c $<
	$(CC) $(CFLAGS) $(PYTHON_INCLUDE) -fPIC -o $@ -c $(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.c

CLEANFILES += \
	$(BASE_DIR)/crypto/python/cppwrapper/dbcryptoWrapper.o \
	$(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.o \
	$(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.c \
	$(BASE_DIR)/crypto/python/cppwrapper/_dbcrypto.so \
	$(BASE_DIR)/crypto/python/cppwrapper/dbcrypto.py
endif

$(BASE_DIR)/dbcore.pc: $(BASE_DIR)/dbcore.pc.in
	cat $< | \
		sed -e 's/@@CFLAGS@@/$(subst /,\/,$(INCLUDES))/' | \
		sed -e 's/@@LIBS@@/$(subst /,\/,$(addprefix -L,$(DIST)))/' > $@

TAGS: $(ALL_H) $(ALL_CPP)
	etags $^

CLEANFILES += \
	$(BASE_DIR)/dbcore.pc \
	TAGS

all2: $(MODULES:%=lib$(MODGROUP)%) $(EXES:%=%_exe) $(BASE_DIR)/dbcore.pc
	@echo Make all finished.
