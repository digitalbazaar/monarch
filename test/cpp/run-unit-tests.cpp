/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <sstream>
#include <list>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/rt/Object.h"
#include "db/rt/Runnable.h"
#include "db/rt/Thread.h"
#include "db/logging/Logging.h"
#include "db/logging/Logger.h"
#include "db/logging/OutputStreamLogger.h"
#include "db/logging/FileLogger.h"

using namespace std;
using namespace db::test;
using namespace db::io;
using namespace db::rt;
using namespace db::util;
using namespace db::logging;

#define DB_TEST_NO_MAIN
#include "test-compress.cpp"
#include "test-config.cpp"
#include "test-crypto.cpp"
#include "test-data.cpp"
#include "test-event.cpp"
#include "test-io.cpp"
#include "test-logging.cpp"
#include "test-mail.cpp"
#include "test-modest.cpp"
#include "test-net.cpp"
#include "test-rt.cpp"
#include "test-sql.cpp"
#include "test-util.cpp"
#undef DB_TEST_NO_MAIN

class DbAllTester : public db::test::Tester
{
protected:
   list<Tester*> mTesters;
   
public:
   DbAllTester()
   {
      mTesters.push_back(new DbCompressTester());
      mTesters.push_back(new DbConfigTester());
      mTesters.push_back(new DbCryptoTester());
      mTesters.push_back(new DbDataTester());
      mTesters.push_back(new DbEventTester());
      mTesters.push_back(new DbIoTester());
      mTesters.push_back(new DbLoggingTester());
      mTesters.push_back(new DbMailTester());
      mTesters.push_back(new DbModestTester());
      mTesters.push_back(new DbNetTester());
      mTesters.push_back(new DbRtTester());
      mTesters.push_back(new DbSqlTester());
      mTesters.push_back(new DbUtilTester());
   }

   ~DbAllTester()
   {
      for(list<Tester*>::iterator i = mTesters.begin();
         i != mTesters.end();
         i++)
      {
         delete *i;
      }
   }
   
   /**
    * Run tests.
    */
   virtual int runTests(TestRunner& tr)
   {
      int rval = 0;
      
      for(list<Tester*>::iterator i = mTesters.begin();
         rval == 0 && i != mTesters.end();
         i++)
      {
         tr.group((*i)->getName());
         rval = (*i)->runAutomaticTests(tr);
         if(rval == 0)
         {
            rval = (*i)->runInteractiveTests(tr);
         }
         tr.ungroup();
      }
      
      return rval;
   }
};

DB_TEST_MAIN(DbAllTester)
