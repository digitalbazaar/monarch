# Makefile to compile the Digital Bazaar C++ libraries

# Clean does not create a target file so this will
# make sure it always runs when typing "make clean"
.PHONY: clean

# base path
BASE_DIR = e:/work/dbcpp/dbcore/trunk

# Compiler
CC = g++

# Include path
# Include path
INCLUDES = \
	-IE:/OpenSSL/include \
	-I$(BASE_DIR)/rt/cpp/pthread \
	-I$(BASE_DIR)/rt/cpp \
	-I$(BASE_DIR)/modest/cpp \
	-I$(BASE_DIR)/util/cpp \
	-I$(BASE_DIR)/io/cpp \
	-I$(BASE_DIR)/crypto/cpp \
	-I$(BASE_DIR)/net/cpp \
	-I$(BASE_DIR)/data/cpp/db/data/xml/expat \
	-I$(BASE_DIR)/data/cpp

# Compiler flags:
# -g	include debug information
CFLAGS = -g -Wall $(INCLUDES)
PYTHON_INCLUDE = -I/usr/include/python2.4

# Archive builder
AR = ar
ARFLAGS = cr

# windows libs
PTHREAD_LIB = $(BASE_DIR)/rt/cpp/pthread/libpthreadGCE2.a
CRYPTO_LIBS = E:/OpenSSL/lib/MinGW/ssleay32.a E:/OpenSSL/lib/MinGW/libeay32.a
XML_LIBS = $(BASE_DIR)/data/cpp/db/data/xml/expat/libexpat.a $(BASE_DIR)/data/cpp/db/data/xml/expat/libexpat.dll
WIN_LIBS = \
	$(PTHREAD_LIB) \
	$(CRYPTO_LIBS) \
	-lws2_32 \
	$(XML_LIBS)

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
	$(BASE_DIR)/data/cpp/build

# The DB dist directories
DIST = \
	$(BASE_DIR)/test/cpp/dist \
	$(BASE_DIR)/rt/cpp/dist \
	$(BASE_DIR)/modest/cpp/dist \
	$(BASE_DIR)/util/cpp/dist \
	$(BASE_DIR)/io/cpp/dist \
	$(BASE_DIR)/crypto/cpp/dist \
	$(BASE_DIR)/net/cpp/dist \
	$(BASE_DIR)/data/cpp/dist

# Library path
LIBS = \
	-L$(BASE_DIR)/libs
#	-L$(BASE_DIR)/test/cpp/dist \
#	-L$(BASE_DIR)/rt/cpp/dist \
#	-L$(BASE_DIR)/modest/cpp/dist \
#	-L$(BASE_DIR)/util/cpp/dist \
#	-L$(BASE_DIR)/io/cpp/dist \
#	-L$(BASE_DIR)/crypto/cpp/dist \
#	-L$(BASE_DIR)/net/cpp/dist \
#	-L$(BASE_DIR)/data/cpp/dist

