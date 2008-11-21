/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/test/Tester.h"

using namespace db::test;

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
#include "test-sqlite3.cpp"
#include "test-mysql.cpp"
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
      addTester(new DbSqlite3Tester());
      addTester(new DbMySqlTester());
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
