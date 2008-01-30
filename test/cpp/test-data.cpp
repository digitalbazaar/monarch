/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <sstream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/util/DynamicObject.h"
#include "db/util/DynamicObjectIterator.h"
#include "db/io/File.h"
#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/io/OStreamOutputStream.h"
#include "db/data/xml/XmlReader.h"
#include "db/data/xml/XmlWriter.h"
#include "db/data/xml/XmlBindingInputStream.h"
#include "db/data/xml/XmlBindingOutputStream.h"
#include "db/data/DataMappingFunctor.h"
#include "db/data/DynamicObjectWriter.h"
#include "db/data/DynamicObjectReader.h"
#include "db/data/DynamicObjectBinding.h"
#include "db/data/json/JsonWriter.h"
#include "db/data/json/JsonReader.h"
#include "db/data/json/JsonInputStream.h"
#include "db/data/json/JsonOutputStream.h"
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
using namespace db::util;

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
   JsonInputStream jis(di);
   jis.setCompact(true);
   ostreamstring oss;
   OStreamOutputStream os(&oss);
   jis.read(oss);
   */
   tr.passIfNoException();
   
   tr.test("Output");
   tr.passIfNoException();

   tr.warning("Fix JSON IO Stream test");
   
   tr.ungroup();
}

class TestContent
{
protected:
   char* mContent;
   
public:
   TestContent()
   {
      mContent = strdup("");
   }
   
   virtual ~TestContent()
   {
      free(mContent);
   }
   
   virtual void setContent(const char* str)
   {
      free(mContent);
      mContent = strdup(str);
   }
   
   virtual const char* getContent()
   {
      return mContent;
   }
};

class TestChild : public TestContent
{
protected:
   int mId;
   
public:
   TestChild()
   {
      mId = 0;
   }
   
   virtual ~TestChild()
   {
   }
   
   virtual void setId(int id)
   {
      mId = id;
   }
   
   virtual int getId()
   {
      return mId;
   }
};

class TestParent : public TestContent
{
protected:
   TestChild* mChild;
   
public:
   TestParent()
   {
      mChild = NULL;
   }
   
   virtual ~TestParent()
   {
      if(mChild != NULL)
      {
         delete mChild;
      }
   }
   
   virtual TestChild* createChild()
   {
      return new TestChild();
   }
   
   virtual void addChild(TestChild* child)
   {
      if(mChild != NULL)
      {
         delete mChild;
      }
      
      mChild = child;
   }
   
   virtual TestChild* getChild()
   {
      return mChild;
   }
};

class TestChildDataBinding : public DataBinding
{
protected:
   DataMappingFunctor<TestChild> mChildContent;
   DataMappingFunctor<TestChild> mChildId;
   
public:
   TestChildDataBinding(TestChild* c = NULL) :
      DataBinding(c),
      mChildContent(&TestChild::setContent, &TestChild::getContent),
      mChildId(&TestChild::setId, &TestChild::getId)
   {
      // set root data name
      setDataName(NULL, "TestChild");
      
      // add mappings
      addDataMapping(NULL, "id", false, true, &mChildId);
      addDataMapping(NULL, "TestContent", true, false, &mChildContent);
   }
   
   virtual ~TestChildDataBinding()
   {
   }
};

class TestParentDataBinding : public DataBinding
{
protected:
   TestParent* mTestParent;
   TestChildDataBinding mChildBinding;
   DataMappingFunctor<TestParent> mTestContent;
   DataMappingFunctor<TestParent, TestChild> mCreateChild;
   
public:
   TestParentDataBinding(TestParent* p) :
      DataBinding(p),
      mTestContent(&TestParent::setContent, &TestParent::getContent),
      mCreateChild(&TestParent::createChild, &TestParent::addChild)
   {
      mTestParent = p;
      
      // set root data name
      setDataName(NULL, "TestContent");
      
      // add mappings
      addDataMapping(NULL, "TestContent", true, false, &mTestContent);
      addDataMapping(NULL, "TestChild", true, true, &mCreateChild);
      
      // add bindings
      addDataBinding(NULL, "TestChild", &mChildBinding);
   }
   
   virtual ~TestParentDataBinding()
   {
   }
   
   virtual void getChildren(DataName* dn, list<void*>& children)
   {
      if(mTestParent->getChild() != NULL)
      {
         children.push_back(mTestParent->getChild());
      }
   }
};

