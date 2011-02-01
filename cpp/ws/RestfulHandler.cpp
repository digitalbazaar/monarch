/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
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
   // free regexes
   for(RegexList::iterator i = mRegexList.begin(); i != mRegexList.end(); ++i)
   {
      free(*i);
   }
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
   HandlerInfo& info = mPathHandlers[paramCount][mt];
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
   HandlerInfo& info = mPathHandlers[paramCount][mt];
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

bool RestfulHandler::addRegexHandler(
   const char* regex,
   PathHandlerRef& handler,
   Message::MethodType mt,
   monarch::validation::ValidatorRef* queryValidator,
   monarch::validation::ValidatorRef* contentValidator,
   uint32_t flags)
{
   bool rval = true;

   // look up the regex
   RegexInfo* rinfo = NULL;
   RegexMap::iterator i = mRegexMap.find(regex);
   if(i != mRegexMap.end())
   {
      rinfo = &(mRegexMap[regex]);
   }
   else
   {
      // new regex, try to compile the pattern
      PatternRef p = Pattern::compile(regex);
      if(p == NULL)
      {
         ExceptionRef e = new Exception(
            "Could not add WebService handler. Invalid regular expression.",
            "monarch.ws.InvalidRegularExpression");
         e->getDetails()["pattern"] = regex;
         Exception::set(e);
         rval = false;
      }
      else
      {
         // insert regex entry
         char* copy = strdup(regex);
         mRegexList.push_back(copy);
         rinfo = &(mRegexMap[copy]);
         rinfo->pattern = p;
      }
   }

   // add handler info
   if(rinfo != NULL)
   {
      HandlerInfo& info = rinfo->methods[mt];
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

   return rval;
}

static Message::MethodType _getMethodType(ServiceChannel* ch, string& method)
{
   Message::MethodType mt = Message::Undefined;

   // allow X-Method-Override on request
   HttpRequestHeader* header = ch->getRequest()->getHeader();
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

   return mt;
}

RestfulHandler::HandlerInfo* RestfulHandler::findHandler(ServiceChannel* ch)
{
   HandlerInfo* rval = NULL;

   // keep track of error response to send: send a 404 if no path is found at
   // all, but if one is found, but there is no method for it, send a 405
   bool send404 = true;

   // get path params and method type from channel
   DynamicObject paramDyno;
   ch->getPathParams(paramDyno);
   int paramCount = paramDyno->length();
   string method;
   Message::MethodType mt = _getMethodType(ch, method);
   DynamicObject validMethods(NULL);

   // try to find path handler using param count
   HandlerMap::iterator hmi = mPathHandlers.find(paramCount);
   if(hmi == mPathHandlers.end())
   {
      // no handler found for specific param count, so check arbitrary count
      hmi = mPathHandlers.find(-1);
   }

   // potential handler found, check method types
   MethodMap::iterator mmi;
   if(hmi != mPathHandlers.end())
   {
      // path match found, so if methods don't match, return a 405 not a 404
      send404 = false;

      // find the map of valid methods for the given handler
      MethodMap* mm = &(*hmi).second;
      mmi = mm->find(mt);

      // if there's no match for a valid method using the given param count,
      // try looking for one using the arbitrary param count
      if(mmi == mm->end() && (*hmi).first != -1)
      {
         hmi = mPathHandlers.find(-1);
         if(hmi != mPathHandlers.end())
         {
            mm = &(*hmi).second;
            mmi = mm->find(mt);
         }
      }

      // see if there is a match for the given request method
      if(mmi != mm->end())
      {
         rval = &((*mmi).second);
      }
      else
      {
         // add valid method types
         validMethods = DynamicObject();
         validMethods->setType(Array);
         for(mmi = mm->begin(); mmi != mm->end(); ++mmi)
         {
            validMethods->append(Message::methodToString(mmi->first));
         }
      }
   }

   // no handler found yet, check regexes
   if(rval == NULL && mRegexList.size() > 0)
   {
      // get relative local path (the part of the path after the base path)
      const char* path = ch->getPath() + (strlen(ch->getBasePath()) - 1);

      // check each regex
      DynamicObject matches;
      for(RegexList::iterator i = mRegexList.begin();
          rval == NULL && i != mRegexList.end(); ++i)
      {
         RegexMap::iterator rmi = mRegexMap.find(*i);
         if(rmi->second.pattern->getSubMatches(path, matches, -1, false, 1))
         {
            send404 = false;

            // look for a handler with a matching method
            MethodMap* mm = &(rmi->second.methods);
            mmi = mm->find(mt);
            if(mmi != mm->end())
            {
               // handler found, set channel handler info
               rval = &((*mmi).second);
               DynamicObject info;
               info["monarch.ws.RestfulHandler"]["matches"] = matches;
               ch->setHandlerInfo(info);
            }
            else if(validMethods.isNull())
            {
               // add valid method types
               validMethods = DynamicObject();
               validMethods->setType(Array);
               for(mmi = mm->begin(); mmi != mm->end(); ++mmi)
               {
                  validMethods->append(Message::methodToString(mmi->first));
               }
            }
         }
      }
   }

   // if no handler found
   if(rval == NULL)
   {
      // no such path found
      if(send404)
      {
         // no handler found, send 404
         ch->getResponse()->getHeader()->setStatus(404, "Not Found");
         ExceptionRef e = new Exception(
            "Resource not found.",
            "monarch.ws.ResourceNotFound");
         e->getDetails()["code"] = 404;
         e->getDetails()["resource"] = ch->getPath();
         Exception::set(e);
      }
      // path match was found, but method wasn't supported
      else
      {
         // 405
         ch->getResponse()->getHeader()->setStatus(405, "Method Not Allowed");
         ExceptionRef e = new Exception(
            "Method not allowed.",
            "monarch.ws.MethodNotAllowed");
         e->getDetails()["code"] = 405;
         e->getDetails()["invalidMethod"] = method.c_str();
         e->getDetails()["validMethods"] = validMethods;

         // set allow header
         string allow = StringTools::join(
            e->getDetails()["validMethods"], ", ");
         ch->getResponse()->getHeader()->setField("Allow", allow.c_str());
         Exception::set(e);
      }
   }

   return rval;
}

void RestfulHandler::handleChannel(ServiceChannel* ch, HandlerInfo* info)
{
   bool pass = false;

   bool validationError = false;
   if(info != NULL && info->handler->canHandleRequest(ch))
   {
      // clear last exception
      Exception::clear();

      // always receive content
      DynamicObject content;
      pass = ch->receiveContent(content);
      bool received = pass;

      // do validation
      if(pass && !info->resourceValidator.isNull())
      {
         DynamicObject params;
         ch->getPathParams(params);
         pass = info->resourceValidator->isValid(params);
      }
      if(pass && !info->queryValidator.isNull())
      {
         DynamicObject query;
         ch->getQuery(query, info->flags & ArrayQuery);
         pass = info->queryValidator->isValid(query);
      }
      if(pass && !info->contentValidator.isNull())
      {
         pass = info->contentValidator->isValid(content);
      }

      // validation error if content received but didn't pass validation
      validationError = (received && !pass);

      if(pass)
      {
         info->handler->handleRequest(ch);
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
            "monarch.ws.WebServiceError");
         e->getDetails()["code"] = 500;
         e->getDetails()["path"] = ch->getPath();
         monarch::rt::Exception::set(e);
      }

      // send exception (client's fault if code < 500)
      bool clientsFault =
         validationError ||
         (e->getDetails()->hasMember("code") &&
         e->getDetails()["code"]->getInt32() < 500);
      ch->sendException(e, clientsFault);
   }
}
