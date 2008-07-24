/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/rt/Thread.h"
#include "db/sql/Row.h"
#include "db/sql/sqlite3/Sqlite3Connection.h"
#include "db/sql/sqlite3/Sqlite3ConnectionPool.h"

using namespace std;
using namespace db::test;
using namespace db::rt;
using namespace db::sql;
using namespace db::sql::sqlite3;

void createSqlite3Table(TestRunner& tr, db::sql::Connection* c)
{
   tr.test("drop table");
   {
      db::sql::Statement* s = c->prepare("DROP TABLE IF EXISTS test");
      assertNoException();
      s->execute();
   }
   tr.passIfNoException();
   
   tr.test("create table");
   {
      db::sql::Statement* s = c->prepare(
         "CREATE TABLE IF NOT EXISTS test (t TEXT, i INT)");
      assertNoException();
      s->execute();
   }
   tr.passIfNoException();
}

void executeSqlite3Statements(TestRunner &tr, db::sql::Connection* c)
{
   tr.test("insert test 1");
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO test (t, i) VALUES ('test!', 1234)");
      assertNoException();
      s->execute();
      assert(s->getLastInsertRowId() > 0);
   }
   tr.passIfNoException();
   
   tr.test("insert test 2");
   {
      c->begin();
      db::sql::Statement* s = c->prepare(
         "INSERT INTO test (t, i) VALUES ('!tset', 4321)");
      assertNoException();
      s->execute();
      assert(s->getLastInsertRowId() > 0);
   }
   tr.passIfNoException();
   
   tr.test("insert positional parameters");
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO test (t, i) VALUES (?, ?)");
      assertNoException();
      s->setText(1, "boundpositional");
      s->setInt32(2, 2222);
      s->execute();
      assert(s->getLastInsertRowId() > 0);
   }
   tr.passIfNoException();
   
   tr.test("insert named parameters");
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO test (t, i) VALUES (:first, :second)");
      assertNoException();
      s->setText(":first", "boundnamed");
      s->setInt32(":second", 2223);
      s->execute();
      assert(s->getLastInsertRowId() > 0);
   }
   tr.passIfNoException();
   
   tr.test("select");
   {
      db::sql::Statement* s = c->prepare("SELECT * FROM test");
      assertNoException();
      s->execute();
      assertNoException();
      
      db::sql::Row* row;
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
            assert(i == 2222);
         }
         else if(strcmp(t.c_str(), "boundnamed") == 0)
         {
            assert(i == 2223);
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

void runSqlite3ConnectionTest(TestRunner &tr)
{
   tr.test("Sqlite3 Connection");
   
   Sqlite3Connection c;
   c.connect("sqlite3::memory:");
   assertNoException();
   
   tr.pass();
}

void runSqlite3StatementTest(TestRunner &tr)
{
   tr.group("Sqlite3 Statement");
   
   // clear any exceptions
   Exception::clearLast();
   
   Sqlite3Connection c;
   c.connect("sqlite3::memory:");
   
   // create table
   createSqlite3Table(tr, &c);
   
   // execute statements
   executeSqlite3Statements(tr, &c);
   
   tr.test("connection close");
   {
      c.close();
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

class Sqlite3ConnectionPoolTest : public Runnable
{
public:
   Sqlite3ConnectionPool* pool;
   TestRunner* tr;
   
   virtual void run()
   {
      db::sql::Connection* c = pool->getConnection();
      executeSqlite3Statements(*tr, c);
      c->close();
   }
};

void runSqlite3ConnectionPoolTest(TestRunner& tr)
{
   tr.group("Sqlite3 ConnectionPool");
   
   // create sqlite3 connection pool
   Sqlite3ConnectionPool cp("sqlite3://localhost/tmp/sqlite3cptest.db", 100);
   assertNoException();
   
   // create table
   db::sql::Connection* c = cp.getConnection();
   createSqlite3Table(tr, c);
   c->close();
   
   // create connection test threads
   int testCount = 300;
   Sqlite3ConnectionPoolTest tests[testCount];
   Thread* threads[testCount];
   
   // create threads, set pool for tests
   for(int i = 0; i < testCount; i++)
   {
      tests[i].pool = &cp;
      tests[i].tr = &tr;
      threads[i] = new Thread(&tests[i]);
   }
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   // run connection threads
   int count = 1;
   for(int i = 0; i < testCount; i++, count++)
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
   
   unsigned long long end = System::getCurrentMilliseconds();
   
   // clean up threads
   for(int i = 0; i < testCount; i++)
   {
      delete threads[i];
   }
   
   cout << endl;
   cout << "Number of independent connection uses: " << testCount << endl;
   cout << "Number of pooled connections created: " << cp.getConnectionCount()
      << endl;
   
   cout << "Total time: " << (end - start) << "ms" << endl;
   
   cout << endl << "ConnectionPool test complete." << endl;
   
   tr.ungroup();
}

class DbSqlite3Tester : public db::test::Tester
{
public:
   DbSqlite3Tester()
   {
      setName("sqlite3");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runSqlite3ConnectionTest(tr);
      // FIXME: significant memory leaks in sqlite3 statement test
      runSqlite3StatementTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      // FIXME: sqlite3 connection pool test causes database lockups
      //runSqlite3ConnectionPoolTest(tr);
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbSqlite3Tester)
#endif
