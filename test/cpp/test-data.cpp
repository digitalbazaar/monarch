/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <sstream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/rt/DynamicObject.h"
#include "db/rt/DynamicObjectIterator.h"
#include "db/io/ByteArrayOutputStream.h"
#include "db/io/File.h"
#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/io/OStreamOutputStream.h"
#include "db/data/Data.h"
#include "db/data/xml/XmlReader.h"
#include "db/data/xml/XmlWriter.h"
#include "db/data/xml/DomParser.h"
#include "db/data/DynamicObjectInputStream.h"
#include "db/data/DynamicObjectOutputStream.h"
#include "db/data/json/JsonWriter.h"
#include "db/data/json/JsonReader.h"
#include "db/data/riff/RiffChunkHeader.h"
#include "db/data/riff/RiffListHeader.h"
#include "db/data/riff/RiffFormHeader.h"
#include "db/modest/Kernel.h"
#include "db/net/Url.h"
#include "db/net/http/HttpClient.h"
#include "db/net/http/HttpConnectionServicer.h"
#include "db/net/http/HttpRequestServicer.h"
#include "db/net/Server.h"

using namespace std;
using namespace db::test;
using namespace db::data;
//using namespace db::data::avi;
using namespace db::data::json;
//using namespace db::data::mpeg;
using namespace db::data::riff;
using namespace db::data::xml;
using namespace db::io;
using namespace db::modest;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;

void runJsonValidTest(TestRunner& tr)
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

   for(int i = 0; tests[i] != NULL; i++)
   {
      char msg[50];
      snprintf(msg, 50, "Parse #%d", i);
      tr.test(msg);
      
      DynamicObject d;
      const char* s = tests[i];
      //cout << s << endl;
      ByteArrayInputStream is(s, strlen(s));
      jr.start(d);
      assertNoException();
      jr.read(&is);
      assertNoException();
      jr.finish();
      assertNoException();
      //cout << s << endl;
      //dumpDynamicObject(d);
      
      tr.passIfNoException();
   }
   
   tr.ungroup();
}

void runJsonInvalidTest(TestRunner& tr)
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

   for(int i = 0; tests[i] != NULL; i++)
   {
      char msg[50];
      snprintf(msg, 50, "Parse #%d", i);
      tr.test(msg);

      DynamicObject d;
      const char* s = tests[i];
      //cout << s << endl;
      ByteArrayInputStream is(s, strlen(s));
      jr.start(d);
      assertNoException();
      jr.read(&is);
      jr.finish();
      assertException();
      Exception::clearLast();
      //jw.write(d, &os);
      //cout << endl;
      
      tr.passIfNoException();
   }
   
   tr.ungroup();
}

void runJsonDJDTest(TestRunner& tr)
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
   
   for(int i = 0; dynos[i] != NULL; i++)
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
      assertNoException();
      b.clear();
      assertNoException();
      
      jw.setCompact(false);
      jw.setIndentation(0, 3);
      //jw.write(d, &os);
      jw.write(d, &bbos);
      ByteArrayInputStream is(b.data(), b.length());
      DynamicObject dr;
      jr.start(dr);
      assertNoException();
      jr.read(&is);
      assertNoException();
      jr.finish();
      assertNoException();
      //jw.write(dr, &os);
      assertNoException();
      b.clear();

      tr.passIfNoException();
   }
   
   tr.ungroup();
}

