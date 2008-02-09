/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/AbstractSocket.h"
#include "db/net/SocketTools.h"
#include "db/net/SocketDefinitions.h"
#include "db/rt/InterruptedException.h"
#include "db/io/PeekInputStream.h"
#include "db/net/SocketInputStream.h"
#include "db/net/SocketOutputStream.h"
#include "db/rt/Thread.h"

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
   
   // input/output uninitialized
   mInputStream = NULL;
   mOutputStream = NULL;
   
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
}

bool AbstractSocket::create(int domain, int type, int protocol)
{
   bool rval = false;
   
   int fd = socket(domain, type, protocol);
   if(fd >= 0)
   {
      // set reuse address flag
      // disables "address already in use" errors by reclaiming ports that
      // are waiting to be cleaned up
      int reuse = 1;
      int error = setsockopt(
         fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
      if(error < 0)
      {
         // close socket
         close();
         
         ExceptionRef e = new SocketException(
            "Could not create Socket!", strerror(errno));
         Exception::setLast(e, false);
      }
      else
      {
         mFileDescriptor = fd;
         rval = true;
      }
   }
   else
   {
      ExceptionRef e = new SocketException(
         "Could not create Socket!", strerror(errno));
      Exception::setLast(e, false);
   }
   
   return rval;
}

bool AbstractSocket::select(bool read, long long timeout)
{
   Exception* e = NULL;
   
   // wait for readability/writability
   int error = SocketTools::select(
      read, (unsigned int)mFileDescriptor, timeout);
   if(error < 0)
   {
      if(errno == EINTR)
      {
         if(read)
         {
            // interrupted exception
            e = new InterruptedException(
               "Socket read interrupted!", strerror(errno));
         }
         else
         {
            // interrupted exception
            e = new InterruptedException(
               "Socket write interrupted!", strerror(errno));
         }
      }
      else
      {
         if(read)
         {
            // error occurred, get string message
            e = new SocketException(
               "Could not read from Socket!", strerror(errno));
         }
         else
         {
            // error occurred, get string message
            e = new SocketException(
               "Could not write to Socket!", strerror(errno));
         }
      }
   }
   else if(error == 0)
   {
      if(read)
      {
         // read timeout occurred
         e = new SocketTimeoutException(
            "Socket read timed out!", strerror(errno));
      }
      else
      {
         // write timeout occurred
         e = new SocketTimeoutException(
            "Socket write timed out!", strerror(errno));
      }
   }
   else
   {
      // get the last error on the socket
      int lastError;
      socklen_t lastErrorLength = sizeof(lastError);
      getsockopt(
         mFileDescriptor, SOL_SOCKET, SO_ERROR,
         (char*)&lastError, &lastErrorLength);
      if(lastError != 0)
      {
         if(read)
         {
            // error occurred, get string message
            e = new SocketException(
               "Could not read from Socket!", strerror(errno));
         }
         else
         {
            // error occurred, get string message
            e = new SocketException(
               "Could not write to Socket!", strerror(errno));
         }
      }
   }
   
   if(e != NULL)
   {
      ExceptionRef ref = e;
      Exception::setLast(ref, false);
   }
   
   return e == NULL;
}

bool AbstractSocket::initializeInput()
{
   if(mInputStream == NULL)
   {
      // create input stream
      mInputStream = new PeekInputStream(new SocketInputStream(this), true);
   }
   
   return true;
}

bool AbstractSocket::initializeOutput()
{
   if(mOutputStream == NULL)
   {
      // create output stream
      mOutputStream = new SocketOutputStream(this);
   }
   
   return true;
}

bool AbstractSocket::shutdownInput()
{
   // delete input stream
   if(mInputStream != NULL)
   {
      delete mInputStream;
      mInputStream = NULL;
   }
   
   return true;
}

bool AbstractSocket::shutdownOutput()
{
   // delete output stream
   if(mOutputStream != NULL)
   {
      delete mOutputStream;
      mOutputStream = NULL;
   }
   
   return true;
}

bool AbstractSocket::bind(SocketAddress* address)
{
   // acquire file descriptor
   if(acquireFileDescriptor(address->getProtocol()))
   {
      // populate address structure
      unsigned int size = 130;
      char addr[size];
      address->toSockAddr((sockaddr*)&addr, size);
      
      // bind
      int error = ::bind(mFileDescriptor, (sockaddr*)&addr, size);
      if(error < 0)
      {
         // shutdown input/output
         shutdownInput();
         shutdownOutput();
         
         ExceptionRef e = new SocketException(
            "Could not bind Socket!", strerror(errno));
         Exception::setLast(e, false);
      }
      else
      {
         // initialize input and output
         initializeInput();
         initializeOutput();
         
         // now bound
         mBound = true;
      }
   }
   
   return mBound;
}

bool AbstractSocket::listen(unsigned int backlog)
{
   if(!isBound())
   {
      ExceptionRef e = new SocketException(
         "Cannot listen on unbound Socket!");
      Exception::setLast(e, false);
   }
   else
   {
      // set backlog
      mBacklog = backlog;
      
      // listen
      int error = ::listen(mFileDescriptor, backlog);
      if(error < 0)
      {
         ExceptionRef e = new SocketException(
            "Could not listen on Socket!", strerror(errno));
         Exception::setLast(e, false);
      }
      else
      {
         // now listening
         mListening = true;
      }
   }
   
   return mListening;
}

Socket* AbstractSocket::accept(unsigned int timeout)
{
   Socket* rval = NULL;
   
   if(!isListening())
   {
      ExceptionRef e = new SocketException(
         "Cannot accept with a non-listening Socket!");
      Exception::setLast(e, false);
   }
   else
   {
      // wait for a connection
      if(select(true, timeout * 1000LL))
      {
         // accept a connection
         int fd = ::accept(mFileDescriptor, NULL, NULL);
         if(fd < 0)
         {
            ExceptionRef e = new SocketException(
               "Could not accept connection!", strerror(errno));
            Exception::setLast(e, false);
         }
         else
         {
            // create a connected Socket
            rval = createConnectedSocket(fd);
         }
      }
   }
   
   return rval;
}

bool AbstractSocket::connect(SocketAddress* address, unsigned int timeout)
{
   // acquire file descriptor
   if(acquireFileDescriptor(address->getProtocol()))
   {
      // populate address structure
      unsigned int size = 130;
      char addr[size];
      address->toSockAddr((sockaddr*)&addr, size);
      
      // make socket non-blocking temporarily
      fcntl(mFileDescriptor, F_SETFL, O_NONBLOCK);
      
      // connect
      int error = ::connect(mFileDescriptor, (sockaddr*)addr, size);
      if(error < 0)
      {
         // wait until the connection can be written to
         if(select(false, timeout * 1000LL))
         {
            // now connected and bound
            mBound = true;
            mConnected = true;
         }
         else
         {
            // shutdown input/output
            shutdownInput();
            shutdownOutput();
         }
      }
      else
      {
         // now connected and bound
         mBound = true;
         mConnected = true;
      }
      
      // restore socket to blocking
      fcntl(mFileDescriptor, F_SETFL, 0);
      
      if(mConnected)
      {
         // initialize input and output
         initializeInput();
         initializeOutput();
      }
   }
   
   return mConnected;
}

bool AbstractSocket::send(const char* b, int length)
{
   bool rval = true;
   
   if(!isBound())
   {
      ExceptionRef e = new SocketException("Cannot write to unbound Socket!");
      Exception::setLast(e, false);
   }
   else
   {
      // send all data (send can fail to send all bytes in one go because the
      // socket send buffer was full)
      unsigned int offset = 0;
      while(rval && length > 0)
      {
         // wait for socket to become writable
         if((rval = select(false, getSendTimeout())))
         {
            // send some data
            int bytes = ::send(mFileDescriptor, b + offset, length, 0);
            if(bytes < 0)
            {
               ExceptionRef e = new SocketException(
                  "Could not write to Socket!", strerror(errno));
               Exception::setLast(e, false);
               rval = false;
            }
            else if(bytes > 0)
            {
               offset += bytes;
               length -= bytes;
            }
         }
      }
   }
   
   return rval;
}

int AbstractSocket::receive(char* b, int length)
{
   int rval = -1;
   
   if(!isBound())
   {
      ExceptionRef e = new SocketException("Cannot read from unbound Socket!");
      Exception::setLast(e, false);
   }
   else
   {
      // wait for data to become available
      if(select(true, getReceiveTimeout()))
      {
         // receive some data
         rval = ::recv(mFileDescriptor, b, length, 0);
         if(rval < -1)
         {
            rval = -1;
            ExceptionRef e = new SocketException(
               "Could not read from Socket!", strerror(errno));
            Exception::setLast(e, false);
         }
      }
   }
   
   return rval;
}

void AbstractSocket::close()
{
   if(mFileDescriptor != -1)
   {
      // shutdown input and output
      shutdownInput();
      shutdownOutput();
      
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

bool AbstractSocket::getLocalAddress(SocketAddress* address)
{
   bool rval = false;
   
   if(!isBound())
   {
      ExceptionRef e = new SocketException(
         "Cannot get local address for an unbound Socket!");
      Exception::setLast(e, false);
   }
   else
   {
      // get address structure
      socklen_t size = 130;
      char addr[size];
      
      // get local information
      int error = getsockname(mFileDescriptor, (sockaddr*)&addr, &size);
      if(error < 0)
      {
         ExceptionRef e = new SocketException(
            "Could not get Socket local address!", strerror(errno));
         Exception::setLast(e, false);
      }
      else
      {
         // convert socket address
         address->fromSockAddr((sockaddr*)&addr, size);
         rval = true;
      }
   }
   
   return rval;
}

bool AbstractSocket::getRemoteAddress(SocketAddress* address)
{
   bool rval = false;
   
   if(!isConnected())
   {
      ExceptionRef e = new SocketException(
         "Cannot get local address for an unconnected Socket!");
      Exception::setLast(e, false);
   }
   else
   {
      // get address structure
      socklen_t size = 130;
      char addr[size];
      
      // get remote information
      int error = getpeername(mFileDescriptor, (sockaddr*)&addr, &size);
      if(error < 0)
      {
         ExceptionRef e = new SocketException(
            "Could not get Socket remote address!", strerror(errno));
         Exception::setLast(e, false);
      }
      else
      {
         // convert socket address
         address->fromSockAddr((sockaddr*)&addr, size);
         rval = true;
      }
   }
   
   return rval;
}

InputStream* AbstractSocket::getInputStream()
{
   return mInputStream;
}

OutputStream* AbstractSocket::getOutputStream()
{
   return mOutputStream;
}

void AbstractSocket::setSendTimeout(unsigned long timeout)
{
   mSendTimeout = timeout;
}

unsigned long AbstractSocket::getSendTimeout()
{
   return mSendTimeout;
}

void AbstractSocket::setReceiveTimeout(unsigned long timeout)
{
   mReceiveTimeout = timeout;
}

unsigned long AbstractSocket::getReceiveTimeout()
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
