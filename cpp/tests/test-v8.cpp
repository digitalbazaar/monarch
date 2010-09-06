/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/OStreamOutputStream.h"
#include "monarch/rt/Exception.h"
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/v8/V8ModuleApi.h"

// FIXME
#include <v8.h>
using namespace v8;

using namespace std;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::test;
using namespace monarch::v8;

namespace mo_test_v8
{

static Handle<Value> LogCallback(const Arguments& args)
{
   if (args.Length() != 0)
   {
      HandleScope scope;
      Handle<Value> arg = args[0];
      String::Utf8Value value(arg);
      //HttpRequestProcessor::Log(*value);
      printf("LOG: %s\n", *value);
   }
   return v8::Undefined();
}

static string runLogScript(const char *scriptstr)
{
   // Create a stack-allocated handle scope.
   HandleScope handle_scope;

   // Create a template for the global object where we set the
   // built-in global functions.
   Handle<ObjectTemplate> global = ObjectTemplate::New();
   global->Set(String::New("log"), FunctionTemplate::New(LogCallback));

   // Create a new context.
   Persistent<Context> context = Context::New(NULL, global);

   // Enter the created context for compiling and
   // running the hello world script.
   Context::Scope context_scope(context);

   // Create a string containing the JavaScript source code.
   Handle<v8::String> source = v8::String::New(scriptstr);

   // Compile the source code.
   Handle<Script> script = Script::Compile(source);

   // Run the script to get the result.
   Handle<Value> result = script->Run();

   // Dispose the persistent context.
   context.Dispose();

   v8::String::AsciiValue ascii(result);
   string resultstr = *ascii;

   return resultstr;
}

static string runFuncScript(const char *scriptstr)
{
   // Create a stack-allocated handle scope.
   HandleScope handle_scope;

   // Create a template for the global object where we set the
   // built-in global functions.
   Handle<ObjectTemplate> global = ObjectTemplate::New();
   global->Set(String::New("log"), FunctionTemplate::New(LogCallback));

   // Create a new context.
   Persistent<Context> context = Context::New(NULL, global);

   // Enter the created context for compiling and
   // running the hello world script.
   Context::Scope context_scope(context);

   // Create a string containing the JavaScript source code.
   Handle<v8::String> source = v8::String::New(scriptstr);

   // Compile the source code.
   Handle<Script> script = Script::Compile(source);

   // Run the script to get the result.
   Handle<Value> result = script->Run();

   // Dispose the persistent context.
   context.Dispose();

   v8::String::AsciiValue ascii(result);
   string resultstr = *ascii;

   return resultstr;
}

static void runV8Test(TestRunner &tr, V8ModuleApi* v8mod)
{
   tr.group("V8");

   // create an engine
   V8EngineRef v8;
   assertNoException(
      v8mod->createEngine(v8));

   tr.test("basic");
   {
      string result;

      assertNoException(
         v8->runScript("'Hello' + ', World!'", result));
      assertStrCmp(result.c_str(), "Hello, World!");

      assertNoException(
         v8->runScript("var x = 10; x", result));
      assertStrCmp(result.c_str(), "10");
   }
   tr.passIfNoException();

   tr.test("log");
   {
      runLogScript("log('Hello, World!')");
   }
   tr.passIfNoException();

   tr.test("f");
   {
      runFuncScript("var f = function() { log('Hello, World!'); }");
   }
   tr.passIfNoException();

   tr.test("dyno");
   {
   }
   tr.passIfNoException();

   tr.ungroup();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      V8ModuleApi* v8mod = dynamic_cast<V8ModuleApi*>(
         tr.getApp()->getKernel()->getModuleApi("monarch.v8.V8"));
      assertNoExceptionSet();
      assert(v8mod != NULL);

      runV8Test(tr, v8mod);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.v8.test", "1.0", mo_test_v8::run)
