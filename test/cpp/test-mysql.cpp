/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/rt/Thread.h"
#include "db/sql/Row.h"
#include "db/sql/mysql/MySqlConnection.h"
#include "db/sql/mysql/MySqlConnectionPool.h"
#include "db/util/Timer.h"

using namespace std;
using namespace db::test;
using namespace db::rt;
using namespace db::sql;
using namespace db::sql::mysql;
using namespace db::util;

void createMySqlTable(TestRunner& tr, db::sql::Connection* c)
{
   tr.test("drop table");
   {
      db::sql::Statement* s = c->prepare(
         "DROP TABLE IF EXISTS test.dbmysqltest");
      assertNoException();
      s->execute();
   }
   tr.passIfNoException();
   
   tr.test("create table");
   {
      db::sql::Statement* s = c->prepare(
         "CREATE TABLE IF NOT EXISTS test.dbmysqltest "
         "(id BIGINT AUTO_INCREMENT, t TEXT, i BIGINT, "
         "PRIMARY KEY (id))");
      assertNoException();
      s->execute();
   }
   tr.passIfNoException();
}

void executeMySqlStatements(TestRunner& tr, db::sql::Connection* c)
{
   tr.test("insert test 1");
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO test.dbmysqltest (t, i) VALUES ('test!', 1234)");
      assertNoException();
      s->execute();
      assert(s->getLastInsertRowId() > 0);
   }
   tr.passIfNoException();
   
   tr.test("insert test 2");
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO test.dbmysqltest (t, i) VALUES ('!tset', 4321)");
      assertNoException();
      s->execute();
      assert(s->getLastInsertRowId() > 0);
   }
   tr.passIfNoException();
   
   tr.test("insert positional parameters test");
   {
      db::sql::Statement* s;
      //unsigned long long start = System::getCurrentMilliseconds();
      for(int i = 0; i < 20; i++)
      {
         s = c->prepare("INSERT INTO test.dbmysqltest (t, i) VALUES (?, ?)");
         assertNoException();
         s->setText(1, "boundpositional");
         s->setInt32(2, 2220 + i);
         s->execute();
         assert(s->getLastInsertRowId() > 0);
         assertNoException();
      }
      //unsigned long long end = System::getCurrentMilliseconds();
      //printf("TIME=%llu ms\n", (end - start));
   }
   tr.passIfNoException();
   
   tr.test("select test");
   {
      db::sql::Statement* s = c->prepare("SELECT t, i FROM test.dbmysqltest");
      assertNoException();
      s->execute();
      assertNoException();
      
      // fetch rows
      Row* row;
      string t;
      int i;
      while((row = s->fetch()) != NULL)
      {
         row->getText("t", t);
         assertNoException();
         row->getInt32("i", i);
         assertNoException();
         
         if(strcmp(t.c_str(), "test!") == 0)
         {
            assert(i == 1234);
         }
         else if(strcmp(t.c_str(), "!tset") == 0)
         {
            assert(i == 4321);
         }
         else if(strcmp(t.c_str(), "boundpositional") == 0)
         {
            assert(i >= 2220);
         }
         else
         {
            // bad row data
            assert(false);
         }
      }
   }
   tr.passIfNoException();
   
   tr.test("select command ordering test");
   {
      db::sql::Statement* s = c->prepare("SELECT t, i FROM test.dbmysqltest");
      assertNoException();
      s->execute();
      assertNoException();
      
      // fetch rows
      Row* row;
      string t;
      int i;
      while((row = s->fetch()) != NULL)
      {
         row->getText("t", t);
         assertNoException();
         row->getInt32("i", i);
         assertNoException();
         
         if(strcmp(t.c_str(), "test!") == 0)
         {
            assert(i == 1234);
         }
         else if(strcmp(t.c_str(), "!tset") == 0)
         {
            assert(i == 4321);
         }
         else if(strcmp(t.c_str(), "boundpositional") == 0)
         {
            assert(i >= 2220);
         }
         else
         {
            // bad row data
            assert(false);
         }
      }
   }
   tr.passIfNoException();
}

void runMySqlConnectionTest(TestRunner& tr)
{
   tr.test("MySql Connection");
   
   MySqlConnection c;
   c.connect("mysql://dbreadclient:k288m2s8f6gk39a@omega.digitalbazaar.com/test");
   c.close();
   assertNoException();
   
   // clean up mysql
   mysql_library_end();
   
   tr.pass();
}

void runMySqlStatementTest(TestRunner& tr)
{
   tr.group("MySql Statement");
   
   // clear any exceptions
   Exception::clearLast();
   
   MySqlConnection c;
   c.connect("mysql://dbwriteclient:k288m2s8f6gk39a@omega.digitalbazaar.com");
   assertNoException();
   
   // create table
   createMySqlTable(tr, &c);
   
   // execute mysql statements
   executeMySqlStatements(tr, &c);
   
   tr.test("connection close");
   {
      c.close();
   }
   tr.passIfNoException();
   
   // clean up mysql
   mysql_library_end();
   
   tr.ungroup();
}

class MySqlConnectionPoolTest : public Runnable
{
public:
   MySqlConnectionPool* pool;
   TestRunner* tr;
   
   virtual void run()
   {
      db::sql::Connection* c = pool->getConnection();
      executeMySqlStatements(*tr, c);
      c->close();
   }
};

void runMySqlConnectionPoolTest(TestRunner& tr)
{
   tr.group("MySql ConnectionPool");
   
   // create mysql connection pool
   MySqlConnectionPool cp(
      "mysql://dbwriteclient:k288m2s8f6gk39a@omega.digitalbazaar.com", 100);
   assertNoException();
   
   // create table
   db::sql::Connection* c = cp.getConnection();
   createMySqlTable(tr, c);
   c->close();
   
   // create connection test threads
   int testCount = 300;
   MySqlConnectionPoolTest tests[testCount];
   Thread* threads[testCount];
   
   // create threads, set pool for tests
   for(int i = 0; i < testCount; i++)
   {
      tests[i].pool = &cp;
      tests[i].tr = &tr;
      threads[i] = new Thread(&tests[i]);
   }
   
   uint64_t startTime = Timer::startTiming();
   
   // run connection threads
   for(int i = 0; i < testCount; i++)
   {
      while(!threads[i]->start(131072))
      {
         threads[i - 1]->join();
      }
   }
   
   // join threads
   for(int i = 0; i < testCount; i++)
   {
      threads[i]->join();
   }
   
   double seconds = Timer::getSeconds(startTime);
   
   // clean up threads
   for(int i = 0; i < testCount; i++)
   {
      delete threads[i];
   }
   
   // clean up mysql
   mysql_library_end();
   
   // print report
   printf("\nNumber of independent connection uses: %d\n", testCount);
   printf("Number of pooled connections created: %d\n",
      cp.getConnectionCount());
   printf("Total time: %g seconds\n", seconds);
   
   tr.ungroup();
}

class DbMySqlTester : public db::test::Tester
{
public:
   DbMySqlTester()
   {
      setName("mysql");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runMySqlConnectionTest(tr);
      runMySqlStatementTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      runMySqlConnectionPoolTest(tr);
      return 0;
   }
};

db::test::Tester* getDbMySqlTester() { return new DbMySqlTester(); }

DB_TEST_WEAK_MAIN
DB_TEST_MAIN(DbMySqlTester)
