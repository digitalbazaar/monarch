/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/test/Test.h"
#include "monarch/test/Tester.h"
#include "monarch/test/TestRunner.h"
#include "monarch/rt/Thread.h"
#include "monarch/sql/Row.h"
#include "monarch/sql/mysql/MySqlConnection.h"
#include "monarch/sql/mysql/MySqlConnectionPool.h"
#include "monarch/sql/mysql/MySqlDatabaseClient.h"
#include "monarch/util/Timer.h"

#include <cstdio>

using namespace std;
using namespace monarch::test;
using namespace monarch::rt;
using namespace monarch::sql;
using namespace monarch::sql::mysql;
using namespace monarch::util;

#define TABLE_TEST "test.dbmysqltest"

void createMySqlTable(TestRunner& tr, monarch::sql::Connection* c)
{
   tr.test("drop table");
   {
      monarch::sql::Statement* s = c->prepare(
         "DROP TABLE IF EXISTS " TABLE_TEST);
      assertNoException();
      s->execute();
   }
   tr.passIfNoException();

   tr.test("create table");
   {
      monarch::sql::Statement* s = c->prepare(
         "CREATE TABLE IF NOT EXISTS " TABLE_TEST
         " (id BIGINT AUTO_INCREMENT, t TEXT, i BIGINT, "
         "PRIMARY KEY (id))");
      assertNoException();
      s->execute();
   }
   tr.passIfNoException();
}

