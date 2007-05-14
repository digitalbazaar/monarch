/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef WebResponse_H
#define WebResponse_H

#include "Object.h"

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
class WebResponse : public virtual db::rt::Object
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
};

} // end namespace net
} // end namespace db
#endif
