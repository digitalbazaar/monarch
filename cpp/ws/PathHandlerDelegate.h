/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_PathHandlerDelegate_H
#define monarch_ws_PathHandlerDelegate_H

#include "monarch/ws/PathHandler.h"

namespace monarch
{
namespace ws
{

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
template<typename Handler, typename Channel = ServiceChannel>
class PathHandlerDelegate : public monarch::ws::PathHandler
{
protected:
   /**
    * Typedef for simple handler function.
    */
   typedef void (Handler::*SimpleFunction)(Channel* ch);

   /**
    * Typedef for DynamicObject handler function.
    */
   typedef bool (Handler::*DynoFunction)(
      Channel*,
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
    * Creates a new PathHandlerDelegate with the specified handler object and
    * function.
    *
    * @param h the handler object.
    * @param f the handler's function for handling a request.
    */
   PathHandlerDelegate(Handler* h, SimpleFunction f);

   /**
    * Creates a new PathHandlerDelegate with the specified handler object and
    * function.
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
    * {@inheritDoc}
    */
   virtual bool canHandleRequest(ServiceChannel* ch);

   /**
    * {@inheritDoc}
    */
   virtual void handleRequest(ServiceChannel* ch);
};

template<typename Handler, typename Channel>
PathHandlerDelegate<Handler, Channel>::PathHandlerDelegate(
   Handler* h, SimpleFunction f) :
   mHandler(h),
   mType(FunctionSimple),
   mSimpleFunction(f)
{
}

template<typename Handler, typename Channel>
PathHandlerDelegate<Handler, Channel>::PathHandlerDelegate(
   Handler* h, DynoFunction f) :
   mHandler(h),
   mType(FunctionDyno),
   mDynoFunction(f)
{
}

template<typename Handler, typename Channel>
PathHandlerDelegate<Handler, Channel>::~PathHandlerDelegate()
{
}

template<typename Handler, typename Channel>
bool PathHandlerDelegate<Handler, Channel>::canHandleRequest(ServiceChannel* ch)
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

   rval = rval && checkAuthentication(ch);

   return rval;
}

template<typename Handler, typename Channel>
void PathHandlerDelegate<Handler, Channel>::handleRequest(ServiceChannel* ch)
{
   bool success = true;

   if(mType == FunctionSimple)
   {
      // handle simple request
      (mHandler->*mSimpleFunction)(static_cast<Channel*>(ch));
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
         success = (mHandler->*mDynoFunction)(
            static_cast<Channel*>(ch), in, out);
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
   // handle the exception
   if(!success && !ch->hasSent())
   {
      monarch::rt::ExceptionRef e = monarch::rt::Exception::get();
      handleChannelException(ch, e);
   }
}

} // end namespace ws
} // end namespace monarch
#endif
