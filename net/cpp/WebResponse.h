/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_WebResponse_H
#define db_net_WebResponse_H

#include "WebConnection.h"

namespace db
{
namespace net
{

// forward declare WebRequest
class WebRequest;

/**
 * A WebResponse provides methods for sending or receiving a response over
 * a WebConnection.
 * 
 * @author Dave Longley
 */
class WebResponse
{
protected:
   /**
    * The WebRequest this response is for.
    */
   WebRequest* mRequest;
   
public:
   /**
    * Creates a new WebResponse for the passed WebRequest.
    * 
    * @param request the WebRequest this response is for.
    */
   WebResponse(WebRequest* request);
   
   /**
    * Destructs this WebResponse.
    */
   virtual ~WebResponse();
   
   /**
    * Gets the WebConnection associated with this response.
    * 
    * @return the WebConnection associated with this response.
    */
   virtual WebConnection* getWebConnection();
};

} // end namespace net
} // end namespace db
#endif
