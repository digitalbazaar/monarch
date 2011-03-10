/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/ws/ChannelExceptionHandler.h"

using namespace monarch::rt;
using namespace monarch::ws;

ChannelExceptionHandler::ChannelExceptionHandler()
{
}

ChannelExceptionHandler::~ChannelExceptionHandler()
{
}

void ChannelExceptionHandler::handleChannelException(
   ServiceChannel* ch, ExceptionRef& e)
{
   // exception will only be null if a developer has failed to set an
   // exception in a service
   if(e.isNull())
   {
      e = new monarch::rt::Exception(
         "An unspecified error occurred. "
         "No exception was set detailing the error.",
         "monarch.ws.WebServiceError");
      e->getDetails()["code"] = 500;
      e->getDetails()["path"] = ch->getPath();
      monarch::rt::Exception::set(e);
   }

   // send exception (client's fault if code < 500)
   bool clientsFault =
      e->getDetails()->hasMember("code") &&
      e->getDetails()["code"]->getInt32() < 500;
   ch->sendException(e, clientsFault);
}
