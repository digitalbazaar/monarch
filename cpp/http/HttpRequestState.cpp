/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpRequestState.h"

using namespace std;
using namespace monarch::http;
using namespace monarch::rt;
using namespace monarch::util;

HttpRequestState::HttpRequestState()
{
}

HttpRequestState::~HttpRequestState()
{
}

void HttpRequestState::beginRequest()
{
   mDetails = new DynamicObject();
   mDetails->setType(Map);
   mTimer.start();
}

Timer* HttpRequestState::getTimer()
{
   return &mTimer;
}

DynamicObject& HttpRequestState::getDetails()
{
   return mDetails;
}
