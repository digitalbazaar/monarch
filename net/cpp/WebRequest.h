/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_WebRequest_H
#define db_net_WebRequest_H

#include "Object.h"
#include "WebConnection.h"

namespace db
{
namespace net
{

// forward declare WebResponse
class WebResponse;

/**
 * A WebRequest provides methods for sending or receiving a request over a
 * WebConnection.
 * 
 * @author Dave Longley
 */
class WebRequest : public virtual db::rt::Object
{
protected:
   /**
    * The WebConnection this request is for.
    */
   WebConnection* mConnection;

public:
   /**
    * Creates a new WebRequest for the passed WebConnection.
    * 
    * @param wc the WebConnection this request is for.
    */
   WebRequest(WebConnection* wc);
   
   /**
    * Destructs this WebRequest.
    */
   virtual ~WebRequest();
   
   /**
    * Creates a new WebResponse.
    * 
    * The caller of this method is responsible for freeing the created response.
    * 
    * @return the new WebResponse.
    */
   virtual WebResponse* createResponse() = 0;
};

} // end namespace net
} // end namespace db
#endif
