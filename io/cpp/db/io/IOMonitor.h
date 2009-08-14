/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_io_IOMonitor_H
#define db_io_IOMonitor_H

#include "db/io/IOEventDelegate.h"

namespace db
{
namespace io
{

/**
 * An IOMonitor is used to notify IOWatchers when a file descriptor is ready
 * to be read from or written to.
 *
 * @author Dave Longley
 */
class IOMonitor
{
protected:

public:
   /**
    * Creates a new IOMonitor.
    */
   IOMonitor();

   /**
    * Destructs this IOMonitor.
    */
   virtual ~IOMonitor();

   /**
    * Adds an IOWatcher for the passed file descriptor and events.
    *
    * @param fd the file descriptor to watch.
    * @param events a bit flag describing what events (read/write) to monitor.
    * @param w the IOWatcher to notify when an event occurs.
    */
   virtual void addWatcher(int fd, int events, IOWatcherRef& w);

   /**
    * Removes the passed IOWatcher.
    *
    * @param w the IOWatcher to remove.
    */
   virtual void removeWatcher(IOWatcherRef& w);
};

} // end namespace io
} // end namespace db
#endif