void runXmlReaderTest(TestRunner& tr)
{
   tr.test("XmlReader");
   
   XmlReader reader;
   
//   string xml;
//   xml.append("<Book><Chapter number=\"1\"><Title>Test Chapter 1</Title>");
//   xml.append("<Content>This is the first chapter of the book.</Content>");
//   xml.append("</Chapter><Chapter number=\"2\"/></Book>");
   
   string xml;
   //xml = "<TestContent>client request<TestChild id=\"1\"/></TestContent>";
   xml.append("<TestContent>This is my content.");
   xml.append("<TestChild id=\"12\">Blah</TestChild></TestContent>");
   
   // main object to populate
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   reader.start(&db);
   reader.read(&bais);
   reader.finish();
   
   //cout << "TestContent data='" << p.getContent() << "'" << endl;
   assertStrCmp(p.getContent(), "This is my content.");
   assert(p.getChild() != NULL);
   //cout << "TestChild data='" << p.getChild()->getContent() << "'" << endl;
   //assertStrCmp(p.getChild()->getContent(), "Blah");
   tr.warning("fix child content test");
   //cout << "TestChild id='" << p.getChild()->getId() << "'" << endl;
   //assert(p.getChild()->getId() == 12);
   
   //tr.passIfNoException();
   tr.fail();
}

void runXmlWriterTest(TestRunner& tr)
{
   tr.test("XmlWriter");
   
   // main object to write out
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   XmlWriter writer;
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   // write out xml
   writer.write(&db, &os);
   //cout << "XML empty='" << oss.str() << "'" << endl;
   assertStrCmp(oss.str().c_str(), "<TestContent/>");
   
   // clear string stream, reset writer
   oss.str("");
   writer.reset();
   
   // set some content
   p.setContent("Moooooooo");
   
   // add child to TestContent
   TestChild* c = new TestChild();
   c->setId(514);
   p.addChild(c);
   
   writer.write(&db, &os);
   //cout << "XML full=\n" << oss.str() << endl;
   assertStrCmp(oss.str().c_str(),
      "<TestContent>Moooooooo<TestChild id=\"514\"/></TestContent>");
   
   tr.passIfNoException();
}

void runXmlReadWriteTest(TestRunner& tr)
{
   tr.test("XmlReadWrite");
   
   XmlReader reader;
   
   string xml;
   xml.append("<TestContent>This is my content.");
   xml.append("<TestChild id=\"12\">Blah</TestChild></TestContent>");
   
   // main object to populate
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   reader.start(&db);
   reader.read(&bais);
   reader.finish();
   
   //cout << "*****DOING XML READ*****" << endl;
   
   //cout << "TestContent data='" << p.getContent() << "'" << endl;
   assertStrCmp(p.getContent(), "This is my content.");
   assert(p.getChild() != NULL);
   //cout << "TestChild data='" << p.getChild()->getContent() << "'" << endl;
   //assertStrCmp(p.getChild()->getContent(), "Blah");
   tr.warning("fix child content test");
   //cout << "TestChild id='" << p.getChild()->getId() << "'" << endl;
   assert(p.getChild()->getId() == 12);
   
   //cout << endl << "*****DOING XML WRITE*****" << endl;
   
   XmlWriter writer;
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   // write out xml
   writer.write(&db, &os);
   
   //cout << "XML=\n" << oss.str() << endl;
   //assertStrCmp(oss.str().c_str(), xml.c_str());
   tr.warning("fix child content test");
   
   //tr.passIfNoException();
   tr.fail();
}

void runXmlBindingInputStreamTest(TestRunner& tr)
{
   tr.test("XmlBindingInputStream");
   
   // main object to read xml from
   TestParent p;
   
   // set some content
   #define STR "This is a sufficiently long section of element data."
   p.setContent(STR);
   
   // add child to TestContent
   TestChild* c = new TestChild();
   c->setId(514);
   p.addChild(c);
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   // create input stream
   XmlBindingInputStream xbis(&db, 20);
   
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   char b[10];
   int numBytes;
   while((numBytes = xbis.read(b, 10)) > 0)
   {
      os.write(b, numBytes);
   }
   
   //cout << "XML=\n" << oss.str() << endl;
   assertStrCmp(oss.str().c_str(),
      "<TestContent>"
      STR
      "<TestChild id=\"514\"/>"
      "</TestContent>");

   #undef STR
   
   tr.passIfNoException();
}

