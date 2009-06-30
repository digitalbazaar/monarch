/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/upnp/SoapEnvelope.h"

#include "db/data/xml/DomReader.h"
#include "db/data/xml/DomWriter.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/io/ByteArrayOutputStream.h"

using namespace std;
using namespace db::data::xml;
using namespace db::io;
using namespace db::rt;
using namespace db::upnp;

#define SOAP_NS_URI        "http://schemas.xmlsoap.org/soap/envelope"
#define SOAP_NS_PREFIX     "soap"
#define ENC_STYLE_NS_URI   "http://schemas.xmlsoap.org/soap/encoding"
#define TARGET_NS_PREFIX   "m"

SoapEnvelope::SoapEnvelope()
{
}
SoapEnvelope::~SoapEnvelope()
{
}

string SoapEnvelope::create(SoapOperation& soapOp)
{
   string envelope;
   
   // create root element
   Element root;
   root["name"] = "Envelope";
   root["namespace"] = SOAP_NS_URI;
   // add soap namespace attribute
   {
      Attribute attr;
      attr["name"] = "xmlns:" SOAP_NS_PREFIX;
      attr["value"] = SOAP_NS_URI;
      root["attributes"][attr["name"]->getString()] = attr;
   }
   // add encoding style attribute
   {
      Attribute attr;
      attr["name"] = "encodingStyle";
      attr["namespace"] = SOAP_NS_URI;
      attr["value"] = ENC_STYLE_NS_URI;
      root["attributes"][attr["name"]->getString()] = attr;
   }
   
   // add body element
   Element body;
   body["name"] = "Body";
   body["namespace"] = SOAP_NS_URI;
   root["children"]["Body"]->append(body);
   // add target namespace attribute
   {
      Attribute attr;
      attr["name"] = "xmlns:" TARGET_NS_PREFIX;
      attr["value"] = soapOp["namespace"]->getString();
      body["attributes"][attr["name"]->getString()] = attr;
   }
   
   // add message
   Element message;
   message["name"] = soapOp["name"]->getString();
   message["namespace"] = soapOp["namespace"]->getString();
   body["children"][message["name"]->getString()]->append(message);
   
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
      param["data"] = (p->getType() == Boolean) ?
         (p->getBoolean() ? "1" : "0") : p->getString();
      message["children"][param["name"]->getString()]->append(param);
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

// a helper function to convert elements into objects using tag names as keys
static void getResults(Element& e, DynamicObject& result)
{
   // convert any children
   if(e["children"]->length() > 0)
   {
      // each entry in children is an array of elements
      DynamicObjectIterator i = e["children"].getIterator();
      while(i->hasNext())
      {
         DynamicObject& elements = i->next();
         ElementIterator ei = elements.getIterator();
         while(ei->hasNext())
         {
            Element& child = ei->next();
            getResults(child, result[child["name"]->getString()]);
         }
      }
   }
   else
   {
      result = e["data"]->getString();
   }
}

bool SoapEnvelope::parse(InputStream* is, SoapResult& result)
{
   bool rval = false;
   
   // prepare passed result
   result->setType(Map);
   result->clear();
   result["fault"] = false;
   result["result"]->setType(Map);
   
   //  parse result
   Element root;
   DomReader reader;
   reader.start(root);
   if((rval = reader.read(is) && reader.finish()))
   {
      // ensure there is a body in the response
      rval = false;
      root["children"]->setType(Array);
      if(root["children"]->hasMember("Body"))
      {
         // ensure body is valid
         Element& body = root["children"]["Body"][0];
         if(body["children"]->length() == 1)
         {
            // see if the body contains a soap fault or message
            Element faultOrMessage = body["children"].first();
            if(faultOrMessage.isNull())
            {
               // body is valid
               rval = true;
               
               // is a fault if name is "Fault" and namespace is
               // the same as the body element
               if(strcmp(
                  faultOrMessage[0]["name"]->getString(), "Fault") == 0 &&
                  faultOrMessage[0]["namespace"] == body["namespace"])
               {
                  result["fault"] = true;
               }
               
               // get results from response message
               getResults(faultOrMessage[0], result["result"]);
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
