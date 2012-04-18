/*
 * Copyright (c) 2007-2012 Digital Bazaar, Inc. All rights reserved.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define __STDC_CONSTANT_MACROS

#include <cstdio>

#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/data/json/JsonLd.h"
#include "monarch/data/json/JsonReader.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/logging/Logging.h"
#include "monarch/io/File.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/io/FileList.h"
#include "monarch/util/StringTools.h"
#include "monarch/validation/Validation.h"

using namespace std;
using namespace monarch::test;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;
namespace v = monarch::validation;

namespace mo_test_jsonld
{

// hack just so code compiles even w/o test suite support
#ifndef HAVE_JSON_LD_TEST_SUITE
#define JSON_LD_TEST_SUITE_DIR ""
#endif

// declare
static void _runJsonLdTestSuiteFromPath(TestRunner& tr, const char* path);

static void _readFile(const char* root, const char* name, DynamicObject& data)
{
   string fullPath;
   fullPath.assign(File::join(root, name));

   File f(fullPath.c_str());
   if(!f->exists() || !f->isFile())
   {
      ExceptionRef e = new Exception("Invalid test file.");
      e->getDetails()["name"] = name;
      Exception::set(e);
   }
   assertNoExceptionSet();
   FileInputStream is(f);
   JsonReader r(false);
   r.start(data);
   bool success = r.read(&is) && r.finish();
   if(!success)
   {
      ExceptionRef e = new Exception("Failure reading test file.");
      e->getDetails()["name"] = name;
      Exception::push(e);
   }
   assertNoExceptionSet();
   is.close();
}

// compares two @ids
static bool _compareIds(DynamicObject a, DynamicObject b)
{
   return a["@id"] < b["@id"];
}

static void _runJsonLdTestSuiteTest(
   TestRunner& tr, const char* root, DynamicObject& test)
{
   // flag to check if warning should be printed
   bool skipped = false;

   tr.test(test["name"]);

   // read input
   DynamicObject input;
   _readFile(root, test["input"], input);
   // expected output
   DynamicObject expect;

   DynamicObject output;
   DynamicObject& type = test["@type"];
   DynamicObject options;
   options["base"]->format("%s%s",
      "http://json-ld.org/test-suite/tests/",
      test["input"]->getString());
   if(type->indexOf("jld:NormalizeTest") != -1)
   {
      // read expected output
      _readFile(root, test["expect"], expect);
      // normalize
      JsonLd::normalize(input, options, output);
   }
   else if(type->indexOf("jld:ExpandTest") != -1)
   {
      // read expected output
      _readFile(root, test["expect"], expect);
      // expand
      JsonLd::expand(input, options, output);
   }
   else if(type->indexOf("jld:CompactTest") != -1)
   {
      // sanity check
      v::ValidatorRef tv = new v::Map(
         "context", new v::Type(String),
         NULL);
      assertNoException(
         tv->isValid(test));

      // read expected output
      _readFile(root, test["expect"], expect);

      // read context
      DynamicObject context;
      _readFile(root, test["context"], context);

      // compact
      JsonLd::compact(input, context["@context"], options, output);
   }
   else if(type->indexOf("jld:FrameTest") != -1)
   {
      // sanity check
      v::ValidatorRef tv = new v::Map(
         "frame", new v::Type(String),
         NULL);
      assertNoException(
         tv->isValid(test));

      // read expected output
      _readFile(root, test["expect"], expect);

      // read frame
      DynamicObject frame;
      _readFile(root, test["frame"], frame);

      // reframe
      JsonLd::frame(input, frame, options, output);
   }
   else
   {
      skipped = true;
   }

   if(!skipped && !expect.isNull() && !Exception::isSet())
   {
      if(type->indexOf("jld:NormalizeTest") != -1)
      {
         if(!JsonLd::compareNormalized(expect, output))
         {
            namedDynoCmp("expect", expect, "output", output);
         }
      }
      else
      {
         if(type->indexOf("jld:FrameTest") != -1)
         {
            // sort @graph arrays by @id
            expect["@graph"].sort(&_compareIds);
            output["@graph"].sort(&_compareIds);
         }
         namedDynoCmp("expect", expect, "output", output);
      }
   }

   tr.passIfNoException();
   if(skipped)
   {
      string warn = StringTools::format(
         "Skipped tests of type \"%s\".",
         JsonWriter::writeToString(type, true, false).c_str());
      tr.warning(warn.c_str());
   }
}

static void _runJsonLdTestSuiteManifest(TestRunner& tr, const char* path)
{
   string dirname;
   string basename;
   File::split(path, dirname, basename);

   // read manifest file
   DynamicObject manifest;
   _readFile(dirname.c_str(), basename.c_str(), manifest);

   // types
   DynamicObject manifestType;
   manifestType = "jld:Manifest";
   DynamicObject compactType;
   compactType = "jld:CompactTest";
   DynamicObject expandType;
   expandType = "jld:ExpandTest";
   DynamicObject frameType;
   frameType = "jld:FrameTest";
   DynamicObject normalizeType;
   normalizeType = "jld:NormalizeTest";

   // sanity check
   v::ValidatorRef tv = new v::Map(
      "@type", new v::Contains(manifestType),
      "name", new v::Type(String),
      "sequence", new v::Each(new v::Any(
         new v::Type(String),
         new v::All(
            new v::Map(
               "name", new v::Type(String),
               "input", new v::Type(String),
               NULL),
            new v::Any(
               new v::Map(
                  "@type", new v::Contains(compactType),
                  "context", new v::Type(String),
                  "expect", new v::Type(String),
                  NULL),
               new v::Map(
                  "@type", new v::Contains(expandType),
                  "expect", new v::Type(String),
                  NULL),
               new v::Map(
                  "@type", new v::Contains(frameType),
                  "frame", new v::Type(String),
                  "expect", new v::Type(String),
                  NULL),
               new v::Map(
                  "@type", new v::Contains(normalizeType),
                  "expect", new v::Type(String),
                  NULL),
               NULL),
            NULL),
         NULL)),
      NULL);
   if(!tv->isValid(manifest))
   {
      string warn = StringTools::format(
         "Unsupported or invalid manifest \"%s\".", path);
      tr.warning(warn.c_str());
      //dumpException();
      Exception::clear();
   }
   else
   {
      tr.group(manifest["name"]);

      // process each test
      int count = 0;
      DynamicObjectIterator i = manifest["sequence"].getIterator();
      while(i->hasNext())
      {
         tr.group(StringTools::format("%04d", ++count).c_str());
         DynamicObject& next = i->next();
         DynamicObjectType type = next->getType();
         if(type == Map)
         {
            _runJsonLdTestSuiteTest(tr, dirname.c_str(), next);
         }
         else if(type == String)
         {
            string full = File::join(dirname.c_str(), next->getString());
            _runJsonLdTestSuiteFromPath(tr, full.c_str());
         }
         tr.ungroup();
      }

      tr.ungroup();
   }
}

static void _runJsonLdTestSuiteFromPath(TestRunner& tr, const char* path)
{
   tr.group(StringTools::format("(%s)", File::basename(path).c_str()).c_str());

   File f(path);
   if(!f->exists())
   {
      tr.test("test file exists");
      tr.fail(StringTools::format("%s not found", path).c_str());
   }
   else
   {
      if(f->isFile())
      {
         _runJsonLdTestSuiteManifest(tr, path);
      }
      else if(f->isDirectory())
      {
         FileList list;
         f->listFiles(list);
         IteratorRef<File> i = list->getIterator();
         while(i->hasNext())
         {
            File& next = i->next();
            if(next->isFile())
            {
               const char* ext = next->getExtension();
               // FIXME: hack, manifests are now JSON-LD files
               if(strstr(next->getBaseName(), "manifest") != NULL &&
                  strcmp(ext, ".jsonld") == 0)
               {
                  string full = File::join(path, next->getBaseName());
                  _runJsonLdTestSuiteManifest(tr, full.c_str());
               }
            }
         }
      }
      else
      {
         tr.test("test file type");
         tr.fail(StringTools::format("%s is invalid", path).c_str());
      }
   }

   tr.ungroup();
}

static void runJsonLdTestSuite(TestRunner& tr)
{
   _runJsonLdTestSuiteFromPath(tr, JSON_LD_TEST_SUITE_DIR);
}

static void runJsonLdTests(TestRunner& tr)
{
   tr.group("JSON-LD");

   tr.test("hasProperty (no prop)");
   {
      DynamicObject d;
      d["p"] = true;
      assert(!JsonLd::hasProperty(d, "!p"));
   }
   tr.passIfNoException();

   tr.test("hasProperty (array no prop)");
   {
      DynamicObject d;
      d["p"]->setType(Array);
      assert(!JsonLd::hasProperty(d, "p"));
   }
   tr.passIfNoException();

   tr.test("hasProperty (has prop)");
   {
      DynamicObject d;
      d["p"] = "v";
      assert(JsonLd::hasProperty(d, "p"));
   }
   tr.passIfNoException();

   tr.test("hasProperty (has prop and sub-prop)");
   {
      DynamicObject d;
      d["p"]["p2"] = "v2";
      assert(JsonLd::hasProperty(d, "p"));
   }
   tr.passIfNoException();

   tr.test("hasProperty (has prop in array)");
   {
      DynamicObject d;
      d["p"]->append("v");
      assert(JsonLd::hasProperty(d, "p"));
   }
   tr.passIfNoException();

   // FIXME: add explict dyno api tests
   // assuming string api is testing dyno one

   tr.test("hasValue (simple)");
   {
      DynamicObject d;
      d["p"]->append("v");
      assert(JsonLd::hasValue(d, "p", "v"));
      assert(!JsonLd::hasValue(d, "p", "v0"));
      assert(!JsonLd::hasValue(d, "p0", "v"));
   }
   tr.passIfNoException();

   tr.test("hasValue (in array)");
   {
      DynamicObject d;
      d["p"]->append("v0");
      d["p"]->append("v1");
      d["p"]->append("v2");
      assert(JsonLd::hasValue(d, "p", "v0"));
      assert(JsonLd::hasValue(d, "p", "v1"));
      assert(JsonLd::hasValue(d, "p", "v2"));
      assert(!JsonLd::hasValue(d, "p", "v3"));
      assert(!JsonLd::hasValue(d, "p0", "v0"));
   }
   tr.passIfNoException();

   tr.test("addValue (new)");
   {
      DynamicObject d;
      JsonLd::addValue(d, "p", "v");

      assert(JsonLd::hasProperty(d, "p"));
      assert(JsonLd::hasValue(d, "p", "v"));
   }
   tr.passIfNoException();

   tr.test("addValue (add)");
   {
      DynamicObject d;
      JsonLd::addValue(d, "p", "v0");
      JsonLd::addValue(d, "p", "v1");

      assert(JsonLd::hasProperty(d, "p"));
      assert(JsonLd::hasValue(d, "p", "v0"));
      assert(JsonLd::hasValue(d, "p", "v1"));
   }
   tr.passIfNoException();

   tr.test("addValue (add 2)");
   {
      DynamicObject d;
      JsonLd::addValue(d, "p0", "v0");
      JsonLd::addValue(d, "p0", "v1");
      JsonLd::addValue(d, "p1", "v2");
      JsonLd::addValue(d, "p1", "v3");

      assert(JsonLd::hasProperty(d, "p0"));
      assert(JsonLd::hasProperty(d, "p1"));
      assert(JsonLd::hasValue(d, "p0", "v0"));
      assert(JsonLd::hasValue(d, "p0", "v1"));
      assert(JsonLd::hasValue(d, "p1", "v2"));
      assert(JsonLd::hasValue(d, "p1", "v3"));
   }
   tr.passIfNoException();

   tr.test("addValue (not list)");
   {
      DynamicObject d;
      JsonLd::addValue(d, "p", "v");
      JsonLd::addValue(d, "p", "v");

      assert(JsonLd::hasValue(d, "p", "v"));

      DynamicObject expect;
      expect["p"] = "v";
      assertNamedDynoCmp("expect", expect, "dyno", d);
   }
   tr.passIfNoException();

   tr.test("addValue (is list)");
   {
      DynamicObject d;
      JsonLd::addValue(d, "p", "v", true);
      JsonLd::addValue(d, "p", "v", true);

      DynamicObject expect;
      expect["p"][0] = "v";
      assertNamedDynoCmp("expect", expect, "dyno", d);
   }
   tr.passIfNoException();

   tr.test("getValues (empty)");
   {
      DynamicObject d;

      DynamicObject values;
      values = JsonLd::getValues(d, "p");

      DynamicObject expect(Array);
      assertNamedDynoCmp("expect", expect, "dyno", values);
   }
   tr.passIfNoException();

   tr.test("getValues (one)");
   {
      DynamicObject d;

      JsonLd::addValue(d, "p", "v");

      DynamicObject values;
      values = JsonLd::getValues(d, "p");

      DynamicObject expect;
      expect[0] = "v";
      assertNamedDynoCmp("expect", expect, "dyno", values);
   }
   tr.passIfNoException();

   tr.test("getValues (many)");
   {
      DynamicObject d;
      JsonLd::addValue(d, "p", "v0", true);
      JsonLd::addValue(d, "p", "v1", true);

      DynamicObject values;
      values = JsonLd::getValues(d, "p");

      DynamicObject expect;
      expect[0] = "v0";
      expect[1] = "v1";
      assertNamedDynoCmp("expect", expect, "dyno", values);
   }
   tr.passIfNoException();

   tr.test("removeProperty");
   {
      DynamicObject d;
      JsonLd::addValue(d, "p", "v");

      assert(JsonLd::hasProperty(d, "p"));
      JsonLd::removeProperty(d, "p");
      assert(!JsonLd::hasProperty(d, "p"));
   }
   tr.passIfNoException();

   tr.test("removeValue (none)");
   {
      DynamicObject d;

      assert(!JsonLd::hasValue(d, "p", "v"));
      JsonLd::removeValue(d, "p", "v");
      assert(!JsonLd::hasValue(d, "p", "v"));
   }
   tr.passIfNoException();

   tr.test("removeValue (one)");
   {
      DynamicObject d;
      JsonLd::addValue(d, "p", "v");

      assert(JsonLd::hasValue(d, "p", "v"));
      JsonLd::removeValue(d, "p", "v");
      assert(!JsonLd::hasValue(d, "p", "v"));
   }
   tr.passIfNoException();

   tr.test("removeValue (many)");
   {
      DynamicObject d;
      JsonLd::addValue(d, "p", "v", true);
      JsonLd::addValue(d, "p", "v", true);

      assert(JsonLd::hasValue(d, "p", "v"));
      JsonLd::removeValue(d, "p", "v");
      assert(!JsonLd::hasValue(d, "p", "v"));
   }
   tr.passIfNoException();

   tr.test("removeValue (many2)");
   {
      DynamicObject d;
      JsonLd::addValue(d, "p", "v", true);
      JsonLd::addValue(d, "p", "v", true);

      assert(JsonLd::hasValue(d, "p", "v"));
      JsonLd::removeValue(d, "p", "v");
      assert(!JsonLd::hasValue(d, "p", "v"));
      // also check if property is gone
      assert(!JsonLd::hasProperty(d, "p"));
   }
   tr.passIfNoException();

   tr.ungroup();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled() || tr.isTestEnabled("json-ld"))
   {
#ifdef HAVE_JSON_LD_TEST_SUITE
      runJsonLdTestSuite(tr);
#endif
      runJsonLdTests(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.jsonld.test", "1.0", mo_test_jsonld::run)
