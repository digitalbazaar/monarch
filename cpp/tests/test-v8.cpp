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

static void runV8Test(TestRunner &tr, V8ModuleApi* v8mod)
{
   tr.group("V8");

   // create an engine
   V8EngineRef v8;
   assertNoException(
      v8mod->createEngine(v8));
   assertNoException(
      v8->initialize());

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

   // test state is preserved between calls to the same engine
   tr.test("state");
   {
      string result;

      assertNoException(
         v8->runScript("var x = 10; x", result));
      assertStrCmp(result.c_str(), "10");

      assertNoException(
         v8->runScript("x", result));
      assertStrCmp(result.c_str(), "10");
   }
   tr.passIfNoException();

   tr.test("log");
   {
      string result;

      assertNoException(
         v8->runScript("log('Hello, World!')", result));
   }
   tr.passIfNoException();

   tr.test("f");
   {
      string result;

      assertNoException(
         v8->runScript("var f = function() { log('Hello, World!'); }", result));
   }
   tr.passIfNoException();

   tr.test("monarch obj");
   {
      string result;

      assertNoException(
         v8->runScript("monarch.test", result));
      assertStrCmp(result.c_str(), "MO!");
   }
   tr.passIfNoException();

   tr.test("d2j");
   {
      string result;

      DynamicObject d;
      d["foo"] = "bar";

      assertNoException(
         v8->setDynamicObject("d", d));

      assertNoException(
         v8->runScript("d2j(d.foo)", result));
      assertStrCmp(result.c_str(), "bar");
   }
   tr.passIfNoException();

   tr.test("j2d");
   {
      string result;
      DynamicObject d;
      DynamicObject expect;

      // null
      {
         assertNoException(
            v8->runScript("d = null", result));
         assertNoException(
            v8->getDynamicObject("d", d));

         DynamicObject nullExpect(NULL);

         assertNamedDynoCmp("expect", nullExpect, "d", d);
      }

      // boolean
      {
         assertNoException(
            v8->runScript("d = true", result));
         assertNoException(
            v8->getDynamicObject("d", d));

         expect->clear();
         expect = true;

         assertNamedDynoCmp("expect", expect, "d", d);
      }

      // int
      {
         assertNoException(
            v8->runScript("d = 123", result));
         assertNoException(
            v8->getDynamicObject("d", d));

         expect->clear();
         expect = 123;

         assertNamedDynoCmp("expect", expect, "d", d);
      }

      // double
      {
         assertNoException(
            v8->runScript("d = 12.3", result));
         assertNoException(
            v8->getDynamicObject("d", d));

         expect->clear();
         expect = 12.3;

         assertNamedDynoCmp("expect", expect, "d", d);
      }

      // array
      {
         assertNoException(
            v8->runScript("d = [true, 'abc', 123]", result));
         assertNoException(
            v8->getDynamicObject("d", d));

         expect->clear();
         expect[0] = true;
         expect[1] = "abc";
         expect[2] = 123;

         assertNamedDynoCmp("expect", expect, "d", d);
      }

      // map
      {
         assertNoException(
            v8->runScript("d = {a:true, b:'abc', c:123}", result));
         assertNoException(
            v8->getDynamicObject("d", d));

         expect->clear();
         expect["a"] = true;
         expect["b"] = "abc";
         expect["c"] = 123;

         assertNamedDynoCmp("expect", expect, "d", d);
      }

      // func
      // ...

      // ext
      // ...

      // date
      // ...

      // complex json
      {
         assertNoException(
            v8->runScript("d = {a:[1,2,'abc'],b:true,c:null,d:{}}", result));

         assertNoException(
            v8->getDynamicObject("d", d));

         expect->clear();
         expect["a"][0] = 1;
         expect["a"][1] = 2;
         expect["a"][2] = "abc";
         expect["b"] = true;
         expect["c"].setNull();
         expect["d"]->setType(Map);

         assertNamedDynoCmp("expect", expect, "d", d);
      }
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
