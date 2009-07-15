/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/rt/Thread.h"
#include "db/sql/Row.h"
#include "db/sql/sqlite3/Sqlite3Connection.h"
#include "db/sql/sqlite3/Sqlite3ConnectionPool.h"
#include "db/util/Timer.h"

using namespace std;
using namespace db::test;
using namespace db::rt;
using namespace db::sql;
using namespace db::sql::sqlite3;
using namespace db::util;

#define TABLE_TEST  "test"
#define TABLE_TEST2 "test2"

void createSqlite3Table(TestRunner* tr, db::sql::Connection* c)
{
   if(tr != NULL)
   {
      tr->test("drop table");
   }
   {
      db::sql::Statement* s = c->prepare("DROP TABLE IF EXISTS " TABLE_TEST);
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
#if 0
   if(tr != NULL)
   {
      tr->test("drop table 2");
   }
   {
      db::sql::Statement* s = c->prepare("DROP TABLE IF EXISTS " TABLE_TEST2);
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
#endif
   if(tr != NULL)
   {
      tr->test("create table");
   }
   {
      db::sql::Statement* s = c->prepare(
         "CREATE TABLE IF NOT EXISTS " TABLE_TEST " (t TEXT, i INT)");
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
#if 0
   if(tr != NULL)
   {
      tr->test("create table 2");
   }
   {
      db::sql::Statement* s = c->prepare(
         "CREATE TABLE IF NOT EXISTS " TABLE_TEST2 " (t TEXT, i INT)");
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
#endif
}

void executeSqlite3Statements(TestRunner* tr, db::sql::Connection* c)
{
   if(tr != NULL)
   {
      tr->test("insert test 1");
   }
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST " (t, i) VALUES ('test!', 1234)");
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
   
   if(tr != NULL)
   {
      tr->test("insert test 2");
   }
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST " (t, i) VALUES ('!tset', 4321)");
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
   
   if(tr != NULL)
   {
      tr->test("insert positional parameters");
   }
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST " (t, i) VALUES (?, ?)");
      assertNoException();
      s->setText(1, "boundpositional");
      s->setInt32(2, 2222);
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
   
   if(tr != NULL)
   {
      tr->test("insert named parameters");
   }
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST " (t, i) VALUES (:first, :second)");
      assertNoException();
      s->setText(":first", "boundnamed");
      s->setInt32(":second", 2223);
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
   
   if(tr != NULL)
   {
      tr->test("select");
   }
   {
      db::sql::Statement* s = c->prepare("SELECT * FROM " TABLE_TEST);
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
      
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
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
#if 0
   if(tr != NULL)
   {
      tr->test("insert test 1");
   }
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST2 " (t, i) VALUES ('test!', 1234)");
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
   
   if(tr != NULL)
   {
      tr->test("insert test 2");
   }
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST2 " (t, i) VALUES ('!tset', 4321)");
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
   
   if(tr != NULL)
   {
      tr->test("insert positional parameters");
   }
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST2 " (t, i) VALUES (?, ?)");
      assertNoException();
      s->setText(1, "boundpositional");
      s->setInt32(2, 2222);
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
   
   if(tr != NULL)
   {
      tr->test("insert named parameters");
   }
   {
      db::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST2 " (t, i) VALUES (:first, :second)");
      assertNoException();
      s->setText(":first", "boundnamed");
      s->setInt32(":second", 2223);
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
   
   if(tr != NULL)
   {
      tr->test("select");
   }
   {
      db::sql::Statement* s = c->prepare("SELECT * FROM " TABLE_TEST2);
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
      
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
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoException();
   }
#endif
}

void runSqlite3ConnectionTest(TestRunner& tr)
{
   tr.test("Sqlite3 Connection");
   
   Sqlite3Connection c;
   c.connect("sqlite3::memory:");
   c.close();
   assertNoException();
   
   tr.pass();
}