# H files
FIND_H = $(wildcard $(dir)/*.h)
DBRT_H = $(foreach dir,$(BASE_DIR)/rt/cpp/db/rt,$(FIND_H))
DBMODEST_H = $(foreach dir,$(BASE_DIR)/modest/cpp/db/modest,$(FIND_H))
DBUTIL_H = $(foreach dir,$(BASE_DIR)/util/cpp/db/util,$(FIND_H)) $(foreach dir,$(BASE_DIR)/util/cpp/db/util/regex,$(FIND_H))
DBIO_H = $(foreach dir,$(BASE_DIR)/io/cpp/db/io,$(FIND_H))
DBCRYPTO_H = $(foreach dir,$(BASE_DIR)/crypto/cpp/db/crypto,$(FIND_H))
DBNET_H = $(foreach dir,$(BASE_DIR)/net/cpp/db/net,$(FIND_H)) $(foreach dir,$(BASE_DIR)/net/cpp/db/net/http,$(FIND_H))
DBDATA_H = $(foreach dir,$(BASE_DIR)/data/cpp/db/data,$(FIND_H)) $(foreach dir,$(BASE_DIR)/data/cpp/db/data/xml,$(FIND_H))

# CPP files
FIND_CPP = $(wildcard $(dir)/*.cpp)
DBRT_CPP = $(foreach dir,$(BASE_DIR)/rt/cpp/db/rt,$(FIND_CPP))
DBMODEST_CPP = $(foreach dir,$(BASE_DIR)/modest/cpp/db/modest,$(FIND_CPP))
DBUTIL_CPP = $(foreach dir,$(BASE_DIR)/util/cpp/db/util,$(FIND_CPP)) $(foreach dir,$(BASE_DIR)/util/cpp/db/util/regex,$(FIND_CPP))
DBIO_CPP = $(foreach dir,$(BASE_DIR)/io/cpp/db/io,$(FIND_CPP))
DBCRYPTO_CPP = $(foreach dir,$(BASE_DIR)/crypto/cpp/db/crypto,$(FIND_CPP))
DBNET_CPP = $(foreach dir,$(BASE_DIR)/net/cpp/db/net,$(FIND_CPP)) $(foreach dir,$(BASE_DIR)/net/cpp/db/net/http,$(FIND_CPP))
DBDATA_CPP = $(foreach dir,$(BASE_DIR)/data/cpp/db/data,$(FIND_CPP)) $(foreach dir,$(BASE_DIR)/data/cpp/db/data/xml,$(FIND_CPP))

# Object files
DBRT_OBJS = $(DBRT_CPP:$(BASE_DIR)/rt/cpp/db/rt/%.cpp=$(BASE_DIR)/rt/cpp/build/%.o)
DBMODEST_OBJS = $(DBMODEST_CPP:$(BASE_DIR)/modest/cpp/db/modest/%.cpp=$(BASE_DIR)/modest/cpp/build/%.o)
DBUTIL_OBJS = $(DBUTIL_CPP:$(BASE_DIR)/util/cpp/db/util/%.cpp=$(BASE_DIR)/util/cpp/build/%.o)
DBIO_OBJS = $(DBIO_CPP:$(BASE_DIR)/io/cpp/db/io/%.cpp=$(BASE_DIR)/io/cpp/build/%.o)
DBCRYPTO_OBJS = $(DBCRYPTO_CPP:$(BASE_DIR)/crypto/cpp/db/crypto/%.cpp=$(BASE_DIR)/crypto/cpp/build/%.o)
DBNET_OBJS = $(DBNET_CPP:$(BASE_DIR)/net/cpp/db/net/%.cpp=$(BASE_DIR)/net/cpp/build/%.o)
DBDATA_OBJS = $(DBDATA_CPP:$(BASE_DIR)/data/cpp/db/data/%.cpp=$(BASE_DIR)/data/cpp/build/%.o)

# Individual DB libraries as make targets
# This will need to be changed for a windows build
DBRT_LIB = $(BASE_DIR)/rt/cpp/dist/libdbrt.a
DBMODEST_LIB = $(BASE_DIR)/modest/cpp/dist/libdbmodest.a
DBUTIL_LIB = $(BASE_DIR)/util/cpp/dist/libdbutil.a
DBIO_LIB = $(BASE_DIR)/io/cpp/dist/libdbio.a
DBCRYPTO_LIB = $(BASE_DIR)/crypto/cpp/dist/libdbcrypto.a
DBNET_LIB = $(BASE_DIR)/net/cpp/dist/libdbnet.a
DBDATA_LIB = $(BASE_DIR)/data/cpp/dist/libdbdata.a 

# DB executables
TEST_EXE = $(BASE_DIR)/test/cpp/dist/test.exe

# Builds all binaries
all: test
	@echo Make all finished.

# Cleans all build and dist files
clean:
	@echo Cleaning all DB object and library files...
	rm -rf $(BUILD) $(DIST) $(BASE_DIR)/libs
	@echo Make clean finished.

# Builds the DB runtime libraries
libdbrt: $(DBRT_OBJS)
	@mkdir -p $(BASE_DIR)/libs
	$(AR) $(ARFLAGS) $(BASE_DIR)/rt/cpp/dist/$@.a $^ $(PTHREAD_LIB)
	#$(CC) $(LIBS) -shared -o $(BASE_DIR)/rt/cpp/dist/$@.so $^ $(PTHREAD_LIB)
	@cp $(BASE_DIR)/rt/cpp/dist/$@.a $(BASE_DIR)/libs/

# Builds the DB modest libraries
libdbmodest: $(DBMODEST_OBJS)
	@mkdir -p $(BASE_DIR)/libs
	$(AR) $(ARFLAGS) $(BASE_DIR)/modest/cpp/dist/$@.a $^
	#$(CC) $(LIBS) -shared -o $(BASE_DIR)/modest/cpp/dist/$@.so $^ -ldbrt $(PTHREAD_LIB)
	@cp $(BASE_DIR)/modest/cpp/dist/$@.a $(BASE_DIR)/libs/

# Builds the DB utilities libraries
libdbutil: $(DBUTIL_OBJS)
	@mkdir -p $(BASE_DIR)/libs
	$(AR) $(ARFLAGS) $(BASE_DIR)/util/cpp/dist/$@.a $^
	#$(CC) $(LIBS) -shared -o $(BASE_DIR)/util/cpp/dist/$@.so $^ -ldbrt $(PTHREAD_LIB)
	@cp $(BASE_DIR)/util/cpp/dist/$@.a $(BASE_DIR)/libs/

# Builds the DB io libraries
libdbio: $(DBIO_OBJS)
	@mkdir -p $(BASE_DIR)/libs
	echo DBIO_CPP
	$(AR) $(ARFLAGS) $(BASE_DIR)/io/cpp/dist/$@.a $^
	#$(CC) $(LIBS) -shared -o $(BASE_DIR)/io/cpp/dist/$@.so $^ -ldbrt -ldbutil $(PTHREAD_LIB)
	@cp $(BASE_DIR)/io/cpp/dist/$@.a $(BASE_DIR)/libs/

# Builds the DB crypto libraries and wrappers
#libdbcrypto: $(DBCRYPTO_OBJS)
#	@mkdir -p $(BASE_DIR)/libs
#	$(AR) $(ARFLAGS) $(BASE_DIR)/crypto/cpp/dist/$@.a $^
#	$(CC) $(LIBS) -shared -o $(BASE_DIR)/crypto/cpp/dist/$@.so $^ -ldbio
#	@cp $(BASE_DIR)/crypto/cpp/dist/$@.so $(BASE_DIR)/libs/

# Builds the DB crypto libraries
libdbcrypto: $(DBCRYPTO_OBJS)
	@mkdir -p $(BASE_DIR)/libs
	$(AR) $(ARFLAGS) $(BASE_DIR)/crypto/cpp/dist/$@.a $(DBCRYPTO_OBJS)
	#$(CC) $(LIBS) -shared -o $(BASE_DIR)/crypto/cpp/dist/$@.so $(DBCRYPTO_OBJS) -ldbrt -ldbutil -ldbio $(WIN_LIBS)
	@cp $(BASE_DIR)/crypto/cpp/dist/$@.a $(BASE_DIR)/libs/

# Builds the DB net libraries
libdbnet: $(DBNET_OBJS)
	@mkdir -p $(BASE_DIR)/libs
	$(AR) $(ARFLAGS) $(BASE_DIR)/net/cpp/dist/$@.a $^ $(CRYPTO_LIBS)
	#$(CC) $(LIBS) -shared -o $(BASE_DIR)/net/cpp/dist/$@.so $^ -ldbrt -ldbmodest -ldbutil -ldbio -ldbcrypto $(WIN_LIBS)
	@cp $(BASE_DIR)/net/cpp/dist/$@.a $(BASE_DIR)/libs/

# Builds the DB data libraries
libdbdata: $(DBDATA_OBJS)
	@mkdir -p $(BASE_DIR)/libs
	$(AR) $(ARFLAGS) $(BASE_DIR)/data/cpp/dist/$@.a $^  $(XML_LIBS)
	#$(CC) $(LIBS) -shared -o $(BASE_DIR)/data/cpp/dist/$@.so $^ -ldbrt -ldbmodest -ldbutil -ldbio -ldbcrypto -ldbnet $(WIN_LIBS)
	@cp $(BASE_DIR)/data/cpp/dist/$@.a $(BASE_DIR)/libs/

# Builds the DB test.exe binary
test: libdbrt libdbmodest libdbutil libdbio libdbcrypto libdbnet libdbdata $(BASE_DIR)/test/cpp/build/main.o
	$(CC) $(CFLAGS) -o $(TEST_EXE) $(BASE_DIR)/test/cpp/build/main.o $(DBRT_LIB) $(DBMODEST_LIB) $(DBUTIL_LIB) $(DBIO_LIB) $(DBCRYPTO_LIB) $(DBNET_LIB) $(DBDATA_LIB) $(WIN_LIBS)

# Builds DB runtime object files
$(BASE_DIR)/rt/cpp/build/%.o: $(BASE_DIR)/rt/cpp/db/rt/%.cpp
	@mkdir -p $(BASE_DIR)/rt/cpp/build
	@mkdir -p $(BASE_DIR)/rt/cpp/dist
	$(CC) $(CFLAGS) -o $@ -c $^

# Builds DB modest object files
$(BASE_DIR)/modest/cpp/build/%.o: $(BASE_DIR)/modest/cpp/db/modest/%.cpp
	@mkdir -p $(BASE_DIR)/modest/cpp/build
	@mkdir -p $(BASE_DIR)/modest/cpp/dist
	$(CC) $(CFLAGS) -o $@ -c $^ -DMODEST_API_EXPORT

# Builds DB utility object files
$(BASE_DIR)/util/cpp/build/%.o: $(BASE_DIR)/util/cpp/db/util/%.cpp
	@mkdir -p $(BASE_DIR)/util/cpp/build/regex
	@mkdir -p $(BASE_DIR)/util/cpp/dist
	$(CC) $(CFLAGS) -o $@ -c $^

# Builds DB io object files
$(BASE_DIR)/io/cpp/build/%.o: $(BASE_DIR)/io/cpp/db/io/%.cpp
	@mkdir -p $(BASE_DIR)/io/cpp/build
	@mkdir -p $(BASE_DIR)/io/cpp/dist
	$(CC) $(CFLAGS) -o $@ -c $^

# Builds DB cryptography object files
$(BASE_DIR)/crypto/cpp/build/%.o: $(BASE_DIR)/crypto/cpp/db/crypto/%.cpp
	@mkdir -p $(BASE_DIR)/crypto/cpp/build
	@mkdir -p $(BASE_DIR)/crypto/cpp/dist
	$(CC) $(CFLAGS) -o $@ -c $^

# Build DB cryptography wrapper for python
$(BASE_DIR)/crypto/python/cppwrapper/dbcryptoWrapper.o: $(BASE_DIR)/crypto/python/cppwrapper/dbcryptoWrapper.cpp
	$(CC) $(CFLAGS) -o $@ -c $^

# Build DB cryptography swig wrapper for python
$(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.o:
	swig -c++ -python -o $(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.c $(BASE_DIR)/crypto/python/cppwrapper/dbcrypto.i
	$(CC) $(CFLAGS) $(PYTHON_INCLUDE) -o $(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.o -c $(BASE_DIR)/crypto/python/cppwrapper/dbcrypto_wrapper.c

# Builds DB net object files
$(BASE_DIR)/net/cpp/build/%.o: $(BASE_DIR)/net/cpp/db/net/%.cpp
	@mkdir -p $(BASE_DIR)/net/cpp/build/http
	@mkdir -p $(BASE_DIR)/net/cpp/dist
	$(CC) $(CFLAGS) -o $@ -c $^

# Builds DB data object files
$(BASE_DIR)/data/cpp/build/%.o: $(BASE_DIR)/data/cpp/db/data/%.cpp
	@mkdir -p $(BASE_DIR)/data/cpp/build/xml
	@mkdir -p $(BASE_DIR)/data/cpp/dist
	$(CC) $(CFLAGS) -o $@ -c $^

# Builds Test object file
$(BASE_DIR)/test/cpp/build/main.o: $(BASE_DIR)/test/cpp/main.cpp
	@mkdir -p $(BASE_DIR)/test/cpp/build
	@mkdir -p $(BASE_DIR)/test/cpp/dist
	$(CC) $(CFLAGS) -o $@ -c $^
