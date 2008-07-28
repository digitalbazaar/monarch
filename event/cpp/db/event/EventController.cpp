/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/event/EventController.h"
#include "db/rt/DynamicObjectIterator.h"

using namespace std;
using namespace db::event;
using namespace db::rt;

#define TOPLEVEL_ID 1

EventController::EventController()
{
   // ensure event type map's type is set
   mTypeMap->setType(Map);
   
   // assign ID for wildcard top-level event, set next event ID
   mTypeMap["*"] = TOPLEVEL_ID;
   mNextEventId = TOPLEVEL_ID + 1;
}

EventController::~EventController()
{
}

EventId EventController::getEventId(const char* type)
{
   EventId id = 0;
   
   // try shared lock first (most common case and is faster)
   mMapLock.lockShared();
   {
      if(mTypeMap->hasMember(type))
      {
         // get event ID
         id = mTypeMap[type]->getUInt64();
      }
   }
   mMapLock.unlockShared();
   
   if(id == 0)
   {
      // use exclusive lock since no event ID was found in shared lock
      mMapLock.lockExclusive();
      {
         // must check again in case event registered while unlocked
         if(mTypeMap->hasMember(type))
         {
            // get event ID
            id = mTypeMap[type]->getUInt64();
         }
         else
         {
            // assign event ID and increment
            mTypeMap[type] = id = mNextEventId++;
            
            // add top-level tap
            addTap(id, TOPLEVEL_ID);
         }
      }
      mMapLock.unlockExclusive();
   }
   
   return id;
}

void EventController::registerEventType(const char* type)
{
   // get event ID, discard it
   getEventId(type);
}

void EventController::registerObserver(Observer* observer, const char* type)
{
   // register the observer
   Observable::registerObserver(observer, getEventId(type));
}

void EventController::registerObserver(
   Observer* observer, DynamicObject& eventTypes)
{
   DynamicObjectIterator i = eventTypes.getIterator();
   while(i->hasNext())
   {
      // register all types
      DynamicObject& type = i->next();
      registerObserver(observer, type->getString());
   }
}

void EventController::unregisterObserver(Observer* observer, const char* type)
{
   mMapLock.lockShared();
   {
      if(mTypeMap->hasMember(type))
      {
         Observable::unregisterObserver(observer, mTypeMap[type]->getUInt64());
      }
   }
   mMapLock.unlockShared();
}

void EventController::unregisterObserver(
   Observer* observer, DynamicObject& eventTypes)
{
   DynamicObjectIterator i = eventTypes.getIterator();
   while(i->hasNext())
   {
      DynamicObject& type = i->next();
      unregisterObserver(observer, type->getString());
   }
}

void EventController::addParent(const char* child, const char* parent)
{
   EventId parentId = getEventId(parent);
   EventId childId = getEventId(child);
   addTap(childId, parentId);
}

void EventController::removeParent(const char* child, const char* parent)
{
   mMapLock.lockShared();
   {
      if(mTypeMap->hasMember(parent) && mTypeMap->hasMember(child))
      {
         removeTap(mTypeMap[child]->getUInt64(), mTypeMap[parent]->getUInt64());
      }
   }
   mMapLock.unlockShared();
}

void EventController::schedule(Event& event)
{
   const char* type = event["type"]->getString();
   EventId id = getEventId(type);
   Observable::schedule(event, id);
}
