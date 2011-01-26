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
   typedef void (Handler::*AuthFunction)(Channel* ch);

   /**
    * The handler object with the handler function as a member.
    */
   Handler* mHandler;

   /**
    * The handler function.
    */
   AuthFunction mAuthFunction;

public:
   /**
    * Creates a new RequestAuthenticatorDelegate with the specified handler
    * object and function.
    *
    * @param h the handler object.
    * @param f the handler's function for authenticating a request.
    */
   RequestAuthenticatorDelegate(Handler* h, AuthFunction f);

   /**
    * Destructs this RequestAuthenticatorDelegate.
    */
   virtual ~RequestAuthenticatorDelegate();

   /**
    * {@inheritDoc}
    */
   virtual bool checkAuthentication(ServiceChannel* ch);
};

template<typename Handler, typename Channel>
RequestAuthenticatorDelegate<Handler, Channel>::RequestAuthenticatorDelegate(
   Handler* h, AuthFunction f) :
   mHandler(h),
   mAuthFunction(f)
{
}

template<typename Handler, typename Channel>
RequestAuthenticatorDelegate<Handler, Channel>::~RequestAuthenticatorDelegate()
{
}

template<typename Handler, typename Channel>
bool RequestAuthenticatorDelegate<Handler, Channel>::checkAuthentication(
   ServiceChannel* ch)
{
   return (mHandler->*mAuthFunction)(static_cast<Channel*>(ch));
}

} // end namespace ws
} // end namespace monarch
#endif
