/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/upnp/ControlPoint.h"

#include "db/data/json/JsonWriter.h"
#include "db/data/xml/DomReader.h"
#include "db/io/ByteArrayOutputStream.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/logging/Logging.h"
#include "db/http/HttpClient.h"
#include "db/upnp/SoapEnvelope.h"
#include "db/util/StringTools.h"

using namespace std;
using namespace db::data::json;
using namespace db::data::xml;
using namespace db::http;
using namespace db::io;
using namespace db::logging;
using namespace db::net;
using namespace db::rt;
using namespace db::upnp;
using namespace db::util;

ControlPoint::ControlPoint()
{
}
ControlPoint::~ControlPoint()
{
}

bool ControlPoint::getDescription(Url* url, string& description)
{
   bool rval = false;

   DB_CAT_DEBUG(DB_UPNP_CAT,
      "Getting UPnP description from url '%s'...",
      url->toString().c_str());

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
         DB_CAT_DEBUG(DB_UPNP_CAT,
            "Get UPnP description response header:\n%s",
            response->getHeader()->toString().c_str());

         // receive response
         ByteBuffer bb(2048);
         ByteArrayOutputStream baos(&bb, true);
         if((rval = client.receiveContent(&baos)))
         {
            DB_CAT_DEBUG(DB_UPNP_CAT,
               "Get UPnP description response body:\n%s",
               string(bb.data(), bb.length()).c_str());
            if(response->getHeader()->getStatusCode() < 400)
            {
               // get description
               description.erase();
               description.append(bb.data(), bb.length());
            }
            else
            {
               // error getting description
               ExceptionRef e = new Exception(
                  "HTTP transmission error.",
                  "db.upnp.HttpError");
               e->getDetails()["statusMessage"] =
                  response->getHeader()->getStatusMessage();
               e->getDetails()["statusCode"] =
                  response->getHeader()->getStatusCode();
            }
         }
      }

      // disconnect
      client.disconnect();
   }

   if(!rval)
   {
      DB_CAT_ERROR(DB_UPNP_CAT,
         "Failed to get UPnP description from url '%s': %s",
         url->toString().c_str(),
         JsonWriter::writeToString(
            Exception::getAsDynamicObject()).c_str());
   }

   return rval;
}

// a helper function to parse devices or sub-devices
static void parseDevice(
   Device& device, Element& root, const char* rootUrl, bool sub)
{
   DB_CAT_DEBUG(DB_UPNP_CAT, "Parsing device from xml: %s",
      JsonWriter::writeToString(root).c_str());

   // get basic device info
   Element& rd = sub ? root : root["children"]["device"][0];
   device["rootURL"] = rootUrl;
   device["deviceType"] =
      rd["children"]["deviceType"][0]["data"]->getString();
   device["manufacturer"] =
      rd["children"]["manufacturer"][0]["data"]->getString();
   device["manufacturerURL"] =
      rd["children"]["manufacturerURL"][0]["data"]->getString();
   device["modelDescription"] =
      rd["children"]["modelDescription"][0]["data"]->getString();
   device["modelName"] =
      rd["children"]["modelName"][0]["data"]->getString();
   device["modelNumber"] =
      rd["children"]["modelNumber"][0]["data"]->getString();
   device["modelURL"] =
      rd["children"]["modelURL"][0]["data"]->getString();
   device["serialNumber"] =
      rd["children"]["serialNumber"][0]["data"]->getString();
   device["UDN"] =
      rd["children"]["UDN"][0]["data"]->getString();
   device["UPC"] =
      rd["children"]["UPC"][0]["data"]->getString();

   // initialize sub-devices and services
   DeviceList& deviceList = device["devices"];
   deviceList->setType(Array);
   ServiceList& serviceList = device["services"];
   serviceList->setType(Array);

   // parse out services
   if(rd["children"]->hasMember("serviceList"))
   {
      Element& sl = rd["children"]["serviceList"][0];
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
         s["rootURL"] = rootUrl;

         // add service to device
         serviceList->append(s);
      }
   }

   // parse out devices
   if(rd["children"]->hasMember("deviceList"))
   {
      Element& dl = rd["children"]["deviceList"][0];
      DynamicObjectIterator di = dl["children"]["device"].getIterator();
      while(di->hasNext())
      {
         Element& dev = di->next();

         // parse sub-device information
         Device d;
         parseDevice(d, dev, rootUrl, true);

         // add device to device list
         deviceList->append(d);
      }
   }

   DB_CAT_DEBUG(DB_UPNP_CAT, "Parsed device: %s",
      JsonWriter::writeToString(device).c_str());
}

bool ControlPoint::getDeviceDescription(Device& device)
{
   bool rval = false;

   // get the location url for the device
   Url url(device["location"]->getString());

   // save the root URL
   string rootUrl = url.getScheme();
   rootUrl.append("://");
   rootUrl.append(url.getAuthority());

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
         // parse root device
         parseDevice(device, root, rootUrl.c_str(), false);
      }
   }

   return rval;
}

