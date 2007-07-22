/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_SocketDataPresenter_H
#define db_net_SocketDataPresenter_H

#include "Socket.h"

namespace db
{
namespace net
{

/**
 * A SocketDataPresenter tries to provide an appropriate SocketWrapper for
 * transforming incoming data for presentation and outgoing data for
 * transmission.
 * 
 * An example of a presentation layer protocol is SSL. If a SocketDataPresenter
 * can detect the SSL protocol for a given Socket, it may be able to provide
 * an SslSocket as a wrapper to handle data presentation.
 * 
 * @author Dave Longley
 */
class SocketDataPresenter
{
public:
   /**
    * Creates a new SocketDataPresenter.
    */
   SocketDataPresenter() {};
   
   /**
    * Destructs this SocketDataPresenter.
    */
   virtual ~SocketDataPresenter() {};
   
   /**
    * Wraps the passed Socket if this presenter can provide the presentation
    * layer for its data. The created Socket should cleanup the passed Socket
    * when it is freed.
    * 
    * @param s the Socket to wrap.
    * 
    * @return the wrapped Socket or NULL if this presenter cannot provide
    *         the presentation layer for the socket data.
    */
   virtual Socket* createPresentationWrapper(Socket* s) = 0;
};

} // end namespace net
} // end namespace db
#endif
