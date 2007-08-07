# Makefile to compile the Digital Bazaar C++ libraries

# Clean does not create a target file so this will
# make sure it always runs when typing "make clean"
.PHONY: clean

# Compiler
CC = g++

# Include path
#INCLUDES = -Irt/cpp -Imodest/cpp -Iutil/cpp -Iio/cpp -Icrypto/cpp -Icrypto/python/cppwrapper -Inet/cpp -Ixml/cpp
INCLUDES = -Irt/cpp -Imodest/cpp -Iutil/cpp -Iio/cpp -Icrypto/cpp -Inet/cpp -Ixml/cpp

# Compiler flags:
# -g	include debug information
CFLAGS = -g $(INCLUDES)
#PYTHON_INCLUDE = -I/usr/include/python2.4

# Archive builder
AR = ar
ARFLAGS = cr

# Library path
LIBS = -Llibs

# Linker flags:
LDFLAGS = $(LIBS)

# The DB build directories
BUILD = \
	test/cpp/build \
        rt/cpp/build \
        modest/cpp/build \
        util/cpp/build \
        io/cpp/build \
        crypto/cpp/build \
        net/cpp/build \
        xml/cpp/build

# The DB dist directories
DIST = \
	test/cpp/dist \
        rt/cpp/dist \
        modest/cpp/dist \
        util/cpp/dist \
        io/cpp/dist \
        crypto/cpp/dist \
        net/cpp/dist \
        xml/cpp/dist

