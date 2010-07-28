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
   handleChannel(ch, findHandler(ch));
}

void RestfulHandler::addHandler(
   PathHandlerRef& handler,
   MethodType mt,
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
   MethodType mt,
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

RestfulHandler::MethodType RestfulHandler::stringToMethod(const char* str)
{
   MethodType rval;

   if(strcmp(str, "GET") == 0)
   {
      rval = Get;
   }
   else if(strcmp(str, "PUT") == 0)
   {
      rval = Put;
   }
   else if(strcmp(str, "POST") == 0)
   {
      rval = Post;
   }
   else if(strcmp(str, "DELETE") == 0)
   {
      rval = Delete;
   }
   else if(strcmp(str, "HEAD") == 0)
   {
      rval = Head;
   }
   else if(strcmp(str, "OPTIONS") == 0)
   {
      rval = Options;
   }
   else if(strcmp(str, "TRACE") == 0)
   {
      rval = Trace;
   }
   else if(strcmp(str, "CONNECT") == 0)
   {
      rval = Connect;
   }
   else
   {
      rval = Undefined;
   }

   return rval;
}

const char* RestfulHandler::methodToString(RestfulHandler::MethodType type)
{
   const char* rval = NULL;

   switch(type)
   {
      case RestfulHandler::Get:
         rval = "GET";
         break;
      case RestfulHandler::Put:
         rval = "PUT";
         break;
      case RestfulHandler::Post:
         rval = "POST";
         break;
      case RestfulHandler::Delete:
         rval = "DELETE";
         break;
      case RestfulHandler::Head:
         rval = "HEAD";
         break;
      case RestfulHandler::Options:
         rval = "OPTIONS";
         break;
      case RestfulHandler::Trace:
         rval = "TRACE";
         break;
      case RestfulHandler::Connect:
         rval = "CONNECT";
         break;
      case RestfulHandler::Undefined:
         rval = NULL;
         break;
   }

   return rval;
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
      MethodType mt = RestfulHandler::Undefined;
      if(header->hasField("X-Method-Override"))
      {
         method = header->getFieldValue("X-Method-Override");
         mt = stringToMethod(method.c_str());
      }
      if(mt == RestfulHandler::Undefined)
      {
         method = header->getMethod();
         mt = stringToMethod(method.c_str());
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
            pass = true;

            // clear last exception, then do validation
            Exception::clear();

            if(!info.resourceValidator.isNull())
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
               DynamicObject content;
               ch->receiveContent(content);
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
               methodToString(mmi->first);
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
