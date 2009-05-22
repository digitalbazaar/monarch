/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_SocketDataPresenterList_H
#define db_net_SocketDataPresenterList_H

#include "db/rt/Collectable.h"
#include "db/rt/SharedLock.h"
#include "db/net/SocketDataPresenter.h"

#include <vector>

namespace db
{
namespace net
{

/**
 * A SocketDataPresenterList is a SocketDataPresenter that maintains a list of
 * other SocketDataPresenters. If a wrapped Socket is requested from this
 * SocketDataPresenter, it will return the first wrapped Socket that can be
 * produced from the list of SocketDataPresenters.
 * 
 * @author Dave Longley
 */
class SocketDataPresenterList : public SocketDataPresenter
{
protected:
   /**
    * A list of available SocketDataPresenters.
    */
   std::vector<SocketDataPresenter*> mDataPresenters;
   
   /**
    * A shared lock for accessing the list.
    */
   db::rt::SharedLock mLock;
   
   /**
    * Set to true to clean up the memory for the SocketDataPresenters in this
    * list, false otherwise.
    */
   bool mCleanup;
   
public:
   /**
    * Creates a new SocketDataPresenterList.
    * 
    * @param cleanup true if the SocketDataPresenters added to this list
    *                should be freed when this list is destructed, false if not.
    */
   SocketDataPresenterList(bool cleanup);
   
   /**
    * Destructs this SocketDataPresenterList.
    */
   virtual ~SocketDataPresenterList();
   
   /**
    * Adds a SocketDataPresenter to this list.
    * 
    * @param sdp the SocketDataPresenter to add.
    */
   virtual void add(SocketDataPresenter* sdp);
   
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

// type definition for reference counted SocketDataPresenterList
typedef db::rt::Collectable<SocketDataPresenterList> SocketDataPresenterListRef;

} // end namespace net
} // end namespace db
#endif
