/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/upnp/ControlPoint.h"

#include "db/io/ByteArrayOutputStream.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/net/http/HttpClient.h"
#include "db/upnp/SoapEnvelope.h"
#include "db/util/StringTools.h"
#include "db/data/xml/DomTypes.h"

using namespace std;
using namespace db::data::xml;
using namespace db::io;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;
using namespace db::upnp;
using namespace db::util;

ControlPoint::ControlPoint()
{
}
ControlPoint::~ControlPoint()
{
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

bool ControlPoint::performAction(
   Action& action, DynamicObject& params,
   Service& service, ActionResult& result)
{
   bool rval = false;
   
   // get the action name
   const char* name = action["actionName"]->getString();
   
   // ensure action exists in service description
   ServiceDescription& desc = service["description"];
   if(!desc->hasMember("actions") || !desc["actions"]->hasMember(name))
   {
      ExceptionRef e = new Exception(
         "Service has no such action.",
         "db.upnp.NoSuchAction");
      e->getDetails()["actionName"] = name;
      e->getDetails()["serviceType"] = service["serviceType"]->getString();
      e->getDetails()["serviceId"] = service["serviceId"]->getString();
      Exception::setLast(e, false);
   }
   else
   {
      // create a soap message
      SoapMessage msg;
      msg["name"] = name;
      msg["namespace"] = service["serviceType"]->getString();
      msg["params"] = params;
      
      // do soap transfer
      rval = doSoap(service, msg, result);
   }
   
   return rval;
}
