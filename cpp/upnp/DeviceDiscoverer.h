/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_upnp_DeviceDiscoverer_H
#define monarch_upnp_DeviceDiscoverer_H

#include "monarch/upnp/TypeDefinitions.h"

namespace monarch
{
namespace upnp
{

/**
 * A DeviceDiscoverer is used to discover UPnP devices on the network.
 *
 * @author Dave Longley
 */
class DeviceDiscoverer
{
public:
   /**
    * Creates a new DeviceDiscoverer.
    */
   DeviceDiscoverer();

   /**
    * Destructs this DeviceDiscoverer.
    */
   virtual ~DeviceDiscoverer();

   /**
    * Discovers UPnP devices on the network during the specified timeout.
    *
    * An SSDP message will be sent to discover UPnP devices of a specific
    * type. The passed search target URI can be "ssdb:all", "upnp:rootdevice",
    * or a device type or UDN.
    *
    * @param devices a list to populate with devices.
    * @param searchTarget the search target URI that specifies the type of
    *                     devices to look for.
    * @param timeout how long this discoverer should wait to find devices,
    *                in milliseconds.
    * @param count the maximum number of devices to find before returning,
    *              0 to wait until until the timeout is reached.
    *
    * @return the number of devices found, -1 if an exception occurred.
    */
   virtual int discover(
      DeviceList& devices,
      const char* searchTarget, uint32_t timeout, int count);
};

} // end namespace upnp
} // end namespace monarch
#endif
