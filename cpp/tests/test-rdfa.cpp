/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include "monarch/data/json/JsonWriter.h"
#include "monarch/data/rdfa/RdfaReader.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/logging/Logging.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"

#include <cstdio>

using namespace std;
using namespace monarch::data::json;
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

      MO_DEBUG("%s", JsonWriter::writeToString(dyno).c_str());
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

      MO_DEBUG("%s", JsonWriter::writeToString(dyno).c_str());
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
      expect["message"] = "RDFa parse error.";
      expect["type"] = "monarch.data.rdfa.RdfaReader.ParseError";
      DynamicObject& graph = expect["details"]["graph"];
      // FIXME: "dc" not present with context simplification on
      //graph["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
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
         ["w3:charNumber"] =
            "6^^<http://www.w3.org/2001/XMLSchema#positiveInteger>";
      graph["http://www.w3.org/ns/rdfa_processing_graph#context"]
         ["w3:lineNumber"] =
            "9^^<http://www.w3.org/2001/XMLSchema#positiveInteger>";
      assertDynoCmp(expect, ex);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
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
      expect["dc:title"] = "<http://example.org/test#you>";
      assertDynoCmp(expect, dyno);
      assertDynoCmp(expect, dyno2);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
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

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
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

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
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

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
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
      expect["@"][0]["@"] = "http://example.org/test#jane";
      expect["@"][0]["foaf:name"] = "Jane";
      expect["@"][0]["ex:authored"]["@"] = "http://example.org/test#chapter";
      expect["@"][0]["ex:authored"]["dc:description"] = "Fun";
      expect["@"][0]["ex:authored"]["dc:title"] = "Chapter One";
      expect["@"][1]["@"] = "http://example.org/test#john";
      expect["@"][1]["foaf:name"] = "John";
      expect["@"][2]["@"] = "http://example.org/test#library";
      expect["@"][2]["ex:contains"]["@"] = "http://example.org/test#book";
      expect["@"][2]["ex:contains"]["dc:contributor"] = "Writer";
      expect["@"][2]["ex:contains"]["dc:title"] = "My Book";
      expect["@"][2]["ex:contains"]["ex:contains"] =
         "<http://example.org/test#chapter>";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
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
      // FIXME: auto simplification removes unused prefixes
      //expect["#"]["dc"] = "http://purl.org/dc/terms/";
      //expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["#"]["ex"] = "http://example.org/vocab#";
      expect["@"] = "http://example.org/test#book";
      expect["ex:prop"][0] = "Prop 1";
      expect["ex:prop"][1] = "Prop 2";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
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
      expect["dc:title"] = "<http://example.org/test#you>";
      assertDynoCmp(expect, dyno);
      assertDynoCmp(expect, dyno2);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("frame w/subjects");
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
         "<span about=\"#library\" typeof=\"ex:Library\" "
            "rel=\"ex:contains\" resource=\"#book\" />\n"
         "<span about=\"#book\" typeof=\"ex:Book\" "
            "property=\"dc:title\">My Book</span>\n"
         "<span about=\"#book\" property=\"dc:contributor\">Writer</span>\n"
         "<span about=\"#book\" rel=\"ex:contains\" resource=\"#chapter\" />\n"
         "<span about=\"#chapter\" typeof=\"ex:Chapter\" "
            "property=\"dc:title\">Chapter One</span>\n"
         "<span about=\"#chapter\" property=\"dc:description\">Fun</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["@"] = "http://example.org/test#library";
      frame["ex:contains"]["@"] = "http://example.org/test#book";
      frame["ex:contains"]["ex:contains"]["@"] =
         "http://example.org/test#chapter";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame);
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
      // FIXME: context simplification removes unused entries
      //expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
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
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("frame w/types");
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
         "<span about=\"#library\" typeof=\"ex:Library\" "
            "rel=\"ex:contains\" resource=\"#book\" />\n"
         "<span about=\"#book\" typeof=\"ex:Book\" "
            "property=\"dc:title\">My Book</span>\n"
         "<span about=\"#book\" property=\"dc:contributor\">Writer</span>\n"
         "<span about=\"#book\" rel=\"ex:contains\" resource=\"#chapter\" />\n"
         "<span about=\"#chapter\" typeof=\"ex:Chapter\" "
            "property=\"dc:title\">Chapter One</span>\n"
         "<span about=\"#chapter\" property=\"dc:description\">Fun</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      frame["ex:contains"]["a"] = "ex:Book";
      frame["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame);
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
      // FIXME: context simplification removes unused entries
      //expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
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
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("frame w/types +extra auto-data");
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
         "<span about=\"#library\" typeof=\"ex:Library\" "
            "rel=\"ex:contains\" resource=\"#book\" />\n"
         "<span about=\"#book\" typeof=\"ex:Book\" "
            "property=\"dc:title\">My Book</span>\n"
         "<span about=\"#book\" property=\"dc:contributor\">Writer</span>\n"
         "<span about=\"#book\" rel=\"ex:authoredBy\" resource=\"#jane\" />\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#book\" rel=\"ex:contains\" resource=\"#chapter\" />\n"
         "<span about=\"#chapter\" typeof=\"ex:Chapter\" "
            "property=\"dc:title\">Chapter One</span>\n"
         "<span about=\"#chapter\" property=\"dc:description\">Fun</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      frame["ex:contains"]["a"] = "ex:Book";
      frame["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame);
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
      expect["@"] = "http://example.org/test#library";
      expect["a"] = "ex:Library";
      expect["ex:contains"]["@"] = "http://example.org/test#book";
      expect["ex:contains"]["a"] = "ex:Book";
      expect["ex:contains"]["ex:authoredBy"]["@"] =
         "http://example.org/test#jane";
      expect["ex:contains"]["ex:authoredBy"]["a"] = "ex:Person";
      expect["ex:contains"]["ex:authoredBy"]["ex:authored"] =
         "<http://example.org/test#chapter>";
      expect["ex:contains"]["ex:authoredBy"]["foaf:name"] = "Jane";
      expect["ex:contains"]["dc:contributor"] = "Writer";
      expect["ex:contains"]["dc:title"] = "My Book";
      expect["ex:contains"]["ex:contains"]["@"] =
         "http://example.org/test#chapter";
      expect["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      expect["ex:contains"]["ex:contains"]["dc:description"] = "Fun";
      expect["ex:contains"]["ex:contains"]["dc:title"] = "Chapter One";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("frame w/types +extra deep-data");
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
         "<span about=\"#library\" typeof=\"ex:Library\" "
            "rel=\"ex:contains\" resource=\"#book\" />\n"
         "<span about=\"#book\" typeof=\"ex:Book\" "
            "property=\"dc:title\">My Book</span>\n"
         "<span about=\"#book\" property=\"dc:contributor\">Writer</span>\n"
         "<span about=\"#book\" rel=\"ex:authoredBy\" resource=\"#jane\" />\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#book\" rel=\"ex:contains\" resource=\"#chapter\" />\n"
         "<span about=\"#chapter\" typeof=\"ex:Chapter\" "
            "property=\"dc:title\">Chapter One</span>\n"
         "<span about=\"#chapter\" property=\"dc:description\">Fun</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      frame["ex:contains"]["a"] = "ex:Book";
      frame["ex:contains"]["ex:authoredBy"]->setType(Map);
      frame["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame);
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
      expect["@"] = "http://example.org/test#library";
      expect["a"] = "ex:Library";
      expect["ex:contains"]["@"] = "http://example.org/test#book";
      expect["ex:contains"]["a"] = "ex:Book";
      expect["ex:contains"]["ex:authoredBy"]["@"] =
         "http://example.org/test#jane";
      expect["ex:contains"]["ex:authoredBy"]["a"] = "ex:Person";
      expect["ex:contains"]["ex:authoredBy"]["ex:authored"] =
         "<http://example.org/test#chapter>";
      expect["ex:contains"]["ex:authoredBy"]["foaf:name"] = "Jane";
      expect["ex:contains"]["dc:contributor"] = "Writer";
      expect["ex:contains"]["dc:title"] = "My Book";
      expect["ex:contains"]["ex:contains"]["@"] =
         "http://example.org/test#chapter";
      expect["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      expect["ex:contains"]["ex:contains"]["dc:description"] = "Fun";
      expect["ex:contains"]["ex:contains"]["dc:title"] = "Chapter One";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("frame w/types +extra shallow-data");
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
         "<span about=\"#library\" typeof=\"ex:Library\" "
            "rel=\"ex:contains\" resource=\"#book\" />\n"
         "<span about=\"#book\" typeof=\"ex:Book\" "
            "property=\"dc:title\">My Book</span>\n"
         "<span about=\"#book\" property=\"dc:contributor\">Writer</span>\n"
         "<span about=\"#book\" rel=\"ex:authoredBy\" resource=\"#jane\" />\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#book\" rel=\"ex:contains\" resource=\"#chapter\" />\n"
         "<span about=\"#chapter\" typeof=\"ex:Chapter\" "
            "property=\"dc:title\">Chapter One</span>\n"
         "<span about=\"#chapter\" property=\"dc:description\">Fun</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      frame["ex:contains"]["a"] = "ex:Book";
      frame["ex:contains"]["ex:authoredBy"] = "";
      frame["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame);
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
      // FIXME: context simplification removes unused entries
      //expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"] = "http://example.org/test#library";
      expect["a"] = "ex:Library";
      expect["ex:contains"]["@"] = "http://example.org/test#book";
      expect["ex:contains"]["a"] = "ex:Book";
      expect["ex:contains"]["ex:authoredBy"] =
         "<http://example.org/test#jane>";
      expect["ex:contains"]["dc:contributor"] = "Writer";
      expect["ex:contains"]["dc:title"] = "My Book";
      expect["ex:contains"]["ex:contains"]["@"] =
         "http://example.org/test#chapter";
      expect["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      expect["ex:contains"]["ex:contains"]["dc:description"] = "Fun";
      expect["ex:contains"]["ex:contains"]["dc:title"] = "Chapter One";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("frame w/types +extra shallow-data in array");
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
         "<span about=\"#library\" typeof=\"ex:Library\" "
            "rel=\"ex:contains\" resource=\"#book\" />\n"
         "<span about=\"#book\" typeof=\"ex:Book\" "
            "property=\"dc:title\">My Book</span>\n"
         "<span about=\"#book\" property=\"dc:contributor\">Writer</span>\n"
         "<span about=\"#book\" rel=\"ex:authoredBy\" resource=\"#jane\" />\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#book\" rel=\"ex:contains\" resource=\"#chapter\" />\n"
         "<span about=\"#chapter\" typeof=\"ex:Chapter\" "
            "property=\"dc:title\">Chapter One</span>\n"
         "<span about=\"#chapter\" property=\"dc:description\">Fun</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      frame["ex:contains"]["a"] = "ex:Book";
      frame["ex:contains"]["ex:authoredBy"][0]->setType(String);
      frame["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame);
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
      // FIXME: context simplification removes unused entries
      //expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"] = "http://example.org/test#library";
      expect["a"] = "ex:Library";
      expect["ex:contains"]["@"] = "http://example.org/test#book";
      expect["ex:contains"]["a"] = "ex:Book";
      expect["ex:contains"]["ex:authoredBy"][0] =
         "<http://example.org/test#jane>";
      expect["ex:contains"]["dc:contributor"] = "Writer";
      expect["ex:contains"]["dc:title"] = "My Book";
      expect["ex:contains"]["ex:contains"]["@"] =
         "http://example.org/test#chapter";
      expect["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      expect["ex:contains"]["ex:contains"]["dc:description"] = "Fun";
      expect["ex:contains"]["ex:contains"]["dc:title"] = "Chapter One";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("frame w/types +explicit");
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
         "<span about=\"#library\" typeof=\"ex:Library\" "
            "rel=\"ex:contains\" resource=\"#book\" />\n"
         "<span about=\"#book\" typeof=\"ex:Book\" "
            "property=\"dc:title\">My Book</span>\n"
         "<span about=\"#book\" property=\"dc:contributor\">Writer</span>\n"
         "<span about=\"#book\" rel=\"ex:contains\" resource=\"#chapter\" />\n"
         "<span about=\"#chapter\" typeof=\"ex:Chapter\" "
            "property=\"dc:title\">Chapter One</span>\n"
         "<span about=\"#chapter\" property=\"dc:description\">Fun</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      frame["a"] = "ex:Library";
      frame["ex:contains"]["a"] = "ex:Book";
      frame["ex:contains"]["dc:contributor"];
      frame["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      frame["ex:contains"]["ex:contains"]["dc:title"];

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame, true);
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
      // FIXME: context simplification removes unused entries
      //expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"] = "http://example.org/test#library";
      expect["a"] = "ex:Library";
      expect["ex:contains"]["@"] = "http://example.org/test#book";
      expect["ex:contains"]["a"] = "ex:Book";
      expect["ex:contains"]["dc:contributor"] = "Writer";
      expect["ex:contains"]["ex:contains"]["@"] =
         "http://example.org/test#chapter";
      expect["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      expect["ex:contains"]["ex:contains"]["dc:title"] = "Chapter One";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("frame w/types +explicit +useArrays");
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
         "<span about=\"#library\" typeof=\"ex:Library\" "
            "rel=\"ex:contains\" resource=\"#book\" />\n"
         "<span about=\"#book\" typeof=\"ex:Book\" "
            "property=\"dc:title\">My Book</span>\n"
         "<span about=\"#book\" property=\"dc:contributor\">Writer</span>\n"
         "<span about=\"#book\" rel=\"ex:contains\" resource=\"#chapter\" />\n"
         "<span about=\"#chapter\" typeof=\"ex:Chapter\" "
            "property=\"dc:title\">Chapter One</span>\n"
         "<span about=\"#chapter\" property=\"dc:description\">Fun</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      frame["a"] = "ex:Library";
      frame["ex:contains"]["a"] = "ex:Book";
      frame["ex:contains"]["dc:contributor"]->setType(Array);
      frame["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      frame["ex:contains"]["ex:contains"]["dc:title"];

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame, true);
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
      // FIXME: context simplification removes unused entries
      //expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"] = "http://example.org/test#library";
      expect["a"] = "ex:Library";
      expect["ex:contains"]["@"] = "http://example.org/test#book";
      expect["ex:contains"]["a"] = "ex:Book";
      expect["ex:contains"]["dc:contributor"][0] = "Writer";
      expect["ex:contains"]["ex:contains"]["@"] =
         "http://example.org/test#chapter";
      expect["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      expect["ex:contains"]["ex:contains"]["dc:title"] = "Chapter One";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("frame w/types same as triple order");
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
         "<span about=\"#a\" typeof=\"ex:A\" "
            "rel=\"ex:knows\" resource=\"#b\" />\n"
         "<span about=\"#a\" rel=\"ex:knows\" resource=\"#aa\" />\n"
         "<span about=\"#b\" typeof=\"ex:B\" "
            "rel=\"ex:contains\" resource=\"#a\" />\n"
         "<span about=\"#b\" rel=\"ex:contains\" resource=\"#aa\" />\n"
         "<span about=\"#aa\" typeof=\"ex:A\" "
            "property=\"dc:title\">Embedded</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:A";
      frame["ex:knows"]->setType(Array);

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame);
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
      // FIXME: context simplification removes unused entries
      //expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"] = "http://example.org/test#a";
      expect["a"] = "ex:A";
      expect["ex:knows"][0]["@"] = "http://example.org/test#aa";
      expect["ex:knows"][0]["a"] = "ex:A";
      expect["ex:knows"][0]["dc:title"] = "Embedded";
      expect["ex:knows"][1]["@"] = "http://example.org/test#b";
      expect["ex:knows"][1]["a"] = "ex:B";
      expect["ex:knows"][1]["ex:contains"][0] = "<http://example.org/test#a>";
      expect["ex:knows"][1]["ex:contains"][1] = "<http://example.org/test#aa>";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("frame w/types same as triple order +1 type");
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
         "<span about=\"#a\" typeof=\"ex:A\" "
            "rel=\"ex:knows\" resource=\"#b\" />\n"
         "<span about=\"#a\" rel=\"ex:knows\" resource=\"#aa\" />\n"
         "<span about=\"#b\" typeof=\"ex:B\" "
            "rel=\"ex:contains\" resource=\"#a\" />\n"
         "<span about=\"#b\" rel=\"ex:contains\" resource=\"#aa\" />\n"
         "<span about=\"#aa\" typeof=\"ex:A\" "
            "property=\"dc:title\">Embedded</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:A";
      frame["ex:knows"][0]["a"] = "ex:A";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame, true);
      DynamicObject dyno;
      assertNoException(
         reader.start(dyno));
      assertNoException(
         reader.read(&bais));
      assertNoException(
         reader.finish());

      DynamicObject expect;
      expect["#"]["ex"] = "http://example.org/vocab#";
      // FIXME: context simplification removes unused entries
      //expect["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      //expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"] = "http://example.org/test#a";
      expect["a"] = "ex:A";
      expect["ex:knows"][0]["@"] = "http://example.org/test#aa";
      expect["ex:knows"][0]["a"] = "ex:A";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("array frame w/types same as triple order +1 type");
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
         "<span about=\"#a\" typeof=\"ex:A\" "
            "rel=\"ex:knows\" resource=\"#b\" />\n"
         "<span about=\"#a\" rel=\"ex:knows\" resource=\"#aa\" />\n"
         "<span about=\"#b\" typeof=\"ex:B\" "
            "rel=\"ex:contains\" resource=\"#a\" />\n"
         "<span about=\"#b\" rel=\"ex:contains\" resource=\"#aa\" />\n"
         "<span about=\"#aa\" typeof=\"ex:A\" "
            "property=\"dc:title\">Embedded</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame[0]["#"]["ex"] = "http://example.org/vocab#";
      frame[0]["a"] = "ex:A";
      frame[0]["ex:knows"][0]["a"] = "ex:A";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame, true);
      DynamicObject dyno;
      assertNoException(
         reader.start(dyno));
      assertNoException(
         reader.read(&bais));
      assertNoException(
         reader.finish());

      DynamicObject expect;
      expect["#"]["ex"] = "http://example.org/vocab#";
      // FIXME: context simplification removes unused entries
      //expect["#"]["dc"] = "http://purl.org/dc/elements/1.1/";
      //expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"][0]["@"] = "http://example.org/test#a";
      expect["@"][0]["a"] = "ex:A";
      expect["@"][0]["ex:knows"][0] = "<http://example.org/test#aa>";
      expect["@"][1]["@"] = "http://example.org/test#aa";
      expect["@"][1]["a"] = "ex:A";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("frame w/types same as triple order +2 types");
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
         "<span about=\"#a\" typeof=\"ex:A\" "
            "rel=\"ex:knows\" resource=\"#b\" />\n"
         "<span about=\"#a\" rel=\"ex:knows\" resource=\"#aa\" />\n"
         "<span about=\"#b\" typeof=\"ex:B\" "
            "rel=\"ex:contains\" resource=\"#a\" />\n"
         "<span about=\"#b\" rel=\"ex:contains\" resource=\"#aa\" />\n"
         "<span about=\"#aa\" typeof=\"ex:A\" "
            "property=\"dc:title\">Embedded</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:A";
      frame["ex:knows"][0]["a"][0] = "ex:A";
      frame["ex:knows"][0]["a"][1] = "ex:B";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame);
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
      // FIXME: context simplification removes unused entries
      //expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"] = "http://example.org/test#a";
      expect["a"] = "ex:A";
      expect["ex:knows"][0]["@"] = "http://example.org/test#aa";
      expect["ex:knows"][0]["a"] = "ex:A";
      expect["ex:knows"][0]["dc:title"] = "Embedded";
      expect["ex:knows"][1]["@"] = "http://example.org/test#b";
      expect["ex:knows"][1]["a"] = "ex:B";
      expect["ex:knows"][1]["ex:contains"][0] = "<http://example.org/test#a>";
      expect["ex:knows"][1]["ex:contains"][1] = "<http://example.org/test#aa>";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
   }
   tr.passIfNoException();

   tr.test("frame w/types reversed from triple order");
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
         "<span about=\"#a\" typeof=\"ex:A\" "
            "rel=\"ex:knows\" resource=\"#b\" />\n"
         "<span about=\"#a\" rel=\"ex:knows\" resource=\"#aa\" />\n"
         "<span about=\"#b\" typeof=\"ex:B\" "
            "rel=\"ex:contains\" resource=\"#a\" />\n"
         "<span about=\"#b\" rel=\"ex:contains\" resource=\"#aa\" />\n"
         "<span about=\"#aa\" typeof=\"ex:A\" "
            "property=\"dc:title\">Embedded</span>\n"
         "<span about=\"#john\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">John</span>\n"
         "<span about=\"#jane\" typeof=\"ex:Person\" "
            "property=\"foaf:name\">Jane</span>\n"
         "<span about=\"#jane\" rel=\"ex:authored\" resource=\"#chapter\" />\n"
         "</p></body>\n"
         "</html>";

      DynamicObject frame;
      frame["#"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:B";
      frame["ex:contains"][0]["a"] = "ex:A";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      reader.setBaseUri("http://example.org/test");
      reader.setFrame(frame);
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
      // FIXME: context simplification removes unused entries
      //expect["#"]["foaf"] = "http://xmlns.org/foaf/0.1/";
      expect["@"] = "http://example.org/test#b";
      expect["a"] = "ex:B";
      expect["ex:contains"][0]["@"] = "http://example.org/test#a";
      expect["ex:contains"][0]["a"] = "ex:A";
      expect["ex:contains"][0]["ex:knows"][0] = "<http://example.org/test#aa>";
      expect["ex:contains"][0]["ex:knows"][1] = "<http://example.org/test#b>";
      expect["ex:contains"][1]["@"] = "http://example.org/test#aa";
      expect["ex:contains"][1]["a"] = "ex:A";
      expect["ex:contains"][1]["dc:title"] = "Embedded";
      assertDynoCmp(expect, dyno);

      MO_DEBUG("%s", JsonWriter::writeToString(expect).c_str());
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
