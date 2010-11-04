/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/ws/RestfulHandler.h"

using namespace std;
using namespace monarch::http;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::ws;

RestfulHandler::RestfulHandler()
{
}

RestfulHandler::~RestfulHandler()
{
}

void RestfulHandler::operator()(ServiceChannel* ch)
{
   // enforce secure connection if appropriate
   if(mSecureOnly && !ch->getRequest()->getConnection()->isSecure())
   {
      // send 404
      ch->getResponse()->getHeader()->setStatus(404, "Not Found");
      ch->sendNoContent();
   }
   else
   {
      handleChannel(ch, findHandler(ch));
   }
}

void RestfulHandler::addHandler(
   PathHandlerRef& handler,
   Message::MethodType mt,
   int paramCount,
   monarch::validation::ValidatorRef* queryValidator,
   monarch::validation::ValidatorRef* contentValidator,
   uint32_t flags)
{
   HandlerInfo& info = mHandlers[paramCount][mt];
   if(queryValidator != NULL)
   {
      info.queryValidator = *queryValidator;
   }
   if(contentValidator != NULL)
   {
      info.contentValidator = *contentValidator;
   }
   info.handler = handler;
   info.flags = flags;
}

void RestfulHandler::addHandler(
   PathHandlerRef& handler,
   Message::MethodType mt,
   monarch::validation::ValidatorRef* resourceValidator,
   monarch::validation::ValidatorRef* queryValidator,
   monarch::validation::ValidatorRef* contentValidator,
   uint32_t flags)
{
   int paramCount = (resourceValidator != NULL)
      ? (*resourceValidator)->length()
      : 0;
   HandlerInfo& info = mHandlers[paramCount][mt];
   if(resourceValidator != NULL)
   {
      info.resourceValidator = *resourceValidator;
   }
   if(queryValidator != NULL)
   {
      info.queryValidator = *queryValidator;
   }
   if(contentValidator != NULL)
   {
      info.contentValidator = *contentValidator;
   }
   info.handler = handler;
   info.flags = flags;
}

RestfulHandler::HandlerMap::iterator RestfulHandler::findHandler(
   ServiceChannel* ch)
{
   HandlerMap::iterator hmi;

   DynamicObject paramDyno;
   ch->getPathParams(paramDyno);
   int paramCount = paramDyno->length();
   hmi = mHandlers.find(paramCount);
   if(hmi == mHandlers.end())
   {
      // no handler found for specific param count, so check arbitrary params
      hmi = mHandlers.find(-1);
   }

   return hmi;
}

void RestfulHandler::handleChannel(ServiceChannel* ch, HandlerMap::iterator hmi)
{
   bool pass = false;

   if(hmi == mHandlers.end())
   {
      // no handler found, send 404
      ch->getResponse()->getHeader()->setStatus(404, "Not Found");
      ExceptionRef e = new Exception(
         "Resource not found.", "monarch.ws.ResourceNotFound", 404);
      e->getDetails()["resource"] = ch->getPath();
      Exception::set(e);
   }
   else
   {
      MethodMap* mm = &(*hmi).second;

      // allow X-Method-Override on request
      HttpRequestHeader* header = ch->getRequest()->getHeader();
      string method;
      Message::MethodType mt = Message::Undefined;
      if(header->hasField("X-Method-Override"))
      {
         method = header->getFieldValue("X-Method-Override");
         mt = Message::stringToMethod(method.c_str());
      }
      if(mt == Message::Undefined)
      {
         method = header->getMethod();
         mt = Message::stringToMethod(method.c_str());
      }

      // find the map of valid methods for the given handler
      MethodMap::iterator mmi = mm->find(mt);
      if(mmi == mm->end() && (*hmi).first != -1)
      {
         hmi = mHandlers.find(-1);
         if(hmi != mHandlers.end())
         {
            mm = &(*hmi).second;
            mmi = mm->find(mt);
         }
      }

      // see if there is a resource match for the given request method
      if(mmi != mm->end())
      {
         HandlerInfo& info = (*mmi).second;
         if(info.handler->canHandleRequest(ch))
         {
            // clear last exception
            Exception::clear();

            // always receive content
            DynamicObject content;
            pass = ch->receiveContent(content);

            // do validation
            if(pass && !info.resourceValidator.isNull())
            {
               DynamicObject params;
               ch->getPathParams(params);
               pass = info.resourceValidator->isValid(params);
            }
            if(pass && !info.queryValidator.isNull())
            {
               DynamicObject query;
               ch->getQuery(query, info.flags & ArrayQuery);
               pass = info.queryValidator->isValid(query);
            }
            if(pass && !info.contentValidator.isNull())
            {
               pass = info.contentValidator->isValid(content);
            }

            if(pass)
            {
               info.handler->handleRequest(ch);
            }
         }
      }
      // there is no resource match for the given request method
      else
      {
         // 405
         ch->getResponse()->getHeader()->setStatus(405, "Method Not Allowed");
         ExceptionRef e = new Exception(
            "Method not allowed.",
            "monarch.ws.MethodNotAllowed", 405);
         e->getDetails()["invalidMethod"] = method.c_str();

         // add valid types
         e->getDetails()["validMethods"]->setType(Array);
         for(mmi = mm->begin(); mmi != mm->end(); ++mmi)
         {
            e->getDetails()["validMethods"]->append() =
               Message::methodToString(mmi->first);
         }

         // set allow header
         string allow = StringTools::join(
            e->getDetails()["validMethods"], ", ");
         ch->getResponse()->getHeader()->setField("Allow", allow.c_str());
         Exception::set(e);
      }
   }

   // if handling failed and nothing has been sent to the client yet, then
   // send an exception
   if(!pass && !ch->hasSent())
   {
      // get last exception (create one if necessary -- but this will only
      // happen if a developer has failed to set an exception in a service)
      ExceptionRef e = monarch::rt::Exception::get();
      if(e.isNull())
      {
         e = new monarch::rt::Exception(
            "An unspecified error occurred. "
            "No exception was set detailing the error.",
            "monarch.ws.WebServiceError", 500);
         e->getDetails()["path"] = ch->getPath();
         monarch::rt::Exception::set(e);
      }

      // send exception (client's fault if code < 500)
      ch->sendException(e, e->getCode() < 500);
   }
}
