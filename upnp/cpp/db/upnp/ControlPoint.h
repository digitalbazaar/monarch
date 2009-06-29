/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_upnp_ControlPoint_H
#define db_upnp_ControlPoint_H

#include "db/upnp/TypeDefinitions.h"

namespace db
{
namespace upnp
{

/**
 * A UPnP Control Point. This is a single point that is capable of controlling
 * UPnP devices via their UPnP services.
 * 
 * @author Dave Longley
 */
class ControlPoint
{
public:
   /**
    * Creates a new ControlPoint.
    */
   ControlPoint();
   
   /**
    * Destructs this ControlPoint.
    */
   virtual ~ControlPoint();
   
   /**
    * Performs a UPnP action on a UPnP service.
    * 
    * @param action the UPnP action to perform.
    * @param params the parameters for the action (as a name=value map).
    * @param service the service to perform the action on.
    * @param result the map to put the return values in.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool performAction(
      Action& action, db::rt::DynamicObject& params,
      Service& service, ActionResult& result);
};

} // end namespace upnp
} // end namespace db
#endif
