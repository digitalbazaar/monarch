/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/v8/V8Controller.h"
#include "monarch/v8/V8Engine.h"

#include <v8.h>

using namespace monarch::kernel;
using namespace monarch::v8;

V8Controller::V8Controller() :
   mKernel(NULL)
{
}

V8Controller::~V8Controller()
{
}

bool V8Controller::initialize(MicroKernel* kernel)
{
   bool rval = true;

   mKernel = kernel;

   return rval;
}

void V8Controller::cleanup()
{
   mKernel = NULL;
}

bool V8Controller::createEngine(V8EngineRef& v8Engine)
{
   bool rval = true;

   v8Engine = new V8Engine();

   return rval;
}
