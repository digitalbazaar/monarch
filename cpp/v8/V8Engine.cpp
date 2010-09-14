/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/v8/V8Engine.h"

using namespace std;
using namespace v8;
using namespace monarch::rt;
using namespace monarch::v8;

V8Engine::V8Engine()
{
}

V8Engine::~V8Engine()
{
   // Dispose the persistent context.
   mContext.Dispose();
}

// FIXME: add options to control default init process
bool V8Engine::initialize(V8Controller* c)
{
   mController = c;

   // Handle scope for temporary handles.
   HandleScope handle_scope;

   // Create a new context.
   mContext = Context::New(NULL, c->getGlobals());

   return true;
}

bool V8Engine::setDynamicObject(const char* name, DynamicObject& dyno)
{
   bool rval = true;

   // lock V8 while script is running
   Locker locker;

   // Handle scope for temporary handles.
   HandleScope handleScope;

   // Enter context.
   Context::Scope contextScope(mContext);

   Handle<Object> jsDyno = V8Controller::wrapDynamicObject(&dyno);
   mContext->Global()->Set(String::New(name), jsDyno);

   return rval;
}

bool V8Engine::getDynamicObject(const char* name, DynamicObject& dyno)
{
   bool rval = true;

   // lock V8 while script is running
   Locker locker;

   // Handle scope for temporary handles.
   HandleScope handleScope;

   // Enter context.
   Context::Scope contextScope(mContext);

   Handle<Value> jsDyno = mContext->Global()->Get(String::New(name));
   dyno = V8Controller::j2d(jsDyno);

   return rval;
}

bool V8Engine::runScript(const char* js, std::string& result)
{
   bool rval = true;

   // lock V8 while script is running
   Locker locker;

   // Create a stack-allocated handle scope.
   HandleScope handle_scope;

   // Enter the engine context for compiling and running the script.
   Context::Scope context_scope(mContext);

   // Create a string containing the JavaScript source code.
   Handle< ::v8::String> source = ::v8::String::New(js);

   // Compile the source code.
   Handle< ::v8::Script> script = ::v8::Script::Compile(source);

   // Run the script to get the result.
   Handle<Value> resultval = script->Run();

   ::v8::String::AsciiValue ascii(resultval);
   result = *ascii;

   return rval;
}
