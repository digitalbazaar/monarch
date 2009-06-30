/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_upnp_SoapEnvelope_H
#define db_upnp_SoapEnvelope_H

#include "db/io/InputStream.h"
#include "db/rt/DynamicObject.h"

#include <string>

namespace db
{
namespace upnp
{

/**
 * A SoapOperation is a remote procedural call that uses SOAP. It can be
 * serialized to XML.
 * 
 * @param name the name for the operation.
 * @param namespace the namespace URI that defines the XML elements.
 * @param params the parameters for the operation.
 */
typedef db::rt::DynamicObject SoapOperation;

/**
 * A SoapResult is the result of a SoapOperation. It may be a fault or
 * a map return values.
 * 
 * @param fault a boolean that is set to true if the result is a fault.
 * @param result a map containing the fault details or the return values
 *               as name=value pairs. 
 */
typedef db::rt::DynamicObject SoapResult;

/**
 * A SoapEnvelope object can create and parse SOAP envelopes that are used
 * to communicate using SOAP.
 * 
 * @author Dave Longley
 */
class SoapEnvelope
{
public:
   /**
    * Creates a new SoapEnvelope.
    */
   SoapEnvelope();
   
   /**
    * Destructs this SoapEnvelope.
    */
   virtual ~SoapEnvelope();
   
   /**
    * Produces a soap envelope that contains a soap operation.
    * 
    * @param soapOp the soap operation to put in the envelope.
    * 
    * @return the envelope as a string or a string of length 0 on error.
    */
   virtual std::string create(SoapOperation& soapOp);
   
   /**
    * Parses a soap envelope from an input stream.
    * 
    * @param is the input stream to read the soap envelope from.
    * @param result the result to populate.
    * 
    * @return true if successful, false if not.
    */
   virtual bool parse(db::io::InputStream* is, SoapResult& result);
};

} // end namespace upnp
} // end namespace db
#endif
