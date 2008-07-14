/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/IOMonitor.h"

using namespace db::io;

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
