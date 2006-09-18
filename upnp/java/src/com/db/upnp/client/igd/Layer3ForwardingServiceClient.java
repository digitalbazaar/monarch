/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client.igd;

import com.db.net.soap.RpcSoapEnvelope;
import com.db.net.soap.SoapFault;
import com.db.net.soap.SoapOperation;
import com.db.net.soap.SoapOperationParameter;
import com.db.upnp.client.ClientUPnPServiceImplementation;
import com.db.upnp.client.UPnPServiceClient;
import com.db.upnp.device.UPnPDevice;
import com.db.upnp.service.UPnPError;
import com.db.upnp.service.UPnPService;
import com.db.xml.XmlElement;

/**
 * A Layer3ForwardingServiceClient is a client for a Layer3ForwardingService.
 * 
 * @author Dave Longley
 */
public class Layer3ForwardingServiceClient
implements ClientUPnPServiceImplementation
{
   /**
    * The UPnPService this client implementation is for.
    */
   protected UPnPService mService;
   
   /**
    * The UPnPServiceClient that is used to communicate with the UPnPService.
    */
   protected UPnPServiceClient mServiceClient;
   
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
      // store service
      mService = service;
      
      // no service client set yet
      setServiceClient(null);
   }
   
   /**
    * Sets the default connection service for the Internet Gateway with the
    * Layer3Forwarding service.
    * 
    * @param device the device with the new default connection service.
    * @param service the new default connection service.
    * 
    * @return a UPnPError:
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
   public UPnPError setDefaultConnectionService(
      UPnPDevice device, UPnPService service)
   {
      // create a comma-separated 2-tuple string to uniquely identify the
      // passed device and service
      String serviceString =
         device.getUdn() + "," + service.getServiceId();
      
      return setDefaultConnectionService(serviceString);
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
    * @return a UPnPError:
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
   public UPnPError setDefaultConnectionService(String serviceString)
   {
      UPnPError rval = new UPnPError(); 
      
      // create a soap operation
      SoapOperation operation = new SoapOperation(
         "SetDefaultConnectionService", mService.getServiceType());
      
      // add parameters
      operation.addParameter(new SoapOperationParameter(
         "NewDefaultConnectionService", serviceString, null));
      
      // create a rpc soap envelope
      RpcSoapEnvelope envelope = new RpcSoapEnvelope();
      
      // set the operation
      envelope.setSoapOperation(operation);
      
      // send the envelope
      getServiceClient().sendSoapEnvelope(envelope, mService);
      if(envelope.containsSoapFault())
      {
         // pull out the envelope's soap fault
         SoapFault fault = envelope.getSoapFault();
         
         // get the detail
         XmlElement detail = fault.getFaultDetail();
         if(detail != null)
         {
            rval.convertFromXmlElement(detail);
         }
      }
      
      return rval;
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
    * @return the default connection service from the Layer3Forwarding service
    *         or an error code:
    * 
    * "402 Invalid Args" One of following: not enough IN arguments, too many IN
    * arguments, no IN argument by that name, one or more IN arguments are of
    * the wrong data type.
    * 
    * "501 Action Failed" May be returned in current state if service prevents
    * invoking of that action.
    */
   public String getDefaultConnectionService()
   {
      String rval = "0";
      
      // create a soap operation
      SoapOperation operation = new SoapOperation(
         "GetDefaultConnectionService", mService.getServiceType());
      
      // create a rpc soap envelope
      RpcSoapEnvelope envelope = new RpcSoapEnvelope();
      
      // set the operation
      envelope.setSoapOperation(operation);
      
      // send the envelope
      getServiceClient().sendSoapEnvelope(envelope, mService);
      if(envelope.containsSoapOperation())
      {
         // pull out the envelope's soap operation
         operation = envelope.getSoapOperation();
         
         // get the parameter in the operation
         if(operation.getParameters().size() == 1)
         {
            // get the result
            SoapOperationParameter result =
               (SoapOperationParameter)operation.getParameters().get(0);
            rval = result.getValue();
         }
      }
      else if(envelope.containsSoapFault())
      {
         // pull out the envelope's soap fault
         SoapFault fault = envelope.getSoapFault();
         
         // get the detail
         XmlElement detail = fault.getFaultDetail();
         if(detail != null)
         {
            UPnPError error = new UPnPError();
            error.convertFromXmlElement(detail);
            rval = error.getErrorCode() + " " + error.getErrorDescription();
         }
      }
      
      return rval;      
   }
   
   /**
    * Sets the UPnPServiceClient to use to communicate with the service.
    * 
    * @param client the UPnPServiceClient to use to communicate with the
    *               service.
    */
   public void setServiceClient(UPnPServiceClient client)
   {
      mServiceClient = client;
   }
   
   /**
    * Gets the UPnPServiceClient to use to communicate with the service.
    * 
    * @return the UPnPServiceClient to use to communicate with the service.
    */
   public UPnPServiceClient getServiceClient()
   {
      return mServiceClient;
   }
}
