/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/upnp/ControlPoint.h"

#include "db/data/xml/DomReader.h"
#include "db/io/ByteArrayOutputStream.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/net/http/HttpClient.h"
#include "db/upnp/SoapEnvelope.h"
#include "db/util/StringTools.h"

using namespace std;
using namespace db::data::xml;
using namespace db::io;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;
using namespace db::upnp;
using namespace db::util;

#define DEVICE_TYPE_IGD \
   "urn:schemas-upnp-org:device:InternetGatewayDevice:1"
#define DEVICE_TYPE_WAN \
   "urn:schemas-upnp-org:device:WANDevice:1"
#define DEVICE_TYPE_WAN_CONNECTION \
   "urn:schemas-upnp-org:device:WANConnectionDevice:1"
#define SERVICE_TYPE_WAN_IP_CONNECTION \
   "urn:schemas-upnp-org:service:WANIPConnection:1"

#define UPNPERROR_NoSuchEntryInArray 714

ControlPoint::ControlPoint()
{
}
ControlPoint::~ControlPoint()
{
}

bool ControlPoint::getDescription(Url* url, string& description)
{
   bool rval = false;
   
   // do http connection
   HttpClient client;
   if((rval = client.connect(url)))
   {
      // create special headers
      DynamicObject headers;
      headers["Connection"] = "close";
      
      // do get
      Url path(url->getPath());
      HttpResponse* response = client.get(&path, &headers);
      if((rval = (response != NULL)))
      {
         // FIXME: ensure response code is appropriate, etc.
         
         // receive response
         ByteBuffer bb(2048);
         ByteArrayOutputStream baos(&bb, true);
         if((rval = client.receiveContent(&baos)))
         {
            // get description
            description.erase();
            description.append(bb.data(), bb.length());
         }
      }
      
      // disconnect
      client.disconnect();
   }
   
   return rval;
}

// a helper function to parse devices or sub-devices
static void parseDevice(Device& device, Element& root)
{
   // get basic device info
   device["deviceType"] =
      root["children"]["deviceType"][0]["data"]->getString();
   device["manufacturer"] =
      root["children"]["manufacturer"][0]["data"]->getString();
   device["manufacturerURL"] =
      root["children"]["manufacturerURL"][0]["data"]->getString();
   device["modelDescription"] =
      root["children"]["modelDescription"][0]["data"]->getString();
   device["modelName"] =
      root["children"]["modelName"][0]["data"]->getString();
   device["modelNumber"] =
      root["children"]["modelNumber"][0]["data"]->getString();
   device["modelURL"] =
      root["children"]["modelURL"][0]["data"]->getString();
   device["serialNumber"] =
      root["children"]["serialNumber"][0]["data"]->getString();
   device["UDN"] =
      root["children"]["UDN"][0]["data"]->getString();
   device["UPC"] =
      root["children"]["UPC"][0]["data"]->getString();
   
   // initialize sub-devices and services
   DeviceList& deviceList = device["devices"];
   deviceList->setType(Array);
   ServiceList& serviceList = device["services"];
   serviceList->setType(Array);
   
   // parse out services
   if(root["children"]->hasMember("serviceList"))
   {
      Element& sl = root["children"]["serviceList"][0];
      DynamicObjectIterator si = sl["children"]["service"].getIterator();
      while(si->hasNext())
      {
         Element& service = si->next();
         
         // get service information
         Service s;
         s["serviceType"] =
            service["children"]["serviceType"][0]["data"]->getString();
         s["serviceId"] =
            service["children"]["serviceId"][0]["data"]->getString();
         s["SCPDURL"] =
            service["children"]["SCPDURL"][0]["data"]->getString();
         s["controlURL"] =
            service["children"]["controlURL"][0]["data"]->getString();
         s["eventSubURL"] =
            service["children"]["eventSubURL"][0]["data"]->getString();
         
         // add service to device
         serviceList->append(s);
      }
   }   
   
   // parse out devices
   if(root["children"]->hasMember("deviceList"))
   {
      Element& dl = root["children"]["deviceList"][0];
      DynamicObjectIterator di = dl["children"]["device"].getIterator();
      while(di->hasNext())
      {
         Element& dev = di->next();
         
         // parse sub-device information
         Device d;
         parseDevice(d, dev);
         
         // add device to device list
         deviceList->append(d);
      }
   }
}

bool ControlPoint::getDeviceDescription(Device& device)
{
   bool rval = false;
   
   // get the location url for the device
   Url url(device["location"]->getString());
   
   // get description
   string description;
   rval = getDescription(&url, description);
   if(rval)
   {
      //  parse result
      Element root;
      DomReader reader;
      ByteArrayInputStream bais(description.c_str(), description.length());
      reader.start(root);
      if((rval = reader.read(&bais) && reader.finish()))
      {
         // parse device
         parseDevice(device, root);
      }
   }
   
   return rval;
}

