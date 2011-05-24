/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/io/ByteArrayOutputStream.h"
#include "monarch/io/File.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/io/MutatorInputStream.h"
#include "monarch/data/CharacterSetMutator.h"
#include "monarch/data/Data.h"
#include "monarch/data/xml/XmlReader.h"
#include "monarch/data/xml/XmlWriter.h"
#include "monarch/data/xml/DomReader.h"
#include "monarch/data/xml/DomWriter.h"
#include "monarch/data/DynamicObjectInputStream.h"
#include "monarch/data/DynamicObjectOutputStream.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/data/json/JsonReader.h"
#include "monarch/data/riff/RiffChunkHeader.h"
#include "monarch/data/riff/RiffListHeader.h"
#include "monarch/data/riff/RiffFormHeader.h"
#include "monarch/logging/Logging.h"
#include "monarch/util/Data.h"
#include "monarch/util/Timer.h"

using namespace std;
using namespace monarch::test;
using namespace monarch::data;
//using namespace monarch::data::avi;
using namespace monarch::data::json;
//using namespace monarch::data::mpeg;
using namespace monarch::data::riff;
using namespace monarch::data::xml;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;

namespace mo_test_data
{

static void runJsonValidTest(TestRunner& tr)
{
   tr.group("JSON (Valid)");

   JsonWriter jw;
   JsonReader jr;
   FileOutputStream os(FileOutputStream::StdOut);

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
   FileOutputStream os(FileOutputStream::StdOut);

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
   FileOutputStream os(FileOutputStream::StdOut);

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
   FileOutputStream os(FileOutputStream::StdOut);

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
      ByteArrayOutputStream baos;
      writer.write(dyno, &baos);
      assertStrCmp(baos.str().c_str(), "<string/>");
   }

   {
      DynamicObject dyno;
      dyno = 5;

      XmlWriter writer;
      ByteArrayOutputStream baos;
      writer.write(dyno, &baos);
      assertStrCmp(baos.str().c_str(), "<number>5</number>");
   }

   {
      DynamicObject dyno;
      dyno[0] = 5;
      dyno[1] = 1;
      dyno[2] = 4;

      XmlWriter writer;
      ByteArrayOutputStream baos;
      writer.write(dyno, &baos);
      assertStrCmp(baos.str().c_str(),
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
   }

   {
      DynamicObject dyno;
      dyno["aNumber"] = 514;
      dyno["cow"] = "Moooooooo";

      DynamicObject child;
      child["id"] = 514;
      dyno["child"] = child;

      XmlWriter writer;
      ByteArrayOutputStream baos;
      writer.write(dyno, &baos);
      assertStrCmp(baos.str().c_str(),
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
      ByteArrayOutputStream baos;
      writer.write(dyno, &baos);
      assertStrCmp(baos.str().c_str(),
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
      ByteArrayOutputStream baos;
      writer.write(dyno, &baos);
      assertStrCmp(baos.str().c_str(),
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

      ByteArrayOutputStream baos;
      XmlWriter writer;
      writer.write(dyno, &baos);
      assertStrCmp(xml.c_str(), baos.str().c_str());
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

      ByteArrayOutputStream baos;
      XmlWriter writer;
      writer.write(dyno, &baos);
      assertStrCmp(xml.c_str(), baos.str().c_str());
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

      ByteArrayOutputStream baos;
      XmlWriter writer;
      writer.write(dyno, &baos);
      assertStrCmp(xml.c_str(), baos.str().c_str());
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

      ByteArrayOutputStream baos;
      XmlWriter writer;
      writer.write(dyno, &baos);
      assertStrCmp(xml.c_str(), baos.str().c_str());
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

      ByteArrayOutputStream baos;
      XmlWriter writer;
      writer.write(dyno, &baos);
      assertStrCmp(xml.c_str(), baos.str().c_str());
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

      ByteArrayOutputStream baos;
      XmlWriter writer;
      writer.write(dyno, &baos);
      assertStrCmp(xml.c_str(), baos.str().c_str());
   }

   tr.passIfNoException();
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

      ByteArrayOutputStream baos;
      DomWriter writer;
      writer.setCompact(true);
      //writer.setIndentation(0, 1);
      writer.write(root, &baos);
      assertStrCmp(xml.c_str(), baos.str().c_str());
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

      ByteArrayOutputStream baos;
      DomWriter writer;
      writer.setCompact(true);
      //writer.setIndentation(0, 1);
      writer.write(root, &baos);

      assertStrCmp(xml.c_str(), baos.str().c_str());
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

      runXmlReaderTest(tr);
      runXmlWriterTest(tr);
      runXmlReadWriteTest(tr);
      runDomReadWriteTest(tr);
      runDomReaderCrashTest(tr);
      runDomReadWriteNamespaceTest(tr);
      runDomWriteNamespaceTest(tr);

      runExceptionSerializationTest(tr);

      runSwapTest(tr);

      runFourccTest(tr);
      runRiffTest(tr);
      runAviTest(tr);

      runCharacterSetMutatorTest(tr);
   }
   if(tr.isTestEnabled("json-reader-speed"))
   {
      runJsonReaderSpeedTest(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.data.test", "1.0", mo_test_data::run)
