/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_upnp_ControlPoint_H
#define db_upnp_ControlPoint_H

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
};

} // end namespace upnp
} // end namespace db
#endif
