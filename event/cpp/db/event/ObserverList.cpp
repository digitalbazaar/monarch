/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/event/ObserverList.h"

using namespace std;
using namespace db::event;
using namespace db::rt;

ObserverList::ObserverList()
{
}

ObserverList::~ObserverList()
{
}

void ObserverList::add(Observer* observer)
{
   mObservers.push_back(observer);
}

void ObserverList::add(ObserverRef& observer)
{
   mObserverRefs.push_back(observer);
   mObservers.push_back(&(*observer));
}

void ObserverList::unregisterFrom(Observable* observable)
{
   for(std::list<Observer*>::iterator i = mObservers.begin();
       i != mObservers.end(); i++)
   {
      observable->unregisterObserver(*i);
   }
}

void ObserverList::unregisterFrom(Observable* observable, EventId id)
{
   for(std::list<Observer*>::iterator i = mObservers.begin();
       i != mObservers.end(); i++)
   {
      observable->unregisterObserver(*i, id);
   }
}

void ObserverList::unregisterFrom(EventController* ec)
{
   for(std::list<Observer*>::iterator i = mObservers.begin();
       i != mObservers.end(); i++)
   {
      ec->unregisterObserver(*i);
   }
}

void ObserverList::unregisterFrom(EventController* ec, const char* type)
{
   for(std::list<Observer*>::iterator i = mObservers.begin();
       i != mObservers.end(); i++)
   {
      ec->unregisterObserver(*i, type);
   }
}

void ObserverList::unregisterFrom(
   EventController* ec, DynamicObject& eventTypes)
{
   for(std::list<Observer*>::iterator i = mObservers.begin();
       i != mObservers.end(); i++)
   {
      ec->unregisterObserver(*i, eventTypes);
   }
}

void ObserverList::clear()
{
   mObservers.clear();
   mObserverRefs.clear();
}
