/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/event/EventController.h"
#include "db/rt/DynamicObjectIterator.h"

using namespace std;
using namespace db::event;
using namespace db::rt;

EventController::EventController()
{
   mTypeMap->setType(Map);
   
   // start valid ids at 1
   mNextEventId = 1;
}

EventController::~EventController()
{
}

void EventController::registerEventType(const char* type)
{
   if(!mTypeMap->hasMember(type))
   {
      // event type not assigned an event id, create one & increment
      mTypeMap[type] = mNextEventId++;
   }
}

void EventController::registerObserver(Observer* observer, const char* type)
{
   // register the event type, if necessary
   registerEventType(type);
      
   // register the observer
   Observable::registerObserver(observer, mTypeMap[type]->getUInt64());
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
   if(mTypeMap->hasMember(type))
   {
      Observable::unregisterObserver(observer, mTypeMap[type]->getUInt64());
   }
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
   registerEventType(parent);
   registerEventType(child);
   addTap(mTypeMap[child]->getUInt64(), mTypeMap[parent]->getUInt64());
}

void EventController::removeParent(const char* child, const char* parent)
{
   if(mTypeMap->hasMember(parent) && mTypeMap->hasMember(child))
   {
      removeTap(mTypeMap[child]->getUInt64(), mTypeMap[parent]->getUInt64());
   }
}

void EventController::schedule(Event& event)
{
   const char* type = event["type"]->getString();
   if(mTypeMap->hasMember(type))
   {
      Observable::schedule(event, mTypeMap[type]->getUInt64());
   }
}
