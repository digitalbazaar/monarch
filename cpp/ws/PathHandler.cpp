/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/ws/PathHandler.h"

using namespace monarch::rt;
using namespace monarch::ws;

PathHandler::PathHandler(bool secureOnly) :
   mSecureOnly(secureOnly),
   mExceptionHandler(this),
   mExceptionHandlerRef(NULL)
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
      // check all authentication methods unless denied
      // save last exception for failed attempt
      ExceptionRef e(NULL);
      RequestAuthenticator::Result rc;
      bool denied = false;
      for(RequestAuthList::iterator i = mAuthMethods.begin();
          !denied && i != mAuthMethods.end(); ++i)
      {
         // clear exceptions from previous failures
         Exception::clear();

         // check authentication method
         rc = (*i)->checkAuthentication(ch);

         // if authentication passed, set rval to true if not yet set
         if(rc == RequestAuthenticator::Success && !rval)
         {
            rval = true;
         }
         // authentication was attempted by client but failed, save exception
         else if(rc == RequestAuthenticator::Failure)
         {
            e = Exception::get();
         }
         // request was denied, save exception
         else if(rc == RequestAuthenticator::Deny)
         {
            denied = true;
            e = Exception::get();
            rval = false;
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
      // set default HTTP response code if not yet set
      if(ch->getResponse()->getHeader()->getStatusCode() == 0)
      {
         // FIXME: change to 401 Unauthorized? currently not used because
         // spec seems to indicate that the client could send WWW-Authenticate
         // which is not necessarily the method of authentication used
         ch->getResponse()->getHeader()->setStatus(400, "Bad Request");
      }

      // set top-level exception
      ExceptionRef e = new Exception(
         "WebService authentication failed. Access denied.",
         "monarch.ws.AccessDenied");
      e->getDetails()["httpStatusCode"] = 403;
      e->getDetails()["public"] = true;
      e->getDetails()["path"] = ch->getRequest()->getHeader()->getPath();
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
   // try to handle request
   else if(canHandleRequest(ch))
   {
      handleRequest(ch);
   }
   // exception, could not handle request
   else
   {
      ExceptionRef e = Exception::get();
      handleChannelException(ch, e);
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

void PathHandler::setExceptionHandler(ChannelExceptionHandler* h, bool cleanup)
{
   if(cleanup)
   {
      setExceptionHandlerRef(h);
   }
   else
   {
      mExceptionHandlerRef.setNull();
      mExceptionHandler = h;
   }
}

void PathHandler::setExceptionHandlerRef(ChannelExceptionHandlerRef h)
{
   mExceptionHandlerRef = h;
   mExceptionHandler = &(*mExceptionHandlerRef);
}

void PathHandler::handleChannelException(
   ServiceChannel* ch, ExceptionRef& e)
{
   // exception will only be null if a developer has failed to set one when
   // an error condition occurred
   if(e.isNull())
   {
      e = new monarch::rt::Exception(
         "An unspecified error occurred. "
         "No exception was set detailing the error.",
         "monarch.ws.WebServiceError");
      e->getDetails()["httpStatusCode"] = 500;
      e->getDetails()["path"] = ch->getPath();
      monarch::rt::Exception::set(e);
   }

   // use default handler
   if(mExceptionHandler == this)
   {
      ChannelExceptionHandler::handleChannelException(ch, e);
   }
   // use custom handler
   else
   {
      mExceptionHandler->handleChannelException(ch, e);
   }
}
