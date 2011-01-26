/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
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
   return checkAuthentication(ch);
}

bool PathHandler::checkAuthentication(ServiceChannel* ch)
{
   bool rval = false;

   // no authentication methods
   if(mAuthMethods.size() == 0)
   {
      rval = true;
      ch->setAuthenticationMethod(NULL, NULL);
   }
   else
   {
      // check until one method works
      for(RequestAuthList::iterator i = mAuthMethods.begin();
          !rval && i != mAuthMethods.end(); ++i)
      {
         Exception::clear();
         rval = (*i)->checkAuthentication(ch);
      }
   }

   if(!rval)
   {
      // set top-level exception
      ExceptionRef e = new Exception(
         "WebService authentication failed. Access denied.",
         "monarch.ws.AccessDenied");
      Exception::push(e);
   }

   return rval;
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

void PathHandler::addRequestAuthenticator(RequestAuthenticatorRef method)
{
   mAuthMethods.push_back(method);
}
