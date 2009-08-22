/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_SocketTools_H
#define db_net_SocketTools_H

#include "db/net/SocketDefinitions.h"
#include <sys/types.h>

namespace db
{
namespace net
{

/**
 * The SocketTools class provides helper methods for using sockets.
 *
 * @author Dave Longley
 */
class SocketTools
{
private:
   /**
    * Creates a new SocketTools object.
    */
   SocketTools() {};

public:
   /**
    * Causes the current thread to wait until the given file descriptor becomes
    * ready for some kind of IO operation or until the given timeout is
    * reached. This method allows this thread to be interrupted during the
    * select().
    *
    * Note: errno can be set as such:
    *
    * EBADF  An invalid file descriptor was in the set (i.e. already closed).
    * EINTR  A signal was caught.
    * EINVAL nfds is negative or the value in the timeout is invalid.
    * ENOMEM unable to allocate memory.
    *
    * @param read true if the IO operation to wait for is a read operation,
    *             false if it is a write operation.
    * @param fd the file descriptor to monitor.
    * @param timeout the timeout to use (0 for indefinite wait, -1 for polling).
    *
    * @return >= 1 if the file descriptor was updated, 0 if it was not and
    *         it timed out, -1 if an error occurred and errno is set
    *         appropriately.
    */
   static int select(bool read, unsigned int fd, long long timeout);

   /**
    * Causes the current thread to wait until one or more of the given
    * file descriptors become ready for some kind of IO operation or until
    * the given timeout is reached. This method allows this thread to be
    * interrupted during the select().
    *
    * Note: errno can be set as such:
    *
    * EBADF  An invalid file descriptor was in the set (i.e. already closed).
    * EINTR  A signal was caught.
    * EINVAL nfds is negative or the value in the timeout is invalid.
    * ENOMEM unable to allocate memory.
    *
    * @param nfds the highest-numbered file descriptor in any of the given
    *             three sets, plus 1.
    * @param readfds a set of file descriptors to monitor for readability.
    * @param writefds a set of file descriptors to monitor for writeability.
    * @param exceptfds a set of file descriptors to monitor for exceptions.
    * @param timeout the timeout to use (0 for indefinite wait, -1 for polling).
    * @param sigmask a signal mask to set atomically so that whatever signals
    *                are not masked will be received inside of the select()
    *                call.
    *
    * @return the number of file descriptors contained in the three returned
    *         descriptor sets which may be 0, -1 if an error occurred and
    *         errno is set appropriately.
    */
   static int select(
      int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
      long long timeout, const sigset_t* sigmask = NULL);
};

} // end namespace net
} // end namespace db
#endif