void runJsonVerifyDJDTest(TestRunner& tr)
{
   tr.group("JSON (Verify Dyno->JSON->Dyno)");
   
   JsonWriter jw;
   JsonReader jr;
   OStreamOutputStream os(&cout);
   
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
   td[tdcount  ]["dyno"][0] = "\x01";
   td[tdcount++]["JSON"] = "[\"\\u0001\"]";
   // test if UTF-16 C escapes translate into a UTF-8 JSON string
   td[tdcount  ]["dyno"][0] =
      "\u040e \u045e \u0404 \u0454 \u0490 \u0491";
   td[tdcount++]["JSON"] =
      "[\"\xd0\x8e \xd1\x9e \xd0\x84 \xd1\x94 \xd2\x90 \xd2\x91\"]";
   
   for(int i = 0; i < tdcount; i++)
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
      assertNoException();
      jw.write(d, &bbos);
      assertNoException();
      
      // Verify written string
      assert(strlen(s) == (unsigned int)b.length());
      assert(strncmp(s, b.data(), b.length()) == 0);
      
      ByteArrayInputStream is(b.data(), b.length());
      DynamicObject dr;
      jr.start(dr);
      assertNoException();
      jr.read(&is);
      assertNoException();
      jr.finish();
      assertNoException();
      //jw.write(dr, &os);
      assertNoException();
      b.clear();
      
      assert(d == dr);
      
      tr.passIfNoException();
   }
   
   tr.ungroup();
}

void runJsonIOStreamTest(TestRunner& tr)
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

void runXmlReaderTest(TestRunner& tr)
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
      
      assertException();
      //cout << endl << Exception::getLast()->getMessage() << endl;
      Exception::clearLast();
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
      
      assert(dyno["myarray"]->length() == 4);
      assert(dyno["myarray"][0]->getUInt32() == 514);
      assert(dyno["myarray"][1]->getDouble() == 5.14);
      assert(dyno["myarray"][2]->getInt32() == -514);
      assert(dyno["myarray"][3]->getDouble() == -5.14);
   }
   
   tr.passIfNoException();
}

