/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include "monarch/net/SocketTools.h"

#include "monarch/net/WindowsSupport.h"
#include "monarch/rt/Thread.h"
#include "monarch/rt/System.h"

#include <limits.h>
#ifndef WIN32
#include <poll.h>
#endif

using namespace monarch::net;
using namespace monarch::rt;

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

int SocketTools::poll(bool read, int fd, int64_t timeout)
{
#if defined(WIN32) || defined(MACOS)
   return select(read, fd, timeout);
#else
   int rval = 0;

   // create poll set
   struct pollfd fds;
   fds.fd = fd;
   fds.events = read ? POLLIN : POLLOUT;

   // set 20 millisecond interrupt check timeout (currently necessary to
   // enable thread interruptions)
   int intck = INT32_C(20);

   // keep selecting (polling) until timeout is reached
   int64_t remaining = (timeout <= 0) ? intck : timeout;

   int to;
   if(timeout < 0)
   {
      // create instant timeout (polling)
      to = 0;
   }
   else
   {
      // create intck millisecond timeout
      to = (remaining < intck ? (int)remaining : intck);
   }

   uint64_t start = System::getCurrentMilliseconds();
   uint64_t end;

   Thread* t = Thread::currentThread();
   while(remaining > 0 && rval == 0 && !t->isInterrupted())
   {
      // wait for file descriptor to be updated
      rval = ::poll(&fds, 1, to);

      // check for error events
      if(rval > 0)
      {
         // remote side hung up
         if(fds.revents & POLLHUP)
         {
            rval = -1;
            errno = EPIPE;
         }
         // file descriptor not open
         else if(fds.revents & POLLNVAL)
         {
            rval = -1;
            errno = EBADF;
         }
         // error
         else if(fds.revents & POLLERR)
         {
            // some kind of IO error
            rval = -1;
            errno = EIO;
         }
      }
      else if(rval == -1 && errno == EINTR)
      {
         // no error, syscall interrupted
         rval = 0;
      }

      // decrement timeout
      if(rval == 0 && timeout != 0)
      {
         // decrement remaining time
         end = System::getCurrentMilliseconds();
         remaining -= (end - start);
         start = end;
         to = (remaining < intck ? (int)remaining : intck);
      }
   }

   if(rval > 0)
   {
      if(t->isInterrupted())
      {
         rval = -1;
         errno = EINTR;

         // set interrupted exception
         ExceptionRef e = t->createInterruptedException();
         Exception::set(e);
      }
   }

   return rval;
#endif
}

