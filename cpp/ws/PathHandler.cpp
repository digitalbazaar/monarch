/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/ws/PathHandler.h"

using namespace monarch::rt;
using namespace monarch::ws;

PathHandler::PathHandler(bool secureOnly) :
   mSecureOnly(secureOnly)
{
}

PathHandler::~PathHandler()
{
}

bool PathHandler::canHandleRequest(ServiceChannel* ch)
{
   return true;
}

void PathHandler::handleRequest(ServiceChannel* ch)
{
   // default handler sends not found
   ch->getResponse()->getHeader()->setStatus(404, "Not Found");
   ch->sendNoContent();
}

void PathHandler::operator()(ServiceChannel* ch)
{
   // enforce secure connection if appropriate
   if(mSecureOnly && !ch->getRequest()->getConnection()->isSecure())
   {
      // send 404
      ch->getResponse()->getHeader()->setStatus(404, "Not Found");
      ch->sendNoContent();
   }
   else if(canHandleRequest(ch))
   {
      handleRequest(ch);
   }
   else
   {
      // send exception (client's fault if code < 500)
      ExceptionRef e = Exception::get();
      ch->sendException(e, e->getCode() < 500);
   }
}

bool PathHandler::secureConnectionRequired()
{
   return mSecureOnly;
}
