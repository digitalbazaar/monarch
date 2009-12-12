/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/io/IOMonitor.h"

using namespace monarch::io;

IOMonitor::IOMonitor()
{
}

IOMonitor::~IOMonitor()
{
}

void IOMonitor::addWatcher(int fd, int events, IOWatcherRef& w)
{
   // FIXME:

   // FIXME: remove this
   w->fdUpdated(fd, events);
}

void IOMonitor::removeWatcher(IOWatcherRef& w)
{
   // FIXME:
}
