/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/test/Tester.h"

using namespace db::test;

db::test::Tester* getDbRtTester();
db::test::Tester* getDbModestTester();
db::test::Tester* getDbUtilTester();
db::test::Tester* getDbIoTester();
db::test::Tester* getDbCryptoTester();
db::test::Tester* getDbNetTester();
db::test::Tester* getDbEventTester();
db::test::Tester* getDbFiberTester();
db::test::Tester* getDbMailTester();
db::test::Tester* getDbSqlite3Tester();
#ifdef HAVE_MYSQL
db::test::Tester* getDbMySqlTester();
#endif
db::test::Tester* getDbDataTester();
db::test::Tester* getDbCompressTester();
db::test::Tester* getDbConfigTester();
db::test::Tester* getDbLoggingTester();
db::test::Tester* getDbValidationTester();
#ifdef HAVE_SPHINX
db::test::Tester* getDbSphinxClientTester();
#endif

class DbAllTester : public db::test::Tester
{
public:
   DbAllTester()
   {
      setName("dbcore");
      addTester(getDbRtTester());
      addTester(getDbModestTester());
      addTester(getDbUtilTester());
      addTester(getDbIoTester());
      addTester(getDbCryptoTester());
      addTester(getDbNetTester());
      addTester(getDbEventTester());
      addTester(getDbFiberTester());
      addTester(getDbMailTester());
      addTester(getDbSqlite3Tester());
#ifdef HAVE_MYSQL
      addTester(getDbMySqlTester());
#endif
      addTester(getDbDataTester());
      addTester(getDbCompressTester());
      addTester(getDbConfigTester());
      addTester(getDbLoggingTester());
      addTester(getDbValidationTester());
#ifdef HAVE_SPHINX
      addTester(getDbSphinxClientTester());
#endif
   }

   ~DbAllTester() {}
};

DB_TEST_MAIN(DbAllTester)
