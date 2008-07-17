/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/AbstractSocket.h"

#include "db/net/SocketTools.h"
#include "db/net/SocketDefinitions.h"
#include "db/io/PeekInputStream.h"
#include "db/net/SocketInputStream.h"
#include "db/net/SocketOutputStream.h"
#include "db/rt/Thread.h"
#include "db/rt/DynamicObject.h"

#include <cstdlib>
#include <cstring>

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
   
   // default to synchronous IO
   mAsyncIO = false;
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
         
         std::string msg = "Could not create Socket! ";
         msg.append(strerror(errno));
         ExceptionRef e = new Exception(msg.c_str(), SOCKET_EXCEPTION_TYPE);
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
      std::string msg = "Could not create Socket! ";
      msg.append(strerror(errno));
      ExceptionRef e = new Exception(msg.c_str(), SOCKET_EXCEPTION_TYPE);
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
            e = new Exception(
               "Socket read interrupted!", "db.io.InterruptedException");
            e->getDetails()["error"] = strerror(errno);
         }
         else
         {
            // interrupted exception
            e = new Exception(
               "Socket write interrupted!", "db.io.InterruptedException");
            e->getDetails()["error"] = strerror(errno);
         }
      }
      else
      {
         if(read)
         {
            // error occurred, get string message
            e = new Exception(
               "Could not read from Socket!", SOCKET_EXCEPTION_TYPE);
            e->getDetails()["error"] = strerror(errno);
         }
         else
         {
            // error occurred, get string message
            e = new Exception(
               "Could not write to Socket!", SOCKET_EXCEPTION_TYPE);
            e->getDetails()["error"] = strerror(errno);
         }
      }
   }
   else if(error == 0)
   {
      if(read)
      {
         // read timeout occurred
         e = new Exception(
            "Socket read timed out!", SOCKET_TIMEOUT_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
      }
      else
      {
         // write timeout occurred
         e = new Exception(
            "Socket write timed out!", SOCKET_TIMEOUT_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
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
      if(lastError != 0 && lastError != EINPROGRESS)
      {
         if(read)
         {
            // error occurred, get string message
            e = new Exception(
               "Could not read from Socket!", SOCKET_EXCEPTION_TYPE);
            e->getDetails()["error"] = strerror(lastError);
         }
         else
         {
            // error occurred, get string message
            e = new Exception(
               "Could not write to Socket!", SOCKET_EXCEPTION_TYPE);
            e->getDetails()["error"] = strerror(lastError);
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
         
         ExceptionRef e = new Exception(
            "Could not bind Socket!", SOCKET_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
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
      ExceptionRef e = new Exception(
         "Cannot listen on unbound Socket!", SOCKET_EXCEPTION_TYPE);
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
         ExceptionRef e = new Exception(
            "Could not listen on Socket!", SOCKET_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
         Exception::setLast(e, false);
      }
      else
      {
         // now listening
         mListening = true;
         
         // set socket to non-blocking so accept() calls can be interrupted
         fcntl(mFileDescriptor, F_SETFL, O_NONBLOCK);
      }
   }
   
   return mListening;
}

Socket* AbstractSocket::accept(unsigned int timeout)
{
   Socket* rval = NULL;
   
   if(!isListening())
   {
      ExceptionRef e = new Exception(
         "Cannot accept with a non-listening Socket!", SOCKET_EXCEPTION_TYPE);
      Exception::setLast(e, false);
   }
   else
   {
      // try to accept a connection
      int fd = ::accept(mFileDescriptor, NULL, NULL);
      if(fd < 0)
      {
         // see if no connection was currently available
         if(errno == EAGAIN)
         {
            // reset fd to zero (no error), wait for a connection
            fd = 0;
            if(select(true, timeout * 1000LL))
            {
               fd = ::accept(mFileDescriptor, NULL, NULL);
            }
         }
      }
      
      if(fd < 0)
      {
         ExceptionRef e = new Exception(
            "Could not accept connection!", SOCKET_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
         Exception::setLast(e, false);
      }
      else if(fd != 0)
      {
         // create a connected Socket
         rval = createConnectedSocket(fd);
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
      
      // temporarily make socket non-blocking
      fcntl(mFileDescriptor, F_SETFL, O_NONBLOCK);
      
      // connect
      int error = ::connect(mFileDescriptor, (sockaddr*)addr, size);
      if(error < 0)
      {
         switch(errno)
         {
            // connecting
            case EINPROGRESS:
            // already connected
            case EALREADY:
            // no error
            case EAGAIN:
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
               break;
            default:
               {
                  // could not connect
                  ExceptionRef e = new Exception(
                     "Cannot connect Socket!", SOCKET_EXCEPTION_TYPE);
                  e->getDetails()["error"] = strerror(errno);
                  Exception::setLast(e, false);
               }
               break;
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
      ExceptionRef e = new Exception(
         "Cannot write to unbound Socket!", SOCKET_EXCEPTION_TYPE);
      Exception::setLast(e, false);
   }
   else
   {
      // loop until all data is sent (send() call may need to be called
      // multiple times since it will not send all data if the send buffer
      // fills up and hasn't been emptied fast enough)
      unsigned int offset = 0;
      int bytes;
      while(rval && length > 0)
      {
         // try to send some data, don't block, don't send SIGPIPE
         bytes = ::send(
            mFileDescriptor, b + offset, length, MSG_DONTWAIT | MSG_NOSIGNAL);
         if(bytes < 0)
         {
            // see if socket buffer is full (EAGAIN)
            if(errno == EAGAIN)
            {
               if(isIOAsynchronous())
               {
                  // using asynchronous IO
                  ExceptionRef e = new Exception(
                     "Socket would block during write.",
                     SOCKET_EXCEPTION_TYPE ".WouldBlock");
                  e->getDetails()["wouldBlock"] = true;
                  Exception::setLast(e, false);
                  rval = false;
               }
               else
               {
                  // wait for socket to become writable
                  rval = select(false, getSendTimeout());
               }
            }
            else
            {
               // actual socket error
               ExceptionRef e = new Exception(
                  "Could not write to Socket!", SOCKET_EXCEPTION_TYPE);
               e->getDetails()["error"] = strerror(errno);
               Exception::setLast(e, false);
               rval = false;
            }
         }
         else if(bytes > 0)
         {
            offset += bytes;
            length -= bytes;
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
      ExceptionRef e = new Exception(
         "Cannot read from unbound Socket!", SOCKET_EXCEPTION_TYPE);
      Exception::setLast(e, false);
   }
   else
   {
      // try to receive some data, don't block
      rval = ::recv(mFileDescriptor, b, length, MSG_DONTWAIT);
      if(rval < 0)
      {
         // see if no data is available (EGAIN)
         if(errno == EAGAIN)
         {
            if(isIOAsynchronous())
            {
               // using asynchronous IO
               ExceptionRef e = new Exception(
                  "Socket would block during write.",
                  SOCKET_EXCEPTION_TYPE ".WouldBlock");
               e->getDetails()["wouldBlock"] = true;
               Exception::setLast(e, false);
            }
            else
            {
               // wait for data to become available
               if(select(true, getReceiveTimeout()))
               {
                  // receive data (should not block)
                  rval = ::recv(mFileDescriptor, b, length, 0);
               }
            }
         }
         else
         {
            // socket error
            ExceptionRef e = new Exception(
               "Could not read from Socket!", SOCKET_EXCEPTION_TYPE);
            e->getDetails()["error"] = strerror(errno);
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

inline bool AbstractSocket::isBound()
{
   return mBound;
}

inline bool AbstractSocket::isListening()
{
   return mListening;
}

inline bool AbstractSocket::isConnected()
{
   return mConnected;
}

bool AbstractSocket::getLocalAddress(SocketAddress* address)
{
   bool rval = false;
   
   if(!isBound())
   {
      ExceptionRef e = new Exception(
         "Cannot get local address for an unbound Socket!",
         SOCKET_EXCEPTION_TYPE);
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
         ExceptionRef e = new Exception(
            "Could not get Socket local address!",
            SOCKET_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
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
      ExceptionRef e = new Exception(
         "Cannot get local address for an unconnected Socket!",
         SOCKET_EXCEPTION_TYPE);
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
         ExceptionRef e = new Exception(
            "Could not get Socket remote address!",
            SOCKET_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
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

inline InputStream* AbstractSocket::getInputStream()
{
   return mInputStream;
}

inline OutputStream* AbstractSocket::getOutputStream()
{
   return mOutputStream;
}

inline void AbstractSocket::setSendTimeout(uint32_t timeout)
{
   mSendTimeout = timeout;
}

inline uint32_t AbstractSocket::getSendTimeout()
{
   return mSendTimeout;
}

inline void AbstractSocket::setReceiveTimeout(uint32_t timeout)
{
   mReceiveTimeout = timeout;
}

inline uint32_t AbstractSocket::getReceiveTimeout()
{
   return mReceiveTimeout;
}

inline unsigned int AbstractSocket::getBacklog()
{
   return mBacklog;
}

inline int AbstractSocket::getFileDescriptor()
{
   return mFileDescriptor;
}

inline void AbstractSocket::setAsynchronousIO(bool async)
{
   mAsyncIO = async;
}

inline bool AbstractSocket::isIOAsynchronous()
{
   return mAsyncIO;
}
