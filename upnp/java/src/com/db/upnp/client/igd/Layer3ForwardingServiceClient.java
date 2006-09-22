/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client.igd;

import com.db.upnp.client.AbstractClientUPnPServiceImplementation;
import com.db.upnp.device.UPnPDevice;
import com.db.upnp.service.UPnPErrorException;
import com.db.upnp.service.UPnPService;
import com.db.util.BoxingHashMap;

/**
 * A Layer3ForwardingServiceClient is a client for a Layer3ForwardingService.
 * 
 * @author Dave Longley
 */
public class Layer3ForwardingServiceClient
extends AbstractClientUPnPServiceImplementation
{
   /**
    * The service type for a Layer3Forwarding Service.
    */
   public static final String LAYER3_FORWARDING_SERVICE_TYPE =
      "urn:schemas-upnp-org:service:Layer3Forwarding:1";
   
   /**
    * Creates a new Layer3ForwardingServiceClient.
    * 
    * @param service the UPnPService this client implementation is for.
    */
   public Layer3ForwardingServiceClient(UPnPService service)
   {
      super(service);
   }
   
   /**
    * Sets the default connection service for the Internet Gateway with the
    * Layer3Forwarding service.
    * 
    * @param device the device with the new default connection service.
    * @param service the new default connection service.
    * 
    * @exception UPnPErrorException thrown if a UPnPError occurs:
    * 
    * "402 Invalid Args" One of following: not enough IN arguments, too many IN
    * arguments, no IN argument by that name, one or more IN arguments are of
    * the wrong data type.
    * 
    * "501 Action Failed" May be returned in current state if service prevents
    * invoking of that action.
    * 
    * "720 InvalidDeviceUUID" The UUID of a device specified in the action
    * arguments is invalid.
    * 
    * "721 InvalidServiceID" The Service ID of a service specified in the action
    * arguments is invalid.
    * 
    * "723 InvalidConnServiceSelection" The selected connection service
    * instance cannot be set as a default connection.   
    */
   public void setDefaultConnectionService(
      UPnPDevice device, UPnPService service)
   throws UPnPErrorException
   {
      // create a comma-separated 2-tuple string to uniquely identify the
      // passed device and service
      String serviceString =
         device.getUdn() + "," + service.getServiceId();
      
      setDefaultConnectionService(serviceString);
   }
   
   /**
    * Sets the default connection service for the Internet Gateway with the
    * Layer3Forwarding service.
    * 
    * The passed service string must be a comma-separated 2-tuple that uniquely
    * identifies the connection service:
    * 
    * uuid:device-UUID:WANConnectionDevice:v,urn:upnp-org:serviceId:serviceID
    * 
    * An example is:
    * 
    * "uuid:44f5824f-c57d-418c-a131-f22b34e14111:WANConnectionDevice:1,
    * urn:upnporg:serviceId:WANPPPConn1"
    * 
    * @param serviceString the new default connection service.
    * 
    * @exception UPnPErrorException thrown if a UPnPError occurs:
    * 
    * "402 Invalid Args" One of following: not enough IN arguments, too many IN
    * arguments, no IN argument by that name, one or more IN arguments are of
    * the wrong data type.
    * 
    * "501 Action Failed" May be returned in current state if service prevents
    * invoking of that action.
    * 
    * "720 InvalidDeviceUUID" The UUID of a device specified in the action
    * arguments is invalid.
    * 
    * "721 InvalidServiceID" The Service ID of a service specified in the action
    * arguments is invalid.
    * 
    * "723 InvalidConnServiceSelection" The selected connection service
    * instance cannot be set as a default connection.   
    */
   public void setDefaultConnectionService(String serviceString)
   throws UPnPErrorException
   {
      performAction("SetDefaultConnectionService", new Object[]{serviceString});
   }
   
   /**
    * Gets the comma-separated 2-tuple string representing the default
    * connection service for the Internet Gateway the Layer3Forwarding service
    * is for.
    * 
    * The returned service string is a comma-separated 2-tuple that uniquely
    * identifies the connection service:
    * 
    * uuid:device-UUID:WANConnectionDevice:v,urn:upnp-org:serviceId:serviceID
    * 
    * An example is:
    * 
    * "uuid:44f5824f-c57d-418c-a131-f22b34e14111:WANConnectionDevice:1,
    * urn:upnporg:serviceId:WANPPPConn1"
    * 
    * @return the default connection service from the Layer3Forwarding service.
    * 
    * @exception UPnPErrorException thrown if a UPnPError occurs:
    * 
    * "402 Invalid Args" One of following: not enough IN arguments, too many IN
    * arguments, no IN argument by that name, one or more IN arguments are of
    * the wrong data type.
    * 
    * "501 Action Failed" May be returned in current state if service prevents
    * invoking of that action.
    */
   public String getDefaultConnectionService() throws UPnPErrorException
   {
      BoxingHashMap retvals =
         performAction("GetDefaultConnectionService", null);
      
      return retvals.getString("NewDefaultConnectionService");
   }
}