void runXmlBindingOutputStreamTest(TestRunner& tr)
{
   tr.test("XmlBindingOutputStream");
   
   string xml1;
   string xml2;
   xml1.append("<TestContent>This is the first.");
   xml2.append("<TestChild id=\"64\">Blah</TestChild> Second.</TestContent>");
   
   // main object to populate
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   // create output stream for writing to binding
   XmlBindingOutputStream xbos(&db);
   
   // write xml to output stream
   xbos.write(xml1.c_str(), xml1.length());
   xbos.write(xml2.c_str(), xml2.length());
   //xbos.write((xml1 + xml2).c_str(), xml1.length() + xml2.length());
   
   //cout << "TestContent data='" << p.getContent() << "'" << endl;
   assertStrCmp(p.getContent(), "This is the first. Second.");
   tr.warning("check child in content result");
   assert(p.getChild() != NULL);
   //cout << "TestChild data='" << p.getChild()->getContent() << "'" << endl;
   //assertStrCmp(p.getChild()->getContent(), "Blah");
   tr.warning("fix child content test");
   //cout << "TestChild id='" << p.getChild()->getId() << "'" << endl;
   assert(p.getChild()->getId() == 64);
   
   //tr.passIfNoException();
   tr.fail();
}

class XmlHttpRequestServicer : public HttpRequestServicer
{
public:
   XmlHttpRequestServicer(const char* path) : HttpRequestServicer(path)
   {
   }
   
   virtual ~XmlHttpRequestServicer()
   {
   }
   
   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
      // receive body
      ostringstream oss;
      OStreamOutputStream os(&oss);
      request->receiveBody(&os);
      string xml = oss.str();
      
      // xml object to populate
      TestParent p2;
      TestParentDataBinding db2(&p2);
      
      // read object from xml
      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      reader.start(&db2);
      reader.read(&bais);
      reader.finish();
      
      assertStrCmp(p2.getContent(), "client request");
      assert(p2.getChild()->getId() == 1);
      
      // send 200 OK
      response->getHeader()->setStatus(200, "OK");
      response->getHeader()->setField("Content-Type", "text/xml");
      response->getHeader()->setField("Transfer-Encoding", "chunked");
      response->getHeader()->setField("Connection", "close");
      response->sendHeader();
      
      OutputStream* bos = response->getBodyOutputStream();
      
      // create xml object to write out
      TestParent p;
      p.setContent("server response");
      TestChild* c = new TestChild();
      c->setId(2);
      p.addChild(c);
      
      // data binding for object
      TestParentDataBinding db(&p);
      
      // write out xml
      XmlWriter writer;
      writer.write(&db, bos);
      
      // close and clean up output stream
      bos->close();
      delete bos;
   }
};

void runXmlHttpServerTest(TestRunner& tr)
{
   tr.test("XmlHttpServer");
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("localhost", 19100);
   
   // create SSL/generic http connection servicer
   HttpConnectionServicer hcs;
   server.addConnectionService(&address, &hcs);
   
   // create xml http request servicer
   XmlHttpRequestServicer test1("/test");
   hcs.addRequestServicer(&test1, false);
   
   server.start();
   assertNoException();
   
   // connect
   Url url("http://localhost:19100");
   HttpConnection* hc = HttpClient::createConnection(&url);
   assert(hc != NULL);
   
   // send request header
   HttpRequest* request = (HttpRequest*)hc->createRequest();
   request->getHeader()->setMethod("POST");
   request->getHeader()->setPath("/test");
   request->getHeader()->setVersion("HTTP/1.1");
   request->getHeader()->setField("Host", "localhost:19100");
   request->getHeader()->setField("Content-Type", "text/xml");
   request->getHeader()->setField("Transfer-Encoding", "chunked");
   request->sendHeader();
   assertNoException();
   
   // send request body
   OutputStream* bos = request->getBodyOutputStream();
   
   // create xml object to write out
   TestParent p;
   p.setContent("client request");
   TestChild* c = new TestChild();
   c->setId(1);
   p.addChild(c);
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   // write out xml
   XmlWriter writer;
   writer.write(&db, bos);
   
   // close and clean up output stream
   bos->close();
   delete bos;
   
   // receive response header
   HttpResponse* response = (HttpResponse*)request->createResponse();
   response->receiveHeader();
   assertNoException();
   
   // receive response body
   ostringstream oss;
   OStreamOutputStream os(&oss);
   response->receiveBody(&os);
   string xml = oss.str();
   
   // xml object to populate
   TestParent p2;
   TestParentDataBinding db2(&p2);
   
   // read object from xml
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   XmlReader reader;
   reader.start(&db2);
   reader.read(&bais);
   reader.finish();
   
   assertStrCmp(p2.getContent(), "server response");
   assert(p2.getChild()->getId() == 2);
   
   // clean up request and response
   delete request;
   delete response;
   
   // close and clean up connection
   hc->close();
   delete hc;
   
   // stop server
   server.stop();
   
   // stop kernel engine
   k.getEngine()->stop();
   
   tr.pass();
}

