/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_PathHandlerDelegate_H
#define monarch_ws_PathHandlerDelegate_H

#include "monarch/ws/PathHandler.h"

namespace monarch
{
namespace ws
{

// type definition for a resource handler
typedef monarch::ws::PathHandlerRef ResourceHandler;

/**
 * A PathHandlerDelegate is a PathHandler that provides a means to map a
 * function to handle a client's request to an object's member function.
 *
 * The handler function can either:
 *
 * 1. Be simple and take a ServiceChannel like a PathHandler normally would.
 * 2. Take a ServiceChannel and two DynamicObjects, one with the client's
 *    content already read and converted to an object and the other to be
 *    set to the object to send back to the client. This function can also
 *    return false to send an exception to the client.
 *
 * @author Dave Longley
 */
template<typename Handler>
class PathHandlerDelegate : public monarch::ws::PathHandler
{
protected:
   /**
    * Typedef for simple handler function.
    */
   typedef void (Handler::*SimpleFunction)(ServiceChannel* ch);

   /**
    * Typedef for DynamicObject handler function.
    */
   typedef bool (Handler::*DynoFunction)(
      ServiceChannel*,
      monarch::rt::DynamicObject& in, monarch::rt::DynamicObject& out);

   /**
    * The handler object with the handler function as a member.
    */
   Handler* mHandler;

   /**
    * The type for the function.
    */
   enum Type
   {
      FunctionSimple,
      FunctionDyno
   } mType;

   /**
    * The handler function.
    */
   union
   {
      SimpleFunction mSimpleFunction;
      DynoFunction mDynoFunction;
   };

public:
   /**
    * Creates a new PathHandlerDelegate with the specified handler object,
    * function, and optional PathHandler guard.
    *
    * @param h the handler object.
    * @param f the handler's function for handling a request.
    */
   PathHandlerDelegate(Handler* h, SimpleFunction f);

   /**
    * Creates a new PathHandlerDelegate with the specified handler object,
    * function, and optional PathHandler guard.
    *
    * @param h the handler object.
    * @param f the handler's function for handling a request.
    */
   PathHandlerDelegate(Handler* h, DynoFunction f);

   /**
    * Destructs this PathHandlerDelegate.
    */
   virtual ~PathHandlerDelegate();

   /**
    * Checks to see if the handler can handle the client's request. If not,
    * an exception must be set that will be sent to the client.
    *
    * @param ch the communication channel with the client.
    *
    * @return true if this action can be performed.
    */
   virtual bool canHandleRequest(ServiceChannel* ch);

   /**
    * Handles the client's request. Does whatever is necessary to handle the
    * client's request and sends a response.
    *
    * @param ch the communication channel with the client.
    */
   virtual void handleRequest(ServiceChannel* ch);
};

template<typename Handler>
PathHandlerDelegate<Handler>::PathHandlerDelegate(
   Handler* h, SimpleFunction f) :
   mHandler(h),
   mType(FunctionSimple),
   mSimpleFunction(f)
{
}

template<typename Handler>
PathHandlerDelegate<Handler>::PathHandlerDelegate(
   Handler* h, DynoFunction f) :
   mHandler(h),
   mType(FunctionDyno),
   mDynoFunction(f)
{
}

template<typename Handler>
PathHandlerDelegate<Handler>::~PathHandlerDelegate()
{
}

template<typename Handler>
bool PathHandlerDelegate<Handler>::canHandleRequest(ServiceChannel* ch)
{
   bool rval = true;

   // receive content if using dyno function
   if(mType == FunctionDyno)
   {
      // on success dyno will be cached in the channel so it can be
      // retrieved in handleRequest(), but on failure it will set an
      // exception that will be sent to the client if we return false
      monarch::rt::DynamicObject in;
      rval = ch->receiveContent(in);
   }

   return rval;
}

template<typename Handler>
void PathHandlerDelegate<Handler>::handleRequest(ServiceChannel* ch)
{
   bool success = true;

   if(mType == FunctionSimple)
   {
      // handle simple request
      (mHandler->*mSimpleFunction)(ch);
   }
   else if(mType == FunctionDyno)
   {
      // input/output dynos
      monarch::rt::DynamicObject in;
      monarch::rt::DynamicObject out;

      // receive content as dyno (cached via canPerformAction)
      success = ch->receiveContent(in);
      if(success)
      {
         success = (mHandler->*mDynoFunction)(ch, in, out);
         if(success)
         {
            if(out.isNull())
            {
               ch->sendNoContent();
            }
            else
            {
               ch->sendContent(out);
            }
         }
      }
   }

   // if handling failed and nothing has been sent to the client yet, then
   // send an exception
   if(!success && !ch->hasSent())
   {
      // get last exception (create one if necessary -- but this will only
      // happen if a developer has failed to set an exception in a service)
      monarch::rt::ExceptionRef e = monarch::rt::Exception::get();
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

} // end namespace ws
} // end namespace monarch
#endif
