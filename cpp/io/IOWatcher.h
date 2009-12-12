/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_io_IOWatcher_H
#define db_io_IOWatcher_H

#include "monarch/rt/Collectable.h"

namespace db
{
namespace io
{

/**
 * An IOWatcher is an interface that is used by an IOMonitor to notify an
 * object that certain IO events have occurred.
 *
 * @author Dave Longley
 */
class IOWatcher
{
public:
   /**
    * Creates a new IOWatcher.
    */
   IOWatcher() {};

   /**
    * Destructs this IOWatcher.
    */
   virtual ~IOWatcher() {};

   /**
    * Called when a file descriptor is updated.
    *
    * @param fd the file descriptor that was updated.
    * @param events a bit flag describing what events (read/write) occurred.
    */
   virtual void fdUpdated(int fd, int events) = 0;
};

// typedef for a reference-counted IOWatcher
typedef db::rt::Collectable<IOWatcher> IOWatcherRef;

} // end namespace io
} // end namespace db
#endif
