/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_WebConnectionServicer_H
#define db_net_WebConnectionServicer_H

#include "ConnectionServicer.h"
#include "WebConnection.h"

namespace db
{
namespace net
{

/**
 * A WebConnectionServicer services a WebConnection in some implementation
 * specific fashion.
 * 
 * @author Dave Longley
 */
class WebConnectionServicer
{
public:
   /**
    * Creates a new WebConnectionServicer.
    */
   WebConnectionServicer() {};
   
   /**
    * Destructs this WebConnectionServicer.
    */
   virtual ~WebConnectionServicer() {};
   
   /**
    * Services the passed Connection. The connection will automatically be
    * closed after it is serviced.
    * 
    * @param c the Connection to service.
    */
   virtual void serviceConnection(Connection* c);
   
   /**
    * Services the passed WebConnection. The connection will automatically be
    * closed after it is serviced.
    * 
    * @param c the WebConnection to service.
    */
   virtual void serviceWebConnection(WebConnection* c) = 0;
};

inline void WebConnectionServicer::serviceConnection(Connection* c)
{
   WebConnection wc(c, false);
   serviceWebConnection(&wc);
}

} // end namespace net
} // end namespace db
#endif
