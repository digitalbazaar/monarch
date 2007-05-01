/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Socket.h"

#ifdef WIN32
   // windows socket library
   #include <winsock.h>
#else
   // data types
   #include <sys/types.h>
   // linux socket library
   #include <sys/socket.h>
   // closes file descriptors with close()
   #include <unistd.h>
#endif

using namespace db::io;
using namespace db::net;

Socket::Socket() : mInputStream(this), mOutputStream(this)
{
   // file descriptor is invalid at this point
   mFileDescriptor = -1;
}

Socket::~Socket()
{
   // close socket
   close();
}

void Socket::create(int type, int protocol) throw (SocketException)
{
   int fd = socket(PF_INET, type, protocol);
   if(fd < 0)
   {
      throw SocketException("Could not create Socket!");
   }
   
   mFileDescriptor = fd;
}

//void Socket::bind(SocketAddress* address) throw(SocketException)
//{
//}
//
//void Socket::accept(Socket* socket, unsigned int timeout)
//throw(SocketException, SocketTimeoutException)
//{
//}
//
//void Socket::connect(SocketAddress* address, unsigned int timeout)
//throw(SocketException)
//{
//}

void Socket::close()
{
#ifdef WIN32
   closesocket(mFileDescriptor);
#else
   ::close(mFileDescriptor);
#endif

   // file descriptor is invalid again
   mFileDescriptor = -1;
}

SocketInputStream& Socket::getInputStream()
{
   return mInputStream;
}

SocketOutputStream& Socket::getOutputStream()
{
   return mOutputStream;
}

int Socket::receive(char* b, int offset, int length) throw(IOException)
{
   int rval = -1;
   
   // FIXME: determine how socket recv blocks/what signals closed socket, etc
   
#ifdef WIN32
   rval = ::recv(mFileDescriptor, b + offset, length, 0);
#else
   rval = ::recv(mFileDescriptor, (void*)(b + offset), length, MSG_NOSIGNAL);
#endif
   
   if(rval < -1)
   {
      throw IOException("Could not read from Socket!");
   }
   
   return rval;
}

void Socket::send(char* b, int offset, int length) throw(IOException)
{
#ifdef WIN32
   int rc = ::send(mFileDescriptor, b + offset, length, 0);
#else
   int rc = ::send(mFileDescriptor, (void*)(b + offset), length, MSG_NOSIGNAL);
#endif
   
   if(rc < 0)
   {
      throw IOException("Could not write to Socket!");
   }
}
