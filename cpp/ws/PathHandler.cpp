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
   }
   else
   {
      // check all authentication methods, save last exception for failed
      // attempt
      ExceptionRef e(NULL);
      int rc;
      for(RequestAuthList::iterator i = mAuthMethods.begin();
          i != mAuthMethods.end(); ++i)
      {
         // clear exceptions from previous failures
         Exception::clear();

         // check authentication method
         rc = (*i)->checkAuthentication(ch);

         // if authentication passed, set rval to true if not yet set
         if(rc == 1 && !rval)
         {
            rval = true;
         }
         // authentication was attempted by client but failed, save exception
         else if(rc == -1)
         {
            e = Exception::get();
         }
      }

      /* Special case: If an authentication method was attempted by a client
         (vs. not attempted by the client) and it failed *and* the only method
         that passed was NULL (anonymous), then fail. */
      if(!e.isNull() && rval && ch->getAuthenticationMethod() == NULL)
      {
         Exception::set(e);
         rval = false;
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
   // handle anonymous
   if(method.isNull())
   {
      method = new RequestAuthenticator();
   }

   // add method
   mAuthMethods.push_back(method);
}
