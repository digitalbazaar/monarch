/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/io/ByteArrayInputStream.h"

using namespace std;
using namespace monarch::test;
using namespace monarch::data::rdfa;
using namespace monarch::io;
using namespace monarch::modest;
using namespace monarch::rt;

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
         "<head><title>Speed Test</title></head>\n"
         "<body><p>\n"
         "<span about=\"#foo\" rel=\"dc:title\" resource=\"#you\" />\n"
         "</p></body>\n"
         "</html>";

      ByteArrayInputStream bais(rdfa.c_str(), rdfa.length());
      RdfaReader reader;
      DynamicObject dyno;
      reader.start(dyno);
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
         "<head><title>Speed Test</title></head>\n"
         "<body><p>\n"
         "<span about=\"#foo\" rel=\"dc:title\" resource=\"#you\" />\n"
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

      // FIXME: assert dyno
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
