/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <sstream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
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
using namespace db::logging;

#define DB_TEST_NO_MAIN
#include "test-rt.cpp"
#include "test-modest.cpp"
#include "test-util.cpp"
#include "test-io.cpp"
#include "test-crypto.cpp"
#include "test-net.cpp"
#include "test-event.cpp"
#include "test-fiber.cpp"
#include "test-mail.cpp"
#include "test-sql.cpp"
#include "test-data.cpp"
#include "test-compress.cpp"
#include "test-config.cpp"
#include "test-logging.cpp"
#include "test-validation.cpp"
#include "test-sphinx.cpp"
#undef DB_TEST_NO_MAIN

class DbAllTester : public db::test::Tester
{
public:
   DbAllTester()
   {
      addTester(new DbRtTester());
      addTester(new DbModestTester());
      addTester(new DbUtilTester());
      addTester(new DbIoTester());
      addTester(new DbCryptoTester());
      addTester(new DbNetTester());
      addTester(new DbEventTester());
      addTester(new DbFiberTester());
      addTester(new DbMailTester());
      addTester(new DbSqlTester());
      addTester(new DbDataTester());
      addTester(new DbCompressTester());
      addTester(new DbConfigTester());
      addTester(new DbLoggingTester());
      addTester(new DbValidationTester());
      addTester(new DbSphinxClientTester());
   }

   ~DbAllTester() {}
};

DB_TEST_MAIN(DbAllTester)