#ifdef WIN32
int SocketTools::select(bool read, unsigned int fd, int64_t timeout)
#else
int SocketTools::select(bool read, int fd, int64_t timeout)
#endif
{
   int rval = 0;

   // create a file descriptor set to read on
   fd_set rfds;
   FD_ZERO(&rfds);

   // create a file descriptor set to write on
   fd_set wfds;
   FD_ZERO(&wfds);

   // create a file descriptor set to check for exceptions on
   fd_set exfds;
   FD_ZERO(&exfds);

   // add file descriptor to sets
   FD_SET(fd, &rfds);
   FD_SET(fd, &wfds);
   FD_SET(fd, &exfds);

   // "n" parameter is the highest numbered descriptor plus 1
#ifdef WIN32
   unsigned int n = fd + 1;
#else
   int n = fd + 1;
#endif

   // set 20 millisecond interrupt check timeout (necessary because
   // windows lacks SIGNAL support to do interruptions properly)
   int64_t intck = INT64_C(20);

   // keep selecting (polling) until timeout is reached, if timeout is
   // indefinite (0), then set remaining to intck and never decrement it
   int64_t remaining = (timeout <= 0) ? intck : timeout;

   struct timeval to;
   to.tv_sec = 0;
   if(timeout < 0)
   {
      // create instant timeout (polling)
      to.tv_usec = 0;
   }
   else
   {
      // create intck millisecond timeout (1 millisecond is 1000 microseconds)
      to.tv_usec = (remaining < intck ? remaining : intck) * INT64_C(1000);
   }

   uint64_t start = System::getCurrentMilliseconds();
   uint64_t end;

   Thread* t = Thread::currentThread();
   while(remaining > 0 && rval == 0 && !t->isInterrupted())
   {
      // wait for file descriptors to be updated
      if(read)
      {
         // wait for readability
         rval = ::select(n, &rfds, NULL, &exfds, &to);
      }
      else
      {
         // wait for readability and writability
         //
         // Note: We must test to see if the pipe is broken by
         // testing for readability -- as it will occur if the
         // connection closes due to TCP sending an RST to the
         // socket ... and this will cause recv() to return 0
         rval = ::select(n, &rfds, &wfds, &exfds, &to);
         if(rval > 0)
         {
            if(FD_ISSET(fd, &rfds))
            {
               // readability flag switched, check to see if the connection has
               // been shutdown, by seeing if recv() will return 0 (do a peek
               // so as not to disturb real data)
               char buf;
               int flags = MSG_PEEK;
#ifdef MSG_DONTWAIT
               flags |= MSG_DONTWAIT;
#endif
               rval = SOCKET_MACRO_recv(fd, &buf, 1, flags);
               if(rval == 0)
               {
                  // connection closed
                  rval = -1;
                  errno = EPIPE;
               }
            }
            if(rval > 0)
            {
               // connection not closed, see if write was detected
               rval = FD_ISSET(fd, &wfds) ? 1 : 0;
            }
         }
      }

      if(rval < 0 && (errno == 0 || errno == EINPROGRESS || errno == EINTR))
      {
         // no error, just timed out, operation in progress,
         // or syscall interrupted
         rval = 0;

         // Note: handling EINTR could be changed to interrupt the current
         // thread if appropriate
      }

      // select() implementation may alter sets or timeout, so reset them
      // if calling select() again
      if(rval == 0)
      {
         // clear sets and re-add file descriptor
         FD_ZERO(&rfds);
         FD_ZERO(&wfds);
         FD_ZERO(&exfds);
         FD_SET(fd, &rfds);
         FD_SET(fd, &wfds);
         FD_SET(fd, &exfds);

         // reset timeout
         to.tv_sec = 0;
         if(timeout < 0)
         {
            // instant polling
            to.tv_usec = 0;
         }
         else if(timeout == 0)
         {
            // indefinite, do not decrement remaining
            to.tv_usec = intck * INT64_C(1000);
         }
         else
         {
            // decrement remaining time
            end = System::getCurrentMilliseconds();
            remaining -= (end - start);
            start = end;
            to.tv_usec =
               (remaining < intck ? remaining : intck) * INT64_C(1000);
         }
      }
   }

   if(rval > 0)
   {
      if(t->isInterrupted())
      {
         rval = -1;
         errno = EINTR;

         // set interrupted exception
         ExceptionRef e = t->createInterruptedException();
         Exception::set(e);
      }
      else if(FD_ISSET(fd, &exfds) != 0)
      {
         // exception occurred with file descriptor, consider pipe broken,
         // cannot mark it as a bad file descriptor here or else it won't
         // be closed
         rval = -1;
         errno = EPIPE;
      }
   }

   return rval;
}