void executeMySqlStatements(TestRunner& tr, monarch::sql::Connection* c)
{
   tr.test("insert test 1");
   {
      monarch::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST " (t, i) VALUES ('test!', 1234)");
      assertNoException();
      s->execute();
      assert(s->getLastInsertRowId() > 0);
   }
   tr.passIfNoException();

   tr.test("insert test 2");
   {
      monarch::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST " (t, i) VALUES ('!tset', 4321)");
      assertNoException();
      s->execute();
      assert(s->getLastInsertRowId() > 0);
   }
   tr.passIfNoException();

   tr.test("insert positional parameters test");
   {
      monarch::sql::Statement* s;
      //unsigned long long start = System::getCurrentMilliseconds();
      for(int i = 0; i < 20; i++)
      {
         s = c->prepare("INSERT INTO " TABLE_TEST " (t, i) VALUES (?, ?)");
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
      monarch::sql::Statement* s = c->prepare("SELECT t, i FROM " TABLE_TEST);
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
      monarch::sql::Statement* s = c->prepare("SELECT t, i FROM " TABLE_TEST);
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
   Exception::clear();

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

void runMySqlDatabaseClientTest(TestRunner& tr)
{
   tr.group("DatabaseClient");

   // create mysql connection pools
   ConnectionPoolRef readPool = new MySqlConnectionPool(
      "mysql://dbreadclient:k288m2s8f6gk39a@omega.digitalbazaar.com", 1);
   ConnectionPoolRef writePool = new MySqlConnectionPool(
      "mysql://dbwriteclient:k288m2s8f6gk39a@omega.digitalbazaar.com", 1);
   assertNoException();

   // create database client
   DatabaseClientRef dbc = new MySqlDatabaseClient();
   dbc->setDebugLogging(true);
   dbc->setReadConnectionPool(readPool);
   dbc->setWriteConnectionPool(writePool);

   tr.test("initialize");
   {
      dbc->initialize();
   }
   tr.passIfNoException();

   tr.test("define table");
   {
      SchemaObject schema;
      schema["table"] = TABLE_TEST;
      schema["indices"]->append() = "PRIMARY KEY(foo_id)";

      DatabaseClient::addSchemaColumn(schema,
         "foo_id", "BIGINT(20) UNSIGNED AUTO_INCREMENT", "fooId", UInt64);
      DatabaseClient::addSchemaColumn(schema,
         "foo_string", "TEXT", "fooString", String);
      DatabaseClient::addSchemaColumn(schema,
         "foo_flag", "TINYINT(1) UNSIGNED", "fooFlag", Boolean);
      DatabaseClient::addSchemaColumn(schema,
         "foo_int32", "TINYINT(1) UNSIGNED", "fooInt32", Int32);

      dbc->define(schema);
   }
   tr.passIfNoException();

   tr.test("drop table if exists");
   {
      dbc->drop(TABLE_TEST, true);
   }
   tr.passIfNoException();

   tr.test("create table");
   {
      dbc->create(TABLE_TEST, false);
   }
   tr.passIfNoException();

   tr.test("create table if not exists");
   {
      dbc->create(TABLE_TEST, true);
   }
   tr.passIfNoException();

   tr.test("insert");
   {
      DynamicObject row;
      row["fooString"] = "foobar";
      row["fooFlag"] = true;
      row["fooInt32"] = 3;
      SqlExecutableRef se = dbc->insert(TABLE_TEST, row);
      dbc->execute(se);
      assertNoException();
      row["fooId"] = se->lastInsertRowId;

      DynamicObject expect;
      expect["fooId"] = 1;
      expect["fooString"] = "foobar";
      expect["fooFlag"] = true;
      expect["fooInt32"] = 3;
      if(expect != row)
      {
         printf("expected:\n");
         dumpDynamicObject(expect);
         printf("got:\n");
         dumpDynamicObject(row);
      }
      assert(expect == row);
   }
   tr.passIfNoException();

   tr.test("insert again");
   {
      DynamicObject row;
      row["fooString"] = "foobar";
      row["fooFlag"] = false;
      row["fooInt32"] = 3;
      SqlExecutableRef se = dbc->insert(TABLE_TEST, row);
      dbc->execute(se);
      assertNoException();
      row["fooId"] = se->lastInsertRowId;

      DynamicObject expect;
      expect["fooId"] = 2;
      expect["fooString"] = "foobar";
      expect["fooFlag"] = false;
      expect["fooInt32"] = 3;
      if(expect != row)
      {
         printf("expected:\n");
         dumpDynamicObject(expect);
         printf("got:\n");
         dumpDynamicObject(row);
      }
      assert(expect == row);
   }
   tr.passIfNoException();

   tr.test("select one");
   {
      DynamicObject where;
      where["fooId"] = 1;
      SqlExecutableRef se = dbc->selectOne(TABLE_TEST, &where);
      dbc->execute(se);
      assertNoException();

      DynamicObject expect;
      expect["fooId"] = 1;
      expect["fooString"] = "foobar";
      expect["fooFlag"] = true;
      expect["fooInt32"] = 3;
      if(expect != se->result)
      {
         printf("expected:\n");
         dumpDynamicObject(expect);
         printf("got:\n");
         dumpDynamicObject(se->result);
      }
      assert(expect == se->result);
   }
   tr.passIfNoException();

   tr.test("select one specific member");
   {
      DynamicObject where;
      where["fooId"] = 1;
      DynamicObject members;
      members["fooString"];
      SqlExecutableRef se = dbc->selectOne(TABLE_TEST, &where, &members);
      dbc->execute(se);
      assertNoException();

      DynamicObject expect;
      expect["fooString"] = "foobar";
      if(expect != se->result)
      {
         printf("expected:\n");
         dumpDynamicObject(expect);
         printf("got:\n");
         dumpDynamicObject(se->result);
      }
      assert(expect == se->result);
   }
   tr.passIfNoException();

   tr.test("select");
   {
      DynamicObject where;
      where["fooInt32"] = 3;
      SqlExecutableRef se = dbc->select(TABLE_TEST, &where, NULL, 5);
      assert(!se.isNull());
      se->returnRowsFound = true;
      dbc->execute(se);
      assertNoException();
      assert(se->rowsFound == 2);

      DynamicObject expect;
      expect->setType(Array);
      DynamicObject& first = expect->append();
      first["fooId"] = 1;
      first["fooString"] = "foobar";
      first["fooFlag"] = true;
      first["fooInt32"] = 3;
      DynamicObject& second = expect->append();
      second["fooId"] = 2;
      second["fooString"] = "foobar";
      second["fooFlag"] = false;
      second["fooInt32"] = 3;
      if(expect != se->result)
      {
         printf("expected:\n");
         dumpDynamicObject(expect);
         printf("got:\n");
         dumpDynamicObject(se->result);
      }
      assert(expect == se->result);
   }
   tr.passIfNoException();

   tr.test("update");
   {
      DynamicObject row;
      row["fooString"] = "foobar2";
      DynamicObject where;
      where["fooId"] = 2;
      SqlExecutableRef se = dbc->update(TABLE_TEST, row, &where);
      dbc->execute(se);
      assert(se->rowsAffected = 1);
   }
   tr.passIfNoException();

   tr.test("update w/limit");
   {
      DynamicObject row;
      row["fooString"] = "bar";
      DynamicObject where;
      where["fooId"] = 2;
      SqlExecutableRef se = dbc->update(TABLE_TEST, row, &where, 1);
      dbc->execute(se);
      assert(se->rowsAffected = 1);
   }
   tr.passIfNoException();

   tr.test("select updated one");
   {
      DynamicObject where;
      where["fooString"] = "bar";
      SqlExecutableRef se = dbc->selectOne(TABLE_TEST, &where);
      dbc->execute(se);
      assertNoException();

      DynamicObject expect;
      expect["fooId"] = 2;
      expect["fooString"] = "bar";
      expect["fooFlag"] = false;
      expect["fooInt32"] = 3;
      if(expect != se->result)
      {
         printf("expected:\n");
         dumpDynamicObject(expect);
         printf("got:\n");
         dumpDynamicObject(se->result);
      }
      assert(expect == se->result);
   }
   tr.passIfNoException();

   tr.test("select updated");
   {
      DynamicObject where;
      where["fooString"] = "bar";
      SqlExecutableRef se = dbc->select(TABLE_TEST, &where);
      dbc->execute(se);
      assertNoException();

      DynamicObject expect;
      expect[0]["fooId"] = 2;
      expect[0]["fooString"] = "bar";
      expect[0]["fooFlag"] = false;
      expect[0]["fooInt32"] = 3;
      if(expect != se->result)
      {
         printf("expected:\n");
         dumpDynamicObject(expect);
         printf("got:\n");
         dumpDynamicObject(se->result);
      }
      assert(expect == se->result);
   }
   tr.passIfNoException();

   tr.test("select IN()");
   {
      DynamicObject where;
      where["fooString"]->append() = "bar";
      where["fooString"]->append() = "foobar";
      SqlExecutableRef se = dbc->select(TABLE_TEST, &where);
      dbc->execute(se);
      assertNoException();

      DynamicObject expect;
      expect->setType(Array);
      DynamicObject& first = expect->append();
      first["fooId"] = 1;
      first["fooString"] = "foobar";
      first["fooFlag"] = true;
      first["fooInt32"] = 3;
      DynamicObject& second = expect->append();
      second["fooId"] = 2;
      second["fooString"] = "bar";
      second["fooFlag"] = false;
      second["fooInt32"] = 3;
      if(expect != se->result)
      {
         printf("expected:\n");
         dumpDynamicObject(expect);
         printf("got:\n");
         dumpDynamicObject(se->result);
      }
      assert(expect == se->result);
   }
   tr.passIfNoException();

   tr.test("insert on duplicate key update");
   {
      DynamicObject row;
      row["fooId"] = 1;
      row["fooString"] = "duplicate key update";
      SqlExecutableRef se = dbc->insertOnDuplicateKeyUpdate(TABLE_TEST, row);
      dbc->execute(se);
      assert(se->rowsAffected = 1);
   }
   tr.passIfNoException();

   tr.test("select duplicate key updated");
   {
      DynamicObject where;
      where["fooString"] = "duplicate key update";
      SqlExecutableRef se = dbc->selectOne(TABLE_TEST, &where);
      dbc->execute(se);
      assertNoException();

      DynamicObject expect;
      expect["fooId"] = 1;
      expect["fooString"] = "duplicate key update";
      expect["fooFlag"] = true;
      expect["fooInt32"] = 3;
      if(expect != se->result)
      {
         printf("expected:\n");
         dumpDynamicObject(expect);
         printf("got:\n");
         dumpDynamicObject(se->result);
      }
      assert(expect == se->result);
   }
   tr.passIfNoException();

   tr.test("remove w/limit");
   {
      DynamicObject where;
      where["fooId"] = 1;
      SqlExecutableRef se = dbc->remove(TABLE_TEST, &where, 1);
      dbc->execute(se);
      assert(se->rowsAffected == 1);
   }
   tr.passIfNoException();

   tr.test("select again");
   {
      SqlExecutableRef se = dbc->select(TABLE_TEST);
      dbc->execute(se);
      assertNoException();

      DynamicObject expect;
      expect[0]["fooId"] = 2;
      expect[0]["fooString"] = "bar";
      expect[0]["fooFlag"] = false;
      expect[0]["fooInt32"] = 3;
      if(expect != se->result)
      {
         printf("expected:\n");
         dumpDynamicObject(expect);
         printf("got:\n");
         dumpDynamicObject(se->result);
      }
      assert(expect == se->result);
   }
   tr.passIfNoException();

   tr.ungroup();
}

class MySqlConnectionPoolTest : public Runnable
{
public:
   MySqlConnectionPool* pool;
   TestRunner* tr;

   virtual void run()
   {
      monarch::sql::Connection* c = pool->getConnection();
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
   monarch::sql::Connection* c = cp.getConnection();
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

class DbMySqlTester : public monarch::test::Tester
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
      runMySqlDatabaseClientTest(tr);
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

monarch::test::Tester* getDbMySqlTester() { return new DbMySqlTester(); }


DB_TEST_MAIN(DbMySqlTester)