# H files
FIND_H = $(wildcard $(dir)/*.h)
DBRT_H = $(foreach dir,rt/cpp,$(FIND_H))
DBMODEST_H = $(foreach dir,modest/cpp,$(FIND_H))
DBUTIL_H = $(foreach dir,util/cpp,$(FIND_H)) $(foreach dir,util/cpp/regex,$(FIND_H))
DBIO_H = $(foreach dir,io/cpp,$(FIND_H))
DBCRYPTO_H = $(foreach dir,crypto/cpp,$(FIND_H))
DBNET_H = $(foreach dir,net/cpp,$(FIND_H)) $(foreach dir,net/cpp/http,$(FIND_H))
DBXML_H = $(foreach dir,xml/cpp,$(FIND_H))

# CPP files
FIND_CPP = $(wildcard $(dir)/*.cpp)
DBRT_CPP = $(foreach dir,rt/cpp,$(FIND_CPP))
DBMODEST_CPP = $(foreach dir,modest/cpp,$(FIND_CPP))
DBUTIL_CPP = $(foreach dir,util/cpp,$(FIND_CPP)) $(foreach dir,util/cpp/regex,$(FIND_CPP))
DBIO_CPP = $(foreach dir,io/cpp,$(FIND_CPP))
DBCRYPTO_CPP = $(foreach dir,crypto/cpp,$(FIND_CPP))
DBNET_CPP = $(foreach dir,net/cpp,$(FIND_CPP)) $(foreach dir,net/cpp/http,$(FIND_CPP))
DBXML_CPP = $(foreach dir,xml/cpp,$(FIND_CPP))

# Object files
DBRT_OBJS = $(DBRT_CPP:rt/cpp/%.cpp=rt/cpp/build/%.o)
DBMODEST_OBJS = $(DBMODEST_CPP:modest/cpp/%.cpp=modest/cpp/build/%.o)
DBUTIL_OBJS = $(DBUTIL_CPP:util/cpp/%.cpp=util/cpp/build/%.o)
DBIO_OBJS = $(DBIO_CPP:io/cpp/%.cpp=io/cpp/build/%.o)
DBCRYPTO_OBJS = $(DBCRYPTO_CPP:crypto/cpp/%.cpp=crypto/cpp/build/%.o)
DBNET_OBJS = $(DBNET_CPP:net/cpp/%.cpp=net/cpp/build/%.o)
DBXML_OBJS = $(DBXML_CPP:xml/cpp/%.cpp=xml/cpp/build/%.o)

# Individual DB libraries as make targets
# This will need to be changed for a windows build
DBRT_LIB = rt/cpp/dist/libdbrt.a
DBMODEST_LIB = modest/cpp/dist/libdbmodest.a
DBUTIL_LIB = util/cpp/dist/libdbutil.a
DBIO_LIB = io/cpp/dist/libdbio.a
DBCRYPTO_LIB = crypto/cpp/dist/libdbcrypto.a
DBNET_LIB = net/cpp/dist/libdbnet.a
DBXML_LIB = xml/cpp/dist/libdbxml.a

# DB executables
TEST_EXE = test/cpp/dist/test.exe

# Builds all binaries
all: test
	@echo Make all finished.

# Cleans all build and dist files
clean:
	@echo Cleaning all DB object and library files...
	rm -rf $(BUILD) $(DIST) libs
	@echo Make clean finished.

# Builds the DB runtime libraries
libdbrt: $(DBRT_OBJS)
	@mkdir -p libs
	$(AR) $(ARFLAGS) rt/cpp/dist/$@.a $^
	$(CC) $(LIBS) -shared -o rt/cpp/dist/$@.so $^
	@cp rt/cpp/dist/$@.so libs/

# Builds the DB modest libraries
libdbmodest: $(DBMODEST_OBJS)
	@mkdir -p libs
	$(AR) $(ARFLAGS) modest/cpp/dist/$@.a $^
	$(CC) $(LIBS) -shared -o modest/cpp/dist/$@.so $^ -ldbrt
	@cp modest/cpp/dist/$@.so libs/

# Builds the DB utilities libraries
libdbutil: $(DBUTIL_OBJS)
	@mkdir -p libs
	$(AR) $(ARFLAGS) util/cpp/dist/$@.a $^
	$(CC) $(LIBS) -shared -o util/cpp/dist/$@.so $^ -ldbrt
	@cp util/cpp/dist/$@.so libs/

# Builds the DB io libraries
libdbio: $(DBIO_OBJS)
	@mkdir -p libs
	echo DBIO_CPP
	$(AR) $(ARFLAGS) io/cpp/dist/$@.a $^
	$(CC) $(LIBS) -shared -o io/cpp/dist/$@.so $^ -ldbutil
	@cp io/cpp/dist/$@.so libs/

# Builds the DB crypto libraries and wrappers
libdbcrypto: $(DBCRYPTO_OBJS)
	@mkdir -p libs
	$(AR) $(ARFLAGS) crypto/cpp/dist/$@.a $^
	$(CC) $(LIBS) -shared -o crypto/cpp/dist/$@.so $^ -ldbio
	@cp crypto/cpp/dist/$@.so libs/

# Builds the DB crypto libraries and wrappers
#libdbcrypto: $(DBCRYPTO_OBJS) crypto/python/cppwrapper/dbcryptoWrapper.o crypto/python/cppwrapper/dbcrypto_wrapper.o
#	$(AR) $(ARFLAGS) crypto/cpp/dist/$@.a $^
#	$(CC) -shared -o crypto/cpp/dist/$@.so $^
#	$(CC) -shared crypto/python/cppwrapper/dbcrypto_wrapper.o $(DBRT_LIB) $(DBIO_LIB) $(DBCRYPTO_LIB) #$(DBUTIL_LIB) -lpthread -lcrypto -lssl -o crypto/python/cppwrapper/_dbcrypto.so

# Builds the DB net libraries
libdbnet: $(DBNET_OBJS)
	@mkdir -p libs
	$(AR) $(ARFLAGS) net/cpp/dist/$@.a $^
	$(CC) $(LIBS) -shared -o net/cpp/dist/$@.so $^ -ldbmodest -ldbcrypto
	@cp net/cpp/dist/$@.so libs/

# Builds the DB xml libraries
libdbxml: $(DBXML_OBJS)
	@mkdir -p libs
	$(AR) $(ARFLAGS) xml/cpp/dist/$@.a $^
	$(CC) $(LIBS) -shared -o xml/cpp/dist/$@.so $^
	@cp xml/cpp/dist/$@.so libs/

# Builds the DB test.exe binary
test: libdbrt libdbmodest libdbutil libdbio libdbcrypto libdbnet test/cpp/build/main.o
	$(CC) $(CFLAGS) -o $(TEST_EXE) test/cpp/build/main.o rt/cpp/dist/libdbrt.a modest/cpp/dist/libdbmodest.a util/cpp/dist/libdbutil.a io/cpp/dist/libdbio.a crypto/cpp/dist/libdbcrypto.a net/cpp/dist/libdbnet.a -lpthread -lcrypto -lssl

#	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TEST_EXE) #test/cpp/build/main.o -ldbrt -ldbmodest -ldbutil -ldbio -ldbcrypto -ldbnet -lcrypto -lssl -lpthread

# Builds DB runtime object files
rt/cpp/build/%.o: rt/cpp/%.cpp
	@mkdir -p rt/cpp/build
	@mkdir -p rt/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Builds DB modest object files
modest/cpp/build/%.o: modest/cpp/%.cpp
	@mkdir -p modest/cpp/build
	@mkdir -p modest/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^ -DMODEST_API_EXPORT

# Builds DB utility object files
util/cpp/build/%.o: util/cpp/%.cpp
	@mkdir -p util/cpp/build/regex
	@mkdir -p util/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Builds DB io object files
io/cpp/build/%.o: io/cpp/%.cpp
	@mkdir -p io/cpp/build
	@mkdir -p io/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Builds DB cryptography object files
crypto/cpp/build/%.o: crypto/cpp/%.cpp
	@mkdir -p crypto/cpp/build
	@mkdir -p crypto/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Build DB cryptography wrapper for python
#crypto/python/cppwrapper/dbcryptoWrapper.o: crypto/python/cppwrapper/dbcryptoWrapper.cpp
#	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Build DB cryptography swig wrapper for python
#crypto/python/cppwrapper/dbcrypto_wrapper.o:
#	swig -c++ -python -o crypto/python/cppwrapper/dbcrypto_wrapper.c crypto/python/cppwrapper/dbcrypto.i
#	$(CC) $(CFLAGS) $(PYTHON_INCLUDE) -fPIC -o crypto/python/cppwrapper/dbcrypto_wrapper.o -c #crypto/python/cppwrapper/dbcrypto_wrapper.c

# Builds DB net object files
net/cpp/build/%.o: net/cpp/%.cpp
	@mkdir -p net/cpp/build/http
	@mkdir -p net/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Builds DB xml object files
xml/cpp/build/%.o: xml/cpp/%.cpp
	@mkdir -p xml/cpp/build
	@mkdir -p xml/cpp/dist
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

# Builds Test object file
test/cpp/build/main.o: test/cpp/main.cpp
	@mkdir -p test/cpp/build
	@mkdir -p test/cpp/dist
	$(CC) $(CFLAGS) -o $@ -c $^
