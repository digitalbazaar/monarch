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
   // start valid ids at 1
   mNextEventId = 1;
}

EventController::~EventController()
{
}

void EventController::assignEventId(const char* eventType)
{
   if(!mTypeMap->hasMember(eventType))
   {
      // event type not assigned an event id, create one & increment
      mTypeMap[eventType] = mNextEventId++;
   }
}

void EventController::registerObserver(Observer* observer, const char* type)
{
   // assign an id, if necessary
   assignEventId(type);
      
   // register the observer
   Observable::registerObserver(
      observer, mTypeMap[type]->getUInt64());
}

void EventController::registerObserver(
   Observer* observer, DynamicObject& eventTypes)
{
   DynamicObjectIterator index = eventTypes.getIterator();
   while(index->hasNext())
   {
      // register all types
      DynamicObject type = index->next();
      registerObserver(observer, type->getString());
   }
}

void EventController::unregisterObserver(Observer* observer, const char* type)
{
   Observable::unregisterObserver(
         observer, mTypeMap[type]->getUInt64());
}

void EventController::unregisterObserver(
   Observer* observer, DynamicObject& eventTypes)
{
   DynamicObjectIterator index = eventTypes.getIterator();
   while(index->hasNext())
   {
      DynamicObject type = index->next();
      unregisterObserver(observer, type->getString());
   }
}

void EventController::addParent(
   const char* child, const char* parent)
{
   if(mTypeMap->hasMember(parent) && mTypeMap->hasMember(child))
   {
      Observable::addTap(
         mTypeMap[child]->getUInt64(), mTypeMap[parent]->getUInt64());
   }
}

void EventController::removeParent(
   const char* child, const char* parent)
{
   if(mTypeMap->hasMember(parent) && mTypeMap->hasMember(child))
   {
      Observable::removeTap(
         mTypeMap[child]->getUInt64(), mTypeMap[parent]->getUInt64());
   }
}

void EventController::schedule(Event& event)
{
   if(mTypeMap->hasMember(event["type"]->getString()))
   {
      Observable::schedule(
         event, mTypeMap[event["type"]->getString()]->getUInt64());
   }
}
