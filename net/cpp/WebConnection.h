/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef WebConnection_H
#define WebConnection_H

#include "Connection.h"
#include "WebRequest.h"

namespace db
{
namespace net
{

/**
 * A WebConnection is a class that represents an internet connection that
 * uses web requests and web responses to communicate.
 * 
 * @author Dave Longley
 */
class WebConnection : public Connection
{
public:
   /**
    * Creates a new WebConnection that wraps the passed Socket.
    * 
    * @param s the Socket for this WebConnection.
    * @param true to clean up the Socket when this WebConnection is destructed,
    *        false to leave it alone. 
    */
   WebConnection(Socket* s, bool cleanup);
   
   /**
    * Destructs this WebConnection.
    */
   virtual ~WebConnection();
};

} // end namespace net
} // end namespace db
#endif
