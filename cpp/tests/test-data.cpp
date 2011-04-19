/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include <iostream>
#include <sstream>

#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/io/ByteArrayOutputStream.h"
#include "monarch/io/File.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/io/OStreamOutputStream.h"
#include "monarch/io/MutatorInputStream.h"
#include "monarch/data/CharacterSetMutator.h"
#include "monarch/data/Data.h"
#include "monarch/data/TemplateInputStream.h"
#include "monarch/data/xml/XmlReader.h"
#include "monarch/data/xml/XmlWriter.h"
#include "monarch/data/xml/DomReader.h"
#include "monarch/data/xml/DomWriter.h"
#include "monarch/data/DynamicObjectInputStream.h"
#include "monarch/data/DynamicObjectOutputStream.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/data/json/JsonReader.h"
#include "monarch/data/json/JsonLd.h"
#include "monarch/data/json/JsonLdFrame.h"
#include "monarch/data/riff/RiffChunkHeader.h"
#include "monarch/data/riff/RiffListHeader.h"
#include "monarch/data/riff/RiffFormHeader.h"
#include "monarch/logging/Logging.h"
#include "monarch/http/HttpClient.h"
#include "monarch/http/HttpConnectionServicer.h"
#include "monarch/http/HttpRequestServicer.h"
#include "monarch/modest/Kernel.h"
#include "monarch/net/Server.h"
#include "monarch/util/Data.h"
#include "monarch/util/Timer.h"
#include "monarch/util/Url.h"

using namespace std;
using namespace monarch::test;
using namespace monarch::data;
//using namespace monarch::data::avi;
using namespace monarch::data::json;
//using namespace monarch::data::mpeg;
using namespace monarch::data::riff;
using namespace monarch::data::xml;
using namespace monarch::io;
using namespace monarch::modest;
using namespace monarch::net;
using namespace monarch::http;
using namespace monarch::rt;
using namespace monarch::util;

