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

// forward declare WebConnection
class WebConnection;

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
};

} // end namespace net
} // end namespace db
#endif