void runSqlite3StatementTest(TestRunner& tr)
{
   tr.group("Sqlite3 Statement");
   
   // clear any exceptions
   Exception::clear();
   
   Sqlite3Connection c;
   c.connect("sqlite3::memory:");
   
   // create table
   createSqlite3Table(&tr, &c);
   
   // execute statements
   executeSqlite3Statements(&tr, &c);
   
   tr.test("connection close");
   {
      c.close();
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

void runSqlite3TableTest(TestRunner& tr)
{
   tr.group("Sqlite3 Table");
   
   // clear any exceptions
   Exception::clear();
   
   Sqlite3Connection c;
   c.connect("sqlite3::memory:");
   
   // clean up table if it exists
   tr.test("drop table if exists");
   {
      db::sql::Statement* s = c.prepare(
         "DROP TABLE IF EXISTS " TABLE_TEST);
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   tr.passIfNoException();

   // create a fresh table
   tr.test("create table");
   {
      db::sql::Statement* s = c.prepare(
         "CREATE TABLE " TABLE_TEST " (t TEXT, i INT)");
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   tr.passIfNoException();
   
   // drop table
   tr.test("drop table");
   {
      db::sql::Statement* s = c.prepare(
         "DROP TABLE " TABLE_TEST);
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
   }
   tr.passIfNoException();

   tr.test("connection close");
   {
      c.close();
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

void runSqlite3TableMigrationTest(TestRunner& tr)
{
   tr.group("Sqlite3 Table Migration (1)");
   {
      // test table migration algorithm
      // - begin transaction
      // - alter t1 name to t1_old
      // - create new t1
      // - copy t1_old data to t1
      // - drop t1_old
      // - commit
      
      // clear any exceptions
      Exception::clear();
      
      Sqlite3Connection c;
      c.connect("sqlite3::memory:");
      
      tr.test("create test table");
      {
         db::sql::Statement* s = c.prepare("CREATE TABLE t1 (t TEXT, i INT)");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("begin");
      {
         c.begin();
      }
      tr.passIfNoException();
      
      tr.test("rename");
      {
         db::sql::Statement* s = c.prepare("ALTER TABLE t1 RENAME TO t1_old");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("create new table");
      {
         db::sql::Statement* s = c.prepare("CREATE TABLE t1 (t TEXT, i INT)");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("copy data");
      {
         db::sql::Statement* s =
            c.prepare("INSERT INTO t1 SELECT * FROM t1_old");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("drop old table");
      {
         db::sql::Statement* s = c.prepare("DROP TABLE t1_old");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
      
      tr.test("commit");
      {
         c.commit();
      }
      tr.passIfNoException();
   
      tr.test("connection close");
      {
         c.close();
      }
      tr.passIfNoException();
   }
   tr.ungroup();

   tr.group("Sqlite3 Table Migration (2)");
   {
      // test table migration algorithm 2
      // - begin transaction
      // - create temp table t1_new (new schema)
      // - copy/migrate t1 data to t1_new
      // - drop t1
      // - create table t1 (new schema)
      // - copy t1_new data to t1
      // - drop t1_new
      // - commit
      
      // clear any exceptions
      Exception::clear();
      
      Sqlite3Connection c;
      c.connect("sqlite3::memory:");
      
      tr.test("create test table");
      {
         db::sql::Statement* s = c.prepare("CREATE TABLE t1 (t TEXT, i INT)");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("begin");
      {
         c.begin();
      }
      tr.passIfNoException();
      
      tr.test("create new temp table");
      {
         db::sql::Statement* s =
            c.prepare("CREATE TEMPORARY TABLE t1_new (t TEXT, i INT)");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("copy data");
      {
         db::sql::Statement* s =
            c.prepare("INSERT INTO t1_new SELECT * FROM t1");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("drop old table");
      {
         db::sql::Statement* s = c.prepare("DROP TABLE t1");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
      
      tr.test("create new table");
      {
         db::sql::Statement* s =
            c.prepare("CREATE TABLE t1 (t TEXT, i INT)");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("copy data");
      {
         db::sql::Statement* s =
            c.prepare("INSERT INTO t1 SELECT * FROM t1_new");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("drop temp table");
      {
         db::sql::Statement* s = c.prepare("DROP TABLE t1_new");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
      
      tr.test("commit");
      {
         c.commit();
      }
      tr.passIfNoException();
   
      tr.test("connection close");
      {
         c.close();
      }
      tr.passIfNoException();
   }
   tr.ungroup();
   
   tr.group("Sqlite3 Table Migration (3)");
   {
      // test table migration algorithm 3
      // - begin transaction
      // - create temp table t1_old with old data
      // - drop t1
      // - create table t1 with new schema
      // - copy/migrate t1_old data to t1
      // - drop t1_old
      // - commit
      
      // clear any exceptions
      Exception::clear();
      
      Sqlite3Connection c;
      c.connect("sqlite3::memory:");
      
      tr.test("create test table");
      {
         db::sql::Statement* s = c.prepare("CREATE TABLE t1 (t TEXT, i INT)");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("begin");
      {
         c.begin();
      }
      tr.passIfNoException();
      
      tr.test("create new temp table");
      {
         db::sql::Statement* s =
            c.prepare("CREATE TEMPORARY TABLE t1_old AS SELECT * FROM t1");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("drop old table");
      {
         db::sql::Statement* s = c.prepare("DROP TABLE t1");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
      
      tr.test("create new table");
      {
         db::sql::Statement* s =
            c.prepare("CREATE TABLE t1 (t TEXT, i INT)");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("copy data");
      {
         db::sql::Statement* s =
            c.prepare("INSERT INTO t1 SELECT * FROM t1_old");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
   
      tr.test("drop temp table");
      {
         db::sql::Statement* s = c.prepare("DROP TABLE t1_old");
         assertNoException();
         int success = s->execute();
         assertNoException();
         assert(success);
      }
      tr.passIfNoException();
      
      tr.test("commit");
      {
         c.commit();
      }
      tr.passIfNoException();
   
      tr.test("connection close");
      {
         c.close();
      }
      tr.passIfNoException();
   }
   tr.ungroup();
}

class Sqlite3ThreadTest : public Runnable
{
public:
   Sqlite3Connection* connection;
   
   virtual void run()
   {
      connection = new Sqlite3Connection();
      connection->connect("sqlite3::memory:");
      //connection->connect("sqlite3:///tmp/sqlite3cptest.db");
   }
};

void runSqlite3ThreadTest(TestRunner& tr)
{
   tr.group("Sqlite3 multithread");
   
   // create sqlite3 connection in another thread
   Sqlite3ThreadTest runnable;
   Thread t(&runnable);
   t.start();
   t.join();
   
   // use sqlite3 connection in this thread
   tr.test("connection created in separate thread");
   {
      db::sql::Connection* c = runnable.connection;
      
      // create table
      createSqlite3Table(NULL, c);
      
      // execute statements
      executeSqlite3Statements(NULL, c);
      
      // close connection
      c->close();
      delete c;
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

void runSqlite3ReuseTest(TestRunner& tr)
{
   tr.group("Reuse");
   
   // clear any exceptions
   Exception::clear();
   
   // create sqlite3 connection pool
   Sqlite3ConnectionPool cp("sqlite3::memory:", 1);
   assertNoException();
   
   tr.test("create table");
   {
      // create table
      db::sql::Connection* c = cp.getConnection();
      assert(c != NULL);
      db::sql::Statement* s = c->prepare(
         "CREATE TABLE IF NOT EXISTS " TABLE_TEST " (t TEXT, i INT)");
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
      c->close();
   }
   tr.passIfNoException();
   
   tr.test("insert row");
   {
      // create table
      db::sql::Connection* c = cp.getConnection();
      assert(c != NULL);
      db::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST " (t, i) VALUES ('test!', 1234)");
      assertNoException();
      int success = s->execute();
      assertNoException();
      assert(success);
      c->close();
   }
   tr.passIfNoException();
   
   tr.test("select single row");
   {
      // select single row
      db::sql::Connection* c = cp.getConnection();
      assert(c != NULL);
      db::sql::Statement* s = c->prepare(
         "SELECT * FROM " TABLE_TEST " WHERE i=:i LIMIT 1");
      assertNoException();
      s->setInt32(":i", 1234);
      int success = s->execute();
      assertNoException();
      assert(success);
      
      Row* row = s->fetch();
      assert(row != NULL);
      string t;
      int i;
      
      row->getText("t", t);
      assertNoException();
      row->getInt32("i", i);
      assertNoException();
      
      assertStrCmp(t.c_str(), "test!");
      assert(i == 1234);
      
      c->close();
   }
   tr.passIfNoException();
   
   tr.test("select single row again");
   {
      // select single row
      db::sql::Connection* c = cp.getConnection();
      assert(c != NULL);
      db::sql::Statement* s = c->prepare(
         "SELECT * FROM " TABLE_TEST " WHERE i=:i LIMIT 1");
      assertNoException();
      s->setInt32(":i", 1234);
      int success = s->execute();
      assertNoException();
      assert(success);
      
      Row* row = s->fetch();
      assert(row != NULL);
      string t;
      int i;
      
      row->getText("t", t);
      assertNoException();
      row->getInt32("i", i);
      assertNoException();
      
      assertStrCmp(t.c_str(), "test!");
      assert(i == 1234);
      
      c->close();
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
      executeSqlite3Statements(NULL, c);
      //executeSqlite3Statements(*tr, c);
      c->close();
   }
};

void runSqlite3ConnectionPoolTest(TestRunner& tr)
{
   tr.group("Sqlite3 ConnectionPool");
   
   // create sqlite3 connection pool
   Sqlite3ConnectionPool cp("sqlite3:///tmp/sqlite3cptest.db", 1);
   assertNoException();
   
   // create table
   db::sql::Connection* c = cp.getConnection();
   createSqlite3Table(NULL, c);
   c->close();
   
   // create connection test threads
   int testCount = 200;
   Sqlite3ConnectionPoolTest tests[testCount];
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
   
   // print report
   printf("\nNumber of independent connection uses: %d\n", testCount);
   printf("Number of pooled connections created: %d\n",
      cp.getConnectionCount());
   printf("Total time: %g seconds\n", seconds);
   
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
      runSqlite3StatementTest(tr);
      runSqlite3TableTest(tr);
      runSqlite3TableMigrationTest(tr);
      runSqlite3ThreadTest(tr);
      runSqlite3ReuseTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      runSqlite3ConnectionPoolTest(tr);
      return 0;
   }
};

db::test::Tester* getDbSqlite3Tester() { return new DbSqlite3Tester(); }


DB_TEST_MAIN(DbSqlite3Tester)
