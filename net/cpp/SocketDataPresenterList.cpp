/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SocketDataPresenterList.h"

using namespace std;
using namespace db::net;

SocketDataPresenterList::SocketDataPresenterList(bool cleanup)
{
   mCleanup = cleanup;
}

SocketDataPresenterList::~SocketDataPresenterList()
{
   if(mCleanup)
   {
      // clean up data presenters
      for(list<SocketDataPresenter*>::iterator i = mDataPresenters.begin();
          i != mDataPresenters.end(); i++)
      {
         delete *i;
      }
   }
}

void SocketDataPresenterList::add(SocketDataPresenter* sdp)
{
   lock();
   {
      mDataPresenters.push_back(sdp);
   }
   unlock();
}

Socket* SocketDataPresenterList::createPresentationWrapper(
   Socket* s, bool& secure)
{
   Socket* rval = NULL;
   
   lock();
   {
      // use data presenters to create a socket wrapper
      for(list<SocketDataPresenter*>::iterator i = mDataPresenters.begin();
          i != mDataPresenters.end() && rval == NULL; i++)
      {
         rval = (*i)->createPresentationWrapper(s, secure);
      }
      
      // no presentation wrapper, so just use regular socket
      if(rval == NULL)
      {
         rval = s;
         secure = false;
      }
   }
   unlock();
   
   return rval;
}
