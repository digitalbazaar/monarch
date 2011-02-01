/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_RequestAuthenticatorDelegate_H
#define monarch_ws_RequestAuthenticatorDelegate_H

#include "monarch/ws/RequestAuthenticator.h"

namespace monarch
{
namespace ws
{

/**
 * A RequestAuthenticatorDelegate is a RequestAuthenticator that provides a
 * means to map a function to authenticate a client's request to an object's
 * member function.
 *
 * @author Dave Longley
 */
template<typename Handler, typename Channel = ServiceChannel>
class RequestAuthenticatorDelegate : public monarch::ws::RequestAuthenticator
{
protected:
   /**
    * Typedef for authentication function.
    */
   typedef int (Handler::*SimpleFunction)(Channel* ch);

   /**
    * Typedef for authentication function w/user-data dyno.
    */
   typedef int (Handler::*DynoFunction)(
      Channel* ch, monarch::rt::DynamicObject&);

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

   /**
    * User data to pass to a dyno function.
    */
   monarch::rt::DynamicObject mDyno;

public:
   /**
    * Creates a new RequestAuthenticatorDelegate with the specified handler
    * object and function.
    *
    * @param h the handler object.
    * @param f the handler's function for authenticating a request.
    */
   RequestAuthenticatorDelegate(Handler* h, SimpleFunction f);

   /**
    * Creates a new RequestAuthenticatorDelegate with the specified handler
    * object, function, and user-data to pass to the function.
    *
    * @param h the handler object.
    * @param f the handler's function for authenticating a request.
    * @param data the user-data to pass to the function.
    */
   RequestAuthenticatorDelegate(
      Handler* h, DynoFunction f, monarch::rt::DynamicObject& data);

   /**
    * Destructs this RequestAuthenticatorDelegate.
    */
   virtual ~RequestAuthenticatorDelegate();

   /**
    * {@inheritDoc}
    */
   virtual int checkAuthentication(ServiceChannel* ch);
};

template<typename Handler, typename Channel>
RequestAuthenticatorDelegate<Handler, Channel>::RequestAuthenticatorDelegate(
   Handler* h, SimpleFunction f) :
   mHandler(h),
   mType(FunctionSimple),
   mSimpleFunction(f),
   mDyno(NULL)
{
}

template<typename Handler, typename Channel>
RequestAuthenticatorDelegate<Handler, Channel>::RequestAuthenticatorDelegate(
   Handler* h, DynoFunction f, monarch::rt::DynamicObject& data) :
   mHandler(h),
   mType(FunctionDyno),
   mDynoFunction(f),
   mDyno(data)
{
}

template<typename Handler, typename Channel>
RequestAuthenticatorDelegate<Handler, Channel>::~RequestAuthenticatorDelegate()
{
}

template<typename Handler, typename Channel>
int RequestAuthenticatorDelegate<Handler, Channel>::checkAuthentication(
   ServiceChannel* ch)
{
   int rval;

   if(mType == FunctionDyno)
   {
      rval = (mHandler->*mDynoFunction)(static_cast<Channel*>(ch), mDyno);
   }
   else
   {
      rval = (mHandler->*mSimpleFunction)(static_cast<Channel*>(ch));
   }

   return rval;
}

} // end namespace ws
} // end namespace monarch
#endif
