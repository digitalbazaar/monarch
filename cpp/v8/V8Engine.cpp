/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/v8/V8Engine.h"

using namespace std;
using namespace v8;
using namespace monarch::rt;
using namespace monarch::v8;

#define EXCEPTION_PREFIX "monarch.v8"

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

   // Create a new context and store persistent reference
   Handle<Context> context = Context::New(NULL, c->getGlobals());
   mContext = Persistent<Context>::New(context);

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

   Local<Value> jsDyno = mContext->Global()->Get(String::New(name));
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

   // We're just about to compile the script; set up an error handler to
   // catch any exceptions the script might throw.
   TryCatch tryCatch;

   // Create a string containing the JavaScript source code.
   Local< ::v8::String> source = ::v8::String::New(js);

   // Script containing the compiled source.
   Local< ::v8::Script> script;

   // Result of the script after it has been run.
   Local<Value> resultval;

   if(rval)
   {
      // Compile the source code.
      script = ::v8::Script::Compile(source);
      if(script.IsEmpty())
      {
         String::Utf8Value error(tryCatch.Exception());
         // The script failed to compile
         ExceptionRef e = new rt::Exception(
            "Script failed to compile.",
            EXCEPTION_PREFIX ".CompileError");
         e->getDetails()["error"] = *error;
         rt::Exception::set(e);
         rval = false;
      }
   }


   if(rval)
   {
      // Run the script to get the result.
      resultval = script->Run();
      if(resultval.IsEmpty())
      {
         String::Utf8Value error(tryCatch.Exception());
         // The script failed to run
         ExceptionRef e = new rt::Exception(
            "Script failed to run.",
            EXCEPTION_PREFIX ".RunError");
         e->getDetails()["error"] = *error;
         rt::Exception::set(e);
         rval = false;
      }
   }

   if(rval)
   {
      ::v8::String::AsciiValue ascii(resultval);
      result = *ascii;
   }

   return rval;
}
