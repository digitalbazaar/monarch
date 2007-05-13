/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "AbstractSocket.h"
#include "SocketDefinitions.h"
#include "InterruptedException.h"
#include "PeekInputStream.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "Thread.h"

using namespace db::io;
using namespace db::net;
using namespace db::rt;

AbstractSocket::AbstractSocket()
{
   // file descriptor is invalid at this point
   mFileDescriptor = -1;
   
   // not bound, listening, or connected
   mBound = false;
   mListening = false;
   mConnected = false;
   
   // create input and output streams
   mInputStream = new PeekInputStream(new SocketInputStream(this), true);
   mOutputStream = new SocketOutputStream(this);
   
   // no receive or send timeouts (socket will block)
   mReceiveTimeout = 0;
   mSendTimeout = 0;
   
   // default backlog is 50
   mBacklog = 50;
}

AbstractSocket::~AbstractSocket()
{
   // close socket
   close();
   
   // destruct input and output streams
   delete mInputStream;
   delete mOutputStream;
}

void AbstractSocket::populateAddressStructure(
   SocketAddress* address, sockaddr_in& addr)
{
   // the address family is internet (AF_INET = address family internet)
   addr.sin_family = AF_INET;
   
   // htons = "Host To Network Short" which means order the short in
   // network byte order (big-endian)
   addr.sin_port = htons(address->getPort());
   
   // converts an address to network byte order
   inet_aton(address->getAddress().c_str(), &addr.sin_addr);
   
   // zero-out the rest of the address structure
   memset(&addr.sin_zero, '\0', 8);
}