bool ControlPoint::getServiceDescription(Service& service)
{
   bool rval = false;
   
   // get the description url for the service
   Url url(service["SCPDURL"]->getString());
   
   // get description
   string description;
   rval = getDescription(&url, description);
   if(rval)
   {
      //  parse result
      Element root;
      DomReader reader;
      ByteArrayInputStream bais(description.c_str(), description.length());
      reader.start(root);
      if((rval = reader.read(&bais) && reader.finish()))
      {
         // save description in service
         service["description"] = root;
         service["actions"]->setType(Array);
         
         // parse out actions
         if(root["children"]->hasMember("actionList"))
         {
            Element& actionList = root["children"]["actionList"][0];
            if(actionList["children"]->hasMember("action"))
            {
               Element& action = actionList["children"]["action"][0];
               
               // get action basics
               Action a;
               a["name"] =
                  action["children"]["name"][0]["data"]->getString();
               a["argumentList"]->setType(Array);
               
               // add action arguments
               Element& argList = action["children"]["argumentList"][0];
               DynamicObjectIterator argi =
                  argList["children"]["argument"].getIterator();
               while(argi->hasNext())
               {
                  DynamicObject& argument = argi->next();
                  ActionArgument arg;
                  arg["name"] =
                     argument["children"]["name"][0["data"]]->getString();
                  arg["direction"] =
                     argument["children"]["direction"][0]["data"]->getString();
                  arg["retval"] =
                     argument["children"]["retval"][0]["data"]->getString();
                  a["argumentList"]->append(arg);
               }
               
               // add action to service
               service["actions"]->append(a);
            }
         }
      }
   }
   
   return rval;
}

/**
 * A helper function that sends a soap envelope and gets its result.
 * 
 * @param service the service to connect to.
 * @param msg the soap message to send.
 * @param result the result to populate.
 * 
 * @return true if successful, false if an exception occurred.
 */
static bool doSoap(
   Service& service, SoapMessage& msg, ActionResult& result)
{
   bool rval = false;
   
   // create the soap envelope
   SoapEnvelope env;
   string envelope = env.create(msg);
   if(envelope.length() > 0)
   {
      // get the control url for the service
      Url url(service["controlURL"]->getString());
      
      // do http connection
      HttpClient client;
      if((rval = client.connect(&url)))
      {
         // create special headers
         DynamicObject headers;
         headers["Connection"] = "close";
         headers["Content-Length"] = envelope.length();
         headers["Content-Type"] = "text/xml; charset=\"utf-8\"";
         headers["Soapaction"] = StringTools::format("\"%s#%s\"",
            service["serviceType"]->getString(),
            msg["name"]->getString()).c_str();
         
         // do post
         Url path(url.getPath());
         ByteArrayInputStream bais(envelope.c_str(), envelope.length());
         HttpResponse* response = client.post(&path, &headers, &bais);
         if((rval = (response != NULL)))
         {
            // FIXME: ensure response code is appropriate, etc.
            
            // receive response
            ByteBuffer bb(1024);
            ByteArrayOutputStream baos(&bb, true);
            if((rval = client.receiveContent(&baos)))
            {
               // parse soap response
               ByteArrayInputStream bais2(&bb);
               SoapResult sr;
               rval = env.parse(&bais2, sr);
               if(!rval)
               {
                  // failure to parse response
                  ExceptionRef e = new Exception(
                     "Could not parse soap response.",
                     "db.upnp.InvalidSoapResponse");
                  Exception::setLast(e, true);
               }
               else if(sr["fault"]->getBoolean())
               {
                  // soap fault received
                  ExceptionRef e = new Exception(
                     "Soap fault received.",
                     "db.upnp.SoapFault");
                  e->getDetails()["fault"] = sr["result"];
                  Exception::setLast(e, false);
                  rval = false;
               }
               else
               {
                  // return result as message parameters
                  result = sr["result"]["message"]["params"];
               }
            }
         }
         
         // disconnect
         client.disconnect();
      }
   }
   
   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not perform soap transfer.",
         "db.upnp.SoapTransferError");
      Exception::setLast(e, true);
   }
   
   return rval;
}

