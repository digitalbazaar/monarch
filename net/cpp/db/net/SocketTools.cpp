/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/SocketTools.h"
#include "db/net/SocketDefinitions.h"
#include "db/rt/Thread.h"
#include "db/rt/System.h"

using namespace db::net;
using namespace db::rt;

int SocketTools::select(bool read, unsigned int fd, long long timeout)
{
   int rval = 0;
   
   // create a file descriptor set to read/write on
   fd_set rwfds;
   FD_ZERO(&rwfds);
   
   // create a file descriptor set to check for exceptions on
   fd_set exfds;
   FD_ZERO(&exfds);
   
   // add file descriptor to sets
   FD_SET(fd, &rwfds);
   FD_SET(fd, &exfds);
   
   // "n" parameter is the highest numbered descriptor plus 1
   int n = fd + 1;
   
   // set 20 millisecond interrupt check timeout (necessary because
   // windows lacks SIGNAL support to do interruptions properly)
   long long intck = 20LL;
   
   // keep selecting (polling) until timeout is reached
   long long remaining = (timeout <= 0) ? intck : timeout;
   
   struct timeval to;
   if(timeout < 0)
   {
      // create instant timeout (polling)
      to.tv_sec = 0;
      to.tv_usec = 0;
   }
   else
   {
      // create intck millisecond timeout (1 millisecond is 1000 microseconds)
      to.tv_sec = 0;
      to.tv_usec = (remaining < intck ? remaining : intck) * 1000LL;
   }
   
   unsigned long long start = System::getCurrentMilliseconds();
   unsigned long long end;
   
   Thread* t = Thread::currentThread();
   while(remaining > 0 && rval == 0 && !t->isInterrupted())
   {
      // wait for file descriptors to be updated
      if(read)
      {
         // wait for readability
         rval = ::select(n, &rwfds, NULL, &exfds, &to);
      }
      else
      {
         // wait for writability
         rval = ::select(n, NULL, &rwfds, &exfds, &to);
      }
      
      if(rval < 0 && errno == 0)
      {
         // no error, just timed out
         rval = 0;
         
         // NOTE: select() may EINTR here but it is up to
         // the calling method to determine what to do about it
      }
      
      // select() implementation may alter sets or timeout, so reset them
      // if calling select() again (not interrupted and timeout >= 0)
      if(rval == 0 && timeout >= 0)
      {
         // clear sets and re-add file descriptor
         FD_ZERO(&rwfds);
         FD_ZERO(&exfds);
         FD_SET(fd, &rwfds);
         FD_SET(fd, &exfds);
         
         // reset timeout
         to.tv_sec = 0;
         to.tv_usec = intck * 1000LL;
      }
      
      if(timeout != 0)
      {
         // decrement remaining time
         end = System::getCurrentMilliseconds();
         remaining -= (end - start);
         start = end;
         to.tv_usec = (remaining < intck ? remaining : intck) * 1000LL;
      }
   }
   
   if(t->isInterrupted())
   {
      rval = -1;
      errno = EINTR;
      
      // set interrupted exception
      ExceptionRef e = t->createInterruptedException();
      Exception::setLast(e, false);
   }
   else if(rval > 0 && FD_ISSET(fd, &exfds) != 0)
   {
      // an exception occurred with the file descriptor
      rval = -1;
      errno = EBADF;
   }
   
   return rval;
}

int SocketTools::select(
   int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
   long long timeout, const sigset_t* sigmask)
{
   int rval = 0;
   
// Note: disabled due to a lack of support in windows
//   // create timeout
//   struct timeval* tv = NULL;
//   struct timeval to;
//   if(timeout > 0)
//   {
//      // set timeout (1 millisecond is 1000 microseconds) 
//      to.tv_sec = timeout / 1000LL;
//      to.tv_usec = (timeout % 1000LL) * 1000LL;
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
   long long intck = 20LL;
   
   // keep selecting (polling) until timeout is reached
   long long remaining = (timeout <= 0) ? intck : timeout;
   
   struct timeval to;
   if(timeout < 0)
   {
      // create instant timeout (polling)
      to.tv_sec = 0;
      to.tv_usec = 0;
   }
   else
   {
      // create 20 millisecond timeout (1 millisecond is 1000 microseconds)
      to.tv_sec = 0;
      to.tv_usec = (remaining < intck ? remaining : intck) * 1000LL;
   }
   
   unsigned long long start = System::getCurrentMilliseconds();
   unsigned long long end;
   
   Thread* t = Thread::currentThread();
   while(remaining > 0 && rval == 0 && !t->isInterrupted())
   {
      // wait for file descriptors to be updated
      rval = ::select(nfds, readfds, writefds, exceptfds, &to);
      
      if(rval < 0 && errno == 0)
      {
         // no error, just timed out
         rval = 0;
         
         // NOTE: select() may EINTR here but it is up to
         // the calling method to determine what to do about it
      }
      
      // select() implementation may alter sets or timeout, so reset them
      // if calling select() again (not interrupted and timeout >= 0)
      if(rval == 0 && timeout >= 0)
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
         to.tv_usec = intck * 1000LL;
      }
      
      if(timeout != 0)
      {
         // decrement remaining time
         end = System::getCurrentMilliseconds();
         remaining -= (end - start);
         start = end;
         to.tv_usec = (remaining < intck ? remaining : intck) * 1000LL;
      }
   }
   
   if(t->isInterrupted())
   {
      rval = -1;
      errno = EINTR;
      
      // set interrupted exception
      ExceptionRef e = t->createInterruptedException();
      Exception::setLast(e, false);
   }
   
   return rval;
}
