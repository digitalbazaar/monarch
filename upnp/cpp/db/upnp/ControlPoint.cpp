/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/upnp/ControlPoint.h"

#include "db/data/xml/DomReader.h"
#include "db/data/xml/DomWriter.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/io/ByteArrayOutputStream.h"
#include "db/net/http/HttpClient.h"
#include "db/util/StringTools.h"

using namespace std;
using namespace db::data::xml;
using namespace db::io;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;
using namespace db::upnp;
using namespace db::util;

#define SOAP_NS_URI        "http://schemas.xmlsoap.org/soap/envelope/"
#define SOAP_NS_PREFIX     "soap"
#define ENC_STYLE_NS_URI   "http://schemas.xmlsoap.org/soap/encoding/"
#define TARGET_NS_PREFIX   "m"

ControlPoint::ControlPoint()
{
}
ControlPoint::~ControlPoint()
{
}

/**
 * A helper function to produce a soap envelope as a string.
 * 
 * @param soapOp the soap operation to put in the envelope.
 * 
 * @return the envelope as a string or a string of length 0 on error.
 */
static string createSoapEnvelope(SoapOperation& soapOp)
{
   string envelope;
   
   // create root element
   Element root;
   root["name"] = "Envelope";
   // add soap namespace attribute
   {
      Attribute& attr = root["attributes"]->append();
      attr["name"] = "xmlns:" SOAP_NS_PREFIX;
      attr["value"] = SOAP_NS_URI;
   }
   // add encoding style attribute
   {
      Attribute& attr = root["attributes"]->append();
      attr["name"] = "encodingStyle";
      attr["namespace"] = SOAP_NS_URI;
      attr["value"] = ENC_STYLE_NS_URI;
   }
   
   // add body element
   Element body;
   body["name"] = "Body";
   root["children"]->append(body);
   // add target namespace attribute
   {
      Attribute& attr = body["attributes"]->append();
      attr["name"] = "xmlns:" TARGET_NS_PREFIX;
      attr["value"] = soapOp["namespace"]->getString();
   }
   
   // add message
   Element message;
   message["name"] = soapOp["name"]->getString();
   message["namespace"] = soapOp["namespace"]->getString();
   body->append(message);
   
   // add message parameters
   DynamicObjectIterator pi = soapOp["params"].getIterator();
   while(pi->hasNext())
   {
      DynamicObject& p = pi->next();
      
      // add param
      Element param;
      param["name"] = pi->getName();
      param["namespace"] = soapOp["namespace"]->getString();
      // use 0/1 for booleans
      param["value"] = (p->getType() == Boolean) ?
         (p->getBoolean() ? "1" : "0") : p->getString();
      message->append(param);
   }
   
   // write envelope to string
   DomWriter writer;
   writer.setCompact(true);
   ByteBuffer bb(1024);
   ByteArrayOutputStream baos(&bb, true);
   if(writer.write(root, &baos))
   {
      envelope.append(bb.data(), bb.length());
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not create soap envelope.",
         "db.upnp.InvalidSoapEnvelope");
      Exception::setLast(e, true);
   }
   
   return envelope;
}

/**
 * Parses the passed soap envelope.
 * 
 * @param is the input stream to read the soap envelope from.
 * @param result the result to populate.
 * 
 * @return true if successful, false if not.
 */
static bool parseEnvelope(InputStream* is, ActionResult& result)
{
   bool rval = false;
   
   //  parse result
   Element root;
   DomReader reader;
   reader.start(root);
   if((rval = reader.read(is) && reader.finish()))
   {
      // ensure there is a body in the response
      rval = false;
      root["children"]->setType(Array);
      ElementIterator children = root["children"].getIterator();
      while(!rval && children->hasNext())
      {
         // ensure body is valid
         Element& child = children->next();
         if(strcmp(child["name"]->getString(), "Body") == 0 &&
            child["children"]->getType() == Array &&
            child["children"]->length() == 1)
         {
            // get parameters of response message
            rval = true;
            Element message = child["children"][0];
            message["children"]->setType(Array);
            ElementIterator pi = message["children"].getIterator();
            while(pi->hasNext())
            {
               Element& param = pi->next();
               result[param["name"]->getString()] =
                  param["value"]->getString();
            }
         }
      }
      
      if(!rval)
      {
         ExceptionRef e = new Exception(
            "Invalid or no soap body found in response envelope.",
            "db.upnp.InvalidSoapEnvelope");
         Exception::setLast(e, false);
      }
   }
   
   return rval;
}

/**
 * A helper function that sends a soap envelope and gets its result.
 * 
 * @param service the service to connect to.
 * @param soapOp the soap operation to perform.
 * @param result the result to populate.
 * 
 * @return true if successful, false if an exception occurred.
 */
static bool doSoap(
   Service& service, SoapOperation& soapOp, ActionResult& result)
{
   bool rval = false;
   
   // create the soap envelope
   string envelope = createSoapEnvelope(soapOp);
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
            soapOp["name"]->getString()).c_str();
         
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
               // parse response
               ByteArrayInputStream bais2(&bb);
               rval = parseEnvelope(&bais2, result);
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
      // create a soap envelope
      SoapOperation soapOp;
      soapOp["name"] = name;
      soapOp["namespace"] = service["serviceType"]->getString();
      soapOp["params"] = params;
      
      // do soap transfer
      rval = doSoap(service, soapOp, result);
   }
   
   return rval;
}
