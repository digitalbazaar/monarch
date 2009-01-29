/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_fiber_FiberMessageCenter_H
#define db_fiber_FiberMessageCenter_H

#include "db/fiber/MessagableFiber.h"
#include "db/rt/SharedLock.h"

#include <map>

namespace db
{
namespace fiber
{

/**
 * A FiberMessageCenter distributes messages to MessagableFibers.
 * 
 * @author Dave Longley
 */
class FiberMessageCenter
{
protected:
   /**
    * A map of registered FiberIds to MessagableFibers.
    */
   typedef std::map<FiberId, MessagableFiber*> FiberMap;
   FiberMap mFibers;
   
   /**
    * A lock for registering/unregistering/messaging fibers.
    */
   db::rt::SharedLock mMessageLock;
   
public:
   /**
    * Creates a new FiberMessageCenter.
    */
   FiberMessageCenter();
   
   /**
    * Destructs this FiberMessageCenter.
    */
   virtual ~FiberMessageCenter();
   
   /**
    * Registers a fiber with this message center so it can receive messages.
    * This method is called automatically by MessagableFibers when they
    * start up, however, it can be manually called as well if messages need
    * to be delivered prior to the fiber starting. Note that the fiber must
    * have already been assigned a fiber ID, i.e. via addFiber() on a
    * FiberScheduler. 
    * 
    * @param fiber the fiber to register with this message center.
    */
   virtual void registerFiber(MessagableFiber* fiber);
   
   /**
    * Unregisters a fiber with this message center. This method is called
    * automatically by MessagableFibers before they exit.
    * 
    * @param fiber the fiber to unregister with this message center.
    */
   virtual void unregisterFiber(MessagableFiber* fiber);
   
   /**
    * Sends a message to a registered fiber with the given ID.
    * 
    * @param id the ID of the fiber to send the message to.
    * @param msg the message to send.
    * 
    * @return true if the message was delivered, false if no such fiber exists.
    */
   virtual bool sendMessage(FiberId id, db::rt::DynamicObject& msg);
};

} // end namespace fiber
} // end namespace db
#endif
