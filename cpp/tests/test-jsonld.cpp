/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include <cstdio>

#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/data/json/JsonLd.h"
#include "monarch/data/json/JsonLdFrame.h"
#include "monarch/logging/Logging.h"

using namespace std;
using namespace monarch::test;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::rt;

namespace mo_test_jsonld
{

static void runJsonLdTest(TestRunner& tr)
{
   tr.group("JSON-LD");

   tr.test("normalize (simple id)");
   {
      DynamicObject in;
      in["@"] = "http://example.org/test#example";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect[0]["@"]["@iri"] = "http://example.org/test#example";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (no subject identifier)");
   {
      DynamicObject in;
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["a"] = "ex:Foo";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect[0]["http://www.w3.org/1999/02/22-rdf-syntax-ns#type"]["@iri"] =
         "http://example.org/vocab#Foo";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (no subject identifier plus embed w/subject)");
   {
      DynamicObject in;
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["a"] = "ex:Foo";
      in["ex:embed"]["@"] = "http://example.org/test#example";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect[0]["http://www.w3.org/1999/02/22-rdf-syntax-ns#type"]["@iri"] =
         "http://example.org/vocab#Foo";
      expect[0]["http://example.org/vocab#embed"]["@iri"] =
         "http://example.org/test#example";
      expect[1]["@"]["@iri"] = "http://example.org/test#example";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (bnode embed)");
   {
      DynamicObject in;
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@"] = "http://example.org/test#example";
      in["a"] = "ex:Foo";
      in["ex:embed"]["a"] = "ex:Bar";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect[0]["@"]["@iri"] = "http://example.org/test#example";
      expect[0]["http://www.w3.org/1999/02/22-rdf-syntax-ns#type"]["@iri"] =
         "http://example.org/vocab#Foo";
      expect[0]["http://example.org/vocab#embed"]
         ["http://www.w3.org/1999/02/22-rdf-syntax-ns#type"]["@iri"] =
            "http://example.org/vocab#Bar";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (named bnode embed failure)");
   {
      DynamicObject in;
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@"] = "http://example.org/test#example";
      in["ex:embed"]["@"] = "_:bnode1";
      in["ex:embed"]["ex:foo"] = "bar";

      DynamicObject out;
      JsonLd::normalize(in, out);
   }
   tr.passIfException();

   // FIMXE: add test for bnode diamond structure fails

   tr.test("normalize (multiple rdf types)");
   {
      DynamicObject in;
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@"] = "http://example.org/test#example";
      in["a"][0] = "ex:Foo";
      in["a"][1] = "ex:Bar";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect[0]["@"]["@iri"] = "http://example.org/test#example";
      expect[0]["http://www.w3.org/1999/02/22-rdf-syntax-ns#type"][0]["@iri"] =
         "http://example.org/vocab#Foo";
      expect[0]["http://www.w3.org/1999/02/22-rdf-syntax-ns#type"][1]["@iri"] =
         "http://example.org/vocab#Bar";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (coerce CURIE value)");
   {
      DynamicObject in;
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@context"]["@coerce"]["xsd:anyURI"] = "ex:foo";
      in["@"] = "http://example.org/test#example";
      in["a"] = "ex:Foo";
      in["ex:foo"] = "ex:Bar";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect[0]["@"]["@iri"] = "http://example.org/test#example";
      expect[0]["http://www.w3.org/1999/02/22-rdf-syntax-ns#type"]["@iri"] =
         "http://example.org/vocab#Foo";
      expect[0]["http://example.org/vocab#foo"]["@iri"] =
         "http://example.org/vocab#Bar";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (single subject complex)");
   {
      DynamicObject in;
      in["@context"]["dc"] = "http://purl.org/dc/elements/1.1/";
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@context"]["@coerce"]["xsd:anyURI"] = "ex:contains";
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
      expect[0]["@"]["@iri"] = "http://example.org/test#book";
      expect[0]["http://purl.org/dc/elements/1.1/contributor"] =
         "Writer";
      expect[0]["http://purl.org/dc/elements/1.1/title"] = "My Book";
      expect[0]["http://example.org/vocab#contains"]["@iri"] =
         "http://example.org/test#chapter";
      expect[1]["@"]["@iri"] = "http://example.org/test#chapter";
      expect[1]["http://purl.org/dc/elements/1.1/description"] = "Fun";
      expect[1]["http://purl.org/dc/elements/1.1/title"] = "Chapter One";
      expect[2]["@"]["@iri"] = "http://example.org/test#library";
      expect[2]["http://example.org/vocab#contains"]["@iri"] =
         "http://example.org/test#book";
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (multiple subjects - complex)");
   {
      DynamicObject in;
      in["@context"]["dc"] = "http://purl.org/dc/elements/1.1/";
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@context"]["@coerce"]["xsd:anyURI"][0] = "ex:authored";
      in["@context"]["@coerce"]["xsd:anyURI"][1] = "ex:contains";
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
      expect[0]["@"]["@iri"] = "http://example.org/test#book";
      expect[0]["http://purl.org/dc/elements/1.1/contributor"] =
         "Writer";
      expect[0]["http://purl.org/dc/elements/1.1/title"] = "My Book";
      expect[0]["http://example.org/vocab#contains"]["@iri"] =
         "http://example.org/test#chapter";
      expect[1]["@"]["@iri"] = "http://example.org/test#chapter";
      expect[1]["http://purl.org/dc/elements/1.1/description"] = "Fun";
      expect[1]["http://purl.org/dc/elements/1.1/title"] = "Chapter One";
      expect[2]["@"]["@iri"] = "http://example.org/test#jane";
      expect[2]["http://example.org/vocab#authored"]["@iri"] =
         "http://example.org/test#chapter";
      expect[2]["http://xmlns.com/foaf/0.1/name"] = "Jane";
      expect[3]["@"]["@iri"] = "http://example.org/test#john";
      expect[3]["http://xmlns.com/foaf/0.1/name"] = "John";
      expect[4]["@"]["@iri"] = "http://example.org/test#library";
      expect[4]["http://example.org/vocab#contains"]["@iri"] =
         "http://example.org/test#book";
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (datatype)");
   {
      DynamicObject in;
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@context"]["@coerce"]["xsd:dateTime"] = "ex:validFrom";
      in["@"] = "http://example.org/test#example";
      in["ex:validFrom"] = "2011-01-25T00:00:00+0000";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect[0]["@"]["@iri"] = "http://example.org/test#example";
      expect[0]["http://example.org/vocab#validFrom"]["@literal"] =
         "2011-01-25T00:00:00+0000";
      expect[0]["http://example.org/vocab#validFrom"]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#dateTime";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (type-coerced datatype)");
   {
      DynamicObject in;
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@context"]["@coerce"]["xsd:dateTime"] = "ex:validFrom";
      in["@"] = "http://example.org/test#example";
      in["ex:validFrom"] = "2011-01-25T00:00:00Z";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect[0]["@"]["@iri"] = "http://example.org/test#example";
      expect[0]["http://example.org/vocab#validFrom"]["@literal"] =
         "2011-01-25T00:00:00Z";
      expect[0]["http://example.org/vocab#validFrom"]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#dateTime";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (type-coerced datatype, double reference)");
   {
      DynamicObject in;
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@context"]["@coerce"]["xsd:dateTime"] = "ex:date";
      in["@"] = "http://example.org/test#example";
      DynamicObject date;
      date = "2011-01-25T00:00:00Z";
      in["ex:date"][0] = date;
      in["ex:date"][1] = date;

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect[0]["@"]["@iri"] = "http://example.org/test#example";
      expect[0]["http://example.org/vocab#date"][0]["@literal"] =
         "2011-01-25T00:00:00Z";
      expect[0]["http://example.org/vocab#date"][0]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#dateTime";
      expect[0]["http://example.org/vocab#date"][1]["@literal"] =
         "2011-01-25T00:00:00Z";
      expect[0]["http://example.org/vocab#date"][1]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#dateTime";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (type-coerced datatype, cycle)");
   {
      DynamicObject in;
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@context"]["@coerce"]["xsd:dateTime"] = "ex:date";
      in["@context"]["@coerce"]["xsd:anyURI"] = "ex:parent";
      in["@"] = "http://example.org/test#example1";
      in["ex:date"] = "2011-01-25T00:00:00Z";
      in["ex:embed"]["@"] = "http://example.org/test#example2";
      in["ex:embed"]["ex:parent"] = "http://example.org/test#example1";

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect[0]["@"]["@iri"] = "http://example.org/test#example1";
      expect[0]["http://example.org/vocab#date"]["@literal"] =
         "2011-01-25T00:00:00Z";
      expect[0]["http://example.org/vocab#date"]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#dateTime";
      expect[0]["http://example.org/vocab#embed"]["@iri"] =
         "http://example.org/test#example2";
      expect[1]["@"]["@iri"] = "http://example.org/test#example2";
      expect[1]["http://example.org/vocab#parent"]["@iri"] =
         "http://example.org/test#example1";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
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
      expect["@"] = "http://example.org/test#example";
      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("remove context (complex)");
   {
      DynamicObject in;
      in["@context"]["dc"] = "http://purl.org/dc/elements/1.1/";
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@context"]["@coerce"]["xsd:anyURI"][0] = "ex:authored";
      in["@context"]["@coerce"]["xsd:anyURI"][1] = "ex:contains";
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
         JsonLd::removeContext(in, out));

      DynamicObject expect;
      expect["@"][0]["@"] = "http://example.org/test#chapter";
      expect["@"][0]["http://purl.org/dc/elements/1.1/description"] = "Fun";
      expect["@"][0]["http://purl.org/dc/elements/1.1/title"] = "Chapter One";
      expect["@"][1]["@"] = "http://example.org/test#jane";
      expect["@"][1]["http://example.org/vocab#authored"]["@iri"] =
         "http://example.org/test#chapter";
      expect["@"][1]["http://xmlns.com/foaf/0.1/name"] = "Jane";
      expect["@"][2]["@"] = "http://example.org/test#john";
      expect["@"][2]["http://xmlns.com/foaf/0.1/name"] = "John";
      expect["@"][3]["@"] = "http://example.org/test#library";
      expect["@"][3]["http://example.org/vocab#contains"]["@"] =
         "http://example.org/test#book";
      expect["@"][3]["http://example.org/vocab#contains"]
         ["http://purl.org/dc/elements/1.1/contributor"] = "Writer";
      expect["@"][3]["http://example.org/vocab#contains"]
         ["http://purl.org/dc/elements/1.1/title"] = "My Book";
      expect["@"][3]["http://example.org/vocab#contains"]
         ["http://example.org/vocab#contains"]["@iri"] =
            "http://example.org/test#chapter";
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("add context");
   {
      DynamicObject in;
      in["@"][0]["@"]["@iri"] = "http://example.org/test#chapter";
      in["@"][0]["http://purl.org/dc/elements/1.1/description"] = "Fun";
      in["@"][0]["http://purl.org/dc/elements/1.1/title"] = "Chapter One";
      in["@"][1]["@"]["@iri"] = "http://example.org/test#jane";
      in["@"][1]["http://example.org/vocab#authored"] =
         "http://example.org/test#chapter";
      in["@"][1]["http://xmlns.com/foaf/0.1/name"] = "Jane";
      in["@"][2]["@"]["@iri"] = "http://example.org/test#john";
      in["@"][2]["http://xmlns.com/foaf/0.1/name"] = "John";
      in["@"][3]["@"]["@iri"] = "http://example.org/test#library";
      in["@"][3]["http://example.org/vocab#contains"]["@"] =
         "http://example.org/test#book";
      in["@"][3]["http://example.org/vocab#contains"]
         ["http://purl.org/dc/elements/1.1/contributor"] = "Writer";
      in["@"][3]["http://example.org/vocab#contains"]
         ["http://purl.org/dc/elements/1.1/title"] = "My Book";
      in["@"][3]["http://example.org/vocab#contains"]
         ["http://example.org/vocab#contains"] =
            "http://example.org/test#chapter";

      DynamicObject ctx;
      ctx["dc"] = "http://purl.org/dc/elements/1.1/";
      ctx["ex"] = "http://example.org/vocab#";
      ctx["foaf"] = "http://xmlns.com/foaf/0.1/";
      DynamicObject out;
      assertNoException(
         JsonLd::addContext(ctx, in, out));

      DynamicObject expect;
      expect["@context"]["dc"] = "http://purl.org/dc/elements/1.1/";
      expect["@context"]["ex"] = "http://example.org/vocab#";
      expect["@context"]["foaf"] = "http://xmlns.com/foaf/0.1/";
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

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("add context (reduced context)");
   {
      DynamicObject in;
      in["@"] = "http://example.org/test#thing";
      in["http://purl.org/dc/terms/title"] = "Title";

      DynamicObject ctx;
      ctx["dc"] = "http://purl.org/dc/terms/";
      ctx["ex"] = "http://example.org/test#";
      ctx["foaf"] = "http://xmlns.com/foaf/0.1/";
      DynamicObject out;
      assertNoException(
         JsonLd::addContext(ctx, in, out));

      DynamicObject expect;
      expect["@context"]["dc"] = "http://purl.org/dc/terms/";
      expect["@context"]["ex"] = "http://example.org/test#";
      expect["@"] = "ex:thing";
      expect["dc:title"] = "Title";
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("add context (coerced ex:contains)");
   {
      DynamicObject in;
      in["@"] = "http://example.org/test#book";
      in["http://example.org/vocab#contains"]["@iri"] =
         "http://example.org/test#chapter";
      in["http://purl.org/dc/elements/1.1/title"] = "Title";

      DynamicObject ctx;
      ctx["dc"] = "http://purl.org/dc/elements/1.1/";
      ctx["ex"] = "http://example.org/vocab#";
      ctx["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      ctx["@coerce"]["xsd:anyURI"] = "ex:contains";

      DynamicObject out;
      assertNoException(
         JsonLd::addContext(ctx, in, out));

      DynamicObject expect;
      expect["@context"]["dc"] = "http://purl.org/dc/elements/1.1/";
      expect["@context"]["ex"] = "http://example.org/vocab#";
      expect["@context"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      expect["@context"]["@coerce"]["xsd:anyURI"] = "ex:contains";
      expect["@"] = "http://example.org/test#book";
      expect["ex:contains"] = "http://example.org/test#chapter";
      expect["dc:title"] = "Title";
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("remove context (coerced ex:contains)");
   {
      DynamicObject in;
      in["@context"]["dc"] = "http://purl.org/dc/elements/1.1/";
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@context"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      in["@context"]["@coerce"]["xsd:anyURI"] = "ex:contains";
      in["@"] = "http://example.org/test#book";
      in["ex:contains"] = "http://example.org/test#chapter";
      in["dc:title"] = "Title";

      DynamicObject out;
      assertNoException(
         JsonLd::removeContext(in, out));

      DynamicObject expect;
      expect["@"] = "http://example.org/test#book";
      expect["http://example.org/vocab#contains"]["@iri"] =
         "http://example.org/test#chapter";
      expect["http://purl.org/dc/elements/1.1/title"] = "Title";

      assertNamedDynoCmp("expect", expect, "result", out);
   }
   tr.passIfNoException();

   tr.test("remove type-coercion context and re-add");
   {
      DynamicObject ctx;
      ctx["ex"] = "http://example.org/vocab#";
      ctx["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      ctx["@coerce"]["xsd:dateTime"] = "ex:date";
      ctx["@coerce"]["xsd:anyURI"] = "ex:parent";

      DynamicObject in;
      in["@context"] = ctx.clone();
      in["@"] = "http://example.org/test#example1";
      in["ex:date"] = "2011-01-25T00:00:00Z";
      in["ex:embed"]["@"] = "http://example.org/test#example2";
      in["ex:embed"]["ex:parent"] = "http://example.org/test#example1";

      DynamicObject expect = in.clone();

      // remove context
      DynamicObject woCtx;
      assertNoException(
         JsonLd::removeContext(in, woCtx));

      // re-add context
      DynamicObject out;
      assertNoException(
         JsonLd::addContext(ctx, woCtx, out));

      assertNamedDynoCmp("expect", expect, "out", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("expand native types");
   {
      DynamicObject in;
      in["@context"]["d"] = "http://purl.org/dc/elements/1.1/";
      in["@context"]["e"] = "http://example.org/vocab#";
      in["@context"]["f"] = "http://xmlns.com/foaf/0.1/";
      in["@"] = "http://example.org/test";
      in["e:bool"] = true;
      in["e:double"] = 1.23;
      in["e:double-zero"] = 0.0;
      in["e:int"] = 123;

      DynamicObject out;
      assertNoException(
         JsonLd::removeContext(in, out));

      DynamicObject expect;
      expect["@"] = "http://example.org/test";
      expect["http://example.org/vocab#bool"]["@literal"] =
         "true";
      expect["http://example.org/vocab#bool"]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#boolean";
      expect["http://example.org/vocab#double"]["@literal"] =
         "1.230000e+00";
      expect["http://example.org/vocab#double"]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#double";
      expect["http://example.org/vocab#double-zero"]["@literal"] =
         "0.000000e+00";
      expect["http://example.org/vocab#double-zero"]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#double";
      expect["http://example.org/vocab#int"]["@literal"] =
         "123";
      expect["http://example.org/vocab#int"]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#integer";
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("change context");
   {
      DynamicObject in;
      in["@context"]["d"] = "http://purl.org/dc/elements/1.1/";
      in["@context"]["e"] = "http://example.org/vocab#";
      in["@context"]["f"] = "http://xmlns.com/foaf/0.1/";
      in["@context"]["@coerce"]["xsd:anyURI"][0] = "e:authored";
      in["@context"]["@coerce"]["xsd:anyURI"][1] = "e:contains";
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
      ctx["@coerce"]["xsd:anyURI"][0] = "ex:authored";
      ctx["@coerce"]["xsd:anyURI"][1] = "ex:contains";
      DynamicObject out;
      assertNoException(
         JsonLd::changeContext(ctx, in, out));

      DynamicObject expect;
      expect["@context"]["dc"] = "http://purl.org/dc/elements/1.1/";
      expect["@context"]["ex"] = "http://example.org/vocab#";
      expect["@context"]["foaf"] = "http://xmlns.com/foaf/0.1/";
      expect["@context"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      expect["@context"]["@coerce"]["xsd:anyURI"][0] = "ex:authored";
      expect["@context"]["@coerce"]["xsd:anyURI"][1] = "ex:contains";
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

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("change context (native types)");
   {
      DynamicObject in;
      in["@context"]["d"] = "http://purl.org/dc/elements/1.1/";
      in["@context"]["e"] = "http://example.org/vocab#";
      in["@context"]["f"] = "http://xmlns.com/foaf/0.1/";
      in["@"] = "http://example.org/test";
      in["e:bool"] = true;
      in["e:double"] = 1.23;
      in["e:double-zero"] = 0.0;
      in["e:int"] = 123;

      DynamicObject ctx;
      ctx["dc"] = "http://purl.org/dc/elements/1.1/";
      ctx["ex"] = "http://example.org/vocab#";
      ctx["foaf"] = "http://xmlns.com/foaf/0.1/";
      DynamicObject out;
      assertNoException(
         JsonLd::changeContext(ctx, in, out));

      DynamicObject expect;
      expect["@context"]["ex"] = "http://example.org/vocab#";
      expect["@"] = "http://example.org/test";
      expect["ex:bool"] = true;
      expect["ex:double"] = 1.230000e+00;
      expect["ex:double-zero"] = 0.000000e+00;
      expect["ex:int"] = 123;
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("normalize (check types)");
   {
      DynamicObject in;
      in["@context"]["d"] = "http://purl.org/dc/elements/1.1/";
      in["@context"]["e"] = "http://example.org/vocab#";
      in["@context"]["f"] = "http://xmlns.com/foaf/0.1/";
      in["@"] = "http://example.org/test";
      in["e:bool"] = true;
      in["e:double"] = 1.23;
      in["e:double-zero"] = 0.0;
      in["e:int"] = 123;

      DynamicObject out;
      assertNoException(
         JsonLd::normalize(in, out));

      DynamicObject expect;
      expect[0]["@"]["@iri"] = "http://example.org/test";
      expect[0]["http://example.org/vocab#bool"]["@literal"] =
         "true";
      expect[0]["http://example.org/vocab#bool"]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#boolean";
      expect[0]["http://example.org/vocab#double"]["@literal"] =
         "1.230000e+00";
      expect[0]["http://example.org/vocab#double"]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#double";
      expect[0]["http://example.org/vocab#double-zero"]["@literal"] =
         "0.000000e+00";
      expect[0]["http://example.org/vocab#double-zero"]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#double";
      expect[0]["http://example.org/vocab#int"]["@literal"] =
         "123";
      expect[0]["http://example.org/vocab#int"]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#integer";
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("change context (native types w/ coerce map)");
   {
      DynamicObject in;
      in["@context"]["e"] = "http://example.org/vocab#";
      in["@"] = "http://example.org/test";
      in["e:bool"] = true;
      in["e:double"] = 1.23;
      in["e:int"] = 123;

      DynamicObject ctx;
      ctx["ex"] = "http://example.org/vocab#";
      ctx["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      ctx["@coerce"]["xsd:boolean"] = "ex:bool";
      ctx["@coerce"]["xsd:double"] = "ex:double";
      ctx["@coerce"]["xsd:integer"] = "ex:int";
      DynamicObject out;
      assertNoException(
         JsonLd::changeContext(ctx, in, out));

      DynamicObject expect;
      expect["@context"] = ctx;
      expect["@"] = "http://example.org/test";
      expect["ex:bool"] = true;
      expect["ex:double"] = 1.23;
      expect["ex:int"] = 123;
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("add context (with coerce)");
   {
      DynamicObject in;
      in["@"] = "http://example.org/test";
      in["http://example.org/test#int"]["@literal"] =
         "123";
      in["http://example.org/test#int"]["@datatype"] =
         "http://www.w3.org/2001/XMLSchema#integer";

      DynamicObject ctx;
      ctx["ex"] = "http://example.org/test#";
      ctx["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      ctx["@coerce"]["xsd:integer"] = "ex:int";

      DynamicObject out;
      assertNoException(
         JsonLd::addContext(ctx, in, out));

      DynamicObject expect;
      expect["@context"]["ex"] = "http://example.org/test#";
      expect["@context"]["xsd"] = "http://www.w3.org/2001/XMLSchema#";
      expect["@context"]["@coerce"]["xsd:integer"] = "ex:int";
      expect["@"] = "http://example.org/test";
      expect["ex:int"] = "123";
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   // common reframe test data and expected results
   DynamicObject reframeData;
   DynamicObject reframeExpect;
   {
      DynamicObject& d = reframeData["library"];
      d["@context"]["dc"] = "http://purl.org/dc/elements/1.1/";
      d["@context"]["ex"] = "http://example.org/vocab#";
      d["@"][0]["@"] = "http://example.org/test#library";
      d["@"][0]["a"] = "ex:Library";
      d["@"][0]["ex:contains"] = "http://example.org/test#book";
      d["@"][1]["@"] = "http://example.org/test#book";
      d["@"][1]["a"] = "ex:Book";
      d["@"][1]["dc:contributor"] = "Writer";
      d["@"][1]["dc:title"] = "My Book";
      d["@"][1]["ex:contains"] = "http://example.org/test#chapter";
      d["@"][2]["@"] = "http://example.org/test#chapter";
      d["@"][2]["a"] = "ex:Chapter";
      d["@"][2]["dc:description"] = "Fun";
      d["@"][2]["dc:title"] = "Chapter One";

      DynamicObject& e = reframeExpect["library"];
      e["@context"]["dc"] = "http://purl.org/dc/elements/1.1/";
      e["@context"]["ex"] = "http://example.org/vocab#";
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
      d["@context"]["dc"] = "http://purl.org/dc/elements/1.1/";
      d["@context"]["ex"] = "http://example.org/vocab#";
      d["@"] = "http://example.org/test#library";
      d["a"] = "ex:Library";
      d["ex:contains"] = "http://example.org/test#book";

      DynamicObject& e = reframeExpect["top"];
      e = d.clone();
      e["@context"]->removeMember("dc");
   }

   tr.test("reframe");
   {
      DynamicObject in = reframeData["library"];

      DynamicObject frame;
      frame["@context"]["ex"] = "http://example.org/vocab#";
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

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("reframe w/CURIE value");
   {
      DynamicObject in;
      in["@context"]["dc"] = "http://purl.org/dc/elements/1.1/";
      in["@context"]["ex"] = "http://example.org/vocab#";
      in["@"][0]["@"] = "http://example.org/test#library";
      in["@"][0]["a"] = "ex:Library";
      in["@"][0]["ex:contains"] = "http://example.org/test#book";
      in["@"][1]["@"] = "http://example.org/test#book";
      in["@"][1]["a"] = "ex:Book";
      in["@"][1]["dc:contributor"] = "Writer";
      in["@"][1]["dc:title"] = "My Book";
      in["@"][1]["ex:contains"] = "http://example.org/test#chapter";
      in["@"][2]["@"] = "http://example.org/test#chapter";
      in["@"][2]["a"] = "ex:Chapter";
      in["@"][2]["dc:description"] = "Fun";
      in["@"][2]["dc:title"] = "Chapter One";
      in["@"][2]["ex:act"] = "ex:ActOne";

      DynamicObject frame;
      frame["@context"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      frame["ex:contains"]["a"] = "ex:Book";
      frame["ex:contains"]["ex:contains"]["a"] = "ex:Chapter";
      JsonLdFrame jlf;
      jlf.setFrame(frame);

      DynamicObject out;
      assertNoException(
         jlf.reframe(in, out));

      DynamicObject expect;
      expect["@context"]["dc"] = "http://purl.org/dc/elements/1.1/";
      expect["@context"]["ex"] = "http://example.org/vocab#";
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
      expect["ex:contains"]["ex:contains"]["ex:act"] = "ex:ActOne";
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("reframe (empty)");
   {
      DynamicObject in = reframeData["library"];

      DynamicObject frame;
      frame["@context"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:BOGUS";
      JsonLdFrame jlf;
      jlf.setFrame(frame);

      DynamicObject out;
      assertNoException(
         jlf.reframe(in, out));

      DynamicObject expect;
      expect->setType(Map);
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
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

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("reframe (type)");
   {
      DynamicObject in = reframeData["library"];

      DynamicObject frame;
      frame["@context"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      JsonLdFrame jlf;
      jlf.setFrame(frame);

      DynamicObject out;
      assertNoException(
         jlf.reframe(in, out));

      DynamicObject expect = reframeExpect["library"];
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("reframe (cleared output)");
   {
      DynamicObject in = reframeData["library"];

      DynamicObject frame;
      frame["@context"]["ex"] = "http://example.org/vocab#";
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

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.test("reframe (top level)");
   {
      DynamicObject in = reframeData["top"];

      DynamicObject frame;
      frame["@context"]["ex"] = "http://example.org/vocab#";
      frame["a"] = "ex:Library";
      JsonLdFrame jlf;
      jlf.setFrame(frame);

      DynamicObject out;
      assertNoException(
         jlf.reframe(in, out));

      DynamicObject expect = reframeExpect["top"];
      assertNamedDynoCmp("expect", expect, "result", out);

      MO_DEBUG("\nINPUT: %s\nOUTPUT: %s",
         JsonWriter::writeToString(in).c_str(),
         JsonWriter::writeToString(out).c_str());
   }
   tr.passIfNoException();

   tr.ungroup();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled() || tr.isTestEnabled("json-ld"))
   {
      runJsonLdTest(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.jsonld.test", "1.0", mo_test_jsonld::run)