bool ControlPoint::getServiceDescription(Service& service)
{
   bool rval = false;

   // get the description url for the service
   Url url;
   url.format("%s%s",
      service["rootURL"]->getString(),
      service["SCPDURL"]->getString());

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
         DB_CAT_DEBUG(DB_UPNP_CAT, "Parsing service from xml: %s",
            JsonWriter::writeToString(root).c_str());

         // parse out actions
         service["actions"]->setType(Map);
         if(root["children"]->hasMember("actionList"))
         {
            Element& actionList = root["children"]["actionList"][0];
            if(actionList["children"]->hasMember("action"))
            {
               DynamicObjectIterator ai =
                  actionList["children"]["action"].getIterator();
               while(ai->hasNext())
               {
                  DynamicObject& action = ai->next();

                  // get action basics
                  Action a;
                  a["name"] =
                     action["children"]["name"][0]["data"]->getString();
                  a["arguments"]->setType(Map);
                  a["arguments"]["in"]->setType(Array);
                  a["arguments"]["out"]->setType(Array);

                  // add action arguments
                  if(action["children"]->hasMember("argumentList"))
                  {
                     Element& argList = action["children"]["argumentList"][0];
                     DynamicObjectIterator argi =
                        argList["children"]["argument"].getIterator();
                     while(argi->hasNext())
                     {
                        DynamicObject& argument = argi->next();

                        // build argument
                        DynamicObject name;
                        name =
                           argument["children"]["name"]
                           [0]["data"]->getString();
                        const char* direction =
                           argument["children"]["direction"]
                           [0]["data"]->getString();
                        if(strcmp(direction, "in") == 0)
                        {
                           a["arguments"]["in"]->append(name);
                        }
                        else
                        {
                           a["arguments"]["out"]->append(name);
                        }

                        if(argument["children"]->hasMember("retval"))
                        {
                           a["retval"][name->getString()] =
                              argument["children"]["retval"]
                              [0]["data"]->getString();
                        }
                     }

                     // add action to service
                     service["actions"][a["name"]->getString()] = a;
                  }
               }
            }
         }
      }
   }

   if(rval)
   {
      DB_CAT_DEBUG(DB_UPNP_CAT, "Parsed service: %s",
         JsonWriter::writeToString(service).c_str());
   }

   return rval;
}