void runXmlWriterTest(TestRunner& tr)
{
   tr.test("XmlWriter");
   
   {
      DynamicObject dyno;
      
      XmlWriter writer;
      ostringstream oss;
      OStreamOutputStream os(&oss);
      
      writer.write(dyno, &os);
      //std::cout << "XML=\n" << oss.str() << std::endl;
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
      //std::cout << "XML=\n" << oss.str() << std::endl;
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
      //std::cout << "XML=\n" << oss.str() << std::endl;
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
      //std::cout << "XML=\n" << oss.str() << std::endl;
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
      //std::cout << "XML=\n" << oss.str() << std::endl;
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
      //std::cout << "XML=\n" << oss.str() << std::endl;
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

void runXmlReadWriteTest(TestRunner& tr)
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

void runXmlIOStreamTest(TestRunner& tr)
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

void runDomParserTest(TestRunner& tr)
{
   tr.test("DomParser");
   
   {
      string xml =
         "<root>"
          "<book isdn=\"1234\">"
           "<preface>"
            "<paragraph>Paragraph 0</paragraph>"
           "</preface>"
           "<chapter number=\"1\">"
            "<paragraph>Paragraph 1</paragraph>"
           "</chapter>"
           "<chapter number=\"2\">"
            "<paragraph>Paragraph 2</paragraph>"
           "</chapter>"
          "</book>"
          "<magazine issue=\"May\" year=\"2006\">"
           "<page number=\"1\">"
            "<ads>Nothing but ads</ads>"
           "</page>"
          "</magazine>"
         "</root>";
      
      ByteArrayInputStream bais(xml.c_str(), xml.length());
      DomParser dp;
      Element root;
      dp.start(root);
      dp.read(&bais);
      dp.finish();
      
      ostringstream oss;
      OStreamOutputStream os(&oss);
      JsonWriter writer;
      writer.setCompact(false);
      writer.setIndentation(0, 1);
      writer.write(root, &os);
      
      cout << "RESULT=\n" << oss.str() << std::endl;
      
      //assertStrCmp(xml.c_str(), oss.str().c_str());
   }
   
   tr.passIfNoException();
}

void runSwapTest(TestRunner& tr)
{
   tr.group("byte order swapping");
   
   // take value vXX, swap it to sXX, and check with expected eXX
   
   tr.test("16");
   {
      uint16_t v = 0x0123;
      uint16_t s = DB_UINT16_SWAP_LE_BE(v);
      uint16_t e = 0x2301;
      assert(s == e);
   }
   tr.pass();
   
   tr.test("32");
   {
      uint32_t v = 0x01234567U;
      uint32_t s = DB_UINT32_SWAP_LE_BE(v);;
      uint32_t e = 0x67452301U;
      assert(s == e);
   }
   tr.pass();

   tr.test("64");
   {
      uint64_t v = 0x0123456789abcdefULL;
      uint64_t s = DB_UINT64_SWAP_LE_BE(v);
      uint64_t e = 0xefcdab8967452301ULL;
      assert(s == e);
   }
   tr.pass();
   
   tr.ungroup();
}

void runFourccTest(TestRunner& tr)
{
   tr.group("FOURCC");
   
   tr.test("create");
   {
      fourcc_t fc = DB_FOURCC_FROM_CHARS('T','E','S','T');
      fourcc_t fs = DB_FOURCC_FROM_STR("TEST");
      assert(fc == fs);
      
      char b[4];
      DB_FOURCC_TO_STR(fs, b);
      assert(strncmp(b, "TEST", 4) == 0);

      char sb[13];
      snprintf(sb, 13, "fourcc[%" DB_FOURCC_FORMAT "]", DB_FOURCC_ARGS(fs));
      assertStrCmp(sb, "fourcc[TEST]");
   }
   tr.passIfNoException();
   
   tr.test("mask");
   {
      uint8_t m = 0xff;
      uint8_t z = 0x00;
      assert(DB_FOURCC_MASK(1) == DB_FOURCC_FROM_CHARS(m,z,z,z));
      assert(DB_FOURCC_MASK(2) == DB_FOURCC_FROM_CHARS(m,m,z,z));
      assert(DB_FOURCC_MASK(3) == DB_FOURCC_FROM_CHARS(m,m,m,z));
      assert(DB_FOURCC_MASK(4) == DB_FOURCC_FROM_CHARS(m,m,m,m));
   }
   tr.passIfNoException();

   tr.test("cmp");
   {
      fourcc_t f = DB_FOURCC_FROM_STR("TEST");
      assert(DB_FOURCC_CMP_STR(f, "TEST"));
      assert(!DB_FOURCC_CMP_STR(f, "xxxx"));
      assert(f == DB_FOURCC_FROM_STR("TEST"));
      assert(f != DB_FOURCC_FROM_STR("xxxx"));
   }
   tr.passIfNoException();
   
   tr.test("ncmp");
   {
      fourcc_t f = DB_FOURCC_FROM_STR("TEST");

      assert(DB_FOURCC_NCMP_STR(f, "Txxx", 1));
      assert(DB_FOURCC_NCMP_STR(f, "TExx", 2));
      assert(DB_FOURCC_NCMP_STR(f, "TESx", 3));
      assert(DB_FOURCC_NCMP_STR(f, "TEST", 4));

      assert(!DB_FOURCC_NCMP_STR(f, "xxxx", 1));
      assert(!DB_FOURCC_NCMP_STR(f, "xxxx", 2));
      assert(!DB_FOURCC_NCMP_STR(f, "xxxx", 3));
      assert(!DB_FOURCC_NCMP_STR(f, "xxxx", 4));
   }
   tr.passIfNoException();

   tr.ungroup();
}
   

void runRiffTest(TestRunner& tr)
{
   tr.group("RIFF");
   
   tr.test("chunk");
   {
      fourcc_t fourcc = DB_FOURCC_FROM_STR("TEST");
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

void runAviTest(TestRunner& tr)
{
   tr.group("AVI");
   
   tr.test("...");
      // FIXME
   {
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

class DbDataTester : public db::test::Tester
{
public:
   DbDataTester()
   {
      setName("data");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runJsonValidTest(tr);
      runJsonInvalidTest(tr);
      runJsonDJDTest(tr);
      runJsonVerifyDJDTest(tr);
      runJsonIOStreamTest(tr);
      
      runXmlReaderTest(tr);
      runXmlWriterTest(tr);
      runXmlReadWriteTest(tr);
      runXmlIOStreamTest(tr);
      runDomParserTest(tr);
      
      runSwapTest(tr);
      
      runFourccTest(tr);
      runRiffTest(tr);
      runAviTest(tr);

      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbDataTester)
#endif
