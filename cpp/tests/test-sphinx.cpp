/*
 * Copyright (c) 2008-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sphinx/SphinxClient.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/OStreamOutputStream.h"
#include "monarch/rt/Exception.h"
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"

using namespace std;
using namespace monarch::sphinx;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::test;

namespace mo_test_sphinx
{

static void runSphinxClientTest(TestRunner &tr)
{
   tr.group("SphinxClient");

   tr.test("searchd protocol");
   {
      Url url("sphinx://omega.digitalbazaar.com:3312");

      SphinxCommand cmd;
      SphinxResponse response;
      SphinxClient client;

      cmd["type"] = SPHINX_SEARCHD_CMD_SEARCH;
      cmd["query"] = "test";
      cmd["matchOffset"] = 0;
      cmd["matchCount"] = 20;
      cmd["matchMode"] = SPHINX_MATCH_ALL;
      cmd["rankMode"] = SPHINX_RANK_PROXIMITY_BM25;
      cmd["sortMode"] = SPHINX_SORT_RELEVANCE;
      cmd["weights"]->append(100);
      cmd["weights"]->append(1);
      cmd["indexes"] = "*";
      cmd["minId"] = 0;
      cmd["maxId"] = 0;
      cmd["maxMatches"] = 1000;
      cmd["groupSort"] = "@group desc";

      client.execute(url, cmd, response);
      assertNoExceptionSet();

      //printf("\nResponse:\n");
      //dumpDynamicObject(response);
   }
   tr.passIfNoException();

   tr.ungroup();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runSphinxClientTest(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.sphinx.test", "1.0", mo_test_sphinx::run)
