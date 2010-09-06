/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/v8/V8Engine.h"

#include <v8.h>

using namespace std;
using namespace v8;
using namespace monarch::v8;

V8Engine::V8Engine()
{
}

V8Engine::~V8Engine()
{
}

bool V8Engine::runScript(const char* js, std::string& result)
{
   bool rval = true;

   // Create a stack-allocated handle scope.
   HandleScope handle_scope;

   // Create a new context.
   Persistent<Context> context = Context::New();

   // Enter the created context for compiling and
   // running the hello world script.
   Context::Scope context_scope(context);

   // Create a string containing the JavaScript source code.
   Handle<String> source = String::New(js);

   // Compile the source code.
   Handle<Script> script = Script::Compile(source);

   // Run the script to get the result.
   Handle<Value> resultval = script->Run();

   // Dispose the persistent context.
   context.Dispose();

   String::AsciiValue ascii(resultval);
   result = *ascii;

   return rval;
}