/**
 * A helper function that sends a soap envelope and gets its result.
 *
 * @param device the service to connect to.
 * @param service the service to use.
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
      Url url;
      url.format("%s%s",
         service["rootURL"]->getString(),
         service["controlURL"]->getString());

      DB_CAT_DEBUG(DB_UPNP_CAT,
         "Sending SOAP message to url '%s':\n%s",
         url.toString().c_str(), envelope.c_str());

      // do http connection
      HttpClient client;
      if((rval = client.connect(&url)))
      {
         // create special headers
         DynamicObject headers;
         headers["Connection"] = "close";
         headers["Content-Length"] = (uint32_t)envelope.length();
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
            DB_CAT_DEBUG(DB_UPNP_CAT,
               "Received response header:\n%s",
               response->getHeader()->toString().c_str());

            // receive response
            ByteBuffer bb(1024);
            ByteArrayOutputStream baos(&bb, true);
            if((rval = client.receiveContent(&baos)))
            {
               DB_CAT_DEBUG(DB_UPNP_CAT,
                  "Received SOAP message:\n%s",
                  string(bb.data(), bb.length()).c_str());

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
                  Exception::push(e);
               }
               else if(sr["fault"]->getBoolean())
               {
                  // soap fault received
                  ExceptionRef e = new Exception(
                     "Could not perform SOAP transfer. SOAP fault received.",
                     "db.upnp.SoapFault");
                  e->getDetails()["fault"] = sr["message"];
                  Exception::set(e);
                  rval = false;
                  result = sr;
               }
               else
               {
                  // return result as soap result
                  result = sr;
               }
            }
         }

         // disconnect
         client.disconnect();
      }
   }

   if(!rval)
   {
      DB_CAT_ERROR(DB_UPNP_CAT,
         "Could not perform SOAP transfer: %s",
         JsonWriter::writeToString(
            Exception::getAsDynamicObject()).c_str());
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
         if(strcmp(next["deviceType"]->getString(), UPNP_DEVICE_TYPE_WAN) == 0)
         {
            // found wan device
            wd = next;

            DB_CAT_DEBUG(DB_UPNP_CAT,
               "Found device '" UPNP_DEVICE_TYPE_WAN "'");
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
            next["deviceType"]->getString(),
            UPNP_DEVICE_TYPE_WAN_CONNECTION) == 0)
         {
            // found wan connection device
            wcd = next;

            DB_CAT_DEBUG(DB_UPNP_CAT,
               "Found device '" UPNP_DEVICE_TYPE_WAN_CONNECTION "'");
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
            UPNP_SERVICE_TYPE_WAN_IP_CONNECTION) == 0)
         {
            // found wan ip connection service
            wipcs = next;

            DB_CAT_DEBUG(DB_UPNP_CAT,
               "Found service '" UPNP_SERVICE_TYPE_WAN_IP_CONNECTION "'");
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

   // ensure action exists in the service
   if(!service->hasMember("actions") ||
      !service["actions"]->hasMember(actionName))
   {
      ExceptionRef e = new Exception(
         "Service has no such action.",
         "db.upnp.NoSuchAction");
      e->getDetails()["actionName"] = actionName;
      e->getDetails()["serviceType"] = service["serviceType"]->getString();
      e->getDetails()["serviceId"] = service["serviceId"]->getString();
      Exception::set(e);
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
   bool rval = true;

   // if internal client is not specified, get it by connecting to gateway
   if(!pm->hasMember("NewInternalClient") ||
      pm["NewInternalClient"]->length() == 0)
   {
      // get the control url for the service
      Url url;
      url.format("%s%s",
         wipcs["rootURL"]->getString(),
         wipcs["controlURL"]->getString());
      HttpConnectionRef conn = HttpClient::createConnection(&url);
      if(!conn.isNull())
      {
         SocketAddress* addr = conn->getLocalAddress();
         pm["NewInternalClient"] = addr->getAddress();
         conn->close();
      }
      else
      {
         DB_CAT_ERROR(DB_UPNP_CAT,
            "Could not add port mapping, could not connect to '%s'.",
            url.toString().c_str());
         rval = false;
      }
   }

   if(rval)
   {
      DB_CAT_DEBUG(DB_UPNP_CAT,
         "Adding port mapping: %s", JsonWriter::writeToString(pm).c_str());
   }

   // perform the action
   ActionResult result;
   rval = rval && performAction("AddPortMapping", pm, wipcs, result);
   if(!rval)
   {
      DB_CAT_ERROR(DB_UPNP_CAT,
         "Failed to add port mapping: %s, %s",
         JsonWriter::writeToString(pm).c_str(),
         JsonWriter::writeToString(Exception::getAsDynamicObject()).c_str());
   }

   return rval;
}

bool ControlPoint::removePortMapping(PortMapping& pm, Service& wipcs, bool* dne)
{
   bool rval = false;

   DB_CAT_DEBUG(DB_UPNP_CAT,
      "Removing port mapping: %s", JsonWriter::writeToString(pm).c_str());

   // initialize does not exist param
   if(dne != NULL)
   {
      *dne = false;
   }

   // only these 3 parameters must be sent
   PortMapping pm2;
   pm2["NewRemoteHost"] = pm["NewRemoteHost"];
   pm2["NewExternalPort"] = pm["NewExternalPort"];
   pm2["NewProtocol"] = pm["NewProtocol"];

   // perform the action
   ActionResult result;
   rval = performAction("DeletePortMapping", pm2, wipcs, result);
   if(!rval && dne != NULL)
   {
      // handle setting does not exist (dne) parameter so cases where
      // remove if exists can be implemented easily
      DynamicObject params = result["message"]["params"];
      DynamicObject& upnpError = params["detail"]["UPnPError"];
      int32_t code = upnpError["errorCode"]->getInt32();
      if(code == UPNP_ERROR_NoSuchEntryInArray ||
         code == UPNP_ERROR_SpecifiedArrayIndexInvalid)
      {
         // no such entry
         *dne = true;
      }
   }

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
      int32_t code = upnpError["errorCode"]->getInt32();
      if(code == UPNP_ERROR_NoSuchEntryInArray ||
         code == UPNP_ERROR_SpecifiedArrayIndexInvalid)
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
   pm2["NewRemoteHost"] = pm["NewRemoteHost"];
   pm2["NewExternalPort"] = pm["NewExternalPort"];
   pm2["NewProtocol"] = pm["NewProtocol"];

   // perform the action
   ActionResult result;
   rval = performAction("GetSpecificPortMappingEntry", pm2, wipcs, result);
   if(rval)
   {
      pm = result["message"]["params"];
      pm["NewRemoteHost"] = pm2["NewRemoteHost"];
      pm["NewExternalPort"] = pm2["NewExternalPort"];
      pm["NewProtocol"] = pm2["NewProtocol"];
   }
   // handle soap fault
   else if(result["fault"]->getBoolean())
   {
      DynamicObject params = result["message"]["params"];
      DynamicObject& upnpError = params["detail"]["UPnPError"];
      int32_t code = upnpError["errorCode"]->getInt32();
      if(code == UPNP_ERROR_NoSuchEntryInArray ||
         code == UPNP_ERROR_SpecifiedArrayIndexInvalid)
      {
         // no such entry, return null port mapping
         pm.setNull();
         rval = true;
      }
   }

   return rval;
}
