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
         fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
      if(error < 0)
      {
         // close socket
         close();
         
         Thread::setException(new SocketException(
            "Could not create Socket!", strerror(errno)));
      }
      else
      {
         mFileDescriptor = fd;
         rval = true;
      }
   }
   else
   {
      Thread::setException(new SocketException(
         "Could not create Socket!", strerror(errno)));
   }
   
   return rval;
}

bool AbstractSocket::select(bool read, long long timeout)
{
   Exception* exception = NULL;
   
   if(!Thread::interrupted(false))
   {
      // create a file descriptor set to select on
      fd_set fds;
      FD_ZERO(&fds);
      
      // add file descriptor to set
      FD_SET((unsigned int)mFileDescriptor, &fds);
      
      // "n" parameter is the highest numbered descriptor plus 1
      int n = mFileDescriptor + 1;
      
      int error;
      if(read)
      {
         // wait for data to arrive for reading or for an exception
         error = Thread::select(n, &fds, NULL, &fds, timeout);
      }
      else
      {
         // wait for writability or for an exception
         error = Thread::select(n, NULL, &fds, &fds, timeout);
      }
      
      if(error < 0)
      {
         if(errno == EINTR)
         {
            if(read)
            {
               // interrupted exception
               exception = new InterruptedException(
                  "Socket read interrupted!", strerror(errno));
            }
            else
            {
               // interrupted exception
               exception = new InterruptedException(
                  "Socket write interrupted!", strerror(errno));
            }
         }
         
         if(read)
         {
            // error occurred, get string message
            exception = new SocketException(
               "Could not read from Socket!", strerror(errno));
         }
         else
         {
            // error occurred, get string message
            exception = new SocketException(
               "Could not write to Socket!", strerror(errno));
         }
      }
      else if(error == 0)
      {
         if(read)
         {
            // read timeout occurred
            exception = new SocketTimeoutException(
               "Socket read timed out!", strerror(errno));
         }
         else
         {
            // write timeout occurred
            exception = new SocketTimeoutException(
               "Socket write timed out!", strerror(errno));
         }
      }
   }
   
   if(exception == NULL && Thread::interrupted(false))
   {
      if(read)
      {
         exception = new InterruptedException("Socket read interrupted!");
      }
      else
      {
         exception = new InterruptedException("Socket write interrupted!");
      }
   }
   
   if(exception != NULL)
   {
      Thread::setException(exception);
   }
   
   return exception == NULL;
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
   }
   
   return true;
}

bool AbstractSocket::shutdownOutput()
{
   // delete output stream
   if(mOutputStream != NULL)
   {
      delete mOutputStream;
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
         Thread::setException(new SocketException(
            "Could not bind Socket!", strerror(errno)));
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
      Thread::setException(new SocketException(
         "Cannot listen on unbound Socket!")); 
   }
   else
   {
      // set backlog
      mBacklog = backlog;
      
      // listen
      int error = ::listen(mFileDescriptor, backlog);
      if(error < 0)
      {
         Thread::setException(new SocketException(
            "Could not listen on Socket!", strerror(errno)));
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
      Thread::setException(new SocketException(
         "Cannot accept with a non-listening Socket!"));
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
            Thread::setException(new SocketException(
               "Could not accept connection!", strerror(errno)));
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
            // get the last error on the socket
            int lastError;
            socklen_t lastErrorLength = sizeof(lastError);
            getsockopt(
               mFileDescriptor, SOL_SOCKET, SO_ERROR,
               (char*)&lastError, &lastErrorLength);
            if(lastError == 0)
            {
               // now connected and bound
               mBound = true;
               mConnected = true;
            }
            else
            {
               Thread::setException(new SocketException(
                  "Could not connect Socket! Connection refused.",
                  strerror(lastError)));
            }
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

bool AbstractSocket::send(const char* b, unsigned int length)
{
   Exception* exception = NULL;
   
   if(!isBound())
   {
      exception = new SocketException("Cannot write to unbound Socket!");
   }
   else
   {
      // send all data (send can fail to send all bytes in one go because the
      // socket send buffer was full)
      unsigned int offset = 0;
      while(exception == NULL && length > 0)
      {
         // wait for socket to become writable
         if(select(false, getSendTimeout()))
         {
            // send some data
            int bytes = ::send(mFileDescriptor, b + offset, length, 0);
            if(bytes < 0)
            {
               exception = new SocketException(
                  "Could not write to Socket!", strerror(errno));
            }
            else if(bytes > 0)
            {
               offset += bytes;
               length -= bytes;
            }
         }
         else
         {
            exception = Thread::getException();
         }
      }
   }
   
   if(exception != NULL)
   {
      Thread::setException(exception);
   }
   
   return exception == NULL;
}

int AbstractSocket::receive(char* b, unsigned int length)
{
   int rval = -1;
   
   if(!isBound())
   {
      Thread::setException(new SocketException(
         "Cannot read from unbound Socket!"));
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
            Thread::setException(new SocketException(
               "Could not read from Socket!", strerror(errno)));
         }
         else if(rval == 0)
         {
            // socket is closed now
            rval = -1;
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
   Exception* exception = NULL;
   
   if(!isBound())
   {
      exception = new SocketException(
         "Cannot get local address for an unbound Socket!");
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
         exception = new SocketException(
            "Could not get Socket local address!", strerror(errno));
      }
      
      // convert socket address
      address->fromSockAddr((sockaddr*)&addr, size);
   }
   
   if(exception != NULL)
   {
      Thread::setException(exception);
   }
   
   return exception == NULL;
}

bool AbstractSocket::getRemoteAddress(SocketAddress* address)
{
   Exception* exception = NULL;
   
   if(!isConnected())
   {
      exception = new SocketException(
         "Cannot get local address for an unconnected Socket!");
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
         exception = new SocketException(
            "Could not get Socket remote address!", strerror(errno));
      }
      else
      {
         // convert socket address
         address->fromSockAddr((sockaddr*)&addr, size);
      }
   }
   
   if(exception != NULL)
   {
      Thread::setException(exception);
   }
   
   return exception == NULL;
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
