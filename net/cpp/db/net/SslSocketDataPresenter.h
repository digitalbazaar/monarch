/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_SslSocketDataPresenter_H
#define db_net_SslSocketDataPresenter_H

#include "db/net/SocketDataPresenter.h"
#include "db/net/SslContext.h"

namespace db
{
namespace net
{

/**
 * A SslSocketDataPresenter tries to provide an SslSocket for transforming
 * incoming data for presentation and outgoing data for transmission.
 *
 * @author Dave Longley
 */
class SslSocketDataPresenter : public SocketDataPresenter
{
protected:
   /**
    * The SSL context for this presenter.
    */
   SslContext* mContext;

   /**
    * Detects whether or not the passed socket speaks SSL.
    */
   virtual bool detectSsl(Socket* s);

public:
   /**
    * Creates a new SslSocketDataPresenter.
    *
    * @param context the SSL context to use.
    */
   SslSocketDataPresenter(SslContext* context);

   /**
    * Destructs this SslSocketDataPresenter.
    */
   virtual ~SslSocketDataPresenter();

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

} // end namespace net
} // end namespace db
#endif
