/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/ws/RequestAuthenticator.h"

using namespace monarch::rt;
using namespace monarch::ws;

RequestAuthenticator::RequestAuthenticator()
{
}

RequestAuthenticator::~RequestAuthenticator()
{
}

RequestAuthenticator::Result
   RequestAuthenticator::checkAuthentication(ServiceChannel* ch)
{
   // anonymous authentication, nothing to set, always passes
   return Success;
}