int SocketTools::select(
   int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
   int64_t timeout, const sigset_t* sigmask)
{
   int rval = 0;

// Note: disabled due to a lack of support in windows
//   // create timeout
//   struct timeval* tv = NULL;
//   struct timeval to;
//   if(timeout > 0)
//   {
//      // set timeout (1 millisecond is 1000 microseconds)
//      to.tv_sec = timeout / INT64_C(1000);
//      to.tv_usec = (timeout % INT64_C(1000)) * INT64_C(1000);
//      tv = &to;
//   }
//
//   // FIXME: signals supposedly don't make select() return in windows
//   // this needs to be tested and potentially remedied somehow
//
//   // FIXME: furthermore, even if we block SIGINT (interruption signal) up
//   // until we reach the select call -- and then unblock right before it
//   // the signal could still sneak in right before select() is called and
//   // control is transferred to the kernel, and therefore we'd handle the
//   // SIGINT before the select() call and select() wouldn't get interrupted
//   // (there is pselect() for doing that unblocking atomically, but
//   // it's UNIX only) -- this may be solved by writing to another file
//   // descriptor when we receive SIGINTs and checking that file descriptor
//   // as well as the one we are waiting on -- but this might not be a
//   // viable solution for windows
//
//   // block SIGINTs
//   blockSignal(SIGINT);
//
//   Thread* t = Thread::currentThread();
//   if(!t->isInterrupted())
//   {
//      // FIXME: pselect() required here to do atomic unblocking & selecting
//
//      // wait for file descriptors to be updated
//      unblockSignal(SIGINT);
//      rval = ::select(nfds, readfds, writefds, exceptfds, timeout);
//      if(rval < 0)
//      {
//         if(errno == EINTR)
//         {
//            // interrupt thread
//            t->interrupt();
//         }
//      }
//   }
//   else
//   {
//      rval = -1;
//      errno = EINTR;
//   }

   // clone file descriptor sets
   fd_set readfds2;
   fd_set writefds2;
   fd_set exceptfds2;

   if(readfds != NULL)
   {
      readfds2 = *readfds;
   }

   if(writefds != NULL)
   {
      writefds2 = *writefds;
   }

   if(exceptfds != NULL)
   {
      exceptfds2 = *exceptfds;
   }

   // set 20 millisecond interrupt check timeout (necessary because
   // windows lacks SIGNAL support to do interruptions properly)
   int64_t intck = INT64_C(20);

   // keep selecting (polling) until timeout is reached, if timeout is
   // indefinite (0), then set remaining to intck and never decrement it
   int64_t remaining = (timeout <= 0) ? intck : timeout;

   struct timeval to;
   to.tv_sec = 0;
   if(timeout < 0)
   {
      // create instant timeout (polling)
      to.tv_usec = 0;
   }
   else
   {
      // create 20 millisecond timeout (1 millisecond is 1000 microseconds)
      to.tv_usec = (remaining < intck ? remaining : intck) * INT64_C(1000);
   }

   uint64_t start = System::getCurrentMilliseconds();
   uint64_t end;

   Thread* t = Thread::currentThread();
   while(remaining > 0 && rval == 0 && !t->isInterrupted())
   {
      // wait for file descriptors to be updated
      rval = ::select(nfds, readfds, writefds, exceptfds, &to);

      if(rval < 0 && (errno == 0 || errno == EINPROGRESS || errno == EINTR))
      {
         // no error, just timed out, operation in progress,
         // or syscall interrupted
         rval = 0;

         // Note: handling EINTR could be changed to interrupt the current
         // thread if appropriate
      }

      // select() implementation may alter sets or timeout, so reset them
      // if calling select() again
      if(rval == 0)
      {
         // reset file descriptor sets
         if(readfds != NULL)
         {
            *readfds = readfds2;
         }

         if(writefds != NULL)
         {
            *writefds = writefds2;
         }

         if(exceptfds != NULL)
         {
            *exceptfds = exceptfds2;
         }

         // reset timeout
         to.tv_sec = 0;
         if(timeout < 0)
         {
            // instant polling
            to.tv_usec = 0;
         }
         else if(timeout == 0)
         {
            // indefinite, do not decrement remaining
            to.tv_usec = intck * INT64_C(1000);
         }
         else
         {
            // decrement remaining time
            end = System::getCurrentMilliseconds();
            remaining -= (end - start);
            start = end;
            to.tv_usec =
               (remaining < intck ? remaining : intck) * INT64_C(1000);
         }
      }
   }

   if(t->isInterrupted())
   {
      rval = -1;
      errno = EINTR;

      // set interrupted exception
      ExceptionRef e = t->createInterruptedException();
      Exception::set(e);
   }

   return rval;
}

std::string SocketTools::getHostname()
{
   char tmp[HOST_NAME_MAX + 1];
   memset(tmp, 0, HOST_NAME_MAX + 1);
   gethostname(tmp, HOST_NAME_MAX);
   return tmp;
}