Service ControlPoint::getWanIpConnectionService(Device& igd)
{
   /* Note:
    * 
    * An InternetGatewayDevice has a WANDevice in it. Inside the WANDevice
    * there is a WANConnectionDevice. The WANConnectionDevice may have a
    * WANIPConnectionServices which provides port mapping services.
    */
   
   // get the wan device
   Device wd(NULL);
   {
      igd["devices"]->setType(Array);
      DeviceIterator di = igd["devices"].getIterator();
      while(wd.isNull() && di->hasNext())
      {
         Device& next = di->next();
         if(strcmp(next["deviceType"]->getString(), DEVICE_TYPE_WAN) == 0)
         {
            // found wan device
            wd = next;
         }
      }
   }
   
   // get the wan connection device
   Device wcd(NULL);
   if(!wd.isNull())
   {
      wd["devices"]->setType(Array);
      DeviceIterator di = wd["devices"].getIterator();
      while(wcd.isNull() && di->hasNext())
      {
         Device& next = di->next();
         if(strcmp(
            next["deviceType"]->getString(), DEVICE_TYPE_WAN_CONNECTION) == 0)
         {
            // found wan connection device
            wcd = next;
         }
      }
   }
   
   // get the wan ip connection service
   Service wipcs(NULL);
   if(!wcd.isNull())
   {
      wcd["services"]->setType(Array);
      ServiceIterator si = wcd["services"].getIterator();
      while(wipcs.isNull() && si->hasNext())
      {
         Service& next = si->next();
         if(strcmp(
            next["serviceType"]->getString(),
            SERVICE_TYPE_WAN_IP_CONNECTION) == 0)
         {
            // found wan ip connection service
            wipcs = next;
         }
      }
   }
   
   return wipcs;
}

bool ControlPoint::performAction(
   const char* actionName, DynamicObject& params,
   Service& service, ActionResult& result)
{
   bool rval = false;
   
   // ensure action exists in service description
   ServiceDescription& desc = service["description"];
   if(!desc->hasMember("actions") || !desc["actions"]->hasMember(actionName))
   {
      ExceptionRef e = new Exception(
         "Service has no such action.",
         "db.upnp.NoSuchAction");
      e->getDetails()["actionName"] = actionName;
      e->getDetails()["serviceType"] = service["serviceType"]->getString();
      e->getDetails()["serviceId"] = service["serviceId"]->getString();
      Exception::setLast(e, false);
   }
   else
   {
      // create a soap message
      SoapMessage msg;
      msg["name"] = actionName;
      msg["namespace"] = service["serviceType"]->getString();
      msg["params"] = params;
      
      // do soap transfer
      rval = doSoap(service, msg, result);
   }
   
   return rval;
}

bool ControlPoint::addPortMapping(PortMapping& pm, Service& wipcs)
{
   bool rval = false;
   
   // perform the action
   ActionResult result;
   rval = performAction("AddPortMapping", pm, wipcs, result);
   
   return rval;
}

bool ControlPoint::removePortMapping(PortMapping& pm, Service& wipcs)
{
   bool rval = false;
   
   // only these 3 parameters must be sent
   PortMapping pm2;
   pm2["RemoteHost"] = pm["RemoteHost"];
   pm2["ExternalPort"] = pm["ExternalPort"];
   pm2["Protocol"] = pm["Protocol"];
   
   // perform the action
   ActionResult result;
   rval = performAction("DeletePortMapping", pm2, wipcs, result);
   
   return rval;
}

bool ControlPoint::getPortMapping(PortMapping& pm, int index, Service& wipcs)
{
   bool rval = false;
   
   // perform the action
   ActionResult result;
   DynamicObject params;
   params["NewPortMappingIndex"] = index;
   rval = performAction("GetGenericPortMappingEntry", params, wipcs, result);
   if(rval)
   {
      pm = result["message"]["params"];
   }
   // handle soap fault
   else if(result["fault"]->getBoolean())
   {
      params = result["message"]["params"];
      DynamicObject& upnpError = params["detail"]["UPnPError"];
      if(upnpError["errorCode"]->getInt32() == UPNPERROR_NoSuchEntryInArray)
      {
         // no such entry, return null port mapping
         pm.setNull();
         rval = true;
      }
   }
   
   return rval;
}

bool ControlPoint::getPortMapping(PortMapping& pm, Service& wipcs)
{
   bool rval = false;
   
   // only these 3 parameters must be sent
   PortMapping pm2;
   pm2["RemoteHost"] = pm["RemoteHost"];
   pm2["ExternalPort"] = pm["ExternalPort"];
   pm2["Protocol"] = pm["Protocol"];
   
   // perform the action
   ActionResult result;
   rval = performAction("GetSpecificPortMappingEntry", pm2, wipcs, result);
   if(rval)
   {
      pm = result["message"]["params"];
   }
   // handle soap fault
   else if(result["fault"]->getBoolean())
   {
      DynamicObject params = result["message"]["params"];
      DynamicObject& upnpError = params["detail"]["UPnPError"];
      if(upnpError["errorCode"]->getInt32() == UPNPERROR_NoSuchEntryInArray)
      {
         // no such entry, return null port mapping
         pm.setNull();
         rval = true;
      }
   }
   
   return rval;
}
