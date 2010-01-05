/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/SocketDataPresenterList.h"

using namespace std;
using namespace monarch::net;

SocketDataPresenterList::SocketDataPresenterList(bool cleanup) :
   mCleanup(cleanup)
{
}

SocketDataPresenterList::~SocketDataPresenterList()
{
   if(mCleanup)
   {
      // clean up data presenters
      for(vector<SocketDataPresenter*>::iterator i = mDataPresenters.begin();
          i != mDataPresenters.end(); i++)
      {
         delete *i;
      }
   }
}

void SocketDataPresenterList::add(SocketDataPresenter* sdp)
{
   mLock.lockExclusive();
   {
      mDataPresenters.push_back(sdp);
   }
   mLock.unlockExclusive();
}

Socket* SocketDataPresenterList::createPresentationWrapper(
   Socket* s, bool& secure)
{
   Socket* rval = NULL;
   
   mLock.lockShared();
   {
      // use data presenters to create a socket wrapper
      for(vector<SocketDataPresenter*>::iterator i = mDataPresenters.begin();
          i != mDataPresenters.end() && rval == NULL; i++)
      {
         rval = (*i)->createPresentationWrapper(s, secure);
      }
   }
   mLock.unlockShared();
   
   return rval;
}
