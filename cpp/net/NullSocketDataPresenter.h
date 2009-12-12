/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_NullSocketDataPresenter_H
#define db_net_NullSocketDataPresenter_H

#include "monarch/net/SocketDataPresenter.h"

namespace db
{
namespace net
{

/**
 * A NullSocketDataPresenter provides no wrapper for transforming
 * incoming data for presentation and outgoing data for transmission, but
 * instead returns any Socket it is passed as-is.
 *
 * @author Dave Longley
 */
class NullSocketDataPresenter : public SocketDataPresenter
{
public:
   /**
    * Creates a new NullSocketDataPresenter.
    */
   NullSocketDataPresenter() {};

   /**
    * Destructs this NullSocketDataPresenter.
    */
   virtual ~NullSocketDataPresenter() {};

   /**
    * Wraps the passed Socket if this presenter can provide the presentation
    * layer for its data. The created Socket should cleanup the passed Socket
    * when it is freed.
    *
    * @param s the Socket to wrap.
    * @param secure true if the wrapper is considered a secure protocol,
    *               like SSL, false if not.
    *
    * @return the wrapped Socket or NULL if this presenter cannot provide
    *         the presentation layer for the socket data.
    */
   virtual Socket* createPresentationWrapper(Socket* s, bool& secure);
};

inline Socket* NullSocketDataPresenter::createPresentationWrapper(
   Socket* s, bool& secure)
{
   secure = false;
   return s;
}

} // end namespace net
} // end namespace db
#endif
