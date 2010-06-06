/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include "monarch/net/AbstractSocket.h"

#include "monarch/net/WindowsSupport.h"
#include "monarch/net/SocketTools.h"
#include "monarch/io/PeekInputStream.h"
#include "monarch/net/SocketInputStream.h"
#include "monarch/net/SocketOutputStream.h"
#include "monarch/rt/Thread.h"
#include "monarch/rt/DynamicObject.h"

#include <cstdlib>
#include <cstring>

using namespace monarch::io;
using namespace monarch::net;
using namespace monarch::rt;

AbstractSocket::AbstractSocket() :
   // file descriptor is invalid at this point
   mFileDescriptor(-1),
   mCommDomain(SocketAddress::IPv4),
   // not bound, listening, or connected
   mBound(false),
   mListening(false),
   mConnected(false),
   // input/output uninitialized
   mInputStream(NULL),
   mOutputStream(NULL),
   // no receive or send timeouts (socket will block)
   mSendTimeout(0),
   mReceiveTimeout(0),
   // default backlog is 50
   mBacklog(50),
   // default to blocking IO
   mSendNonBlocking(false),
   mReceiveNonBlocking(false)
{
}

AbstractSocket::~AbstractSocket()
{
   // close socket
   AbstractSocket::close();
}

bool AbstractSocket::create(int domain, int type, int protocol)
{
   bool rval = false;

   // IPv6
   if(domain == PF_INET6 || domain == AF_INET6)
   {
      mCommDomain = SocketAddress::IPv6;
   }
   // default to IPv4
   else
   {
      mCommDomain = SocketAddress::IPv4;
   }

   int fd = SOCKET_MACRO_socket(domain, type, protocol);
   if(fd >= 0)
   {
      // set reuse address flag
      // disables "address already in use" errors by reclaiming ports that
      // are waiting to be cleaned up
      int reuse = 1;
      int error = setsockopt(
         fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
#if !defined(MSG_NOSIGNAL) && defined(SO_NOSIGPIPE)
      if(error == 0)
      {
         // nescessary on platforms that don't support MSG_NOSIGNAL option
         // on send()
         int on = 1;
         error = setsockopt(
            fd, SOL_SOCKET, SO_NOSIGPIPE, (void*)&on, sizeof(on));
      }
#endif
      if(error < 0)
      {
         // close socket
         close();

         ExceptionRef e = new Exception(
            "Could not create socket.", SOCKET_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
         Exception::set(e);
      }
      else
      {
         mFileDescriptor = fd;
         rval = true;
      }
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not create socket.", SOCKET_EXCEPTION_TYPE);
      e->getDetails()["error"] = strerror(errno);
      Exception::set(e);
   }

   return rval;
}

bool AbstractSocket::select(bool read, int64_t timeout)
{
   Exception* e = NULL;

   // wait for readability/writability
   int error = SocketTools::select(read, mFileDescriptor, timeout);
   if(error < 0)
   {
      if(errno == EINTR)
      {
         if(read)
         {
            // interrupted exception
            e = new Exception(
               "Socket read interrupted.", "monarch.io.InterruptedException");
            e->getDetails()["error"] = strerror(errno);
         }
         else
         {
            // interrupted exception
            e = new Exception(
               "Socket write interrupted.", "monarch.io.InterruptedException");
            e->getDetails()["error"] = strerror(errno);
         }
      }
      else
      {
         if(read)
         {
            // error occurred, get string message
            e = new Exception(
               "Could not read from socket.", SOCKET_EXCEPTION_TYPE);
            e->getDetails()["error"] = strerror(errno);
         }
         else
         {
            // error occurred, get string message
            e = new Exception(
               "Could not write to socket.", SOCKET_EXCEPTION_TYPE);
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
            "Socket read timed out.", SOCKET_TIMEOUT_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
      }
      else
      {
         // write timeout occurred
         e = new Exception(
            "Socket write timed out.", SOCKET_TIMEOUT_EXCEPTION_TYPE);
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
               "Could not read from socket.", SOCKET_EXCEPTION_TYPE);
            e->getDetails()["error"] = strerror(lastError);
         }
         else
         {
            // error occurred, get string message
            e = new Exception(
               "Could not write to socket.", SOCKET_EXCEPTION_TYPE);
            e->getDetails()["error"] = strerror(lastError);
         }
      }
   }

   if(e != NULL)
   {
      ExceptionRef ref = e;
      Exception::set(ref);
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
   if(acquireFileDescriptor(address->getCommunicationDomain()))
   {
      // populate address structure
      unsigned int size = 130;
      char addr[size];
      address->toSockAddr((sockaddr*)&addr, size);

      // bind
      int error = SOCKET_MACRO_bind(mFileDescriptor, (sockaddr*)&addr, size);
      if(error < 0)
      {
         ExceptionRef e = new Exception(
            "Could not bind socket.", SOCKET_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
         e->getDetails()["address"] = address->getAddress();
         e->getDetails()["port"] = address->getPort();
         Exception::set(e);

         // close socket
         close();
      }
      else
      {
         // initialize input and output
         initializeInput();
         initializeOutput();

         // now bound
         mBound = true;

         // update address
         getLocalAddress(address);
      }
   }

   return mBound;
}

bool AbstractSocket::listen(int backlog)
{
   if(!isBound())
   {
      ExceptionRef e = new Exception(
         "Cannot listen on unbound socket.",
         SOCKET_EXCEPTION_TYPE ".NotBound");
      Exception::set(e);
   }
   else
   {
      // set backlog
      mBacklog = backlog;

      // listen
      int error = SOCKET_MACRO_listen(mFileDescriptor, backlog);
      if(error < 0)
      {
         ExceptionRef e = new Exception(
            "Could not listen on socket.", SOCKET_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
         Exception::set(e);
      }
      else
      {
         // now listening
         mListening = true;

         // set socket to non-blocking so accept() calls can be interrupted
         SOCKET_MACRO_fcntl(mFileDescriptor, F_SETFL, O_NONBLOCK);
      }
   }

   return mListening;
}

Socket* AbstractSocket::accept(int timeout)
{
   Socket* rval = NULL;

   if(!isListening())
   {
      ExceptionRef e = new Exception(
         "Cannot accept with a non-listening socket.",
         SOCKET_EXCEPTION_TYPE ".NotListening");
      Exception::set(e);
   }
   else
   {
      // try to accept a connection
      int fd = SOCKET_MACRO_accept(mFileDescriptor, NULL, NULL);
      if(fd < 0)
      {
         // see if no connection was currently available
         if(errno == EAGAIN)
         {
            // reset fd to zero (no error), wait for a connection
            fd = 0;
            if(select(true, timeout * INT64_C(1000)))
            {
               fd = SOCKET_MACRO_accept(mFileDescriptor, NULL, NULL);
            }
         }
      }

      if(fd < 0)
      {
         ExceptionRef e = new Exception(
            "Could not accept connection.", SOCKET_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
         Exception::set(e);
      }
      else if(fd != 0)
      {
         bool success = true;
#if !defined(MSG_NOSIGNAL) && defined(SO_NOSIGPIPE)
         // nescessary on platforms that don't support MSG_NOSIGNAL option
         // on send()
         int on = 1;
         int error = setsockopt(
            fd, SOL_SOCKET, SO_NOSIGPIPE, (void*)&on, sizeof(on));
         if(error < 0)
         {
            ExceptionRef e = new Exception(
               "Could not set socket options.", SOCKET_EXCEPTION_TYPE);
            e->getDetails()["error"] = strerror(errno);
            Exception::set(e);
            success = false;
         }
#endif
         if(success)
         {
            // create a connected Socket
            rval = createConnectedSocket(fd);
         }
      }
   }

   return rval;
}

bool AbstractSocket::connect(SocketAddress* address, int timeout)
{
   // acquire file descriptor
   if(acquireFileDescriptor(address->getCommunicationDomain()))
   {
      // populate address structure
      unsigned int size = 130;
      char addr[size];
      address->toSockAddr((sockaddr*)&addr, size);

      // make socket non-blocking, blocking is handled via select
      SOCKET_MACRO_fcntl(mFileDescriptor, F_SETFL, O_NONBLOCK);

      // connect
      int error = SOCKET_MACRO_connect(mFileDescriptor, (sockaddr*)addr, size);
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
               if(select(false, timeout * INT64_C(1000)))
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
                  "Cannot connect socket.", SOCKET_EXCEPTION_TYPE);
               e->getDetails()["error"] = strerror(errno);
               e->getDetails()["address"] = address->toString().c_str();
               Exception::set(e);
               break;
            }
         }
      }
      else
      {
         // now connected and bound
         mBound = true;
         mConnected = true;
      }

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
         "Cannot write to unbound socket.",
         SOCKET_EXCEPTION_TYPE ".NotBound");
      Exception::set(e);
      rval = false;
   }
   else
   {
      // loop until all data is sent (send() call may need to be called
      // multiple times since it will not send all data if the send buffer
      // fills up and hasn't been emptied fast enough)
      int sent = 0;
      unsigned int offset = 0;
      int bytes;
      while(rval && length > 0)
      {
         // try to send some data
         // don't block and don't send SIGPIPE per send() if possible
         // Platforms without MSG_NOSIGNAL such as MacOS set a per-socket
         // SO_NOSIGPIPE option.
         int flags = 0;
#ifdef MSG_DONTWAIT
         flags |= MSG_DONTWAIT;
#endif
#ifdef MSG_NOSIGNAL
         flags |= MSG_NOSIGNAL;
#endif
         bytes = SOCKET_MACRO_send(
            mFileDescriptor, b + offset, length, flags);
         if(bytes < 0)
         {
            // see if socket buffer is full (EAGAIN)
            if(errno == EAGAIN)
            {
               if(isSendNonBlocking())
               {
                  // using asynchronous IO
                  ExceptionRef e = new Exception(
                     "Socket would block during write.",
                     SOCKET_EXCEPTION_TYPE ".WouldBlock");
                  e->getDetails()["written"] = sent;
                  e->getDetails()["wouldBlock"] = true;
                  Exception::set(e);
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
                  "Could not write to socket.", SOCKET_EXCEPTION_TYPE);
               e->getDetails()["error"] = strerror(errno);
               Exception::set(e);
               rval = false;
            }
         }
         else if(bytes > 0)
         {
            sent += bytes;
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
         "Cannot read from unbound socket.",
         SOCKET_EXCEPTION_TYPE ".NotBound");
      Exception::set(e);
   }
   else
   {
      // try to receive some data, don't block
      int flags = 0;
#ifdef MSG_DONTWAIT
      flags |= MSG_DONTWAIT;
#endif
      rval = SOCKET_MACRO_recv(mFileDescriptor, b, length, flags);
      if(rval < 0)
      {
         // see if no data is available (EGAIN)
         if(errno == EAGAIN)
         {
            if(isReceiveNonBlocking())
            {
               // using asynchronous IO
               ExceptionRef e = new Exception(
                  "Socket would block during receive.",
                  SOCKET_EXCEPTION_TYPE ".WouldBlock");
               e->getDetails()["wouldBlock"] = true;
               Exception::set(e);
            }
            else
            {
               // wait for data to become available
               if(select(true, getReceiveTimeout()))
               {
                  // receive data (should not block)
                  rval = SOCKET_MACRO_recv(mFileDescriptor, b, length, 0);
               }
            }
         }

         // check for error again
         if(rval < 0 && errno != EAGAIN)
         {
            // socket error
            ExceptionRef e = new Exception(
               "Could not read from socket.", SOCKET_EXCEPTION_TYPE);
            e->getDetails()["error"] = strerror(errno);
            Exception::set(e);
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

      // shutdown and close the socket
      SOCKET_MACRO_shutdown(mFileDescriptor, SHUT_RDWR);
      SOCKET_MACRO_close(mFileDescriptor);

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

bool AbstractSocket::isConnected()
{
   if(mConnected)
   {
      // get the last error on the socket
      int lastError;
      socklen_t lastErrorLength = sizeof(lastError);
      getsockopt(
         mFileDescriptor, SOL_SOCKET, SO_ERROR,
         (char*)&lastError, &lastErrorLength);
      if(lastError == EPIPE)
      {
         // connection severed
         errno = lastError;
         close();
      }
      else
      {
         // check to see if the connection has been shutdown, by seeing
         // if recv() will return 0 (do a peek so as not to disturb data)
         char buf;
         int flags = MSG_PEEK;
#ifdef MSG_DONTWAIT
         flags |= MSG_DONTWAIT;
#endif
         int ret = SOCKET_MACRO_recv(mFileDescriptor, &buf, 1, flags);
         if(ret == 0)
         {
            // connection severed
            errno = EPIPE;
            close();
         }
      }
   }

   return mConnected;
}

bool AbstractSocket::getLocalAddress(SocketAddress* address)
{
   bool rval = false;

   if(!isBound())
   {
      ExceptionRef e = new Exception(
         "Cannot get local address for an unbound socket.",
         SOCKET_EXCEPTION_TYPE ".NotBound");
      Exception::set(e);
   }
   else
   {
      // get address structure
      socklen_t size = 130;
      char addr[size];

      // get local information
      int error = SOCKET_MACRO_getsockname(
         mFileDescriptor, (sockaddr*)&addr, &size);
      if(error < 0)
      {
         ExceptionRef e = new Exception(
            "Could not get socket local address.",
            SOCKET_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
         Exception::set(e);
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
         "Cannot get local address for an unconnected socket.",
         SOCKET_EXCEPTION_TYPE ".Closed");
      Exception::set(e);
   }
   else
   {
      // get address structure
      socklen_t size = 130;
      char addr[size];

      // get remote information
      int error = SOCKET_MACRO_getpeername(
         mFileDescriptor, (sockaddr*)&addr, &size);
      if(error < 0)
      {
         ExceptionRef e = new Exception(
            "Could not get socket remote address.",
            SOCKET_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
         Exception::set(e);
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

inline SocketAddress::CommunicationDomain
   AbstractSocket::getCommunicationDomain()
{
   return mCommDomain;
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

inline int AbstractSocket::getBacklog()
{
   return mBacklog;
}

inline int AbstractSocket::getFileDescriptor()
{
   return mFileDescriptor;
}

inline void AbstractSocket::setSendNonBlocking(bool on)
{
   mSendNonBlocking = on;
}

inline bool AbstractSocket::isSendNonBlocking()
{
   return mSendNonBlocking;
}
inline void AbstractSocket::setReceiveNonBlocking(bool on)
{
   mReceiveNonBlocking = on;
}

inline bool AbstractSocket::isReceiveNonBlocking()
{
   return mReceiveNonBlocking;
}
