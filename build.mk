# Makefile to compile the Digital Bazaar C++ libraries

# Clean does not create a target file so this will
# make sure it always runs when typing "make clean"
.PHONY: clean

# base path
BASE_DIR = /work/src/dbcpp/dbcore/trunk

# Compiler
CC = g++

# Include path
# Include path
INCLUDES = \
	-I$(BASE_DIR)/rt/cpp \
	-I$(BASE_DIR)/modest/cpp \
	-I$(BASE_DIR)/util/cpp \
	-I$(BASE_DIR)/io/cpp \
	-I$(BASE_DIR)/crypto/cpp \
	-I$(BASE_DIR)/net/cpp \
	-I$(BASE_DIR)/data/cpp \
	-I$(BASE_DIR)/database/cpp

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

# The DB build directories
BUILD = \
	$(BASE_DIR)/test/cpp/build \
        $(BASE_DIR)/rt/cpp/build \
        $(BASE_DIR)/modest/cpp/build \
        $(BASE_DIR)/util/cpp/build \
        $(BASE_DIR)/io/cpp/build \
        $(BASE_DIR)/crypto/cpp/build \
        $(BASE_DIR)/net/cpp/build \
        $(BASE_DIR)/data/cpp/build \
        $(BASE_DIR)/database/cpp/build

# The DB dist directories
DIST = \
	$(BASE_DIR)/test/cpp/dist \
        $(BASE_DIR)/rt/cpp/dist \
        $(BASE_DIR)/modest/cpp/dist \
        $(BASE_DIR)/util/cpp/dist \
        $(BASE_DIR)/io/cpp/dist \
        $(BASE_DIR)/crypto/cpp/dist \
        $(BASE_DIR)/net/cpp/dist \
        $(BASE_DIR)/data/cpp/dist \
        $(BASE_DIR)/database/cpp/dist

