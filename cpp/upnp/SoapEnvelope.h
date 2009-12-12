/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_upnp_SoapEnvelope_H
#define db_upnp_SoapEnvelope_H

#include "monarch/io/InputStream.h"
#include "monarch/rt/DynamicObject.h"

#include <string>

namespace db
{
namespace upnp
{

/**
 * A SoapMessage is a remote procedural call (RCP) or a response to an RCP
 * that uses SOAP. It can be serialized to XML.
 *
 * @param name the name for the message.
 * @param namespace the namespace URI that defines the XML elements.
 * @param params the parameters for the message (a map).
 */
typedef monarch::rt::DynamicObject SoapMessage;

/**
 * A SoapResult is the result of a parsed SoapEnvelope. It may or may not
 * be a soap fault.
 *
 * @param fault a boolean that is set to true if the result is a fault.
 * @param message the SoapMessage.
 */
typedef monarch::rt::DynamicObject SoapResult;

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
    * Produces a soap envelope that contains the passed message.
    *
    * @param msg the soap message to put in the envelope.
    *
    * @return the envelope as a string or a string of length 0 on error.
    */
   virtual std::string create(SoapMessage& msg);

   /**
    * Parses a soap envelope from an input stream.
    *
    * @param is the input stream to read the soap envelope from.
    * @param result the result to populate.
    *
    * @return true if successful, false if not.
    */
   virtual bool parse(monarch::io::InputStream* is, SoapResult& result);
};

} // end namespace upnp
} // end namespace db
#endif
