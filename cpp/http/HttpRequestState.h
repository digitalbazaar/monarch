/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_http_HttpRequestState_h
#define monarch_http_HttpRequestState_h

#include "monarch/rt/DynamicObject.h"
#include "monarch/util/Timer.h"

namespace monarch
{
namespace http
{

/**
 * A HttpRequestState holds state of a connection.
 *
 * Provides a common DynamicObject to store request state details. Also
 * provides a timer for each request. beginRequest() should be called when
 * starting to handle a request to clear state and start the timer.
 *
 * @author David I. Lehn
 */
class HttpRequestState
{
protected:
   /**
    * Request timer.
    */
   monarch::util::Timer mTimer;

   /**
    * Storage for connection state.
    */
   monarch::rt::DynamicObject mDetails;

public:
   /**
    * Creates a new HttpRequestState.
    */
   HttpRequestState();

   /**
    * Destructs this HttpRequestState.
    */
   virtual ~HttpRequestState();

   /**
    * Begin handling a new request by clearing state and starting the request
    * timer.
    */
   virtual void beginRequest();

   /**
    * Get the request timer.
    *
    * @return the request timer.
    */
   virtual monarch::util::Timer* getTimer();

   /**
    * Get the details stored in this state.
    *
    * @return a DynamicObject with connection state details.
    */
   virtual monarch::rt::DynamicObject& getDetails();
};

} // end namespace http
} // end namespace monarch
#endif
