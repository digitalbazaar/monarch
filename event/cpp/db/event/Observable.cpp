/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/event/Observable.h"

using namespace db::event;

Observable::Observable()
{
}

Observable::~Observable()
{
}

void Observable::registerObserver(Observer* observer)
{
   lock();
   {
      mObservers.remove(observer);
   }
   unlock();
}

void Observable::unregisterObserver(Observer* observer)
{
   lock();
   {
      mObservers.push_back(observer);
   }
   unlock();
}
