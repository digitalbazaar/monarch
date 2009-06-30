/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_upnp_ControlPoint_H
#define db_upnp_ControlPoint_H

#include "db/net/Url.h"
#include "db/upnp/TypeDefinitions.h"

#include <string>

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
    * Gets a description.
    * 
    * @param url the url to get the description from.
    * @param description the string to populate with the XML description.
    * 
    * @return true if successful, false if not.
    */
   virtual bool getDescription(db::net::Url* url, std::string& description);
   
   /**
    * Gets the description for a device.
    * 
    * @param device the device to get the description for.
    * 
    * @return true if successful, false if not.
    */
   virtual bool getDeviceDescription(Device& device);
   
   /**
    * Gets the description for a service.
    * 
    * @param service the service to get the description for.
    * 
    * @return true if successful, false if not.
    */
   virtual bool getServiceDescription(Service& service);
   
   /**
    * Gets the WANIPConnectionService from an InternetGatewayDevice.
    * 
    * @param igd the internet gateway device.
    * 
    * @return the WANIPConnectionService, NULL if not found.
    */
   virtual Service getWanIpConnectionService(Device& igd);
   
   /**
    * Performs a UPnP action on a UPnP service.
    * 
    * @param actionName the name of the UPnP action to perform.
    * @param params the parameters for the action (as a name=value map).
    * @param service the service to perform the action on.
    * @param result the map to put the return values in.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool performAction(
      const char* actionName, db::rt::DynamicObject& params,
      Service& service, ActionResult& result);
   
   /**
    * Adds a port mapping entry via a wan ip connection service.
    * 
    * This action will create a new port mapping. If the passed external
    * port and PortMapping Protocol part is already mapped to another internal
    * client, then an error will be raised.
    * 
    * Note: Not all NAT implementations will support:
    * Wildcard values (i.e. 0) for the external port
    * Internal port values that are different from the external port
    * Dynamic port mappings (i.e. with finite lease durations)
    * 
    * UPNP error codes:
    * 
    * "402 Invalid Args" One of following: not enough IN arguments, too many IN
    * arguments, no IN argument by that name, one or more IN arguments are of
    * the wrong data type.
    * 
    * "714 NoSuchEntryInArray" The specified array index is out of
    * bounds.
    * 
    * "715 WildCardNotPermittedInSrcIP" The source IP address cannot be
    * wild-carded.
    * 
    * "716 WildCardNotPermittedInExtPort" The external port cannot be
    * wild-carded.
    * 
    * "718 ConflictInMappingEntry" The port mapping entry specified conflicts
    * with a mapping assigned previously to another client.
    * 
    * "724 SamePortValuesRequired" Internal and External port values must be
    * the same.
    * 
    * "725 OnlyPermanentLeasesSupported" The NAT implementation only supports
    * permanent lease times on port mappings.
    * 
    * "726 RemoteHostOnlySupportsWildcard" RemoteHost must be a wildcard and
    * cannot be a specific IP address or DNS name.
    * 
    * "727 ExternalPortOnlySupportsWildcard" ExternalPort must be a wildcard
    * and cannot be a specific port value.
    * 
    * @param pm the port mapping entry.
    * @param wipcs the wan ip connection service.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool addPortMapping(PortMapping& pm, Service& wipcs);
   
   /**
    * Deletes a port mapping entry via a wan ip connection service.
    * 
    * This action will delete a previously created port mapping with the
    * given remote host, external port, and protocol. When an entry is
    * deleted, PortMappingNumberOfEntries decrements.
    * 
    * UPNP error codes:
    * 
    * "402 Invalid Args" One of following: not enough IN arguments, too many IN
    * arguments, no IN argument by that name, one or more IN arguments are of
    * the wrong data type.
    * 
    * "714 NoSuchEntryInArray" There was no entry to delete that matched the
    * passed parameters.
    * 
    * @param pm the port mapping entry.
    * @param wipcs the wan ip connection service.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool removePortMapping(PortMapping& pm, Service& wipcs);
   
   /**
    * Gets a port mapping entry by its index via a wan ip connection service.
    * 
    * This action will retrieve the NAT port mapping with the specified index.
    * A Control point can call this action (incrementally) until null is
    * returned indicating that no entry exists at the given index. All entries
    * are stored in a contiguous array and can be retrieved using an array
    * index that ranges from 0 to PortMappingNumberOfEntries - 1.
    * 
    * UPNP error codes:
    * 
    * "402 Invalid Args" One of following: not enough IN arguments, too many IN
    * arguments, no IN argument by that name, one or more IN arguments are of
    * the wrong data type.
    * 
    * @param pm the port mapping to populate, NULL if no such entry exists.
    * @param index the index in the port mapping entry array at which to
    *              retrieve a generic port mapping entry.
    * @param wipcs the wan ip connection service.
    * 
    * @return true if successful, false if an error occurred.
    */
   virtual bool getPortMapping(PortMapping& pm, int index, Service& wipcs);
   
   /**
    * Gets a specific port mapping entry via a wan ip connection service.
    * 
    * This action will retrieve the NAT port mapping for the given
    * RemoteHost, ExternalPort, and PortMappingProtocol.
    * 
    * UPNP error codes:
    * 
    * "402 Invalid Args" One of following: not enough IN arguments, too many IN
    * arguments, no IN argument by that name, one or more IN arguments are of
    * the wrong data type.
    * 
    * "714 NoSuchEntryInArray" The specified array index is out of
    * bounds.
    * 
    * @param pm the PortMapping with RemoteHost, ExternalPort, and
    *           Protocol set, to be populated with other data or set to NULL.
    * @param wipcs the wan ip connection service.
    * 
    * @return true if successful, false if not.
    */
   virtual bool getPortMapping(PortMapping& pm, Service& wipcs);
};

} // end namespace upnp
} // end namespace db
#endif
