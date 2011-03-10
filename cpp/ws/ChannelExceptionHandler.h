/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_ChannelExceptionHandler_H
#define monarch_ws_ChannelExceptionHandler_H

#include "monarch/rt/Exception.h"
#include "monarch/ws/ServiceChannel.h"

namespace monarch
{
namespace ws
{

/**
 * A ChannelExceptionHandler is an interface that handles an exception that
 * occurs on a ServiceChannel. When it gets called is determined by each
 * different kind of PathHandler, eg: RestfulHandler.
 *
 * The base class has default exception handling code that sends an exception
 * over the channel using the previously set content-type, if there is a known
 * serialization for an exception with that content-type.
 *
 * @author Dave Longley
 */
class ChannelExceptionHandler
{
public:
   /**
    * Creates a new ChannelExceptionHandler.
    */
   ChannelExceptionHandler();

   /**
    * Destructs this ChannelExceptionHandler.
    */
   virtual ~ChannelExceptionHandler();

   /**
    * Handle's the client's request by receiving its content, if any, and
    * sending an appropriate response. The parameter count and request method
    * type are used to further dispatch the request to the appropriate
    * registered handler.
    *
    * @param ch the communication channel with the client.
    * @param e the exception to handle.
    */
   virtual void handleChannelException(
      ServiceChannel* ch, monarch::rt::ExceptionRef& e);
};

// type definition for a reference counted ResourceValidator
typedef monarch::rt::Collectable<ChannelExceptionHandler>
   ChannelExceptionHandlerRef;

} // end namespace ws
} // end namespace monarch
#endif