void runDynamicObjectWriterTest(TestRunner& tr)
{
   tr.test("DynamicObjectWriter");
   
   // main object to write to DynamicObject
   TestParent p;
   
   // set some content
   p.setContent("This is test content.");
   
   // add child to TestContent
   TestChild* c = new TestChild();
   c->setId(514);
   c->setContent("This is child content.");
   p.addChild(c);
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   // create DynamicObjectWriter
   DynamicObjectWriter writer;
   
   // write out to dynamic object
   DynamicObject dyno = writer.write(&db);
   
   assertStrCmp(dyno["TestContent"]->getString(), "This is test content.");
   assertStrCmp(dyno["TestChild"]["TestContent"]->getString(),
      "This is child content.");
   assert(dyno["TestChild"]["id"]->getInt32() == 514);
   assert(dyno->length() == 2);
   
   // test print out code
   //cout << endl;
   //dumpDynamicObject(dyno);
   
   tr.pass();
}

void runDynamicObjectReaderTest(TestRunner& tr)
{
   tr.test("DynamicObjectReader");
   
   // dynamic object to read from
   DynamicObject dyno;
   dyno["TestContent"] = "This is test content.";
   dyno["TestChild"]["id"] = 514;
   dyno["TestChild"]["TestContent"] = "This is child content.";
   
   // main object to populate
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   // create DynamicObjectReader
   DynamicObjectReader reader;
   
   // read in from dynamic object
   reader.read(dyno, &db);
   
   assertStrCmp(p.getContent(), "This is test content.");
   assertStrCmp(p.getChild()->getContent(), "This is child content.");
   assert(p.getChild()->getId() == 514);
   
   tr.pass();
}

void runDynamicObjectBasicBindingTest(TestRunner& tr)
{
   tr.test("DynamicObjectBasicBinding");
   
   // create xml writer
   XmlWriter writer;
   writer.setIndentation(0, 1);
   
   // dynamic object to read from
   DynamicObject dyno1;
   DynamicObject dyno2;
   DynamicObject dyno3;
   DynamicObject dyno4;
   dyno1 = "This is test content.";
   dyno2 = true;
   dyno3 = 1234;
   dyno4 = 123.456789;
   
   DynamicObjectBasicBinding db1(&dyno1);
   DynamicObjectBasicBinding db2(&dyno2);
   DynamicObjectBasicBinding db3(&dyno3);
   DynamicObjectBasicBinding db4(&dyno4);
   
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   string xml[4];
   writer.write(&db1, &os);
   xml[0] = oss.str();
   oss.str("");
   
   writer.write(&db2, &os);
   xml[1] = oss.str();
   oss.str("");
   
   writer.write(&db3, &os);
   xml[2] = oss.str();
   oss.str("");
   
   writer.write(&db4, &os);
   xml[3] = oss.str();
   oss.str("");
   
//   for(int i = 0; i < 4; i++)
//   {
//      cout << "XML " << i << "=" << endl << xml[i] << endl;
//   }
   
   string outxml[4];
   for(int i = 0; i < 4; i++)
   {
      // now try to read dynamic object back in
      DynamicObject dyno5;
      DynamicObjectBasicBinding inBinding(&dyno5);
      XmlReader reader;
      ByteArrayInputStream bais(xml[i].c_str(), xml[i].length());
      reader.start(&inBinding);
      reader.read(&bais);
      reader.finish();
      
      // now send dynamic object back out
      DynamicObjectBasicBinding outBinding(&dyno5);
      writer.write(&outBinding, &os);
      outxml[i] = oss.str();
      oss.str("");
      assertStrCmp(xml[i].c_str(), outxml[i].c_str());
      //cout << "OUT XML " << i << "=" << endl << outxml[i] << endl;
   }
   
   tr.pass();
}

void runDynamicObjectArrayBindingTest(TestRunner& tr)
{
   tr.test("DynamicObjectArrayBinding");
   
   // create xml writer
   XmlWriter writer;
   writer.setIndentation(0, 1);
   
   // dynamic object to read from
   DynamicObject dyno;
   dyno[0] = "This is test content.";
   dyno[1] = true;
   dyno[2] = 1234;
   dyno[3] = 123.456789;
   
   DynamicObject dyno2;
   dyno2[0] = "Another string.";
   dyno2[1] = false;
   dyno2[2] = 4321;
   dyno2[3] = 987.654321;
   
   dyno[4] = dyno2;
   
   DynamicObjectArrayBinding db(&dyno);
   
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   writer.write(&db, &os);
   string xml = oss.str();
   oss.str("");
   
   // now try to read dynamic object back in
   DynamicObject dyno5;
   DynamicObjectArrayBinding inBinding(&dyno5);
   XmlReader reader;
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   reader.start(&inBinding);
   reader.read(&bais);
   reader.finish();
   
   // now send dynamic object back out
   DynamicObjectArrayBinding outBinding(&dyno5);
   writer.write(&outBinding, &os);
   string outxml = oss.str();
   
   //cout << "XML=" << endl << xml << endl;
   //cout << "OUT XML=" << endl << outxml << endl;
   assertStrCmp(xml.c_str(), outxml.c_str());
   
   tr.pass();
}