namespace mo_test_data
{

static void runJsonValidTest(TestRunner& tr)
{
   tr.group("JSON (Valid)");

   JsonWriter jw;
   JsonReader jr;
   OStreamOutputStream os(&cout);

   const char* tests[] = {
      "{}",
      "[]",
      " []",
      "[] ",
      " [] ",
      " [ ] ",
      "[true]",
      "[false]",
      "[null]",
      "[ true]",
      "[true ]",
      "[ true ]",
      "[true, true]",
      "[true , true]",
      "[ true , true ]",
      "[0]",
      "[-0]",
      "[0.0]",
      "[-0.0]",
      "[0.0e0]",
      "[0.0e+0]",
      "[0.0e-0]",
      "[1.0]",
      "[-1.0]",
      "[1.1]",
      "[-1.1]",
      "[0,true]",
      "[[]]",
      "[[{}]]",
      "[[],[]]",
      "[[0]]",
      "[\"\"]",
      "[\"s\"]",
      "{\"k\":\"v\"}",
      "{\"k1\":1, \"k2\":2}",
      "{\"k\":[]}",
      "{\"k\":{}}",
      "[\" \\\" \\\\ \\/ \\b \\f \\n \\r \\t\"]",
      "{\"k\":true}",
      "{\"k\":0}",
      "{\"k\":10}",
      "{\"k\":-10}",
      "{\"k\":0.0e+0}",
      "{\"k\":\"v\",\"k2\":true,\"k3\":1000,\"k4\":\"v\"}",
      "[\"\\u0020\"]",
      "[\"\xc3\x84 \xc3\xa4 \xc3\x96 \xc3\xb6 \xc3\x9c \xc3\xbc \xc3\x9f\"]",
      NULL
   };

   // FIXME add: unicode escapes, raw unicode
   tr.warning("Add JSON tests for: unicode escapes, raw unicode");

   for(int i = 0; tests[i] != NULL; ++i)
   {
      char msg[50];
      snprintf(msg, 50, "Parse #%d", i);
      tr.test(msg);

      DynamicObject d;
      const char* s = tests[i];
      //printf("%s\n", s);
      ByteArrayInputStream is(s, strlen(s));
      jr.start(d);
      assertNoExceptionSet();
      jr.read(&is);
      assertNoExceptionSet();
      jr.finish();
      assertNoExceptionSet();
      //printf("%s\n", s);
      //dumpDynamicObject(d);

      tr.passIfNoException();
   }

   tr.ungroup();
}

static void runJsonInvalidTest(TestRunner& tr)
{
   tr.group("JSON (Invalid)");

   JsonWriter jw;
   JsonReader jr;
   OStreamOutputStream os(&cout);

   const char* tests[] = {
      "",
      " ",
      "{",
      "}",
      "[",
      "]",
      "{}{",
      "[][",
      "[tru]",
      "[junk]",
      "[true,]",
      "[true, ]",
      "[,true]",
      "[ ,true]",
      "[0.]",
      "[0.0e]",
      "[0.0e+]",
      "[0.0e-]",
      "[\"\0\"]",
      "[\"\\z\"]",
      "[\"\0\"]",
      "{\"k\":}",
      "{:\"v\"}",
      "{\"k\":1,}",
      "{,\"k\":1}",
      "{null:0}",
      "[\"\n\"]",
      "[\"\t\"]",
      NULL
   };

   for(int i = 0; tests[i] != NULL; ++i)
   {
      char msg[50];
      snprintf(msg, 50, "Parse #%d", i);
      tr.test(msg);

      DynamicObject d;
      const char* s = tests[i];
      //printf("%s\n", s);
      ByteArrayInputStream is(s, strlen(s));
      jr.start(d);
      assertNoExceptionSet();
      jr.read(&is);
      jr.finish();
      assertExceptionSet();
      Exception::clear();
      //jw.write(d, &os);
      //printf("\n");

      tr.passIfNoException();
   }

   tr.ungroup();
}

static void runJsonDJDTest(TestRunner& tr)
{
   tr.group("JSON (Dyno->JSON->Dyno)");

   DynamicObject dyno0;
   dyno0["email"] = "example@example.com";
   dyno0["AIM"] = "example";

   DynamicObject dyno1;
   dyno1["id"] = 2;
   dyno1["-id"] = -2;
   dyno1["floats"][0] = 0.0;
   dyno1["floats"][1] = -0.0;
   dyno1["floats"][2] = 1.0;
   dyno1["floats"][3] = -1.0;
   dyno1["floats"][4] = 1.23456789;
   dyno1["floats"][5] = -1.23456789;
   dyno1["username"] = "testuser1000";
   dyno1["l33t"] = true;
   dyno1["luser"] = false;
   dyno1["somearray"][0] = "item1";
   dyno1["somearray"][1] = "item2";
   dyno1["somearray"][2] = "item3";
   dyno1["somearray"][3] = dyno0;
   dyno1["contact"] = dyno0;

   JsonWriter jw;
   JsonReader jr;
   OStreamOutputStream os(&cout);

   DynamicObject* dynos[] = {
      &dyno0,
      &dyno1,
      NULL
   };

   for(int i = 0; dynos[i] != NULL; ++i)
   {
      char msg[50];
      snprintf(msg, 50, "Verify #%d", i);
      tr.test(msg);

      DynamicObject d = *dynos[i];

      ByteBuffer b;
      ByteArrayOutputStream bbos(&b);

      jw.setCompact(true);
      //jw.write(dyno1, &os);
      jw.write(d, &bbos);
      assertNoExceptionSet();
      b.clear();
      assertNoExceptionSet();

      jw.setCompact(false);
      jw.setIndentation(0, 3);
      //jw.write(d, &os);
      jw.write(d, &bbos);
      ByteArrayInputStream is(b.data(), b.length());
      DynamicObject dr;
      jr.start(dr);
      assertNoExceptionSet();
      jr.read(&is);
      assertNoExceptionSet();
      jr.finish();
      assertNoExceptionSet();
      //jw.write(dr, &os);
      assertNoExceptionSet();
      b.clear();

      tr.passIfNoException();
   }

   tr.ungroup();
}

static void runJsonInvalidDJTest(TestRunner& tr)
{
   tr.group("JSON (Invalid Dyno->JSON)");

   DynamicObject dyno0;
   dyno0 = 0;

   DynamicObject dyno1;
   dyno1 = "";

   DynamicObject dyno2(NULL);

   JsonWriter jw;

   DynamicObject* dynos[] = {
      &dyno0,
      &dyno1,
      &dyno2,
      NULL
   };

   for(int i = 0; dynos[i] != NULL; ++i)
   {
      char msg[50];
      snprintf(msg, 50, "Verify #%d", i);
      tr.test(msg);

      DynamicObject d = *dynos[i];

      ByteBuffer b;
      ByteArrayOutputStream bbos(&b);

      jw.setCompact(true);
      //jw.write(dyno1, &os);
      jw.write(d, &bbos);
      assertExceptionSet();
      Exception::clear();
      b.clear();
      assertNoExceptionSet();

      tr.passIfNoException();
   }

   tr.ungroup();
}

static DynamicObject makeJSONTests()
{
   int tdcount = 0;
   DynamicObject td;
   td[tdcount  ]["dyno"]->setType(Map);
   td[tdcount++]["JSON"] = "{}";
   td[tdcount  ]["dyno"]->setType(Array);
   td[tdcount++]["JSON"] = "[]";
   td[tdcount  ]["dyno"][0] = true;
   td[tdcount++]["JSON"] = "[true]";
   td[tdcount  ]["dyno"]["k"] = "v";
   td[tdcount++]["JSON"] = "{\"k\":\"v\"}";
   td[tdcount  ]["dyno"][0] = 0;
   td[tdcount++]["JSON"] = "[0]";
   td[tdcount  ]["dyno"][0] = "\n";
   td[tdcount++]["JSON"] = "[\"\\n\"]";
   td[tdcount  ]["dyno"][0] = "C:\\foo\\bar.txt";
   td[tdcount++]["JSON"] = "[\"C:\\\\foo\\\\bar.txt\"]";
   td[tdcount  ]["dyno"][0] = td[0]["dyno"];
   td[tdcount++]["JSON"] = "[{}]";
   td[tdcount  ]["dyno"][0] = -1;
   td[tdcount++]["JSON"] = "[-1]";
   td[tdcount  ]["dyno"][0] = DynamicObject(NULL);
   td[tdcount++]["JSON"] = "[null]";
   td[tdcount  ]["dyno"]["k"] = 0;
   td[tdcount++]["JSON"] = "{\"k\":0}";
   td[tdcount  ]["dyno"]["k"] = 10;
   td[tdcount++]["JSON"] = "{\"k\":10}";
   td[tdcount  ]["dyno"]["k"] = -10;
   td[tdcount++]["JSON"] = "{\"k\":-10}";
   td[tdcount  ]["dyno"]["k"]->setType(Map);
   td[tdcount++]["JSON"] = "{\"k\":{}}";
   td[tdcount  ]["dyno"][0] = "\x01";
   td[tdcount++]["JSON"] = "[\"\\u0001\"]";
   // test if UTF-16 C escapes translate into a UTF-8 JSON string
   td[tdcount  ]["dyno"][0] =
      "\u040e \u045e \u0404 \u0454 \u0490 \u0491";
   td[tdcount++]["JSON"] =
      "[\"\xd0\x8e \xd1\x9e \xd0\x84 \xd1\x94 \xd2\x90 \xd2\x91\"]";

   return td;
}

static void runJsonVerifyDJDTest(TestRunner& tr)
{
   tr.group("JSON (Verify Dyno->JSON->Dyno)");

   JsonWriter jw;
   JsonReader jr;
   OStreamOutputStream os(&cout);

   DynamicObject td = makeJSONTests();
   int tdcount = td->length();

   for(int i = 0; i < tdcount; ++i)
   {
      char msg[50];
      snprintf(msg, 50, "Verify #%d", i);
      tr.test(msg);

      DynamicObject d = td[i]["dyno"];
      const char* s = td[i]["JSON"]->getString();

      ByteBuffer b;
      ByteArrayOutputStream bbos(&b);

      jw.setCompact(true);
      //jw.write(d, &os);
      assertNoExceptionSet();
      jw.write(d, &bbos);
      assertNoExceptionSet();

      // Verify written string
      assert(strlen(s) == (unsigned int)b.length());
      assert(strncmp(s, b.data(), b.length()) == 0);

      ByteArrayInputStream is(b.data(), b.length());
      DynamicObject dr;
      jr.start(dr);
      assertNoExceptionSet();
      jr.read(&is);
      assertNoExceptionSet();
      jr.finish();
      assertNoExceptionSet();
      //jw.write(dr, &os);
      assertNoExceptionSet();
      b.clear();

      assertDynoCmp(d, dr);

      tr.passIfNoException();
   }

   tr.ungroup();
}

static DynamicObject makeJSONValueTests()
{
   DynamicObject td = makeJSONTests();
   int tdcount = td->length();

   td[tdcount  ]["dyno"] = true;
   td[tdcount++]["JSON"] = "true";
   td[tdcount  ]["dyno"] = true;
   td[tdcount++]["JSON"] = " true";
   td[tdcount  ]["dyno"] = true;
   td[tdcount++]["JSON"] = "true ";
   td[tdcount  ]["dyno"] = true;
   td[tdcount++]["JSON"] = " true ";
   td[tdcount  ]["dyno"] = "v";
   td[tdcount++]["JSON"] = "\"v\"";
   td[tdcount  ]["dyno"] = 0;
   td[tdcount++]["JSON"] = "0";
   td[tdcount  ]["dyno"] = 0;
   td[tdcount++]["JSON"] = " 0";
   td[tdcount  ]["dyno"] = 0;
   td[tdcount++]["JSON"] = "0 ";
   td[tdcount  ]["dyno"] = 0;
   td[tdcount++]["JSON"] = " 0 ";
   td[tdcount  ]["dyno"] = -1;
   td[tdcount++]["JSON"] = "-1";
   td[tdcount  ]["dyno"] = 0.0;
   td[tdcount++]["JSON"] = "0.0";
   td[tdcount  ]["dyno"] = DynamicObject(NULL);
   td[tdcount++]["JSON"] = "null";

   return td;
}

static void runJsonValueVerifyJDTest(TestRunner& tr)
{
   tr.group("JSON (verify value fragments)");

   JsonReader jr(false);

   DynamicObject td = makeJSONValueTests();
   int tdcount = td->length();

   for(int i = 0; i < tdcount; ++i)
   {
      char msg[50];
      snprintf(msg, 50, "Verify #%d", i);
      tr.test(msg);

      DynamicObject d = td[i]["dyno"];
      const char* s = td[i]["JSON"]->getString();

      ByteArrayInputStream is(s, strlen(s));
      DynamicObject dr;
      jr.start(dr);
      assertNoExceptionSet();
      jr.read(&is);
      assertNoExceptionSet();
      jr.finish();
      assertNoExceptionSet();
      //jw.write(dr, &os);
      assertNoExceptionSet();

      assertDynoCmp(d, dr);

      tr.passIfNoException();
   }

   tr.ungroup();
}

static void runJsonIOStreamTest(TestRunner& tr)
{
   tr.group("JSON I/O");

   tr.test("Input");
   /*
   DynamicObject di;
   di["title"] = "My Stuff";
   di["public"] = true;
   di["stuff"][0] = "item 1";
   di["stuff"][1] = "item 2";
   JsonWriter writer;
   writer.setCompact(true);
   DynamicObjectInputStream dois(di, &writer, false);
   ostreamstring oss;
   OStreamOutputStream os(&oss);
   dois.read(oss);
   */
   tr.passIfNoException();

   tr.test("Output");
   tr.passIfNoException();

   tr.warning("Fix JSON IO Stream test");

   tr.ungroup();
}

static void runJsonLdTest(TestRunner& tr)
{
   tr.group("JSON-LD");

   tr.test("normalize (id - reflective)");
   {
      DynamicObject in;
      in["@"] = "<http://example.org/test#example>";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test#example>";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (id - no <>)");
   {
      DynamicObject in;
      in["@"] = "http://example.org/test#example";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test#example>";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (no subject identifier)");
   {
      DynamicObject in;
      in["#"]["ex"] = "http://example.org/vocab#";
      in["a"] = "ex:Foo";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>"] =
         "<http://example.org/vocab#Foo>";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (no subject identifier plus embed w/subject)");
   {
      DynamicObject in;
      in["#"]["ex"] = "http://example.org/vocab#";
      in["a"] = "ex:Foo";
      in["ex:embed"]["@"] = "http://example.org/test#example";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"][0]["<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>"] =
         "<http://example.org/vocab#Foo>";
      expect["@"][0]["<http://example.org/vocab#embed>"] =
         "<http://example.org/test#example>";
      expect["@"][1]["@"] = "<http://example.org/test#example>";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (bnode embed)");
   {
      DynamicObject in;
      in["#"]["ex"] = "http://example.org/vocab#";
      in["@"] = "http://example.org/test#example";
      in["a"] = "ex:Foo";
      in["ex:embed"]["a"] = "ex:Bar";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test#example>";
      expect["<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>"] =
         "<http://example.org/vocab#Foo>";
      expect["<http://example.org/vocab#embed>"]
         ["<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>"] =
            "<http://example.org/vocab#Bar>";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (bnode embed w/ existing subject)");
   {
      DynamicObject in;
      in["#"]["ex"] = "http://example.org/vocab#";
      in["@"] = "http://example.org/test#example";
      in["ex:embed"]["@"] = "_:bnode1";
      in["ex:embed"]["ex:foo"] = "bar";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test#example>";
      expect["<http://example.org/vocab#embed>"]
         ["<http://example.org/vocab#foo>"] =
            "bar";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (bnode embed w/ existing subject (norm bnode))");
   {
      DynamicObject in;
      in["#"]["ex"] = "http://example.org/vocab#";
      in["@"] = "http://example.org/test#example";
      in["ex:embed"]["@"] = "<_:bnode1>";
      in["ex:embed"]["ex:foo"] = "bar";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test#example>";
      expect["<http://example.org/vocab#embed>"]
         ["<http://example.org/vocab#foo>"] =
            "bar";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   // FIMXE: add test for bnode diamond structure fails

   tr.test("normalize (multiple types)");
   {
      DynamicObject in;
      in["#"]["ex"] = "http://example.org/vocab#";
      in["@"] = "http://example.org/test#example";
      in["a"][0] = "ex:Foo";
      in["a"][1] = "ex:Bar";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test#example>";
      expect["<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>"][0] =
         "<http://example.org/vocab#Foo>";
      expect["<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>"][1] =
         "<http://example.org/vocab#Bar>";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (CURIE value)");
   {
      DynamicObject in;
      in["#"]["ex"] = "http://example.org/vocab#";
      in["@"] = "http://example.org/test#example";
      in["a"] = "ex:Foo";
      in["ex:foo"] = "<ex:Bar>";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test#example>";
      expect["<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>"] =
         "<http://example.org/vocab#Foo>";
      expect["<http://example.org/vocab#foo>"] =
         "<http://example.org/vocab#Bar>";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (single subject complex)");
   {
      DynamicObject in;
      in["#"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      in["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      in["#"]["ex"] = "http://example.org/vocab#";
      in["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      in["#"]["#types"]["ex:contains"] = "xsd:anyURI";
      in["@"] = "http://example.org/test#library";
      in["ex:contains"]["@"] = "http://example.org/test#book";
      in["ex:contains"]["dc:contributor"] = "Writer";
      in["ex:contains"]["dc:title"] = "My Book";
      in["ex:contains"]["ex:contains"]["@"] = "http://example.org/test#chapter";
      in["ex:contains"]["ex:contains"]["dc:description"] = "Fun";
      in["ex:contains"]["ex:contains"]["dc:title"] = "Chapter One";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"][0]["@"] = "<http://example.org/test#book>";
      expect["@"][0]["<http://purl.org/dc/elements/1.1/contributor>"] =
         "Writer";
      expect["@"][0]["<http://purl.org/dc/elements/1.1/title>"] = "My Book";
      expect["@"][0]["<http://example.org/vocab#contains>"] =
         "<http://example.org/test#chapter>";
      expect["@"][1]["@"] = "<http://example.org/test#chapter>";
      expect["@"][1]["<http://purl.org/dc/elements/1.1/description>"] = "Fun";
      expect["@"][1]["<http://purl.org/dc/elements/1.1/title>"] = "Chapter One";
      expect["@"][2]["@"] = "<http://example.org/test#library>";
      expect["@"][2]["<http://example.org/vocab#contains>"] =
         "<http://example.org/test#book>";
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (multiple subjects - complex)");
   {
      DynamicObject in;
      in["#"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      in["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      in["#"]["ex"] = "http://example.org/vocab#";
      in["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      in["#"]["#types"]["ex:authored"] = "xsd:anyURI";
      in["#"]["#types"]["ex:contains"] = "xsd:anyURI";
      in["@"][0]["@"] = "http://example.org/test#chapter";
      in["@"][0]["dc:description"] = "Fun";
      in["@"][0]["dc:title"] = "Chapter One";
      in["@"][1]["@"] = "http://example.org/test#jane";
      in["@"][1]["ex:authored"] = "http://example.org/test#chapter";
      in["@"][1]["foaf:name"] = "Jane";
      in["@"][2]["@"] = "http://example.org/test#john";
      in["@"][2]["foaf:name"] = "John";
      in["@"][3]["@"] = "http://example.org/test#library";
      in["@"][3]["ex:contains"]["@"] = "http://example.org/test#book";
      in["@"][3]["ex:contains"]["dc:contributor"] = "Writer";
      in["@"][3]["ex:contains"]["dc:title"] = "My Book";
      in["@"][3]["ex:contains"]["ex:contains"] =
         "http://example.org/test#chapter";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"][0]["@"] = "<http://example.org/test#book>";
      expect["@"][0]["<http://purl.org/dc/elements/1.1/contributor>"] =
         "Writer";
      expect["@"][0]["<http://purl.org/dc/elements/1.1/title>"] = "My Book";
      expect["@"][0]["<http://example.org/vocab#contains>"] =
         "<http://example.org/test#chapter>";
      expect["@"][1]["@"] = "<http://example.org/test#chapter>";
      expect["@"][1]["<http://purl.org/dc/elements/1.1/description>"] = "Fun";
      expect["@"][1]["<http://purl.org/dc/elements/1.1/title>"] = "Chapter One";
      expect["@"][2]["@"] = "<http://example.org/test#jane>";
      expect["@"][2]["<http://example.org/vocab#authored>"] =
         "<http://example.org/test#chapter>";
      expect["@"][2]["<http://xmlns.org/foaf/0.1/name>"] = "Jane";
      expect["@"][3]["@"] = "<http://example.org/test#john>";
      expect["@"][3]["<http://xmlns.org/foaf/0.1/name>"] = "John";
      expect["@"][4]["@"] = "<http://example.org/test#library>";
      expect["@"][4]["<http://example.org/vocab#contains>"] =
         "<http://example.org/test#book>";
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (datatype)");
   {
      DynamicObject in;
      in["#"]["ex"] = "http://example.org/vocab#";
      in["@"] = "http://example.org/test#example";
      in["ex:validFrom"] =
         "2011-01-25T00:00:00+0000^^"
         "<http://www.w3.org/2001/XMLSchema#dateTime>";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test#example>";
      expect["<http://example.org/vocab#validFrom>"] =
         "2011-01-25T00:00:00+0000^^"
         "<http://www.w3.org/2001/XMLSchema#dateTime>";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (type-coerced datatype)");
   {
      DynamicObject in;
      in["#"]["ex"] = "http://example.org/vocab#";
      in["#"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      in["#"]["#types"]["ex:validFrom"] = "xsd:dateTime";
      in["@"] = "http://example.org/test#example";
      in["ex:validFrom"] = "2011-01-25T00:00:00Z";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test#example>";
      expect["<http://example.org/vocab#validFrom>"] =
         "2011-01-25T00:00:00Z^^"
         "<http://www.w3.org/2001/XMLSchema#dateTime>";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (type-coerced datatype, double reference)");
   {
      DynamicObject in;
      in["#"]["ex"] = "http://example.org/vocab#";
      in["#"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      in["#"]["#types"]["ex:date"] = "xsd:dateTime";
      in["@"] = "http://example.org/test#example";
      DynamicObject date;
      date = "2011-01-25T00:00:00Z";
      in["ex:date"][0] = date;
      in["ex:date"][1] = date;

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test#example>";
      expect["<http://example.org/vocab#date>"][0] =
         "2011-01-25T00:00:00Z^^"
         "<http://www.w3.org/2001/XMLSchema#dateTime>";
      expect["<http://example.org/vocab#date>"][1] =
         "2011-01-25T00:00:00Z^^"
         "<http://www.w3.org/2001/XMLSchema#dateTime>";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (type-coerced datatype, cycle)");
   {
      DynamicObject in;
      in["#"]["ex"] = "http://example.org/vocab#";
      in["#"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      in["#"]["#types"]["ex:date"] = "xsd:dateTime";
      in["#"]["#types"]["ex:parent"] = "xsd:anyURI";
      in["@"] = "http://example.org/test#example1";
      in["ex:date"] = "2011-01-25T00:00:00Z";
      in["ex:embed"]["@"] = "http://example.org/test#example2";
      in["ex:embed"]["ex:parent"] = "http://example.org/test#example1";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"][0]["@"] = "<http://example.org/test#example1>";
      expect["@"][0]["<http://example.org/vocab#date>"] =
         "2011-01-25T00:00:00Z^^"
         "<http://www.w3.org/2001/XMLSchema#dateTime>";
      expect["@"][0]["<http://example.org/vocab#embed>"] =
         "<http://example.org/test#example2>";
      expect["@"][1]["@"] = "<http://example.org/test#example2>";
      expect["@"][1]["<http://example.org/vocab#parent>"] =
         "<http://example.org/test#example1>";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("remove context (id)");
   {
      DynamicObject in;
      in["@"] = "http://example.org/test#example";

      DynamicObject out;
      assertNoException(
         JsonLd::removeContext(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test#example>";
      assertNamedDynoCmp("expect", expect, "out", out);
   }
   tr.passIfNoException();

   tr.test("remove context (complex)");
   {
      DynamicObject in;
      in["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      in["#"]["ex"] = "http://example.org/vocab#";
      in["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      in["@"][0]["@"] = "http://example.org/test#chapter";
      in["@"][0]["dc:description"] = "Fun";
      in["@"][0]["dc:title"] = "Chapter One";
      in["@"][1]["@"] = "http://example.org/test#jane";
      in["@"][1]["ex:authored"] = "<http://example.org/test#chapter>";
      in["@"][1]["foaf:name"] = "Jane";
      in["@"][2]["@"] = "http://example.org/test#john";
      in["@"][2]["foaf:name"] = "John";
      in["@"][3]["@"] = "http://example.org/test#library";
      in["@"][3]["ex:contains"]["@"] = "http://example.org/test#book";
      in["@"][3]["ex:contains"]["dc:contributor"] = "Writer";
      in["@"][3]["ex:contains"]["dc:title"] = "My Book";
      in["@"][3]["ex:contains"]["ex:contains"] =
         "<http://example.org/test#chapter>";

      DynamicObject out;
      assertNoException(
         JsonLd::removeContext(in, out));

      DynamicObject expect;
      expect["@"][0]["@"] = "<http://example.org/test#chapter>";
      expect["@"][0]["<http://purl.org/dc/elements/1.1/description>"] = "Fun";
      expect["@"][0]["<http://purl.org/dc/elements/1.1/title>"] = "Chapter One";
      expect["@"][1]["@"] = "<http://example.org/test#jane>";
      expect["@"][1]["<http://example.org/vocab#authored>"] =
         "<http://example.org/test#chapter>";
      expect["@"][1]["<http://xmlns.org/foaf/0.1/name>"] = "Jane";
      expect["@"][2]["@"] = "<http://example.org/test#john>";
      expect["@"][2]["<http://xmlns.org/foaf/0.1/name>"] = "John";
      expect["@"][3]["@"] = "<http://example.org/test#library>";
      expect["@"][3]["<http://example.org/vocab#contains>"]["@"] =
         "<http://example.org/test#book>";
      expect["@"][3]["<http://example.org/vocab#contains>"]
         ["<http://purl.org/dc/elements/1.1/contributor>"] = "Writer";
      expect["@"][3]["<http://example.org/vocab#contains>"]
         ["<http://purl.org/dc/elements/1.1/title>"] = "My Book";
      expect["@"][3]["<http://example.org/vocab#contains>"]
         ["<http://example.org/vocab#contains>"] =
            "<http://example.org/test#chapter>";
      assertNamedDynoCmp("expect", expect, "result", out);
   }
   tr.passIfNoException();

   tr.test("add context");
   {
      DynamicObject in;
      in["@"][0]["@"] = "<http://example.org/test#chapter>";
      in["@"][0]["<http://purl.org/dc/elements/1.1/description>"] = "Fun";
      in["@"][0]["<http://purl.org/dc/elements/1.1/title>"] = "Chapter One";
      in["@"][1]["@"] = "<http://example.org/test#jane>";
      in["@"][1]["<http://example.org/vocab#authored>"] =
         "<http://example.org/test#chapter>";
      in["@"][1]["<http://xmlns.org/foaf/0.1/name>"] = "Jane";
      in["@"][2]["@"] = "<http://example.org/test#john>";
      in["@"][2]["<http://xmlns.org/foaf/0.1/name>"] = "John";
      in["@"][3]["@"] = "<http://example.org/test#library>";
      in["@"][3]["<http://example.org/vocab#contains>"]["@"] =
         "<http://example.org/test#book>";
      in["@"][3]["<http://example.org/vocab#contains>"]
         ["<http://purl.org/dc/elements/1.1/contributor>"] = "Writer";
      in["@"][3]["<http://example.org/vocab#contains>"]
         ["<http://purl.org/dc/elements/1.1/title>"] = "My Book";
      in["@"][3]["<http://example.org/vocab#contains>"]
         ["<http://example.org/vocab#contains>"] =
            "<http://example.org/test#chapter>";

      DynamicObject ctx;
      ctx["dc"] = "http://purl.org/dc/elements/1.1/";
      ctx["ex"] = "http://example.org/vocab#";
      ctx["foaf"] = "http://xmlns.org/foaf/0.1/";
      DynamicObject out;
      assertNoException(
         JsonLd::addContext(ctx, in, out));

      DynamicObject expect;
      expect["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      expect["#"]["ex"] = "http://example.org/vocab#";
      expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"][0]["@"] = "http://example.org/test#chapter";
      expect["@"][0]["dc:description"] = "Fun";
      expect["@"][0]["dc:title"] = "Chapter One";
      expect["@"][1]["@"] = "http://example.org/test#jane";
      expect["@"][1]["ex:authored"] = "<http://example.org/test#chapter>";
      expect["@"][1]["foaf:name"] = "Jane";
      expect["@"][2]["@"] = "http://example.org/test#john";
      expect["@"][2]["foaf:name"] = "John";
      expect["@"][3]["@"] = "http://example.org/test#library";
      expect["@"][3]["ex:contains"]["@"] = "http://example.org/test#book";
      expect["@"][3]["ex:contains"]["dc:contributor"] = "Writer";
      expect["@"][3]["ex:contains"]["dc:title"] = "My Book";
      expect["@"][3]["ex:contains"]["ex:contains"] =
         "<http://example.org/test#chapter>";
      assertNamedDynoCmp("expect", expect, "result", out);
   }
   tr.passIfNoException();

   tr.test("add context (reduced context)");
   {
      DynamicObject in;
      in["@"] = "<http://example.org/test#thing>";
      in["<http://purl.org/dc/terms/title>"] = "Title";

      DynamicObject ctx;
      ctx["dc"] = "http://purl.org/dc/terms/";
      ctx["ex"] = "http://example.org/test#";
      ctx["foaf"] = "http://xmlns.org/foaf/0.1/";
      DynamicObject out;
      assertNoException(
         JsonLd::addContext(ctx, in, out));

      DynamicObject expect;
      expect["#"]["dc"] = "http://purl.org/dc/terms/";
      expect["#"]["ex"] = "http://example.org/test#";
      expect["@"] = "ex:thing";
      expect["dc:title"] = "Title";

      assertNamedDynoCmp("expect", expect, "result", out);
   }
   tr.passIfNoException();

   tr.test("add context (no <> on ex:contains)");
   {
      DynamicObject in;
      in["@"] = "<http://example.org/test#book>";
      in["<http://example.org/vocab#contains>"] =
         "<http://example.org/test#chapter>";
      in["<http://purl.org/dc/elements/1.1/title>"] = "Title";

      DynamicObject ctx;
      ctx["dc"] = "http://purl.org/dc/elements/1.1/";
      ctx["ex"] = "http://example.org/vocab#";
      ctx["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      ctx["#types"]["ex:contains"] = "xsd:anyURI";

      DynamicObject out;
      assertNoException(
         JsonLd::addContext(ctx, in, out));

      DynamicObject expect;
      expect["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      expect["#"]["ex"] = "http://example.org/vocab#";
      expect["#"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      expect["#"]["#types"]["ex:contains"] = "xsd:anyURI";
      expect["@"] = "http://example.org/test#book";
      expect["ex:contains"] = "http://example.org/test#chapter";
      expect["dc:title"] = "Title";

      assertNamedDynoCmp("expect", expect, "result", out);
   }
   tr.passIfNoException();

   tr.test("remove context (input has no <> on ex:contains)");
   {
      DynamicObject in;
      in["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      in["#"]["ex"] = "http://example.org/vocab#";
      in["#"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      in["#"]["#types"]["ex:contains"] = "xsd:anyURI";
      in["@"] = "http://example.org/test#book";
      in["ex:contains"] = "http://example.org/test#chapter";
      in["dc:title"] = "Title";

      DynamicObject out;
      assertNoException(
         JsonLd::removeContext(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test#book>";
      expect["<http://example.org/vocab#contains>"] =
         "<http://example.org/test#chapter>";
      expect["<http://purl.org/dc/elements/1.1/title>"] = "Title";

      assertNamedDynoCmp("expect", expect, "result", out);
   }
   tr.passIfNoException();

   tr.test("remove type-coercion context and re-add");
   {
      DynamicObject ctx;
      ctx["ex"] = "http://example.org/vocab#";
      ctx["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      ctx["#types"]["ex:date"] = "xsd:dateTime";
      ctx["#types"]["ex:parent"] = "xsd:anyURI";

      DynamicObject in;
      in["#"] = ctx.clone();
      in["@"] = "http://example.org/test#example1";
      in["ex:date"] = "2011-01-25T00:00:00Z";
      in["ex:embed"]["@"] = "http://example.org/test#example2";
      in["ex:embed"]["ex:parent"] = "http://example.org/test#example1";

      DynamicObject expect = in.clone();

      // remove context
      DynamicObject woCtx;
      assertNoException(
         JsonLd::removeContext(in, woCtx));
      in = woCtx;

      // re-add context
      DynamicObject out;
      assertNoException(
         JsonLd::addContext(ctx, in, out));

      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("change context");
   {
      DynamicObject in;
      in["#"]["d"] = "http://purl.org/dc/elements/1.1/";
      in["#"]["e"] = "http://example.org/vocab#";
      in["#"]["f"] = "http://xmlns.org/foaf/0.1/";
      in["@"][0]["@"] = "http://example.org/test#chapter";
      in["@"][0]["d:description"] = "Fun";
      in["@"][0]["d:title"] = "Chapter One";
      in["@"][1]["@"] = "http://example.org/test#jane";
      in["@"][1]["e:authored"] = "http://example.org/test#chapter";
      in["@"][1]["f:name"] = "Jane";
      in["@"][2]["@"] = "http://example.org/test#john";
      in["@"][2]["f:name"] = "John";
      in["@"][3]["@"] = "http://example.org/test#library";
      in["@"][3]["e:contains"]["@"] = "http://example.org/test#book";
      in["@"][3]["e:contains"]["d:contributor"] = "Writer";
      in["@"][3]["e:contains"]["d:title"] = "My Book";
      in["@"][3]["e:contains"]["e:contains"] =
         "http://example.org/test#chapter";

      DynamicObject ctx;
      ctx["dc"] = "http://purl.org/dc/elements/1.1/";
      ctx["ex"] = "http://example.org/vocab#";
      ctx["foaf"] = "http://xmlns.org/foaf/0.1/";
      DynamicObject out;
      assertNoException(
         JsonLd::changeContext(ctx, in, out));

      DynamicObject expect;
      expect["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      expect["#"]["ex"] = "http://example.org/vocab#";
      expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"][0]["@"] = "http://example.org/test#chapter";
      expect["@"][0]["dc:description"] = "Fun";
      expect["@"][0]["dc:title"] = "Chapter One";
      expect["@"][1]["@"] = "http://example.org/test#jane";
      expect["@"][1]["ex:authored"] = "http://example.org/test#chapter";
      expect["@"][1]["foaf:name"] = "Jane";
      expect["@"][2]["@"] = "http://example.org/test#john";
      expect["@"][2]["foaf:name"] = "John";
      expect["@"][3]["@"] = "http://example.org/test#library";
      expect["@"][3]["ex:contains"]["@"] = "http://example.org/test#book";
      expect["@"][3]["ex:contains"]["dc:contributor"] = "Writer";
      expect["@"][3]["ex:contains"]["dc:title"] = "My Book";
      expect["@"][3]["ex:contains"]["ex:contains"] =
         "http://example.org/test#chapter";
      assertNamedDynoCmp("expect", expect, "result", out);
   }
   tr.passIfNoException();

   tr.test("change context (native types)");
   {
      DynamicObject in;
      in["#"]["d"] = "http://purl.org/dc/elements/1.1/";
      in["#"]["e"] = "http://example.org/vocab#";
      in["#"]["f"] = "http://xmlns.org/foaf/0.1/";
      in["@"] = "http://example.org/test";
      in["e:bool"] = true;
      in["e:double"] = 1.23;
      in["e:double-zero"] = 0.0;
      in["e:int"] = 123;

      DynamicObject ctx;
      ctx["dc"] = "http://purl.org/dc/elements/1.1/";
      ctx["ex"] = "http://example.org/vocab#";
      ctx["foaf"] = "http://xmlns.org/foaf/0.1/";
      DynamicObject out;
      assertNoException(
         JsonLd::changeContext(ctx, in, out));

      DynamicObject expect;
      expect["#"]["ex"] = "http://example.org/vocab#";
      expect["@"] = "http://example.org/test";
      expect["ex:bool"] = true;
      expect["ex:double"] = 1.230000e+00;
      expect["ex:double-zero"] = 0.000000e+00;
      expect["ex:int"] = 123;
      assertNamedDynoCmp("expect", expect, "result", out);
   }
   tr.passIfNoException();

   tr.test("normalize (check types)");
   {
      DynamicObject in;
      in["#"]["d"] = "http://purl.org/dc/elements/1.1/";
      in["#"]["e"] = "http://example.org/vocab#";
      in["#"]["f"] = "http://xmlns.org/foaf/0.1/";
      in["@"] = "http://example.org/test";
      in["e:bool"] = true;
      in["e:double"] = 1.23;
      in["e:double-zero"] = 0.0;
      in["e:int"] = 123;

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test>";
      expect["<http://example.org/vocab#bool>"] =
         "true^^<http://www.w3.org/2001/XMLSchema#boolean>";
      expect["<http://example.org/vocab#double>"] =
         "1.230000e+00^^<http://www.w3.org/2001/XMLSchema#double>";
      expect["<http://example.org/vocab#double-zero>"] =
         "0.000000e+00^^<http://www.w3.org/2001/XMLSchema#double>";
      expect["<http://example.org/vocab#int>"] =
         "123^^<http://www.w3.org/2001/XMLSchema#integer>";
      assertNamedDynoCmp("expect", expect, "result", out);
   }
   tr.passIfNoException();

   tr.test("normalize (double <<*>> check)");
   {
      DynamicObject in;
      in["#"]["#types"]["ex:int"] = "xsd:integer";
      in["#"]["ex"] = "http://example.org/vocab#";
      in["#"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      in["@"] = "http://example.org/test";
      in["ex:int"] = 123;

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect["@"] = "<http://example.org/test>";
      expect["<http://example.org/vocab#int>"] =
         "123^^<http://www.w3.org/2001/XMLSchema#integer>";
      assertNamedDynoCmp("expect", expect, "result", out);
   }
   tr.passIfNoException();

   tr.test("change context (native types w/ type map)");
   {
      DynamicObject in;
      in["#"]["e"] = "http://example.org/vocab#";
      in["@"] = "http://example.org/test";
      in["e:bool"] = true;
      in["e:double"] = 1.23;
      in["e:int"] = 123;

      DynamicObject ctx;
      ctx["ex"] = "http://example.org/vocab#";
      ctx["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      ctx["#types"]["ex:bool"] = "xsd:boolean";
      ctx["#types"]["ex:double"] = "xsd:double";
      ctx["#types"]["ex:int"] = "xsd:integer";
      DynamicObject out;
      assertNoException(
         JsonLd::changeContext(ctx, in, out));

      DynamicObject expect;
      expect["#"] = ctx;
      expect["@"] = "http://example.org/test";
      expect["ex:bool"] = true;
      expect["ex:double"] = 1.23;
      expect["ex:int"] = 123;
      assertNamedDynoCmp("expect", expect, "result", out);
   }
   tr.passIfNoException();

   tr.test("add context (with types)");
   {
      DynamicObject in;
      in["@"] = "<http://example.org/test>";
      in["<http://example.org/test#int>"] =
         "123^^<http://www.w3.org/2001/XMLSchema#integer>";

      DynamicObject ctx;
      ctx["ex"] = "http://example.org/test#";
      ctx["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      ctx["#types"]["ex:int"] = "xsd:integer";

      DynamicObject out;
      assertNoException(
         JsonLd::addContext(ctx, in, out));

      DynamicObject expect;
      expect["#"]["ex"] = "http://example.org/test#";
      expect["#"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      expect["#"]["#types"]["ex:int"] = "xsd:integer";
      expect["@"] = "http://example.org/test";
      expect["ex:int"] = "123";

      assertNamedDynoCmp("expect", expect, "result", out);
   }
   tr.passIfNoException();

   // common reframe test data and expected results
   DynamicObject reframeData;
   DynamicObject reframeExpect;
   {
      DynamicObject& d = reframeData["library"];
      d["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      d["#"]["ex"] = "http://example.org/vocab#";
      d["@"][0]["@"] = "http://example.org/test#library";
      d["@"][0]["a"] = "ex:Library";
      d["@"][0]["ex:contains"] = "<http://example.org/test#book>";
      d["@"][1]["@"] = "<http://example.org/test#book>";
      d["@"][1]["a"] = "ex:Book";
      d["@"][1]["dc:contributor"] = "Writer";
      d["@"][1]["dc:title"] = "My Book";
      d["@"][1]["ex:contains"] = "<http://example.org/test#chapter>";
      d["@"][2]["@"] = "http://example.org/test#chapter";
      d["@"][2]["a"] = "ex:Chapter";
      d["@"][2]["dc:description"] = "Fun";
      d["@"][2]["dc:title"] = "Chapter One";

      DynamicObject& e = reframeExpect["library"];
      e["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      e["#"]["ex"] = "http://example.org/vocab#";
      e["@"] = "http://example.org/test#library";
      e["a"] = "ex:Library";
      e["ex:contains"]["@"] = "http://example.org/test#book";
      e["ex:contains"]["a"] = "ex:Book";
      e["ex:contains"]["dc:contributor"] = "Writer";
      e["ex:contains"]["dc:title"] = "My Book";
      e["ex:contains"]["ex:contains"]["@"] =
         "http://example.org/test#chapter";
      e["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      e["ex:contains"]["ex:contains"]["dc:description"] = "Fun";
      e["ex:contains"]["ex:contains"]["dc:title"] = "Chapter One";
   }
   {
      DynamicObject& d = reframeData["top"];
      d["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      d["#"]["ex"] = "http://example.org/vocab#";
      d["@"] = "http://example.org/test#library";
      d["a"] = "ex:Library";
      d["ex:contains"] = "<http://example.org/test#book>";

      DynamicObject& e = reframeExpect["top"];
      e = d.clone();
      e["#"]->removeMember("dc");
   }

   tr.test("reframe");
   {
      DynamicObject in = reframeData["library"];

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      frame["ex:contains"]["a"] = "ex:Book";
      frame["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      JsonLdFrame jlf;
      jlf.setFrame(frame);

      DynamicObject out;
      assertNoException(
         jlf.reframe(in, out));

      DynamicObject expect = reframeExpect["library"];
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("reframe w/CURIE value");
   {
      DynamicObject in;
      in["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      in["#"]["ex"] = "http://example.org/vocab#";
      in["@"][0]["@"] = "http://example.org/test#library";
      in["@"][0]["a"] = "ex:Library";
      in["@"][0]["ex:contains"] = "<http://example.org/test#book>";
      in["@"][1]["@"] = "<http://example.org/test#book>";
      in["@"][1]["a"] = "ex:Book";
      in["@"][1]["dc:contributor"] = "Writer";
      in["@"][1]["dc:title"] = "My Book";
      in["@"][1]["ex:contains"] = "<http://example.org/test#chapter>";
      in["@"][2]["@"] = "http://example.org/test#chapter";
      in["@"][2]["a"] = "ex:Chapter";
      in["@"][2]["dc:description"] = "Fun";
      in["@"][2]["dc:title"] = "Chapter One";
      in["@"][2]["ex:act"] = "<ex:ActOne>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      frame["ex:contains"]["a"] = "ex:Book";
      frame["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      JsonLdFrame jlf;
      jlf.setFrame(frame);

      DynamicObject out;
      assertNoException(
         jlf.reframe(in, out));

      DynamicObject expect;
      expect["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      expect["#"]["ex"] = "http://example.org/vocab#";
      expect["@"] = "http://example.org/test#library";
      expect["a"] = "ex:Library";
      expect["ex:contains"]["@"] = "http://example.org/test#book";
      expect["ex:contains"]["a"] = "ex:Book";
      expect["ex:contains"]["dc:contributor"] = "Writer";
      expect["ex:contains"]["dc:title"] = "My Book";
      expect["ex:contains"]["ex:contains"]["@"] =
         "http://example.org/test#chapter";
      expect["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      expect["ex:contains"]["ex:contains"]["dc:description"] = "Fun";
      expect["ex:contains"]["ex:contains"]["dc:title"] = "Chapter One";
      expect["ex:contains"]["ex:contains"]["ex:act"] = "<ex:ActOne>";
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("reframe (empty)");
   {
      DynamicObject in = reframeData["library"];

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:BOGUS";
      JsonLdFrame jlf;
      jlf.setFrame(frame);

      DynamicObject out;
      assertNoException(
         jlf.reframe(in, out));

      DynamicObject expect;
      expect->setType(Map);
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("reframe (id)");
   {
      DynamicObject in = reframeData["library"];

      DynamicObject frame;
      frame["@"] = "http://example.org/test#library";
      JsonLdFrame jlf;
      jlf.setFrame(frame);

      DynamicObject out;
      assertNoException(
         jlf.reframe(in, out));

      DynamicObject expect = reframeExpect["library"];
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("reframe (type)");
   {
      DynamicObject in = reframeData["library"];

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      JsonLdFrame jlf;
      jlf.setFrame(frame);

      DynamicObject out;
      assertNoException(
         jlf.reframe(in, out));

      DynamicObject expect = reframeExpect["library"];
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("reframe (cleared output)");
   {
      DynamicObject in = reframeData["library"];

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      JsonLdFrame jlf;
      jlf.setFrame(frame);

      DynamicObject out;
      // add some data to see if out is cleared properly
      out["ex:bogus"] = "data";
      assertNoException(
         jlf.reframe(in, out));

      DynamicObject expect = reframeExpect["library"];
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("reframe (top level)");
   {
      DynamicObject in = reframeData["top"];

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      JsonLdFrame jlf;
      jlf.setFrame(frame);

      DynamicObject out;
      assertNoException(
         jlf.reframe(in, out));

      DynamicObject expect = reframeExpect["top"];
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("INPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.group("filter");
   {
      DynamicObject ctx;
      ctx["dc"] = "http://purl.org/dc/term/";
      ctx["ex"] = "http://example.org/vocab#";

      // for simple tests
      DynamicObject in;
      in["#"] = ctx;
      in["@"][0]["@"] = "ex:test-1";
      in["@"][0]["a"] = "ex:MyType";
      in["@"][0]["dc:title"] = "Test 1";
      in["@"][1]["@"] = "ex:test-2";
      in["@"][1]["a"] = "ex:MyType";
      in["@"][1]["dc:title"] = "Test 2";
      // for deep tests
      DynamicObject in2;
      in2["ex:p-1"]["@"] = "ex:test-2-1";
      in2["ex:p-1"]["a"] = "ex:MyType2";
      in2["ex:p-1"]["dc:title"] = "Test 2-1";
      in2["ex:p-2"]["@"] = "ex:test-2-2";
      in2["ex:p-2"]["a"] = "ex:MyType2";
      in2["ex:p-2"]["dc:title"] = "Test 2-2";
      in["@"][2] = in2;
      // for multi value tests
      DynamicObject in3;
      in3["ex:p-1"]["@"] = "ex:test-3-1";
      in3["ex:p-1"]["a"] = "ex:MyType3";
      in3["ex:p-1"]["dc:title"] = "Test 3-1";
      in3["ex:p-2"]["@"] = "ex:test-3-2";
      in3["ex:p-2"]["a"][0] = "ex:MyType3";
      in3["ex:p-2"]["a"][1] = "ex:MyType4";
      in3["ex:p-2"]["dc:title"] = "Test 3-2";
      in["@"][3] = in3;

      tr.test("id");
      {
         DynamicObject filter;
         filter["#"] = ctx;
         filter["@"] = "ex:test-1";

         DynamicObject out;
         assertNoException(
            JsonLd::filter(ctx, filter, in, out));

         DynamicObject expect;
         expect["#"] = ctx;
         expect["@"][0]["@"] = "ex:test-1";
         expect["@"][0]["a"] = "ex:MyType";
         expect["@"][0]["dc:title"] = "Test 1";

         assertNamedDynoCmp("expect", expect, "result", out);
      }
      tr.passIfNoException();

      tr.test("simplified id");
      {
         DynamicObject filter;
         filter["#"] = ctx;
         filter["@"] = "ex:test-1";

         DynamicObject out;
         assertNoException(
            JsonLd::filter(ctx, filter, in, out, true));

         DynamicObject expect;
         expect["#"] = ctx;
         expect["@"] = "ex:test-1";
         expect["a"] = "ex:MyType";
         expect["dc:title"] = "Test 1";

         assertNamedDynoCmp("expect", expect, "result", out);
      }
      tr.passIfNoException();

      tr.test("type");
      {
         DynamicObject filter;
         filter["#"] = ctx;
         filter["a"] = "ex:MyType";

         DynamicObject out;
         assertNoException(
            JsonLd::filter(ctx, filter, in, out));

         DynamicObject expect;
         expect["#"] = ctx;
         expect["@"][0]["@"] = "ex:test-1";
         expect["@"][0]["a"] = "ex:MyType";
         expect["@"][0]["dc:title"] = "Test 1";
         expect["@"][1]["@"] = "ex:test-2";
         expect["@"][1]["a"] = "ex:MyType";
         expect["@"][1]["dc:title"] = "Test 2";

         assertNamedDynoCmp("expect", expect, "result", out);
      }
      tr.passIfNoException();

      tr.test("deep");
      {
         DynamicObject filter;
         filter["#"] = ctx;
         filter["a"] = "ex:MyType2";

         DynamicObject out;
         assertNoException(
            JsonLd::filter(ctx, filter, in, out));

         DynamicObject expect;
         expect["#"] = ctx;
         expect["@"][0]["@"] = "ex:test-2-1";
         expect["@"][0]["a"] = "ex:MyType2";
         expect["@"][0]["dc:title"] = "Test 2-1";
         expect["@"][1]["@"] = "ex:test-2-2";
         expect["@"][1]["a"] = "ex:MyType2";
         expect["@"][1]["dc:title"] = "Test 2-2";

         assertNamedDynoCmp("expect", expect, "result", out);
      }
      tr.passIfNoException();

      tr.test("multi value");
      {
         DynamicObject filter;
         filter["#"] = ctx;
         filter["a"][0] = "ex:MyType3";
         filter["a"][1] = "ex:MyType4";

         DynamicObject out;
         assertNoException(
            JsonLd::filter(ctx, filter, in, out));

         DynamicObject expect;
         expect["#"] = ctx;
         expect["@"][0]["@"] = "ex:test-3-2";
         expect["@"][0]["a"][0] = "ex:MyType3";
         expect["@"][0]["a"][1] = "ex:MyType4";
         expect["@"][0]["dc:title"] = "Test 3-2";

         assertNamedDynoCmp("expect", expect, "result", out);
      }
      tr.passIfNoException();

      tr.test("graph completeness");
      {
         // FIXME: make a graph with internal references such that a simple
         // search will result in an incomplete output graph.
      }
      tr.passIfNoException();
   }
   tr.ungroup();

   tr.ungroup();
}

static void runXmlReaderTest(TestRunner& tr)
{
   tr.test("XmlReader");

   {
      string xml =
         "<null/>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      assert(dyno.isNull());
   }

   {
      string xml =
         "<object/>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      assert(dyno->length() == 0);
   }

   {
      string xml =
         "<string>This is some content.</string>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      assertStrCmp(dyno->getString(), "This is some content.");
   }

   {
      string xml =
         "<object>"
          "<member name=\"child\">"
           "<string>This is some content.</string>"
          "</member>"
         "</object>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      assertStrCmp(dyno["child"]->getString(), "This is some content.");
   }

   {
      string xml =
         "<object>"
          "<member>"
           "<string>This is some content.</string>"
          "</member>"
         "</object>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      assertExceptionSet();
      //printf("\n%s\n", Exception::getLast()->getMessage());
      Exception::clear();
   }

   {
      string xml =
         "<object>"
          "<member name=\"child\">"
           "<string>This is some content.</string>"
          "</member>"
          "<member name=\"nullChild\">"
           "<null/>"
          "</member>"
          "<member name=\"nullChild2\">"
           "<null>baddata</null>"
          "</member>"
         "</object>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      assertStrCmp(dyno["child"]->getString(), "This is some content.");
      assert(dyno->hasMember("nullChild"));
      assert(dyno->hasMember("nullChild2"));
      assert(dyno["nullChild"].isNull());
      assert(dyno["nullChild2"].isNull());
   }

   {
      string xml =
         "<object>"
          "<member name=\"myarray\">"
           "<array>"
            "<element index=\"0\">"
             "<string>element 1</string>"
            "</element>"
            "<element index=\"2\">"
             "<string>element 3</string>"
            "</element>"
            "<element index=\"1\">"
             "<string/>"
            "</element>"
           "</array>"
          "</member>"
         "</object>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      assert(dyno["myarray"]->length() == 3);
      assertStrCmp(dyno["myarray"][0]->getString(), "element 1");
      assert(dyno["myarray"][1]->length() == 0);
      assertStrCmp(dyno["myarray"][2]->getString(), "element 3");
   }

   {
      string xml =
         "<object>"
          "<member name=\"myarray\">"
           "<array>"
            "<element index=\"0\">"
             "<number>514</number>"
            "</element>"
            "<element index=\"1\">"
             "<number>5.14</number>"
            "</element>"
            "<element index=\"2\">"
             "<number>-514</number>"
            "</element>"
            "<element index=\"3\">"
             "<number>-5.14</number>"
            "</element>"
           "</array>"
          "</member>"
         "</object>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      //dumpDynamicObject(dyno);

      assert(dyno["myarray"]->length() == 4);
      assert(dyno["myarray"][0]->getUInt32() == 514);
      assert(dyno["myarray"][1]->getDouble() == 5.14);
      assert(dyno["myarray"][2]->getInt32() == -514);
      assert(dyno["myarray"][3]->getDouble() == -5.14);
   }

   tr.passIfNoException();
}

static void runXmlWriterTest(TestRunner& tr)
{
   tr.test("XmlWriter");

   {
      DynamicObject dyno;

      XmlWriter writer;
      ostringstream oss;
      OStreamOutputStream os(&oss);

      writer.write(dyno, &os);
      assertStrCmp(oss.str().c_str(), "<string/>");

      os.close();
   }

   {
      DynamicObject dyno;
      dyno = 5;

      XmlWriter writer;
      ostringstream oss;
      OStreamOutputStream os(&oss);

      writer.write(dyno, &os);
      assertStrCmp(oss.str().c_str(), "<number>5</number>");

      os.close();
   }

   {
      DynamicObject dyno;
      dyno[0] = 5;
      dyno[1] = 1;
      dyno[2] = 4;

      XmlWriter writer;
      ostringstream oss;
      OStreamOutputStream os(&oss);

      writer.write(dyno, &os);
      assertStrCmp(oss.str().c_str(),
         "<array>"
         "<element index=\"0\">"
            "<number>5</number>"
         "</element>"
         "<element index=\"1\">"
            "<number>1</number>"
         "</element>"
         "<element index=\"2\">"
            "<number>4</number>"
         "</element>"
         "</array>");

      os.close();
   }

   {
      DynamicObject dyno;
      dyno["aNumber"] = 514;
      dyno["cow"] = "Moooooooo";

      DynamicObject child;
      child["id"] = 514;
      dyno["child"] = child;

      XmlWriter writer;
      ostringstream oss;
      OStreamOutputStream os(&oss);

      writer.write(dyno, &os);
      assertStrCmp(oss.str().c_str(),
         "<object>"
         "<member name=\"aNumber\">"
            "<number>514</number>"
         "</member>"
         "<member name=\"child\">"
            "<object>"
            "<member name=\"id\">"
               "<number>514</number>"
            "</member>"
            "</object>"
         "</member>"
         "<member name=\"cow\">"
            "<string>Moooooooo</string>"
         "</member>"
         "</object>");

      os.close();
   }

   {
      DynamicObject dyno;
      dyno["aNumber"] = 514;
      dyno["cow"] = "Moooooooo";

      DynamicObject child;
      child["id"] = 514;
      dyno["child"] = child;

      XmlWriter writer;
      writer.setCompact(false);
      writer.setIndentation(0, 1);
      ostringstream oss;
      OStreamOutputStream os(&oss);

      writer.write(dyno, &os);
      assertStrCmp(oss.str().c_str(),
         "<object>\n"
         " <member name=\"aNumber\">\n"
         "  <number>514</number>\n"
         " </member>\n"
         " <member name=\"child\">\n"
         "  <object>\n"
         "   <member name=\"id\">\n"
         "    <number>514</number>\n"
         "   </member>\n"
         "  </object>\n"
         " </member>\n"
         " <member name=\"cow\">\n"
         "  <string>Moooooooo</string>\n"
         " </member>\n"
         "</object>");

      os.close();
   }

   {
      DynamicObject dyno;
      dyno["aNumber"] = 514;
      dyno["cow"] = "Moooooooo";

      DynamicObject child;
      child["id"] = 514;
      dyno["child"] = child;

      XmlWriter writer;
      writer.setCompact(false);
      writer.setIndentation(0, 3);
      ostringstream oss;
      OStreamOutputStream os(&oss);

      writer.write(dyno, &os);
      assertStrCmp(oss.str().c_str(),
         "<object>\n"
         "   <member name=\"aNumber\">\n"
         "      <number>514</number>\n"
         "   </member>\n"
         "   <member name=\"child\">\n"
         "      <object>\n"
         "         <member name=\"id\">\n"
         "            <number>514</number>\n"
         "         </member>\n"
         "      </object>\n"
         "   </member>\n"
         "   <member name=\"cow\">\n"
         "      <string>Moooooooo</string>\n"
         "   </member>\n"
         "</object>");

      os.close();
   }

   tr.passIfNoException();
}

static void runXmlReadWriteTest(TestRunner& tr)
{
   tr.test("XmlReadWrite");

   {
      string xml =
         "<null/>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      ostringstream oss;
      OStreamOutputStream os(&oss);
      XmlWriter writer;
      writer.write(dyno, &os);

      assertStrCmp(xml.c_str(), oss.str().c_str());
   }

   {
      string xml =
         "<object/>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      ostringstream oss;
      OStreamOutputStream os(&oss);
      XmlWriter writer;
      writer.write(dyno, &os);

      assertStrCmp(xml.c_str(), oss.str().c_str());
   }

   {
      string xml =
         "<string>This is some content.</string>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      ostringstream oss;
      OStreamOutputStream os(&oss);
      XmlWriter writer;
      writer.write(dyno, &os);

      assertStrCmp(xml.c_str(), oss.str().c_str());
   }

   {
      string xml =
         "<object>"
          "<member name=\"child\">"
           "<string>This is some content.</string>"
          "</member>"
         "</object>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      ostringstream oss;
      OStreamOutputStream os(&oss);
      XmlWriter writer;
      writer.write(dyno, &os);

      assertStrCmp(xml.c_str(), oss.str().c_str());
   }

   {
      string xml =
         "<object>"
          "<member name=\"child\">"
           "<string>This is some content.</string>"
          "</member>"
          "<member name=\"nullChild\">"
           "<null/>"
          "</member>"
         "</object>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      ostringstream oss;
      OStreamOutputStream os(&oss);
      XmlWriter writer;
      writer.write(dyno, &os);

      assertStrCmp(xml.c_str(), oss.str().c_str());
   }

   {
      string xml =
         "<object>"
          "<member name=\"myarray\">"
           "<array>"
            "<element index=\"0\">"
             "<string>element 1</string>"
            "</element>"
            "<element index=\"1\">"
             "<object>"
              "<member name=\"myarray\">"
               "<array>"
                "<element index=\"0\">"
                 "<number>514</number>"
                "</element>"
                "<element index=\"1\">"
                 "<number>5.140000e+00</number>"
                "</element>"
                "<element index=\"2\">"
                 "<number>-514</number>"
                "</element>"
                "<element index=\"3\">"
                 "<number>-5.140000e+00</number>"
                "</element>"
               "</array>"
              "</member>"
             "</object>"
            "</element>"
           "</array>"
          "</member>"
         "</object>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      DynamicObject dyno;
      reader.start(dyno);
      reader.read(&bais);
      reader.finish();

      ostringstream oss;
      OStreamOutputStream os(&oss);
      XmlWriter writer;
      writer.write(dyno, &os);

      assertStrCmp(xml.c_str(), oss.str().c_str());
   }

   tr.passIfNoException();
}

static void runXmlIOStreamTest(TestRunner& tr)
{
   tr.group("XML I/O");

   tr.test("Input");
   /*
   DynamicObject di;
   di["title"] = "My Stuff";
   di["public"] = true;
   di["stuff"][0] = "item 1";
   di["stuff"][1] = "item 2";
   XmlWriter writer;
   writer.setCompact(true);
   DynamicObjectInputStream dois(di, &writer, false);
   ostreamstring oss;
   OStreamOutputStream os(&oss);
   dois.read(oss);
   */
   tr.passIfNoException();

   tr.test("Output");
   tr.passIfNoException();

   tr.warning("Fix XML IO Stream test");

   tr.ungroup();
}

static void runDomReadWriteTest(TestRunner& tr)
{
   tr.test("DomReadWrite");

   {
      string xml =
         "<root>"
          "<book isdn=\"1234\">"
           "<chapter number=\"1\">"
            "<paragraph>Paragraph 1</paragraph>"
           "</chapter>"
           "<chapter number=\"2\">"
            "<paragraph>Paragraph 2</paragraph>"
            "rogue data"
           "</chapter>"
           "<preface>"
            "<paragraph>Paragraph 0</paragraph>"
           "</preface>"
          "</book>"
          "<magazine issue=\"May\" year=\"2006\">"
           "<page number=\"1\">"
            "<ads>Nothing but ads</ads>"
           "</page>"
          "</magazine>"
         "</root>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      DomReader dr;
      Element root;
      dr.start(root);
      dr.read(&bais);
      dr.finish();

      ostringstream oss;
      OStreamOutputStream os(&oss);
      DomWriter writer;
      writer.setCompact(true);
      //writer.setIndentation(0, 1);
      writer.write(root, &os);

      assertStrCmp(xml.c_str(), oss.str().c_str());
   }

   tr.passIfNoException();
}

static void runDomReadWriteNamespaceTest(TestRunner& tr)
{
   tr.test("DomReadWriteNamespace");

   {
      string xml =
         "<soap:Envelope "
         "soap:encodingStyle=\"http://www.w3.org/2001/12/soap-encoding\" "
         "xmlns:soap=\"http://www.w3.org/2001/12/soap-envelope\">"
         "<soap:Body xmlns:m=\"http://www.example.org/stock\">"
         "<m:GetStockPrice>"
         "<m:StockName>IBM</m:StockName>"
         "</m:GetStockPrice>"
         "</soap:Body>"
         "</soap:Envelope>";

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      DomReader dr;
      Element root;
      dr.start(root);
      dr.read(&bais);
      dr.finish();

      //dumpDynamicObject(root);

      ostringstream oss;
      OStreamOutputStream os(&oss);
      DomWriter writer;
      writer.setCompact(true);
      //writer.setIndentation(0, 1);
      writer.write(root, &os);

      assertStrCmp(xml.c_str(), oss.str().c_str());
   }

   tr.passIfNoException();
}

static void runDomWriteNamespaceTest(TestRunner& tr)
{
   tr.test("DomWriteNamespace");

   {
      // create root element
      Element root;
      root["name"] = "Envelope";
      root["namespace"] = "http://www.w3.org/2001/12/soap-envelope";
      // add soap namespace attribute
      {
         Attribute attr;
         attr["name"] = "xmlns:soap";
         attr["value"] = "http://www.w3.org/2001/12/soap-envelope";
         root["attributes"][attr["name"]->getString()] = attr;
      }
      // add encoding style attribute
      {
         Attribute attr;
         attr["name"] = "encodingStyle";
         attr["namespace"] = "http://www.w3.org/2001/12/soap-envelope";
         attr["value"] = "http://www.w3.org/2001/12/soap-encoding";
         root["attributes"][attr["name"]->getString()] = attr;
      }

      // add body element
      Element body;
      body["name"] = "Body";
      body["namespace"] = "http://www.w3.org/2001/12/soap-envelope";
      root["children"][body["name"]->getString()]->append(body);
      // add target namespace attribute
      {
         Attribute attr;
         attr["name"] = "xmlns:m";
         attr["value"] = "http://www.example.org/stock";
         body["attributes"][attr["name"]->getString()] = attr;
      }

      // add message
      Element message;
      message["name"] = "GetStockPrice";
      message["namespace"] = "http://www.example.org/stock";
      body["children"][message["name"]->getString()]->append(message);

      // add param
      Element param;
      param["name"] = "StockName";
      param["namespace"] = "http://www.example.org/stock";
      param["data"] = "IBM";
      message["children"][param["name"]->getString()]->append(param);

      // write envelope to string
      string envelope;
      DomWriter writer;
      writer.setCompact(false);
      writer.setIndentation(0, 1);
      ByteBuffer bb(1024);
      ByteArrayOutputStream baos(&bb, true);
      writer.write(root, &baos);
      assertNoExceptionSet();
      envelope.append(bb.data(), bb.length());

      const char* expect =
         "<soap:Envelope "
         "soap:encodingStyle=\"http://www.w3.org/2001/12/soap-encoding\" "
         "xmlns:soap=\"http://www.w3.org/2001/12/soap-envelope\">\n"
         " <soap:Body xmlns:m=\"http://www.example.org/stock\">\n"
         "  <m:GetStockPrice>\n"
         "   <m:StockName>IBM</m:StockName>\n"
         "  </m:GetStockPrice>\n"
         " </soap:Body>\n"
         "</soap:Envelope>";
      assertStrCmp(expect, envelope.c_str());
   }

   tr.passIfNoException();
}

static void runDomReaderCrashTest(TestRunner& tr)
{
   // this test will segfault if it fails
   tr.test("DomReader Crash");
   {
      string xml =
         "<?xml version=\"1.0\"?>\n"
         "<ResultSet xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
         "xmlns=\"urn:yahoo:maps\" xsi:schemaLocation=\"urn:yahoo:maps "
         "http://api.local.yahoo.com/MapsService/V1/GeocodeResponse.xsd\">"
         "<Result precision=\"address\">"
         "<Latitude>37.130968</Latitude><Longitude>-80.407491</Longitude>"
         "<Address>100 E Main St</Address><City>Christiansburg</City>"
         "<State>VA</State><Zip>24073-3029</Zip><Country>US</Country>"
         "</Result>"
         "<Result precision=\"address\">"
         "<Latitude>37.128598</Latitude><Longitude>-80.410080</Longitude>"
         "<Address>100 W Main St</Address><City>Christiansburg</City>"
         "<State>VA</State><Zip>24073-2944</Zip><Country>US</Country>"
         "</Result></ResultSet>";

      //printf("XML:\n%s\n", xml.c_str());

      ByteArrayInputStream bais(xml.c_str(), xml.length());
      DomReader reader;
      Element root;
      reader.start(root);
      reader.read(&bais);
      assertNoExceptionSet();
      reader.finish();
      assertNoExceptionSet();

      //dumpDynamicObject(root);
   }
   tr.passIfNoException();
}

static void assertSameExceptions(ExceptionRef& e0, ExceptionRef& e1)
{
   // assert both are NULL or not NULL
   assert((e0.isNull() && e1.isNull()) ||
          (!e0.isNull() && !e1.isNull()));

   // check contents if both not NULL
   if(!e0.isNull() && !e1.isNull())
   {
      // compare basic elements
      assertStrCmp(e0->getMessage(), e1->getMessage());
      assertStrCmp(e0->getType(), e1->getType());

      // recursively check cause chain
      // FIXME enable cause checking
      assertSameExceptions(e0->getCause(), e1->getCause());
   }
}

static void runExceptionTest_XML_1(ExceptionRef& e)
{
   // write out exception
   DynamicObject dyno = Exception::convertToDynamicObject(e);
   ByteBuffer buffer(1024);
   ByteArrayOutputStream baos(&buffer, true);
   XmlWriter writer;
   writer.setIndentation(0, 1);
   DynamicObjectInputStream dois(dyno, &writer, false);

   char b[1024];
   int numBytes;
   while((numBytes = dois.read(b, 1024)) > 0)
   {
      baos.write(b, numBytes);
      assertNoExceptionSet();
   }
   assertNoExceptionSet();

   string xml1(buffer.bytes(), buffer.length());
   //printf("xml1=\n%s\n", xml1.c_str());

   // read exception back in
   DynamicObject dyno2;
   XmlReader reader;
   DynamicObjectOutputStream doos(dyno2, &reader, false);
   doos.write(xml1.c_str(), xml1.length());
   assertNoExceptionSet();
   ExceptionRef e2 = Exception::convertToException(dyno2);

   // write exception back out
   DynamicObject dyno3 = Exception::convertToDynamicObject(e2);
   DynamicObjectInputStream dois2(dyno3, &writer, false);
   buffer.clear();
   ByteArrayOutputStream baos2(&buffer, true);

   char b2[1024];
   int numBytes2;
   while((numBytes2 = dois2.read(b2, 1024)) > 0)
   {
      baos2.write(b2, numBytes2);
      assertNoExceptionSet();
   }
   assertNoExceptionSet();

   string xml2(buffer.bytes(), buffer.length());
   //printf("xml2=\n%s\n", xml2.c_str());

   assertStrCmp(xml1.c_str(), xml2.c_str());
   assertSameExceptions(e, e2);
}

static void runExceptionTest_JSON_1(ExceptionRef& e)
{
   // write out exception
   DynamicObject dyno = Exception::convertToDynamicObject(e);
   ByteBuffer buffer(1024);
   ByteArrayOutputStream baos(&buffer, true);
   JsonWriter writer;
   writer.setIndentation(0, 1);
   DynamicObjectInputStream dois(dyno, &writer, false);

   char b[1024];
   int numBytes;
   while((numBytes = dois.read(b, 1024)) > 0)
   {
      baos.write(b, numBytes);
      assertNoExceptionSet();
   }
   assertNoExceptionSet();

   string json1(buffer.bytes(), buffer.length());
   //printf("json1=\n%s\n", json1.c_str());

   // read exception back in
   DynamicObject dyno2;
   JsonReader reader;
   DynamicObjectOutputStream doos(dyno2, &reader, false);
   doos.write(json1.c_str(), json1.length());
   assertNoExceptionSet();
   ExceptionRef e2 = Exception::convertToException(dyno2);

   // write exception back out
   DynamicObject dyno3 = Exception::convertToDynamicObject(e2);
   DynamicObjectInputStream dois2(dyno3, &writer, false);
   buffer.clear();
   ByteArrayOutputStream baos2(&buffer, true);

   char b2[1024];
   int numBytes2;
   while((numBytes2 = dois2.read(b2, 1024)) > 0)
   {
      baos2.write(b2, numBytes2);
      assertNoExceptionSet();
   }
   assertNoExceptionSet();

   string json2(buffer.bytes(), buffer.length());
   //printf("json2=\n%s\n", json1.c_str());

   assertStrCmp(json1.c_str(), json2.c_str());
   assertSameExceptions(e, e2);
}

static void runExceptionTypeTest(TestRunner& tr,
   const char* type, void (*runTestFunc)(ExceptionRef&))
{
   tr.group(type);

   tr.test("simple serialize/deserialize");
   ExceptionRef e = new Exception("e name", "e type");
   runTestFunc(e);
   tr.pass();

   tr.test("simple serialize/deserialize w/ a cause");
   ExceptionRef e2 = new Exception("e2 name", "e2 type");
   ExceptionRef e0 = new Exception("e0 name", "e0 type");
   e2->setCause(e0);
   runTestFunc(e2);
   tr.pass();

   tr.ungroup();
}

static void runExceptionSerializationTest(TestRunner& tr)
{
   tr.group("exception serialization");

   runExceptionTypeTest(tr, "XML", &runExceptionTest_XML_1);
   runExceptionTypeTest(tr, "JSON", &runExceptionTest_JSON_1);

   tr.ungroup();
}

static void runSwapTest(TestRunner& tr)
{
   tr.group("byte order swapping");

   // take value vXX, swap it to sXX, and check with expected eXX

   tr.test("16");
   {
      uint16_t v = 0x0123;
      uint16_t s = MO_UINT16_SWAP_LE_BE(v);
      uint16_t e = 0x2301;
      assert(s == e);
   }
   tr.pass();

   tr.test("32");
   {
      uint32_t v = 0x01234567U;
      uint32_t s = MO_UINT32_SWAP_LE_BE(v);;
      uint32_t e = 0x67452301U;
      assert(s == e);
   }
   tr.pass();

   tr.test("64");
   {
      uint64_t v = UINT64_C(0x0123456789abcdef);
      uint64_t s = MO_UINT64_SWAP_LE_BE(v);
      uint64_t e = UINT64_C(0xefcdab8967452301);
      assert(s == e);
   }
   tr.pass();

   tr.ungroup();
}

static void runFourccTest(TestRunner& tr)
{
   tr.group("FOURCC");

   tr.test("create");
   {
      fourcc_t fc = MO_FOURCC_FROM_CHARS('T','E','S','T');
      fourcc_t fs = MO_FOURCC_FROM_STR("TEST");
      assert(fc == fs);

      char b[4];
      MO_FOURCC_TO_STR(fs, b);
      assert(strncmp(b, "TEST", 4) == 0);

      char sb[13];
      snprintf(sb, 13, "fourcc[%" MO_FOURCC_FORMAT "]", MO_FOURCC_ARGS(fs));
      assertStrCmp(sb, "fourcc[TEST]");
   }
   tr.passIfNoException();

   tr.test("mask");
   {
      uint8_t m = 0xff;
      uint8_t z = 0x00;
      assert(MO_FOURCC_MASK(1) == MO_FOURCC_FROM_CHARS(m,z,z,z));
      assert(MO_FOURCC_MASK(2) == MO_FOURCC_FROM_CHARS(m,m,z,z));
      assert(MO_FOURCC_MASK(3) == MO_FOURCC_FROM_CHARS(m,m,m,z));
      assert(MO_FOURCC_MASK(4) == MO_FOURCC_FROM_CHARS(m,m,m,m));
   }
   tr.passIfNoException();

   tr.test("cmp");
   {
      fourcc_t f = MO_FOURCC_FROM_STR("TEST");
      assert(MO_FOURCC_CMP_STR(f, "TEST"));
      assert(!MO_FOURCC_CMP_STR(f, "xxxx"));
      assert(f == MO_FOURCC_FROM_STR("TEST"));
      assert(f != MO_FOURCC_FROM_STR("xxxx"));
   }
   tr.passIfNoException();

   tr.test("ncmp");
   {
      fourcc_t f = MO_FOURCC_FROM_STR("TEST");

      assert(MO_FOURCC_NCMP_STR(f, "Txxx", 1));
      assert(MO_FOURCC_NCMP_STR(f, "TExx", 2));
      assert(MO_FOURCC_NCMP_STR(f, "TESx", 3));
      assert(MO_FOURCC_NCMP_STR(f, "TEST", 4));

      assert(!MO_FOURCC_NCMP_STR(f, "xxxx", 1));
      assert(!MO_FOURCC_NCMP_STR(f, "xxxx", 2));
      assert(!MO_FOURCC_NCMP_STR(f, "xxxx", 3));
      assert(!MO_FOURCC_NCMP_STR(f, "xxxx", 4));
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runRiffTest(TestRunner& tr)
{
   tr.group("RIFF");

   tr.test("chunk");
   {
      fourcc_t fourcc = MO_FOURCC_FROM_STR("TEST");
      uint32_t size = 0x01020304;
      RiffChunkHeader chunk(fourcc, size);
      assert(chunk.getIdentifier() == fourcc);
      assert(chunk.getChunkSize() == size);

      char expect[8] = {'T', 'E', 'S', 'T', 0x04, 0x03, 0x02, 0x01};
      char offsetexpect[9] = {0, 'T', 'E', 'S', 'T', 0x04, 0x03, 0x02, 0x01};
      char to[8];
      memset(to, 0xFE, 8);
      chunk.convertToBytes(to);
      assert(memcmp(expect, to, 8) == 0);

      // short
      assert(!chunk.convertFromBytes(expect, 7));

      // @ 0
      assert(chunk.convertFromBytes(expect, 8));
      assert(chunk.getIdentifier() == fourcc);
      assert(chunk.getChunkSize() == size);
      memset(to, 0xFE, 8);
      chunk.convertToBytes(to);
      assert(memcmp(expect, to, 8) == 0);

      // offset @ 1
      assert(chunk.convertFromBytes(offsetexpect + 1, 8));
      assert(chunk.getIdentifier() == fourcc);
      assert(chunk.getChunkSize() == size);
      memset(to, 0xFE, 8);
      chunk.convertToBytes(to);
      assert(memcmp(expect, to, 8) == 0);
   }
   tr.passIfNoException();

   tr.test("list");
   {
      // FIXME
   }
   tr.passIfNoException();

   tr.test("form");
   {
      // FIXME
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runAviTest(TestRunner& tr)
{
   tr.group("AVI");

   tr.test("...");
      // FIXME
   {
   }
   tr.passIfNoException();

   tr.ungroup();
}

/**
 * Check for expected template output. Can pass NULL for expect to skip
 * checking for valid output which is useful to check for template
 * processing exceptions.
 *
 * @param tpl the template string.
 * @param vars the vars to use during template processing.
 * @param expect the expected output string or NULL to not check.
 */
static void assertTplCmp(
   const char* tpl, DynamicObject& vars, const char* expect, bool strict)
{
   // create template input stream
   ByteArrayInputStream bais(tpl, strlen(tpl));
   TemplateInputStream tis(vars, strict, &bais, false);

   // parse entire template
   ByteBuffer output(2048);
   ByteArrayOutputStream baos(&output, true);
   tis.parse(&baos);

   if(expect != NULL)
   {
      assertNoExceptionSet();

      // null-terminate output
      output.putByte(0, 1, true);

      // assert expected value
      assertNamedStrCmp("expect", expect, "output", output.data());
   }
}

static void runTemplateInputStreamTest(TestRunner& tr)
{
   tr.group("TemplateInputStream");

   tr.test("parse (valid)");
   {
      const char* tpl =
         "Subject: This is an autogenerated unit test email\r\n"
         "From: testuser@bitmunk.com\r\n"
         "To: support@bitmunk.com\r\n"
         "Cc: support@bitmunk.com\r\n"
         "{*singlelinecomment*}"
         "{* another comment *}"
         "Bcc: {bccAddress1}\r\n"
         "\r\n"
         "This is the test body. I want $10.00.\n"
         "I used a variable: {:ldelim}bccAddress1{:rdelim}"
         " with the value of '{bccAddress1}'.\n"
         "Slash before variable \\{bccAddress1}.\n"
         "2 slashes before variable \\\\{bccAddress1}.\n"
         "Slash before escaped variable \\{:ldelim}bccAddress1{:rdelim}.\n"
         "2 slashes before escaped variable "
         "\\\\{:literal}{bccAddress1}{:end}.\n"
         "{eggs}{bacon}{ham|capitalize}{sausage}.\n"
         "{* This is a multiple line comment \n"
         "  {foo} that should not show \\up at all }\n"
         "*}";

      DynamicObject vars;
      vars["bccAddress1"] = "support@bitmunk.com";
      vars["eggs"] = "This is a ";
      // vars["bacon"] -- no bacon
      vars["ham"] = "number ";
      vars["sausage"] = 5;

      const char* expect =
         "Subject: This is an autogenerated unit test email\r\n"
         "From: testuser@bitmunk.com\r\n"
         "To: support@bitmunk.com\r\n"
         "Cc: support@bitmunk.com\r\n"
         "Bcc: support@bitmunk.com\r\n"
         "\r\n"
         "This is the test body. I want $10.00.\n"
         "I used a variable: {bccAddress1} with the value of "
         "'support@bitmunk.com'.\n"
         "Slash before variable \\support@bitmunk.com.\n"
         "2 slashes before variable \\\\support@bitmunk.com.\n"
         "Slash before escaped variable \\{bccAddress1}.\n"
         "2 slashes before escaped variable \\\\{bccAddress1}.\n"
         "This is a Number 5.\n";

      assertTplCmp(tpl, vars, expect, false);
   }
   tr.passIfNoException();

   tr.test("parse (DOS paths)");
   {
      const char* tpl =
         "The path is {PATH}!";

      DynamicObject vars;
      vars["PATH"] = "C:\\Dox";

      const char* expect =
         "The path is C:\\Dox!";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (DOS paths in template)");
   {
      const char* tpl =
         "The path is C:\\Dox!";

      DynamicObject vars;

      const char* expect =
         "The path is C:\\Dox!";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (invalid - ends in '{')");
   {
      const char* tpl =
         "This template ends in an escape character{";

      DynamicObject vars;
      vars["bccAddress1"] = "support@bitmunk.com";
      vars["eggs"] = "This is a ";
      vars["bacon"] = ""; // -- no bacon
      vars["ham"] = "number ";
      vars["sausage"] = 5;

      assertTplCmp(tpl, vars, NULL, true);
   }
   tr.passIfException();

   tr.test("parse (invalid - missing variable)");
   {
      const char* tpl = "{a}{b}";

      // create variable "a" and omit "b"
      DynamicObject vars;
      vars["a"] = "A!";

      assertTplCmp(tpl, vars, NULL, true);
   }
   tr.passIfException();

   tr.test("parse (invalid - incomplete markup)");
   {
      const char* tpl =
         "{eggs}{bacon}{ham}{sausage}{incompleteMarkup";

      DynamicObject vars;
      vars["eggs"] = "This is a ";
      vars["bacon"] = ""; // -- no bacon
      vars["ham"] = "number ";
      vars["sausage"] = 5;

      assertTplCmp(tpl, vars, NULL, true);
   }
   tr.passIfException();

   tr.test("parse (reuse pipeline)");
   {
      // create template
      const char* tpl1 = "{TEST1}";

      // create variables
      DynamicObject vars;
      vars["TEST1"] = "111";
      vars["TEST2"] = "222";

      // create template input stream
      ByteArrayInputStream bais(tpl1, strlen(tpl1));
      TemplateInputStream tis(vars, false, &bais, false);

      // parse entire template
      ByteBuffer output(2048);
      ByteArrayOutputStream baos(&output, true);
      tis.parse(&baos);

      // null-terminate output
      output.putByte(0, 1, true);

      // assert expected value
      assertStrCmp("111", output.data());

      const char* tpl2 = "{TEST2}";
      bais.setByteArray(tpl2, strlen(tpl2));
      output.clear();
      tis.setInputStream(&bais, false);
      tis.parse(&baos);
      output.putByte(0, 1, true);

      // assert expected value
      assertStrCmp("222", output.data());
   }
   tr.passIfNoException();

   tr.test("parse (map)");
   {
      const char* tpl =
         "The number five: {foo.five[1]}\n"
         "{:each from=foo.items as=item key=key}\n"
         "The item is '{item}', key is '{key}'\n"
         "{:end}";

      DynamicObject vars;
      vars["foo"]["items"]->append("item1");
      vars["foo"]["items"]->append("item2");
      vars["foo"]["items"]->append("item3");
      vars["foo"]["five"]->append(4);
      vars["foo"]["five"]->append(5);

      const char* expect =
         "The number five: 5\n"
         "The item is 'item1', key is '0'\n"
         "The item is 'item2', key is '1'\n"
         "The item is 'item3', key is '2'\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (each)");
   {
      const char* tpl =
         "Item count: {items.length}\n"
         "{:each from=items as=item key=key}\n"
         "The item is '{item}', key is '{key}'\n"
         "{:end}";

      DynamicObject vars;
      vars["items"]["a"] = "item1";
      vars["items"]["b"] = "item2";
      vars["items"]["c"] = "item3";

      const char* expect =
         "Item count: 3\n"
         "The item is 'item1', key is 'a'\n"
         "The item is 'item2', key is 'b'\n"
         "The item is 'item3', key is 'c'\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (eachelse)");
   {
      const char* tpl =
         "Item count: {items.length}\n"
         "{:each from=items as=item key=key}\n"
         "The item is '{item}', key is '{key}'\n"
         "{:eachelse}\n"
         "There are no items.\n"
         "{:end}\n"
         "{:set items.a='item1'}\n"
         "Item count: {items.length}\n"
         "{:each from=items as=item key=key}\n"
         "The item is '{item}', key is '{key}'\n"
         "{:eachelse}\n"
         "There are no items.\n"
         "{:end}";

      DynamicObject vars;
      vars["items"]->setType(Array);

      const char* expect =
         "Item count: 0\n"
         "There are no items.\n"
         "Item count: 1\n"
         "The item is 'item1', key is 'a'\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (invalid - each)");
   {
      const char* tpl =
         "{:each from=items as=item}\n"
         "The item is '{item}'\n";

      DynamicObject vars;
      vars["items"]->append("item1");
      vars["items"]->append("item2");
      vars["items"]->append("item3");

      assertTplCmp(tpl, vars, NULL, true);
   }
   tr.passIfException();

   tr.test("parse (empty each)");
   {
      const char* tpl =
         "Items:\n"
         "{:each from=items as=item}\n"
         "The item is '{item}'\n"
         "{:end}\n"
         "{:if end}\nend{:end}\n";

      DynamicObject vars;
      vars["items"]->setType(Array);
      vars["end"] = true;

      const char* expect =
         "Items:\n"
         "end";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (loop)");
   {
      const char* tpl =
         "Item count: {items.length}\n"
         "{:loop start=0 until=items.length index=current}\n"
         "The item is '{items[current]}', index is {current}\n"
         "{:end}";

      DynamicObject vars;
      vars["items"]->append("item1");
      vars["items"]->append("item2");
      vars["items"]->append("item3");

      const char* expect =
         "Item count: 3\n"
         "The item is 'item1', index is 0\n"
         "The item is 'item2', index is 1\n"
         "The item is 'item3', index is 2\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (loopelse)");
   {
      const char* tpl =
         "Item count: {items.length}\n"
         "{:loop start=0 until=items.length index=current}\n"
         "The item is '{items[current]}', index is {current}\n"
         "{:loopelse}\n"
         "There are no items.\n"
         "{:end}\n"
         "{:set items[0]='item1'}\n"
         "Item count: {items.length}\n"
         "{:loop start=0 until=items.length index=current}\n"
         "The item is '{items[current]}', index is {current}\n"
         "{:loopelse}\n"
         "There are no items.\n"
         "{:end}";

      DynamicObject vars;
      vars["items"]->setType(Array);

      const char* expect =
         "Item count: 0\n"
         "There are no items.\n"
         "Item count: 1\n"
         "The item is 'item1', index is 0\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (invalid - loop)");
   {
      const char* tpl =
         "{:loop start=0 until=items.length}\n"
         "The item is '{items[current]}'\n";

      DynamicObject vars;
      vars["items"]->append("item1");
      vars["items"]->append("item2");
      vars["items"]->append("item3");

      assertTplCmp(tpl, vars, NULL, true);
   }
   tr.passIfException();

   tr.test("parse (empty loop)");
   {
      const char* tpl =
         "Items:\n"
         "{:loop start=0 until=items.length index=current}\n"
         "The item is '{items[current]}'\n"
         "{:end}\n"
         "{:if end}\nend{:end}\n";

      DynamicObject vars;
      vars["items"]->setType(Array);
      vars["end"] = true;

      const char* expect =
         "Items:\n"
         "end";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (include)");
   {
      // write out template
      File file = File::createTempFile("test");
      FileOutputStream fos(file);
      const char* include =
         "{:each from=items as=item}"
         "The item is '{item}'\n"
         "{:end}";
      fos.write(include, strlen(include));
      fos.close();
      assertNoExceptionSet();

      const char* tpl =
         "Items:\n"
         "{:include file=myfile}\n"
         "Again:\n"
         "{:include file=myfile as=foo}"
         "{foo}";

      DynamicObject vars;
      vars["items"]->append("item1");
      vars["items"]->append("item2");
      vars["items"]->append("item3");
      vars["myfile"] = file->getAbsolutePath();

      const char* expect =
         "Items:\n"
         "The item is 'item1'\n"
         "The item is 'item2'\n"
         "The item is 'item3'\n"
         "Again:\n"
         "The item is 'item1'\n"
         "The item is 'item2'\n"
         "The item is 'item3'\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (cascaded include)");
   {
      // write out template 1
      File file1 = File::createTempFile("test");
      {
         FileOutputStream fos(file1);
         const char* include =
            "{:each from=items as=item}"
            "The item is '{item}'\n"
            "{:end}";
         fos.write(include, strlen(include));
         fos.close();
         assertNoExceptionSet();
      }

      // write out template 2
      File file2 = File::createTempFile("test");
      {
         FileOutputStream fos(file2);
         const char* path = file1->getAbsolutePath();
         int len = 100 + strlen(path);
         char tpl[len + 1];
         snprintf(tpl, len,
            "Items:\n"
            "{:include file='%s'}", path);
         fos.write(tpl, strlen(tpl));
         fos.close();
         assertNoExceptionSet();
      }

      // create template
      const char* path = file2->getAbsolutePath();
      int len = 100 + strlen(path);
      char tpl[len + 1];
      snprintf(tpl, len,
         "Double include:\n"
         "{:include file='%s'}", path);

      DynamicObject vars;
      vars["items"]->append("item1");
      vars["items"]->append("item2");
      vars["items"]->append("item3");

      const char* expect =
         "Double include:\n"
         "Items:\n"
         "The item is 'item1'\n"
         "The item is 'item2'\n"
         "The item is 'item3'\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (include w/cache)");
   {
      TemplateCache cache;

      // write out template
      File file = File::createTempFile("test");
      FileOutputStream fos(file);
      const char* include =
         "{:each from=items as=item}"
         "The item is '{item}'\n"
         "{:end}";
      fos.write(include, strlen(include));
      fos.close();
      assertNoExceptionSet();

      const char* tpl =
         "Items:\n"
         "{:include file=myfile}"
         "Again:\n"
         "{:include file=myfile as=foo}"
         "{foo}";

      DynamicObject vars;
      vars["items"]->append("item1");
      vars["items"]->append("item2");
      vars["items"]->append("item3");
      vars["myfile"] = file->getAbsolutePath();

      const char* expect =
         "Items:\n"
         "The item is 'item1'\n"
         "The item is 'item2'\n"
         "The item is 'item3'\n"
         "Again:\n"
         "The item is 'item1'\n"
         "The item is 'item2'\n"
         "The item is 'item3'\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (if)");
   {
      const char* tpl =
         "{:each from=foo.items as=item index=index}"
         "{:if bar}{bar}{:end}"
         "{:if item == 'item1'}"
         "The item is '{item}'\n"
         "{:set found=index}"
         "{:end}"
         "{:end}"
         "{:if found == foo.special-1}"
         "The item is special\n"
         "{:end}";

      DynamicObject vars;
      vars["foo"]["items"]->append("item1");
      vars["foo"]["items"]->append("item2");
      vars["foo"]["items"]->append("item3");
      vars["foo"]["special"] = 1;

      const char* expect =
         "The item is 'item1'\n"
         "The item is special\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (if/elseif/else)");
   {
      const char* tpl =
         "{:each from=foo.items as=item}"
         "{:if item == 'item1'}"
         "The first item is '{item}'\n"
         "{:elseif item == 2}"
         "The second item is '{item}'\n"
         "{:elseif item == foo.third}"
         "The third item is a secret\n"
         "{:elseif item < 5}"
         "The fourth item is '{item}'\n"
         "{:else}"
         "The fifth item is '{item}'\n"
         "{:end}"
         "{:end}";

      DynamicObject vars;
      vars["foo"]["items"]->append("item1");
      vars["foo"]["items"]->append(2);
      vars["foo"]["items"]->append("secret");
      vars["foo"]["items"]->append(4);
      vars["foo"]["items"]->append("item5");
      vars["foo"]["third"] = "secret";

      const char* expect =
         "The first item is 'item1'\n"
         "The second item is '2'\n"
         "The third item is a secret\n"
         "The fourth item is '4'\n"
         "The fifth item is 'item5'\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (pipe)");
   {
      const char* tpl =
         "Item count: {items.length}\n"
         "{:each from=items as=item key=key}"
         "The item is '{item|escape|escape('url')}', "
         "capitalized key is '{key|capitalize}'\n"
         "{:end}"
         "Replaced 'c' in '{cat}' with a 'br': {cat|replace('c','br')}\n"
         "Regex replaced '{foobar}' with '"
         "{foobar|regex('o\\{2\\}','oot')}'\n"
         "Default (undefined): {undefined|default('N/A')}\n"
         "Default (empty string): {empty|default('N/A')}\n"
         "<html><body>{eol|replace('\n','</br>')}</body></html>\n"
         "Truncated 'foobar' to '{foobar|truncate(3)}'\n"
         "Truncated 'foobar' to '{foobar|truncate(3,'')}'\n";

      DynamicObject vars;
      vars["items"]["apple"] = "item&1";
      vars["items"]["banana"] = "item&2";
      vars["items"]["cherry"] = "item&3";
      vars["cat"] = "cat";
      vars["foobar"] = "foobar";
      vars["empty"] = "";
      vars["eol"] = "none\n";

      const char* expect =
         "Item count: 3\n"
         "The item is 'item%26amp%3B1', capitalized key is 'Apple'\n"
         "The item is 'item%26amp%3B2', capitalized key is 'Banana'\n"
         "The item is 'item%26amp%3B3', capitalized key is 'Cherry'\n"
         "Replaced 'c' in 'cat' with a 'br': brat\n"
         "Regex replaced 'foobar' with 'footbar'\n"
         "Default (undefined): N/A\n"
         "Default (empty string): N/A\n"
         "<html><body>none</br></body></html>\n"
         "Truncated 'foobar' to '...'\n"
         "Truncated 'foobar' to 'foo'\n";

      assertTplCmp(tpl, vars, expect, false);
   }
   tr.passIfNoException();

   tr.test("parse (invalid pipe)");
   {
      const char* tpl =
         "Item count: {items.length}\n"
         "{:each from=items as=item key=key}"
         "The item is '{item|unknown}', key is '{key}'\n"
         "{:end}";

      DynamicObject vars;
      vars["items"]["a"] = "item1";
      vars["items"]["b"] = "item2";
      vars["items"]["c"] = "item3";

      assertTplCmp(tpl, vars, NULL, true);
   }
   tr.passIfException();

   tr.test("parse (set/unset)");
   {
      const char* tpl =
         "{:if foo}"
            "bar was {bar}\n"
            "{:set bar='some text'}"
            "bar is now '{bar}'\n"
            "{:set mymap.foo=bar}"
            "mymap.foo is now '{mymap.foo}'\n"
            "{:unset bar}"
            "bar is back to {bar}\n"
            "mymap.foo is still '{mymap.foo}'\n"
            "{:set bar=true}"
            "{:if bar == true}"
               "{:set mymap.foo=bar}"
               "mymap.foo is now {mymap.foo}\n"
               "{:set mymap.foo[0]='in an array'}"
               "mymap.foo[0] is '{mymap.foo[0]}'\n"
               "{:unset mymap}"
               "{:set mynumber=17}"
               "mynumber is {mynumber}\n"
               "{:set mynumber=mynumber+1}"
               "mynumber incremented to {mynumber}\n"
               "{:set mynumber=mynumber-2}"
               "mynumber decremented to {mynumber}\n"
            "{:end}"
         "{:end}";

      DynamicObject vars;
      vars["foo"] = true;
      vars["bar"] = 12;

      const char* expect =
         "bar was 12\n"
         "bar is now 'some text'\n"
         "mymap.foo is now 'some text'\n"
         "bar is back to 12\n"
         "mymap.foo is still 'some text'\n"
         "mymap.foo is now true\n"
         "mymap.foo[0] is 'in an array'\n"
         "mynumber is 17\n"
         "mynumber incremented to 18\n"
         "mynumber decremented to 16\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (each, set local)");
   {
      const char* tpl =
         "{:each from=objs as=obj}"
         "   {:set obj=obj}"
         "{:end}";

      DynamicObject vars;
      vars["objs"][0] = "0";
      vars["objs"][1] = "1";

      // just checking for no exceptions, no interesting output.
      assertTplCmp(tpl, vars, NULL, true);
   }
   tr.passIfNoException();

   tr.test("parse (dump)");
   {
      const char* tpl =
         "{:set foo='A'}"
         "{:dump foo}\n"
         "\n"
         "{foo|json}\n"
         "{:set bar='B'}"
         "{:dump}";

      DynamicObject vars;
      vars["foobar"] = "C";

      // create expected string
      DynamicObject expectVars;
      expectVars["localVars"]["foo"] = "A";
      expectVars["localVars"]["bar"] = "B";
      expectVars["vars"]["foobar"] = "C";
      string expect =
         "\"A\"\n"
         "\"A\"\n";
      expect.append(JsonWriter::writeToString(expectVars, false, false));

      assertTplCmp(tpl, vars, expect.c_str(), true);
   }
   tr.passIfNoException();

   tr.test("parse (date pipe)");
   {
      const char* tpl =
         "{mydate|date('%Y-%m-%d')}\n"
         "{mydate|date('%Y-%m-%d','',0,0)}\n"
         "{longdate|date('%Y-%m-%d','%a, %d %b %Y %H:%M:%S')}\n"
         "{longdate|date('%Y-%m-%d','%a, %d %b %Y %H:%M:%S','UTC')}\n"
         "{longdate|date('%Y-%m-%d','%a, %d %b %Y %H:%M:%S','','')}\n"
         "{longdate|date('%Y-%m-%d %H:%M:%S','%a, %d %b %Y %H:%M:%S','','')}\n"
         "{longdate|date('%Y-%m-%d %H:%M:%S','%a, %d %b %Y %H:%M:%S',0,60)}\n"
         "{longdate|date('%Y-%m-%d %H:%M:%S','%a, %d %b %Y %H:%M:%S',0,-60)}\n";

      DynamicObject vars;
      vars["mydate"] = "2010-01-01 00:01:00";
      vars["longdate"] = "Sat, 21 Jan 2006 03:15:46";

      const char* expect =
         "2010-01-01\n"
         "2010-01-01\n"
         "2006-01-21\n"
         "2006-01-21\n"
         "2006-01-21\n"
         "2006-01-21 03:15:46\n"
         "2006-01-21 02:15:46\n"
         "2006-01-21 04:15:46\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (format pipe)");
   {
      const char* tpl =
         "{amount1|format('%1.2f')}\n"
         "{amount2|format('%i')}\n"
         "{amount2|format('%x')}\n"
         "{amount2|format('%X')}\n";

      DynamicObject vars;
      vars["amount1"] = "8234.125";
      vars["amount2"] = "255";

      const char* expect =
         "8234.12\n"
         "255\n"
         "ff\n"
         "FF\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (decimal pipe)");
   {
      const char* tpl =
         "${money1|decimal('2')}\n"
         "${money1|decimal('2','up')}\n"
         "${money1|decimal('2','down')}\n"
         "${money2|decimal('2')}\n"
         "${money2|decimal('2','up')}\n"
         "${money2|decimal('2','down')}\n"
         "${cents|decimal('2')}\n"
         "${cents|decimal('2','up')}\n"
         "${cents|decimal('2','down')}\n";

      DynamicObject vars;
      vars["money1"] = "8234.12";
      vars["money2"] = "8234.125";
      vars["cents"] = ".125";

      const char* expect =
         "$8234.12\n"
         "$8234.12\n"
         "$8234.12\n"
         "$8234.13\n"
         "$8234.13\n"
         "$8234.12\n"
         "$0.13\n"
         "$0.13\n"
         "$0.12\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (missing map)");
   {
      const char* tpl = "{one.two}";

      DynamicObject vars;
      vars["one"]->setType(Map);

      assertTplCmp(tpl, vars, NULL, true);
   }
   tr.passIfException();

   tr.test("parse (null map)");
   {
      const char* tpl = "{one.two}";

      DynamicObject vars;
      vars["one"].setNull();

      assertTplCmp(tpl, vars, NULL, true);
   }
   tr.passIfException();

   tr.test("parse (null array)");
   {
      const char* tpl = "{one[0]}";

      DynamicObject vars;
      vars["one"].setNull();

      // NOTE: Due to the current design, this test MUST be run in strict
      // mode. Running in non-strict mode will segfault.
      assertTplCmp(tpl, vars, NULL, true);
   }
   tr.passIfException();

   tr.test("parse (string concatenation)");
   {
      const char* tpl =
         "{var+1}\n"
         "{var+'foo'}\n"
         "{var+foo}\n"
         "{var2+'foo'}\n"
         "{var2+foo}\n";

      DynamicObject vars;
      vars["var"] = "start_";
      vars["var2"] = 1;
      vars["foo"] = "bar";

      const char* expect =
         "start_1\n"
         "start_foo\n"
         "start_bar\n"
         "1foo\n"
         "1bar\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (escaped operators)");
   {
      const char* tpl =
         "{var\\-1}\n"
         "{var\\+2}\n"
         "{var\\*3}\n"
         "{var\\/4}\n";

      DynamicObject vars;
      vars["var-1"] = "value1";
      vars["var+2"] = "value2";
      vars["var*3"] = "value3";
      vars["var/4"] = "value4";

      const char* expect =
         "value1\n"
         "value2\n"
         "value3\n"
         "value4\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (escaped operators after array var)");
   {
      const char* tpl =
         "{var:array[0].foo:bar\\-1}\n"
         "{var:array[0].foo:bar\\-2}\n";

      DynamicObject vars;
      vars["var:array"][0]["foo:bar-1"] = "value1";
      vars["var:array"][0]["foo:bar-2"] = "value2";

      const char* expect =
         "value1\n"
         "value2\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (array access on variable with colon)");
   {
      const char* tpl =
         "US ${deposit.ps:payee[0].amount}\n";

      DynamicObject vars;
      vars["deposit"]["ps:payee"][0]["amount"] = "1.00";

      const char* expect =
         "US $1.00\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (other array access with index var within each)");
   {
      const char* tpl =
         "{:each from=bar as=item index=idx}{foo[idx].test}{:end}\n";

      DynamicObject vars;
      vars["foo"][0]["test"] = 1;
      vars["bar"][0] = "empty";

      const char* expect =
         "1";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (other 2D array access with index var within each)");
   {
      const char* tpl =
         "{:each from=bar as=item index=idx}"
         "{foo[idx][bar[idx].len]}{:end}\n";

      DynamicObject vars;
      vars["foo"][0][0] = 1;
      vars["bar"][0]["len"] = 0;

      const char* expect =
         "1";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (subtraction w/undefined unstrict lhs)");
   {
      const char* tpl =
         "{foo-bar}\n";

      DynamicObject vars;
      vars["bar"] = 1;

      const char* expect =
         "-1\n";

      assertTplCmp(tpl, vars, expect, false);
   }
   tr.passIfNoException();

   tr.test("parse (divide by 0)");
   {
      const char* tpl =
         "{foo-bar}\n";

      DynamicObject vars;
      vars["foo"] = 1;
      vars["bar"] = 0;

      const char* expect =
         "1\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (multi-level variable compare)");
   {
      const char* tpl =
         "{:each from=groups as=group index=gnum}"
            "{:each from=group.mo:data as=data index=dnum}"
               "{:if compare.mo:group}"
                  "{:each from=compare.mo:group as=cGroup}"
                     "{:if cGroup.mo:name == group.mo:name}"
                        "{:if cGroup.mo:data}"
                           "{:each from=cGroup.mo:data as=cData}"
                              "{:if data == cData}"
                                 "{data} == {cData}\n"
                              "{:end}"
                           "{:end}"
                        "{:end}"
                     "{:end}"
                  "{:end}"
               "{:end}"
            "{:end}"
         "{:end}";

      DynamicObject vars;

      DynamicObject& groups = vars["groups"];
      groups->setType(Array);
      DynamicObject group1;
      group1["mo:name"] = "Group 1";
      group1["mo:data"]->setType(Array);
      group1["mo:data"]->append("group_1_data_1");
      groups->append(group1);

      DynamicObject group2;
      group2["mo:name"] = "Group 2";
      group2["mo:data"]->setType(Array);
      group2["mo:data"]->append("group_2_data_1");
      group2["mo:data"]->append("group_2_data_2");
      group2["mo:data"]->append("group_2_data_3");
      groups->append(group2);

      DynamicObject& compare = vars["compare"];
      DynamicObject& cgroups = compare["mo:group"];
      cgroups->setType(Array);
      DynamicObject cgroup1;
      cgroup1["mo:name"] = "Group 1";
      cgroup1["mo:data"]->setType(Array);
      cgroup1["mo:data"]->append("group_1_data_1");
      cgroups->append(cgroup1);

      const char* expect =
         "group_1_data_1 == group_1_data_1\n";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (array access)");
   {
      const char* tpl =
         "Item count: {items.length}\n"
         "{:set items[0]='item1'}"
         "Item count: {items.length}\n"
         "{:set items[1]='item2'}"
         "Item count: {items.length}\n"
         "|{:each from=items as=item}{item}|{:end}\n";

      DynamicObject vars;
      vars["items"]->setType(Array);

      const char* expect =
         "Item count: 0\n"
         "Item count: 1\n"
         "Item count: 2\n"
         "|item1|item2|";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.test("parse (array append)");
   {
      const char* tpl =
         "Item count: {items.length}\n"
         "{:set items[]='item1'}"
         "Item count: {items.length}\n"
         "{:set items[]='item2'}"
         "Item count: {items.length}\n"
         "|{:each from=items as=item}{item}|{:end}\n";

      DynamicObject vars;
      vars["items"]->setType(Array);

      const char* expect =
         "Item count: 0\n"
         "Item count: 1\n"
         "Item count: 2\n"
         "|item1|item2|";

      assertTplCmp(tpl, vars, expect, true);
   }
   tr.passIfNoException();

   tr.ungroup();
}

/**
 * Make a DynamicObject with various content to stress test JSON reader/writer.
 *
 * @return test DynamicObject
 */
static DynamicObject makeJsonTestDyno2()
{
   DynamicObject d3;
   d3["a"] = 123;
   d3["b"] = true;
   d3["c"] = "sea";

   DynamicObject loremIpsum;
   loremIpsum =
      "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do "
      "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad "
      "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip "
      "ex ea commodo consequat. Duis aute irure dolor in reprehenderit in "
      "voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur "
      "sint occaecat cupidatat non proident, sunt in culpa qui officia "
      "deserunt mollit anim id est laborum.";

   DynamicObject d;
   d["zeroth"] = false;
   d["first"] = "one";
   d["second"] = 2.0;
   d["third"] = 3;
   d["fourth"]->setType(Array);
   d["fourth"]->append() = d3.clone();
   d["fourth"]->append() = d3.clone();
   d["fourth"]->append() = d3.clone();
   d["fourth"]->append() = d3.clone();
   d["fifth"] = d3.clone();
   d["sixth"].setNull();
   d["seventh"] = loremIpsum.clone();
   d["eighth"]["one"] = loremIpsum.clone();
   d["eighth"]["two"] = loremIpsum.clone();
   d["eighth"]["three"] = loremIpsum.clone();
   d["eighth"]["four"] = loremIpsum.clone();
   d["ninth"] = "WUVT 90.7 FM - The Greatest Radio Station on Earth";

   return d;
}

static void runCharacterSetMutatorTest(TestRunner& tr)
{
   tr.group("CharacterSetMutator");

   tr.test("ISO-8859-1 to UTF-8");
   {
      // FIXME: do something fancy here
      const char* data =
         "foo";

      CharacterSetMutator csm;
      csm.setCharacterSets("UTF-8", "ISO-8859-1");

      ByteArrayInputStream bais(data, strlen(data));
      string out;
      MutatorInputStream mis(&bais, false, &csm, false);
      int numBytes;
      char b[2048];
      while((numBytes = mis.read(b, 2048)) > 0)
      {
         out.append(b, numBytes);
      }
      mis.close();

      //printf("output: '%s'\n", out.c_str());
   }
   tr.passIfNoException();

   tr.test("UTF-8 to ISO-8859-1");
   {
      // FIXME: do something fancy here
      const char* data =
         "foo";

      string out;
      CharacterSetMutator::convert(data, "ISO-8859-1", out, "UTF-8");
      //printf("output: '%s'\n", out.c_str());
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runJsonReaderSpeedTest(TestRunner& tr)
{
   tr.group("JsonReader speed");

   tr.test("speed");
   {
      DynamicObject in = makeJsonTestDyno2();
      string json = JsonWriter::writeToString(in, true);

      Timer t;
      t.start();
      for(int i = 0; i < 10000; ++i)
      {
         DynamicObject out;
         JsonReader::readFromString(out, json.c_str(), json.length());
      }
      printf("%0.2f secs... ", t.getElapsedSeconds());
   }
   tr.passIfNoException();

   tr.ungroup();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runJsonValidTest(tr);
      runJsonInvalidTest(tr);
      runJsonDJDTest(tr);
      runJsonInvalidDJTest(tr);
      runJsonVerifyDJDTest(tr);
      runJsonValueVerifyJDTest(tr);
      runJsonIOStreamTest(tr);
      runJsonLdTest(tr);

      runXmlReaderTest(tr);
      runXmlWriterTest(tr);
      runXmlReadWriteTest(tr);
      runXmlIOStreamTest(tr);
      runDomReadWriteTest(tr);
      runDomReaderCrashTest(tr);
      runDomReadWriteNamespaceTest(tr);
      runDomWriteNamespaceTest(tr);

      runExceptionSerializationTest(tr);

      runSwapTest(tr);

      runFourccTest(tr);
      runRiffTest(tr);
      runAviTest(tr);

      runTemplateInputStreamTest(tr);

      runCharacterSetMutatorTest(tr);
   }
   if(tr.isTestEnabled("json-reader-speed"))
   {
      runJsonReaderSpeedTest(tr);
   }
   if(tr.isTestEnabled("json-ld"))
   {
      runJsonLdTest(tr);
   }
   if(tr.isTestEnabled("template-input-stream"))
   {
      runTemplateInputStreamTest(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.data.test", "1.0", mo_test_data::run)