# H files
FIND_H = $(wildcard $(dir)/*.h)
DBRT_H = $(foreach dir,$(BASE_DIR)/rt/cpp/db/rt,$(FIND_H))
DBMODEST_H = $(foreach dir,$(BASE_DIR)/modest/cpp/db/modest,$(FIND_H))
DBUTIL_H = $(foreach dir,$(BASE_DIR)/util/cpp/db/util,$(FIND_H)) $(foreach dir,$(BASE_DIR)/util/cpp/db/util/regex,$(FIND_H))
DBIO_H = $(foreach dir,$(BASE_DIR)/io/cpp/db/io,$(FIND_H))
DBCRYPTO_H = $(foreach dir,$(BASE_DIR)/crypto/cpp/db/crypto,$(FIND_H))
DBNET_H = $(foreach dir,$(BASE_DIR)/net/cpp/db/net,$(FIND_H)) $(foreach dir,$(BASE_DIR)/net/cpp/db/net/http,$(FIND_H))
DBDATA_H = $(foreach dir,$(BASE_DIR)/data/cpp/db/data,$(FIND_H)) $(foreach dir,$(BASE_DIR)/data/cpp/db/data/xml,$(FIND_H))
DBDATABASE_H = $(foreach dir,$(BASE_DIR)/database/cpp/db/database,$(FIND_H))

# CPP files
FIND_CPP = $(wildcard $(dir)/*.cpp)
DBRT_CPP = $(foreach dir,$(BASE_DIR)/rt/cpp/db/rt,$(FIND_CPP))
DBMODEST_CPP = $(foreach dir,$(BASE_DIR)/modest/cpp/db/modest,$(FIND_CPP))
DBUTIL_CPP = $(foreach dir,$(BASE_DIR)/util/cpp/db/util,$(FIND_CPP)) $(foreach dir,$(BASE_DIR)/util/cpp/db/util/regex,$(FIND_CPP))
DBIO_CPP = $(foreach dir,$(BASE_DIR)/io/cpp/db/io,$(FIND_CPP))
DBCRYPTO_CPP = $(foreach dir,$(BASE_DIR)/crypto/cpp/db/crypto,$(FIND_CPP))
DBNET_CPP = $(foreach dir,$(BASE_DIR)/net/cpp/db/net,$(FIND_CPP)) $(foreach dir,$(BASE_DIR)/net/cpp/db/net/http,$(FIND_CPP))
DBDATA_CPP = $(foreach dir,$(BASE_DIR)/data/cpp/db/data,$(FIND_CPP)) $(foreach dir,$(BASE_DIR)/data/cpp/db/data/xml,$(FIND_CPP))
DBDATABASE_CPP = $(foreach dir,$(BASE_DIR)/database/cpp/db/database,$(FIND_CPP))

# Object files
DBRT_OBJS = $(DBRT_CPP:$(BASE_DIR)/rt/cpp/db/rt/%.cpp=$(BASE_DIR)/rt/cpp/build/%.o)
DBMODEST_OBJS = $(DBMODEST_CPP:$(BASE_DIR)/modest/cpp/db/modest/%.cpp=$(BASE_DIR)/modest/cpp/build/%.o)
DBUTIL_OBJS = $(DBUTIL_CPP:$(BASE_DIR)/util/cpp/db/util/%.cpp=$(BASE_DIR)/util/cpp/build/%.o)
DBIO_OBJS = $(DBIO_CPP:$(BASE_DIR)/io/cpp/db/io/%.cpp=$(BASE_DIR)/io/cpp/build/%.o)
DBCRYPTO_OBJS = $(DBCRYPTO_CPP:$(BASE_DIR)/crypto/cpp/db/crypto/%.cpp=$(BASE_DIR)/crypto/cpp/build/%.o)
DBNET_OBJS = $(DBNET_CPP:$(BASE_DIR)/net/cpp/db/net/%.cpp=$(BASE_DIR)/net/cpp/build/%.o)
DBDATA_OBJS = $(DBDATA_CPP:$(BASE_DIR)/data/cpp/db/data/%.cpp=$(BASE_DIR)/data/cpp/build/%.o)
DBDATABASE_OBJS = $(DBDATABASE_CPP:$(BASE_DIR)/database/cpp/db/database/%.cpp=$(BASE_DIR)/database/cpp/build/%.o)

# Individual DB libraries as make targets
# This will need to be changed for a windows build
DBRT_LIB = $(BASE_DIR)/rt/cpp/dist/libdbrt.a
DBMODEST_LIB = $(BASE_DIR)/modest/cpp/dist/libdbmodest.a
DBUTIL_LIB = $(BASE_DIR)/util/cpp/dist/libdbutil.a
DBIO_LIB = $(BASE_DIR)/io/cpp/dist/libdbio.a
DBCRYPTO_LIB = $(BASE_DIR)/crypto/cpp/dist/libdbcrypto.a
DBNET_LIB = $(BASE_DIR)/net/cpp/dist/libdbnet.a
DBDATA_LIB = $(BASE_DIR)/data/cpp/dist/libdbdata.a
DBDATABASE_LIB = $(BASE_DIR)/database/cpp/dist/libdbdatabase.a

# DB executables
TEST_EXE = $(BASE_DIR)/test/cpp/dist/test.exe

# Builds all binaries
all: test
	@echo Make all finished.

# Cleans all build and dist files
clean:
	@echo Cleaning all DB object and library files...
	rm -rf $(BUILD) $(DIST) $(LIBS_DIR) \
		$(BASE_DIR)/crypto/python/cppwrapper/dbcryptoWrapper.o \
		$(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.o \
		$(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.c \
		$(BASE_DIR)/crypto/python/cppwrapper/_dbcrypto.so \
		$(BASE_DIR)/crypto/python/cppwrapper/dbcrypto.py

	@echo Make clean finished.

# Builds the DB runtime libraries
$(BASE_DIR)/rt/cpp/dist/libdbrt.a: $(DBRT_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(BASE_DIR)/rt/cpp/dist/libdbrt.so: $(DBRT_OBJS)
	@mkdir -p $(LIBS_DIR)
	$(CC) $(LIBS) -shared -o $@ $^
	@cp $@ $(LIBS_DIR)

libdbrt: $(BASE_DIR)/rt/cpp/dist/libdbrt.a $(BASE_DIR)/rt/cpp/dist/libdbrt.so


# Builds the DB modest libraries
$(BASE_DIR)/modest/cpp/dist/libdbmodest.a: $(DBMODEST_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(BASE_DIR)/modest/cpp/dist/libdbmodest.so: $(DBMODEST_OBJS)
	@mkdir -p $(LIBS_DIR)
	$(CC) $(LIBS) -shared -o $@ $^ -ldbrt
	@cp $@ $(LIBS_DIR)

libdbmodest: libdbrt $(BASE_DIR)/modest/cpp/dist/libdbmodest.a $(BASE_DIR)/modest/cpp/dist/libdbmodest.so


# Builds the DB utilities libraries
$(BASE_DIR)/util/cpp/dist/libdbutil.a: $(DBUTIL_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(BASE_DIR)/util/cpp/dist/libdbutil.so: $(DBUTIL_OBJS)
	@mkdir -p $(LIBS_DIR)
	$(CC) $(LIBS) -shared -o $@ $^ -ldbrt
	@cp $@ $(LIBS_DIR)

libdbutil: libdbrt $(BASE_DIR)/util/cpp/dist/libdbutil.a $(BASE_DIR)/util/cpp/dist/libdbutil.so


# Builds the DB io libraries
$(BASE_DIR)/io/cpp/dist/libdbio.a: $(DBIO_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(BASE_DIR)/io/cpp/dist/libdbio.so: $(DBIO_OBJS)
	@mkdir -p $(LIBS_DIR)
	$(CC) $(LIBS) -shared -o $@ $^ -ldbutil
	@cp $@ $(LIBS_DIR)

libdbio: libdbutil $(BASE_DIR)/io/cpp/dist/libdbio.a $(BASE_DIR)/io/cpp/dist/libdbio.so


# Builds the DB crypto libraries and wrappers
#libdbcrypto: $(DBCRYPTO_OBJS)
#	@mkdir -p $(LIBS_DIR)
#	$(AR) $(ARFLAGS) $(BASE_DIR)/crypto/cpp/dist/$@.a $^
#	$(CC) $(LIBS) -shared -o $(BASE_DIR)/crypto/cpp/dist/$@.so $^ -ldbio
#	@cp $(BASE_DIR)/crypto/cpp/dist/$@.so $(LIBS_DIR)

# Builds the DB crypto libraries and wrappers
$(BASE_DIR)/crypto/cpp/dist/libdbcrypto.a: $(DBCRYPTO_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(BASE_DIR)/crypto/cpp/dist/libdbcrypto.so: $(DBCRYPTO_OBJS)
	@mkdir -p $(LIBS_DIR)
	$(CC) $(LIBS) -shared -o $@ $^ -ldbio
	@cp $@ $(LIBS_DIR)

$(BASE_DIR)/crypto/python/cppwrapper/_dbcrypto.so: $(BASE_DIR)/crypto/python/cppwrapper/dbcryptoWrapper.o $(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.o $(DBCRYPTO_LIB) $(DBIO_LIB) $(DBRT_LIB) $(DBUTIL_LIB)
	$(CC) $(LIBS) -shared -o $@ $^ -lpthread -lcrypto -lssl

libdbcrypto: libdbio $(BASE_DIR)/crypto/cpp/dist/libdbcrypto.a $(BASE_DIR)/crypto/cpp/dist/libdbcrypto.so $(BASE_DIR)/crypto/python/cppwrapper/_dbcrypto.so


# Builds the DB net libraries
$(BASE_DIR)/net/cpp/dist/libdbnet.a: $(DBNET_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(BASE_DIR)/net/cpp/dist/libdbnet.so: $(DBNET_OBJS)
	@mkdir -p $(LIBS_DIR)
	$(CC) $(LIBS) -shared -o $@ $^ -ldbmodest -ldbcrypto
	@cp $@ $(LIBS_DIR)

libdbnet: libdbmodest libdbcrypto $(BASE_DIR)/net/cpp/dist/libdbnet.a $(BASE_DIR)/net/cpp/dist/libdbnet.so


# Builds the DB data libraries
$(BASE_DIR)/data/cpp/dist/libdbdata.a: $(DBDATA_OBJS)
	$(AR) $(ARFLAGS) $@ $^ /usr/lib/libexpat.a

$(BASE_DIR)/data/cpp/dist/libdbdata.so: $(DBDATA_OBJS)
	@mkdir -p $(LIBS_DIR)
	$(CC) $(LIBS) -shared -o $@ $^ -lexpat
	@cp $@ $(LIBS_DIR)

libdbdata: $(BASE_DIR)/data/cpp/dist/libdbdata.a $(BASE_DIR)/data/cpp/dist/libdbdata.so


# Builds the DB database libraries
$(BASE_DIR)/database/cpp/dist/libdbdatabase.a: $(DBDATABASE_OBJS)
	$(AR) $(ARFLAGS) $@ $^ /usr/lib/libsqlite3.a

$(BASE_DIR)/database/cpp/dist/libdbdatabase.so: $(DBDATABASE_OBJS)
	@mkdir -p $(LIBS_DIR)
	$(CC) $(LIBS) -shared -o $@ $^ -lsqlite3
	@cp $@ $(LIBS_DIR)

libdbdatabase: $(BASE_DIR)/database/cpp/dist/libdbdatabase.a $(BASE_DIR)/database/cpp/dist/libdbdatabase.so


# Builds the DB test.exe binary
test: libdbrt libdbmodest libdbutil libdbio libdbcrypto libdbnet libdbdata $(BASE_DIR)/test/cpp/build/main.o
	$(CC) $(CFLAGS) -o $(TEST_EXE) $(BASE_DIR)/test/cpp/build/main.o $(DBRT_LIB) $(DBMODEST_LIB) $(DBUTIL_LIB) $(DBIO_LIB) $(DBCRYPTO_LIB) $(DBNET_LIB) $(DBDATA_LIB) -lpthread -lcrypto -lssl -lexpat

#	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TEST_EXE) #test/cpp/build/main.o -ldbrt -ldbmodest -ldbutil -ldbio -ldbcrypto -ldbnet -ldbdata -lcrypto -lssl -lpthread

# Builds DB runtime object files
$(BASE_DIR)/rt/cpp/build/%.o: $(BASE_DIR)/rt/cpp/db/rt/%.cpp
	@mkdir -p $(BASE_DIR)/rt/cpp/build
	@mkdir -p $(BASE_DIR)/rt/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Builds DB modest object files
$(BASE_DIR)/modest/cpp/build/%.o: $(BASE_DIR)/modest/cpp/db/modest/%.cpp
	@mkdir -p $(BASE_DIR)/modest/cpp/build
	@mkdir -p $(BASE_DIR)/modest/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^ -DMODEST_API_EXPORT

# Builds DB utility object files
$(BASE_DIR)/util/cpp/build/%.o: $(BASE_DIR)/util/cpp/db/util/%.cpp
	@mkdir -p $(BASE_DIR)/util/cpp/build/regex
	@mkdir -p $(BASE_DIR)/util/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Builds DB io object files
$(BASE_DIR)/io/cpp/build/%.o: $(BASE_DIR)/io/cpp/db/io/%.cpp
	@mkdir -p $(BASE_DIR)/io/cpp/build
	@mkdir -p $(BASE_DIR)/io/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Builds DB cryptography object files
$(BASE_DIR)/crypto/cpp/build/%.o: $(BASE_DIR)/crypto/cpp/db/crypto/%.cpp
	@mkdir -p $(BASE_DIR)/crypto/cpp/build
	@mkdir -p $(BASE_DIR)/crypto/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Build DB cryptography wrapper for python
$(BASE_DIR)/crypto/python/cppwrapper/dbcryptoWrapper.o: $(BASE_DIR)/crypto/python/cppwrapper/dbcryptoWrapper.cpp
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Build DB cryptography swig wrapper for python
$(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.o: $(BASE_DIR)/crypto/python/cppwrapper/dbcrypto.i
	swig -c++ -python -o $(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.c $<
	$(CC) $(CFLAGS) $(PYTHON_INCLUDE) -fPIC -o $@ -c $(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.c

# Builds DB net object files
$(BASE_DIR)/net/cpp/build/%.o: $(BASE_DIR)/net/cpp/db/net/%.cpp
	@mkdir -p $(BASE_DIR)/net/cpp/build/http
	@mkdir -p $(BASE_DIR)/net/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Builds DB data object files
$(BASE_DIR)/data/cpp/build/%.o: $(BASE_DIR)/data/cpp/db/data/%.cpp
	@mkdir -p $(BASE_DIR)/data/cpp/build/xml
	@mkdir -p $(BASE_DIR)/data/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Builds DB database object files
$(BASE_DIR)/database/cpp/build/%.o: $(BASE_DIR)/database/cpp/db/database/%.cpp
	@mkdir -p $(BASE_DIR)/database/cpp/build/sqlite3
	@mkdir -p $(BASE_DIR)/database/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Builds Test object file
$(BASE_DIR)/test/cpp/build/main.o: $(BASE_DIR)/test/cpp/main.cpp
	@mkdir -p $(BASE_DIR)/test/cpp/build
	@mkdir -p $(BASE_DIR)/test/cpp/dist
	$(CC) $(CFLAGS) -o $@ -c $^