void runDynamicObjectMapBindingTest(TestRunner& tr)
{
   tr.test("DynamicObjectMapBinding");
   
   // create xml writer
   XmlWriter writer;
   writer.setIndentation(0, 1);
   
   // dynamic object to read from
   DynamicObject dyno;
   dyno["astring"] = "This is test content.";
   dyno["aboolean"] = true;
   dyno["aninteger"] = 1234;
   dyno["afloat"] = 123.456789;
   
   DynamicObject dyno2;
   dyno2["astring"] = "Another string.";
   dyno2["aboolean"] = false;
   dyno2["aninteger"] = 4321;
   dyno2["afloat"] = 987.654321;
   
   dyno["anobject"] = dyno2;
   
   DynamicObjectMapBinding db(&dyno);
   
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   writer.write(&db, &os);
   string xml = oss.str();
   oss.str("");
   
   // now try to read dynamic object back in
   DynamicObject dyno5;
   DynamicObjectMapBinding inBinding(&dyno5);
   XmlReader reader;
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   reader.start(&inBinding);
   reader.read(&bais);
   reader.finish();
   
   // now send dynamic object back out
   DynamicObjectMapBinding outBinding(&dyno5);
   writer.write(&outBinding, &os);
   string outxml = oss.str();
   
   //cout << "XML=" << endl << xml << endl;
   //cout << "OUT XML=" << endl << outxml << endl;
   assertStrCmp(xml.c_str(), outxml.c_str());
   
   tr.pass();
}

void runDynamicObjectBindingTest(TestRunner& tr)
{
   tr.test("DynamicObjectBinding");
   
   // create xml writer
   XmlWriter writer;
   writer.setIndentation(0, 1);
   
   // dynamic object to read from
   DynamicObject dyno;
   dyno["astring"] = "This is test content.";
   dyno["aboolean"] = true;
   dyno["aninteger"] = 1234;
   dyno["afloat"] = 123.456789;
   
   DynamicObject dyno2;
   dyno2["astring"] = "Another string.";
   dyno2["aboolean"] = false;
   dyno2["aninteger"] = 4321;
   dyno2["afloat"] = 987.654321;
   
   dyno["anobject"] = dyno2;
   
   DynamicObject dyno3;
   dyno3[0] = "This is test content.";
   dyno3[1] = true;
   dyno3[2] = 1234;
   dyno3[3] = 123.456789;
   
   DynamicObject dyno4;
   dyno4[0] = "Another string.";
   dyno4[1] = false;
   dyno4[2] = 4321;
   dyno4[3] = 987.654321;
   
   dyno3[4] = dyno4;
   
   dyno["dyno3"] = dyno3;
   
   DynamicObjectBinding db(&dyno);
   
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   writer.write(&db, &os);
   string xml = oss.str();
   oss.str("");
   
   // now try to read dynamic object back in
   DynamicObject dyno5;
   DynamicObjectBinding inBinding(&dyno5);
   XmlReader reader;
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   reader.start(&inBinding);
   reader.read(&bais);
   reader.finish();
   
   // now send dynamic object back out
   DynamicObjectBinding outBinding(&dyno5);
   writer.write(&outBinding, &os);
   string outxml = oss.str();
   
   //cout << "XML=" << endl << xml << endl;
   //cout << "OUT XML=" << endl << outxml << endl;
   assertStrCmp(xml.c_str(), outxml.c_str());
   
   tr.pass();
}

void runRiffTest(TestRunner& tr)
{
   tr.group("RIFF");
   
   tr.test("fourcc");
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
      setName("Data");
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
      runXmlBindingInputStreamTest(tr);
      runXmlBindingOutputStreamTest(tr);

      runXmlHttpServerTest(tr);

      runDynamicObjectWriterTest(tr);
      runDynamicObjectReaderTest(tr);
      runDynamicObjectBasicBindingTest(tr);
      runDynamicObjectArrayBindingTest(tr);
      runDynamicObjectMapBindingTest(tr);
      runDynamicObjectBindingTest(tr);

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