void AbstractSocket::create(int type, int protocol) throw(SocketException)
{
   // use PF_INET = "protocol family internet" (which just so happens to have
   // the same value as AF_INET but that's only because different protocols
   // were never used with the same address family
   int fd = socket(PF_INET, type, protocol);
   if(fd < 0)
   {
      throw SocketException("Could not create Socket!", strerror(errno));
   }
   
   // set reuse address flag
   // disables "address already in use" errors by reclaiming ports that
   // are waiting to be cleaned up
   int reuse = 1;
   int error = setsockopt(
      fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
   if(error < 0)
   {
      // close socket
      close();
      
      // throw exception
      throw SocketException("Could not create Socket!", strerror(errno));
   }
   
   mFileDescriptor = fd;
}

void AbstractSocket::select(bool read, unsigned long long timeout)
throw(SocketException)
{
   // get the current thread
   Thread* thread = Thread::currentThread();
   if(thread != NULL && thread->isInterrupted())
   {
      if(read)
      {
         throw InterruptedException("Socket read interrupted!");
      }
      else
      {
         throw InterruptedException("Socket write interrupted!");
      }
   }
   
   // create a file descriptor set to select on
   fd_set fds;
   FD_ZERO(&fds);
   
   // add file descriptor to set
   FD_SET((unsigned int)mFileDescriptor, &fds);
   
   // "n" parameter is the highest numbered descriptor plus 1
   int n = mFileDescriptor + 1;
   
   // create timeout
   struct timeval* tv = NULL;
   struct timeval to;
   if(timeout > 0)
   {
      // set timeout (1 millisecond is 1000 microseconds) 
      to.tv_sec = timeout / 1000LL;
      to.tv_usec = (timeout % 1000LL) * 1000LL;
      tv = &to;
   }
   
   int error;
   if(read)
   {
      // wait for data to arrive for reading or for an exception
      error = ::select(n, &fds, NULL, &fds, tv);
   }
   else
   {
      // wait for writability or for an exception
      error = ::select(n, NULL, &fds, &fds, tv);
   }
   
   if(error < 0)
   {
      if(errno == EINTR)
      {
         if(read)
         {
            // throw interrupted exception
            throw InterruptedException(
               "Socket read interrupted!", strerror(errno));
         }
         else
         {
            // throw interrupted exception
            throw InterruptedException(
               "Socket write interrupted!", strerror(errno));
         }
      }
      
      if(read)
      {
         // error occurred, get string message
         throw SocketException("Could not read from Socket!", strerror(errno));
      }
      else
      {
         // error occurred, get string message
         throw SocketException("Could not write to Socket!", strerror(errno));
      }
   }
   else if(error == 0)
   {
      if(read)
      {
         // read timeout occurred
         throw SocketTimeoutException(
            "Socket read timed out!", strerror(errno));
      }
      else
      {
         // write timeout occurred
         throw SocketTimeoutException(
            "Socket write timed out!", strerror(errno));
      }
   }
}

void AbstractSocket::bind(SocketAddress* address) throw(SocketException)
{
   // initialize as necessary
   initialize();
   
   // create sockaddr_in (internet socket address) structure
   struct sockaddr_in addr;
   
   // populate address structure
   populateAddressStructure(address, addr);
   
   // bind
   int error = ::bind(
      mFileDescriptor, (struct sockaddr*)&addr, sizeof(addr));
   if(error < 0)
   {
      throw new SocketException("Could not bind Socket!", strerror(errno));
   }
   
   // now bound
   mBound = true;
}

void AbstractSocket::bind(unsigned short port)
{
   SocketAddress address("0.0.0.0", port);
   bind(&address);
}

void AbstractSocket::listen(unsigned int backlog) throw(SocketException)
{
   if(!isBound())
   {
      // throw exception
      throw SocketException("Cannot listen on unbound Socket!"); 
   }
   
   // set backlog
   mBacklog = backlog;
   
   // listen
   int error = ::listen(mFileDescriptor, backlog);
   if(error < 0)
   {
      throw SocketException("Could not listen on Socket!", strerror(errno));
   }
   
   // now listening
   mListening = true;
}

Socket* AbstractSocket::accept(unsigned int timeout) throw(SocketException)
{
   if(!isListening())
   {
      throw SocketException("Cannot accept with a non-listening Socket!");
   }
   
   // create address object
   struct sockaddr_in addr;
   socklen_t addrSize = sizeof(addr);
   
   // wait for a connection
   select(true, timeout * 1000LL);
   
   // accept a connection
   int fd = ::accept(mFileDescriptor, (sockaddr*)&addr, &addrSize);
   if(fd < 0)
   {
      throw SocketException("Could not accept connection!", strerror(errno));
   }
   
   // create a connected Socket
   return createConnectedSocket(fd);
}

void AbstractSocket::connect(SocketAddress* address, unsigned int timeout)
throw(SocketException)
{
   // initialize as necessary
   initialize();
   
   // create sockaddr_in (internet socket address) structure
   struct sockaddr_in addr;
   
   // populate address structure
   populateAddressStructure(address, addr);
   
   // make socket non-blocking temporarily
   fcntl(mFileDescriptor, F_SETFL, O_NONBLOCK);
   
   // connect
   int error = ::connect(
      mFileDescriptor, (struct sockaddr*)&addr, sizeof(addr));
   if(error < 0)
   {
      try
      {
         // wait until the connection can be written to
         select(false, timeout * 1000LL);
      }
      catch(SocketTimeoutException &e)
      {
         // restore socket to blocking
         fcntl(mFileDescriptor, F_SETFL, 0);
         
         // throw exception
         throw SocketTimeoutException(
            "Socket connection timed out!", e.getCode());
      }
      
      // get the last error on the socket
      int lastError;
      socklen_t lastErrorLength = sizeof(lastError);
      getsockopt(
         mFileDescriptor, SOL_SOCKET, SO_ERROR,
         (char*)&lastError, &lastErrorLength);
      if(lastError != 0)
      {
         // restore socket to blocking
         fcntl(mFileDescriptor, F_SETFL, 0);
         
         // throw exception
         throw SocketException(
            "Could not connect Socket! Connection refused.",
            strerror(lastError));
      }
   }
   
   // restore socket to blocking
   fcntl(mFileDescriptor, F_SETFL, 0);
   
   // now connected and bound
   mBound = true;
   mConnected = true;
}

void AbstractSocket::send(const char* b, unsigned int length) throw(IOException)
{
   if(!isConnected())
   {
      throw SocketException("Cannot write to unconnected Socket!");
   }
   
   // send all data (send can fail to send all bytes in one go because the
   // socket send buffer was full)
   unsigned int offset = 0;
   while(length > 0)
   {
      // wait for socket to become writable
      select(false, getSendTimeout());
      
      // send some data
      int bytes = ::send(mFileDescriptor, b + offset, length, 0);
      if(bytes < 0)
      {
         throw SocketException("Could not write to Socket!", strerror(errno));
      }
      else if(bytes > 0)
      {
         offset += bytes;
         length -= bytes;
      }
   }
}

int AbstractSocket::receive(char* b, unsigned int length) throw(IOException)
{
   int rval = -1;
   
   if(!isConnected())
   {
      throw SocketException("Cannot read from unconnected Socket!");
   }
   
   // wait for data to become available
   select(true, getReceiveTimeout());
   
   // receive some data
   rval = ::recv(mFileDescriptor, b, length, 0);
   if(rval < -1)
   {
      throw SocketException("Could not read from Socket!", strerror(errno));
   }
   else if(rval == 0)
   {
      // socket is closed now
      rval = -1;
   }
   
   return rval;
}

void AbstractSocket::close()
{
   if(mFileDescriptor != -1)
   {
      // close the socket
      ::close(mFileDescriptor);
      
      // file descriptor is invalid again
      mFileDescriptor = -1;
      
      // not bound, listening, or connected
      mBound = false;
      mListening = false;
      mConnected = false;
   }
}

bool AbstractSocket::isBound()
{
   return mBound;
}

bool AbstractSocket::isListening()
{
   return mListening;
}

bool AbstractSocket::isConnected()
{
   return mConnected;
}

void AbstractSocket::getLocalAddress(SocketAddress* address)
throw(SocketException)
{
   if(!isBound())
   {
      throw SocketException("Cannot get local address for an unbound Socket!");
   }
   
   sockaddr_in addr;
   socklen_t addrSize = sizeof(addr);
   
   int error = getsockname(mFileDescriptor, (sockaddr*)&addr, &addrSize);
   if(error < 0)
   {
      throw SocketException(
         "Could not get Socket local address!", strerror(errno));
   }
   
   error = getsockname(mFileDescriptor, (sockaddr*)&addr, &addrSize);
   if(error < 0)
   {
      throw SocketException(
         "Could not get Socket local port!", strerror(errno));
   }
   
   // set address and port
   address->setAddress(inet_ntoa(addr.sin_addr));
   address->setPort(addr.sin_port);
}

void AbstractSocket::getRemoteAddress(SocketAddress* address)
throw(SocketException)
{
   if(!isConnected())
   {
      throw SocketException(
         "Cannot get local address for an unconnected Socket!");
   }
   
   sockaddr_in addr;
   socklen_t addrSize = sizeof(addr);
   
   int error = getpeername(mFileDescriptor, (sockaddr*)&addr, &addrSize);
   if(error < 0)
   {
      throw SocketException(
         "Could not get Socket remote address!", strerror(errno));
   }
   
   error = getpeername(mFileDescriptor, (sockaddr*)&addr, &addrSize);
   if(error < 0)
   {
      throw SocketException(
         "Could not get Socket remote port!", strerror(errno));
   }
   
   // set address and port
   address->setAddress(inet_ntoa(addr.sin_addr));
   address->setPort(addr.sin_port);
}

InputStream* AbstractSocket::getInputStream()
{
   return mInputStream;
}

OutputStream* AbstractSocket::getOutputStream()
{
   return mOutputStream;
}

void AbstractSocket::setSendTimeout(unsigned long long timeout)
{
   mSendTimeout = timeout;
}

unsigned long long AbstractSocket::getSendTimeout()
{
   return mSendTimeout;
}

void AbstractSocket::setReceiveTimeout(unsigned long long timeout)
{
   mReceiveTimeout = timeout;
}

unsigned long long AbstractSocket::getReceiveTimeout()
{
   return mReceiveTimeout;
}

unsigned int AbstractSocket::getBacklog()
{
   return mBacklog;
}

int AbstractSocket::getFileDescriptor()
{
   return mFileDescriptor;
}
