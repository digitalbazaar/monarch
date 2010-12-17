/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include "monarch/data/json/JsonWriter.h"
#include "monarch/data/rdfa/RdfaReader.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"

#include <cstdio>

using namespace std;
using namespace monarch::data::rdfa;
using namespace monarch::io;
using namespace monarch::modest;
using namespace monarch::rt;
using namespace monarch::test;

namespace mo_test_data_rdfa
{

static void runRdfaReaderTest(TestRunner& tr)
{
   tr.group("RdfaReader");

   tr.test("no base URI");
   {
      string rdfa =
         "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML+RDFa 1.0//EN\" "
         "\"http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd\">\n"
         "<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
         "      xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
         "<head><title>Test</title></head>\n"
         "<body><p>\n"
         "<span about=\"#foo\" rel=\"dc:title\" resource=\"#you\" />\n"
         "</p></body>\n"
         "</html>";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      DynamicObject dyno;
      assertException(
         reader.start(dyno));
   }
   tr.passIfException();

   tr.test("no base URI (readFromString)");
   {
      string rdfa = "...";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      DynamicObject dyno;
      assertException(
         RdfaReader::readFromString(dyno, "", 0, ""));
   }
   tr.passIfException();

   tr.test("error");
   {
      string rdfa =
         "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML+RDFa 1.0//EN\" "
         "\"http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd\">\n"
         "<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
         "      xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
         "<head><title>Test</title></head>\n"
         "<body><p>\n"
         "<span about=\"#foo\" rel=\"dc:title\" resource=\"#you\" />\n"
         "<p>\n"
         "</p></body>\n"
         "</html>";

      // custom context
      DynamicObject ctx;
      ctx["w3"] = "http://www.w3.org/2009/pointers#";

      // Check with low level API
      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setContext(ctx);
      DynamicObject dyno;
      assertNoException(
         reader.start(dyno));
      assertException(
         reader.read(&bais));

      // check exception
      DynamicObject ex = Exception::getAsDynamicObject();

      DynamicObject expect;
      expect["code"] = 0;
      expect["message"] = "RDFa parse error.";
      expect["type"] = "monarch.data.rdfa.RdfaReader.ParseError";
      DynamicObject& graph = expect["details"]["graph"];
      graph["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      graph["#"]["w3"] = "http://www.w3.org/2009/pointers#";
      graph["@"] = "_:bnode0";
      graph["http://purl.org/dc/terms/description"] =
         "XML parsing error: mismatched tag at line 9, column 6.";
      graph["a"] = "http://www.w3.org/ns/rdfa_processing_graph#Error";
      graph["http://www.w3.org/ns/rdfa_processing_graph#context"]
         ["@"] = "_:bnode1";
      graph["http://www.w3.org/ns/rdfa_processing_graph#context"]
         ["a"] = "w3:LineCharPointer";
      graph["http://www.w3.org/ns/rdfa_processing_graph#context"]
         ["w3:charNumber"] = "6";
      graph["http://www.w3.org/ns/rdfa_processing_graph#context"]
         ["w3:lineNumber"] = "9";
      assertDynoCmp(expect, ex);
   }
   tr.passIfException();

   tr.test("simple");
   {
      string rdfa =
         "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML+RDFa 1.0//EN\" "
         "\"http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd\">\n"
         "<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
         "      xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
         "<head><title>Test</title></head>\n"
         "<body><p>\n"
         "<span about=\"#foo\" rel=\"dc:title\" resource=\"#you\" />\n"
         "</p></body>\n"
         "</html>";

      // Check with low level API
      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      DynamicObject dyno;
      assertNoException(
         reader.start(dyno));
      assertNoException(
         reader.read(&bais));
      assertNoException(
         reader.finish());

      // Check with readFromString
      DynamicObject dyno2;
      assertNoException(
         RdfaReader::readFromString(
            dyno2, rdfa.c_str(), rdfa.length(), "http://example.org/test"));

      DynamicObject expect;
      expect["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      expect["@"] = "http://example.org/test#foo";
      expect["dc:title"] = "http://example.org/test#you";
      assertDynoCmp(expect, dyno);
      assertDynoCmp(expect, dyno2);
   }
   tr.passIfNoException();

   tr.test("single embed");
   {
      string rdfa =
         "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML+RDFa 1.0//EN\" "
         "\"http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd\">\n"
         "<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
         "      xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
         "      xmlns:ex=\"http://example.org/vocab#\">\n"
         "<head><title>Test</title></head>\n"
         "<body><p>\n"
         "<span about=\"#library\" rel=\"ex:contains\" resource=\"#book\" />\n"
         "<span about=\"#book\" property=\"dc:title\">My Book</span>\n"
         "</p></body>\n"
         "</html>";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      DynamicObject dyno;
      assertNoException(
         reader.start(dyno));
      assertNoException(
         reader.read(&bais));
      assertNoException(
         reader.finish());

      DynamicObject expect;
      expect["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      expect["#"]["ex"] = "http://example.org/vocab#";
      expect["@"] = "http://example.org/test#library";
      expect["ex:contains"]["@"] = "http://example.org/test#book";
      expect["ex:contains"]["dc:title"] = "My Book";
      assertDynoCmp(expect, dyno);
   }
   tr.passIfNoException();

   tr.test("double embed");
   {
      string rdfa =
         "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML+RDFa 1.0//EN\" "
         "\"http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd\">\n"
         "<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
         "      xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
         "      xmlns:ex=\"http://example.org/vocab#\">\n"
         "<head><title>Test</title></head>\n"
         "<body><p>\n"
         "<span about=\"#library\" rel=\"ex:contains\" resource=\"#book\" />\n"
         "<span about=\"#book\" property=\"dc:title\">My Book</span>\n"
         "<span about=\"#book\" property=\"dc:contributor\">Writer</span>\n"
         "<span about=\"#book\" rel=\"ex:contains\" resource=\"#chapter\" />\n"
         "<span about=\"#chapter\" property=\"dc:title\">Chapter One</span>\n"
         "<span about=\"#chapter\" property=\"dc:description\">Fun</span>\n"
         "</p></body>\n"
         "</html>";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      DynamicObject dyno;
      assertNoException(
         reader.start(dyno));
      assertNoException(
         reader.read(&bais));
      assertNoException(
         reader.finish());

      DynamicObject expect;
      expect["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      expect["#"]["ex"] = "http://example.org/vocab#";
      expect["@"] = "http://example.org/test#library";
      expect["ex:contains"]["@"] = "http://example.org/test#book";
      expect["ex:contains"]["dc:contributor"] = "Writer";
      expect["ex:contains"]["dc:title"] = "My Book";
      expect["ex:contains"]["ex:contains"]["@"] =
         "http://example.org/test#chapter";
      expect["ex:contains"]["ex:contains"]["dc:description"] = "Fun";
      expect["ex:contains"]["ex:contains"]["dc:title"] = "Chapter One";
      assertDynoCmp(expect, dyno);
   }
   tr.passIfNoException();

   tr.test("2-subgraphs");
   {
      string rdfa =
         "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML+RDFa 1.0//EN\" "
         "\"http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd\">\n"
         "<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
         "      xmlns:foaf=\"http://xmlns.org/foaf/0.1/\">\n"
         "<head><title>Test</title></head>\n"
         "<body><p>\n"
         "<span about=\"#john\" property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" property=\"foaf:name\">Jane</span>\n"
         "</p></body>\n"
         "</html>";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      DynamicObject dyno;
      assertNoException(
         reader.start(dyno));
      assertNoException(
         reader.read(&bais));
      assertNoException(
         reader.finish());

      DynamicObject expect;
      expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"][0]["@"] = "http://example.org/test#jane";
      expect["@"][0]["foaf:name"] = "Jane";
      expect["@"][1]["@"] = "http://example.org/test#john";
      expect["@"][1]["foaf:name"] = "John";
      assertDynoCmp(expect, dyno);
   }
   tr.passIfNoException();

   tr.test("double-referenced embed, 3-subgraphs");
   {
      string rdfa =
         "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML+RDFa 1.0//EN\" "
         "\"http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd\">\n"
         "<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
         "      xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
         "      xmlns:ex=\"http://example.org/vocab#\"\n"
         "      xmlns:foaf=\"http://xmlns.org/foaf/0.1/\">\n"
         "<head><title>Test</title></head>\n"
         "<body><p>\n"
         "<span about=\"#library\" rel=\"ex:contains\" resource=\"#book\" />\n"
         "<span about=\"#book\" property=\"dc:title\">My Book</span>\n"
         "<span about=\"#book\" property=\"dc:contributor\">Writer</span>\n"
         "<span about=\"#book\" rel=\"ex:contains\" resource=\"#chapter\" />\n"
         "<span about=\"#chapter\" property=\"dc:title\">Chapter One</span>\n"
         "<span about=\"#chapter\" property=\"dc:description\">Fun</span>\n"
         "<span about=\"#john\" property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      DynamicObject dyno;
      assertNoException(
         reader.start(dyno));
      assertNoException(
         reader.read(&bais));
      assertNoException(
         reader.finish());

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
      assertDynoCmp(expect, dyno);
   }
   tr.passIfNoException();

   tr.test("multiple values");
   {
      string rdfa =
         "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML+RDFa 1.0//EN\" "
         "\"http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd\">\n"
         "<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
         "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
         "      xmlns:ex=\"http://example.org/vocab#\"\n"
         "      xmlns:foaf=\"http://xmlns.org/foaf/0.1/\">\n"
         "<head></head>\n"
         "<body><p>\n"
         "<span about=\"#book\" property=\"ex:prop\">Prop 1</span>\n"
         "<span about=\"#book\" property=\"ex:prop\">Prop 2</span>\n"
         "</p></body>\n"
         "</html>";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      DynamicObject dyno;
      assertNoException(
         reader.start(dyno));
      assertNoException(
         reader.read(&bais));
      assertNoException(
         reader.finish());

      DynamicObject expect;
      expect["#"]["dc"] = "http://purl.org/dc/terms/";
      expect["#"]["ex"] = "http://example.org/vocab#";
      expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"] = "http://example.org/test#book";
      expect["ex:prop"][0] = "Prop 1";
      expect["ex:prop"][1] = "Prop 2";
      assertDynoCmp(expect, dyno);
   }
   tr.passIfNoException();

   tr.test("HTML5+RDFa");
   {
      string rdfa =
         "<!DOCTYPE html>\n"
         "<html xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
         "<head><title>Test</title></head>\n"
         "<body><p>\n"
         "<span about=\"#foo\" rel=\"dc:title\" resource=\"#you\" />\n"
         "</p></body>\n"
         "</html>";

      // Check with low level API
      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      DynamicObject dyno;
      assertNoException(reader.start(dyno));
      assertNoException(reader.read(&bais));
      assertNoException(reader.finish());

      // Check with readFromString
      DynamicObject dyno2;
      assertNoException(
         RdfaReader::readFromString(
            dyno2, rdfa.c_str(), rdfa.length(), "http://example.org/test"));

      DynamicObject expect;
      expect["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      expect["@"] = "http://example.org/test#foo";
      expect["dc:title"] = "http://example.org/test#you";
      assertDynoCmp(expect, dyno);
      assertDynoCmp(expect, dyno2);
   }
   tr.passIfNoException();

   tr.ungroup();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runRdfaReaderTest(tr);
   }
   /*if(tr.isTestEnabled("rdfa-reader-speed"))
   {
      runRdfaReaderSpeedTest(tr);
   }*/
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.data.rdfa.test", "1.0", mo_test_data_rdfa::run)
