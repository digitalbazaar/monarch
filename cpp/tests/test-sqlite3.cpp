/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/json/JsonWriter.h"
#include "monarch/rt/Thread.h"
#include "monarch/sql/Row.h"
#include "monarch/sql/StatementBuilder.h"
#include "monarch/sql/sqlite3/Sqlite3Connection.h"
#include "monarch/sql/sqlite3/Sqlite3ConnectionPool.h"
#include "monarch/sql/sqlite3/Sqlite3DatabaseClient.h"
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/util/Timer.h"

#include <cstdio>

using namespace std;
using namespace monarch::rt;
using namespace monarch::sql;
using namespace monarch::sql::sqlite3;
using namespace monarch::test;
using namespace monarch::util;

#define TABLE_TEST_1 "test_1"
#define TABLE_TEST_2 "test_2"
#define TABLE_TEST_3 "test_3"

namespace mo_test_sqlite3
{

static void createSqlite3Table(TestRunner* tr, monarch::sql::Connection* c)
{
   if(tr != NULL)
   {
      tr->test("drop table");
   }
   {
      monarch::sql::Statement* s = c->prepare(
         "DROP TABLE IF EXISTS " TABLE_TEST_1);
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoExceptionSet();
   }
#if 0
   if(tr != NULL)
   {
      tr->test("drop table 2");
   }
   {
      monarch::sql::Statement* s = c->prepare(
         "DROP TABLE IF EXISTS " TABLE_TEST_2);
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoExceptionSet();
   }
#endif
   if(tr != NULL)
   {
      tr->test("create table");
   }
   {
      monarch::sql::Statement* s = c->prepare(
         "CREATE TABLE IF NOT EXISTS " TABLE_TEST_1 " (t TEXT, i INT)");
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoExceptionSet();
   }
#if 0
   if(tr != NULL)
   {
      tr->test("create table 2");
   }
   {
      monarch::sql::Statement* s = c->prepare(
         "CREATE TABLE IF NOT EXISTS " TABLE_TEST_2 " (t TEXT, i INT)");
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoExceptionSet();
   }
#endif
}

static void executeSqlite3Statements(
   TestRunner* tr, monarch::sql::Connection* c)
{
   if(tr != NULL)
   {
      tr->test("insert test 1");
   }
   {
      // Note: test preparef
      monarch::sql::Statement* s = c->preparef(
         "INSERT INTO %s (t, i) VALUES ('test!', 1234)",
         TABLE_TEST_1);
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoExceptionSet();
   }

   if(tr != NULL)
   {
      tr->test("insert test 2");
   }
   {
      monarch::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST_1 " (t, i) VALUES ('!tset', 4321)");
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoExceptionSet();
   }

   if(tr != NULL)
   {
      tr->test("insert positional parameters");
   }
   {
      monarch::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST_1 " (t, i) VALUES (?, ?)");
      assertNoExceptionSet();
      s->setText(1, "boundpositional");
      s->setInt32(2, 2222);
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoExceptionSet();
   }

   if(tr != NULL)
   {
      tr->test("insert named parameters");
   }
   {
      monarch::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST_1 " (t, i) VALUES (:first, :second)");
      assertNoExceptionSet();
      s->setText(":first", "boundnamed");
      s->setInt32(":second", 2223);
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoExceptionSet();
   }

   if(tr != NULL)
   {
      tr->test("select");
   }
   {
      monarch::sql::Statement* s = c->prepare("SELECT * FROM " TABLE_TEST_1);
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);

      Row* row;
      string t;
      int i;
      while((row = s->fetch()) != NULL)
      {
         row->getText("t", t);
         assertNoExceptionSet();
         row->getInt32("i", i);
         assertNoExceptionSet();

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
      assertNoExceptionSet();
   }
#if 0
   if(tr != NULL)
   {
      tr->test("insert test 1");
   }
   {
      monarch::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST_2 " (t, i) VALUES ('test!', 1234)");
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoExceptionSet();
   }

   if(tr != NULL)
   {
      tr->test("insert test 2");
   }
   {
      monarch::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST_2 " (t, i) VALUES ('!tset', 4321)");
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoExceptionSet();
   }

   if(tr != NULL)
   {
      tr->test("insert positional parameters");
   }
   {
      monarch::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST_2 " (t, i) VALUES (?, ?)");
      assertNoExceptionSet();
      s->setText(1, "boundpositional");
      s->setInt32(2, 2222);
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoExceptionSet();
   }

   if(tr != NULL)
   {
      tr->test("insert named parameters");
   }
   {
      monarch::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST_2 " (t, i) VALUES (:first, :second)");
      assertNoExceptionSet();
      s->setText(":first", "boundnamed");
      s->setInt32(":second", 2223);
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   if(tr != NULL)
   {
      tr->passIfNoException();
   }
   else
   {
      assertNoExceptionSet();
   }

   if(tr != NULL)
   {
      tr->test("select");
   }
   {
      monarch::sql::Statement* s = c->prepare("SELECT * FROM " TABLE_TEST_2);
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);

      Row* row;
      string t;
      int i;
      while((row = s->fetch()) != NULL)
      {
         row->getText("t", t);
         assertNoExceptionSet();
         row->getInt32("i", i);
         assertNoExceptionSet();

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
      assertNoExceptionSet();
   }
#endif
}

static void runSqlite3ConnectionTest(TestRunner& tr)
{
   tr.test("Sqlite3 Connection");

   Sqlite3Connection c;
   c.connect("sqlite3::memory:");
   c.close();
   assertNoExceptionSet();

   tr.pass();
}

static void runSqlite3PrepareManyTest(TestRunner& tr)
{
   // testing create and cleanup of many statements
   // originially used to help find a memory corruption bug
   tr.test("Sqlite3 Prepare Many");
   {
      Sqlite3Connection c;
      c.connect("sqlite3::memory:");
      int n = 100;
      while(n--)
      {
         c.prepare("SELECT 1");
         c.cleanupPreparedStatements();
      }
      c.close();
      assertNoExceptionSet();
   }
   tr.pass();
}

static void runSqlite3StatementTest(TestRunner& tr)
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

static void runSqlite3TableTest(TestRunner& tr)
{
   tr.group("Sqlite3 Table");

   // clear any exceptions
   Exception::clear();

   Sqlite3Connection c;
   c.connect("sqlite3::memory:");

   // clean up table if it exists
   tr.test("drop table if exists");
   {
      monarch::sql::Statement* s = c.prepare(
         "DROP TABLE IF EXISTS " TABLE_TEST_1);
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   tr.passIfNoException();

   // create a fresh table
   tr.test("create table");
   {
      monarch::sql::Statement* s = c.prepare(
         "CREATE TABLE " TABLE_TEST_1 " (t TEXT, i INT)");
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
   }
   tr.passIfNoException();

   // drop table
   tr.test("drop table");
   {
      monarch::sql::Statement* s = c.prepare(
         "DROP TABLE " TABLE_TEST_1);
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
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

static void runSqlite3TableMigrationTest(TestRunner& tr)
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
         monarch::sql::Statement* s = c.prepare("CREATE TABLE t1 (t TEXT, i INT)");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
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
         monarch::sql::Statement* s = c.prepare("ALTER TABLE t1 RENAME TO t1_old");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
         assert(success);
      }
      tr.passIfNoException();

      tr.test("create new table");
      {
         monarch::sql::Statement* s = c.prepare("CREATE TABLE t1 (t TEXT, i INT)");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
         assert(success);
      }
      tr.passIfNoException();

      tr.test("copy data");
      {
         monarch::sql::Statement* s =
            c.prepare("INSERT INTO t1 SELECT * FROM t1_old");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
         assert(success);
      }
      tr.passIfNoException();

      tr.test("drop old table");
      {
         monarch::sql::Statement* s = c.prepare("DROP TABLE t1_old");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
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
         monarch::sql::Statement* s = c.prepare("CREATE TABLE t1 (t TEXT, i INT)");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
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
         monarch::sql::Statement* s =
            c.prepare("CREATE TEMPORARY TABLE t1_new (t TEXT, i INT)");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
         assert(success);
      }
      tr.passIfNoException();

      tr.test("copy data");
      {
         monarch::sql::Statement* s =
            c.prepare("INSERT INTO t1_new SELECT * FROM t1");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
         assert(success);
      }
      tr.passIfNoException();

      tr.test("drop old table");
      {
         monarch::sql::Statement* s = c.prepare("DROP TABLE t1");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
         assert(success);
      }
      tr.passIfNoException();

      tr.test("create new table");
      {
         monarch::sql::Statement* s =
            c.prepare("CREATE TABLE t1 (t TEXT, i INT)");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
         assert(success);
      }
      tr.passIfNoException();

      tr.test("copy data");
      {
         monarch::sql::Statement* s =
            c.prepare("INSERT INTO t1 SELECT * FROM t1_new");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
         assert(success);
      }
      tr.passIfNoException();

      tr.test("drop temp table");
      {
         monarch::sql::Statement* s = c.prepare("DROP TABLE t1_new");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
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
         monarch::sql::Statement* s = c.prepare("CREATE TABLE t1 (t TEXT, i INT)");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
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
         monarch::sql::Statement* s =
            c.prepare("CREATE TEMPORARY TABLE t1_old AS SELECT * FROM t1");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
         assert(success);
      }
      tr.passIfNoException();

      tr.test("drop old table");
      {
         monarch::sql::Statement* s = c.prepare("DROP TABLE t1");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
         assert(success);
      }
      tr.passIfNoException();

      tr.test("create new table");
      {
         monarch::sql::Statement* s =
            c.prepare("CREATE TABLE t1 (t TEXT, i INT)");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
         assert(success);
      }
      tr.passIfNoException();

      tr.test("copy data");
      {
         monarch::sql::Statement* s =
            c.prepare("INSERT INTO t1 SELECT * FROM t1_old");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
         assert(success);
      }
      tr.passIfNoException();

      tr.test("drop temp table");
      {
         monarch::sql::Statement* s = c.prepare("DROP TABLE t1_old");
         assertNoExceptionSet();
         int success = s->execute();
         assertNoExceptionSet();
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

static void runSqlite3ThreadTest(TestRunner& tr)
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
      monarch::sql::Connection* c = runnable.connection;

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

static void runSqlite3ReuseTest(TestRunner& tr)
{
   tr.group("Reuse");

   // clear any exceptions
   Exception::clear();

   // create sqlite3 connection pool
   Sqlite3ConnectionPool cp("sqlite3::memory:", 1);
   assertNoExceptionSet();

   tr.test("create table");
   {
      // create table
      monarch::sql::Connection* c = cp.getConnection();
      assert(c != NULL);
      monarch::sql::Statement* s = c->prepare(
         "CREATE TABLE IF NOT EXISTS " TABLE_TEST_1 " (t TEXT, i INT)");
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
      c->close();
   }
   tr.passIfNoException();

   tr.test("insert row");
   {
      // create table
      monarch::sql::Connection* c = cp.getConnection();
      assert(c != NULL);
      monarch::sql::Statement* s = c->prepare(
         "INSERT INTO " TABLE_TEST_1 " (t, i) VALUES ('test!', 1234)");
      assertNoExceptionSet();
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);
      c->close();
   }
   tr.passIfNoException();

   tr.test("select single row");
   {
      // select single row
      monarch::sql::Connection* c = cp.getConnection();
      assert(c != NULL);
      monarch::sql::Statement* s = c->prepare(
         "SELECT * FROM " TABLE_TEST_1 " WHERE i=:i LIMIT 1");
      assertNoExceptionSet();
      s->setInt32(":i", 1234);
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);

      Row* row = s->fetch();
      assert(row != NULL);
      string t;
      int i;

      row->getText("t", t);
      assertNoExceptionSet();
      row->getInt32("i", i);
      assertNoExceptionSet();

      assertStrCmp(t.c_str(), "test!");
      assert(i == 1234);

      c->close();
   }
   tr.passIfNoException();

   tr.test("select single row again");
   {
      // select single row
      monarch::sql::Connection* c = cp.getConnection();
      assert(c != NULL);
      monarch::sql::Statement* s = c->prepare(
         "SELECT * FROM " TABLE_TEST_1 " WHERE i=:i LIMIT 1");
      assertNoExceptionSet();
      s->setInt32(":i", 1234);
      int success = s->execute();
      assertNoExceptionSet();
      assert(success);

      Row* row = s->fetch();
      assert(row != NULL);
      string t;
      int i;

      row->getText("t", t);
      assertNoExceptionSet();
      row->getInt32("i", i);
      assertNoExceptionSet();

      assertStrCmp(t.c_str(), "test!");
      assert(i == 1234);

      c->close();
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runSqlite3DatabaseClientTest(TestRunner& tr)
{
   tr.group("DatabaseClient");

   // create sqlite3 connection pool
   Sqlite3ConnectionPool* cp = new Sqlite3ConnectionPool("sqlite3::memory:", 1);
   ConnectionPoolRef pool(cp);
   assertNoExceptionSet();

   // create database client
   DatabaseClientRef dbc = new Sqlite3DatabaseClient();
   dbc->setDebugLogging(true);
   dbc->setReadConnectionPool(pool);
   dbc->setWriteConnectionPool(pool);

   tr.test("initialize");
   {
      dbc->initialize();
   }
   tr.passIfNoException();

   tr.test("define table");
   {
      SchemaObject schema;
      schema["table"] = TABLE_TEST_1;

      // stored in object as string, in database as uint64
      DatabaseClient::addSchemaColumn(schema,
         "foo_id", "INTEGER PRIMARY KEY", "fooId", String, UInt64);
      DatabaseClient::addSchemaColumn(schema,
         "foo_string", "TEXT", "fooString", String);
      DatabaseClient::addSchemaColumn(schema,
         "foo_flag", "INTEGER", "fooFlag", Boolean);
      DatabaseClient::addSchemaColumn(schema,
         "foo_int32", "INTEGER", "fooInt32", Int32);

      dbc->define(schema);
   }
   tr.passIfNoException();

   tr.test("create table");
   {
      dbc->create(TABLE_TEST_1, false);
   }
   tr.passIfNoException();

   tr.test("create table if not exists");
   {
      dbc->create(TABLE_TEST_1, true);
   }
   tr.passIfNoException();

   tr.test("insert");
   {
      DynamicObject row;
      row["fooString"] = "foobar";
      row["fooFlag"] = true;
      row["fooInt32"] = 3;
      SqlExecutableRef se = dbc->insert(TABLE_TEST_1, row);
      dbc->execute(se);
      assertNoExceptionSet();
      row["fooId"] = se->lastInsertRowId;
      row["fooId"]->setType(String);

      DynamicObject expect;
      expect["fooId"] = "1";
      expect["fooString"] = "foobar";
      expect["fooFlag"] = true;
      expect["fooInt32"] = 3;
      assertNamedDynoCmp("expected", expect, "got", row);
   }
   tr.passIfNoException();

   tr.test("insert again");
   {
      DynamicObject row;
      row["fooString"] = "foobar";
      row["fooFlag"] = false;
      row["fooInt32"] = 3;
      SqlExecutableRef se = dbc->insert(TABLE_TEST_1, row);
      dbc->execute(se);
      assertNoExceptionSet();
      row["fooId"] = se->lastInsertRowId;
      row["fooId"]->setType(String);

      DynamicObject expect;
      expect["fooId"] = "2";
      expect["fooString"] = "foobar";
      expect["fooFlag"] = false;
      expect["fooInt32"] = 3;
      assertNamedDynoCmp("expected", expect, "got", row);
   }
   tr.passIfNoException();

   tr.test("select one");
   {
      DynamicObject where;
      where["fooId"] = "1";
      SqlExecutableRef se = dbc->selectOne(TABLE_TEST_1, &where);
      dbc->execute(se);
      assertNoExceptionSet();

      DynamicObject expect;
      expect["fooId"] = "1";
      expect["fooString"] = "foobar";
      expect["fooFlag"] = true;
      expect["fooInt32"] = 3;
      assertNamedDynoCmp("expected", expect, "got", se->result);
   }
   tr.passIfNoException();

   tr.test("select one specific member");
   {
      DynamicObject where;
      where["fooId"] = "1";
      DynamicObject members;
      members["fooString"];
      SqlExecutableRef se = dbc->selectOne(TABLE_TEST_1, &where, &members);
      dbc->execute(se);
      assertNoExceptionSet();

      DynamicObject expect;
      expect["fooString"] = "foobar";
      assertNamedDynoCmp("expected", expect, "got", se->result);
   }
   tr.passIfNoException();

   tr.test("select");
   {
      DynamicObject where;
      where["fooInt32"] = 3;
      SqlExecutableRef se = dbc->select(TABLE_TEST_1, &where, NULL, 5);
      dbc->execute(se);
      assertNoExceptionSet();

      DynamicObject expect;
      expect->setType(Array);
      DynamicObject& first = expect->append();
      first["fooId"] = "1";
      first["fooString"] = "foobar";
      first["fooFlag"] = true;
      first["fooInt32"] = 3;
      DynamicObject& second = expect->append();
      second["fooId"] = "2";
      second["fooString"] = "foobar";
      second["fooFlag"] = false;
      second["fooInt32"] = 3;
      assertNamedDynoCmp("expected", expect, "got", se->result);
   }
   tr.passIfNoException();

   tr.test("update");
   {
      DynamicObject row;
      row["fooString"] = "foobar2";
      DynamicObject where;
      where["fooId"] = "2";
      SqlExecutableRef se = dbc->update(TABLE_TEST_1, row, &where);
      dbc->execute(se);
      assert(se->rowsAffected = 1);
   }
   tr.passIfNoException();

   tr.test("update w/limit");
   {
      DynamicObject row;
      row["fooString"] = "bar";
      DynamicObject where;
      where["fooId"] = "2";
      SqlExecutableRef se = dbc->update(TABLE_TEST_1, row, &where, 1);
      dbc->execute(se);
      assert(se->rowsAffected = 1);
   }
   tr.passIfNoException();

   tr.test("select updated one");
   {
      DynamicObject where;
      where["fooString"] = "bar";
      SqlExecutableRef se = dbc->selectOne(TABLE_TEST_1, &where);
      dbc->execute(se);
      assertNoExceptionSet();

      DynamicObject expect;
      expect["fooId"] = "2";
      expect["fooString"] = "bar";
      expect["fooFlag"] = false;
      expect["fooInt32"] = 3;
      assertNamedDynoCmp("expected", expect, "got", se->result);
   }
   tr.passIfNoException();

   tr.test("select updated");
   {
      DynamicObject where;
      where["fooString"] = "bar";
      SqlExecutableRef se = dbc->select(TABLE_TEST_1, &where);
      dbc->execute(se);
      assertNoExceptionSet();

      DynamicObject expect;
      expect[0]["fooId"] = "2";
      expect[0]["fooString"] = "bar";
      expect[0]["fooFlag"] = false;
      expect[0]["fooInt32"] = 3;
      assertNamedDynoCmp("expected", expect, "got", se->result);
   }
   tr.passIfNoException();

   tr.test("select IN()");
   {
      DynamicObject where;
      where["fooString"]->append() = "bar";
      where["fooString"]->append() = "foobar";
      SqlExecutableRef se = dbc->select(TABLE_TEST_1, &where);
      dbc->execute(se);
      assertNoExceptionSet();

      DynamicObject expect;
      expect->setType(Array);
      DynamicObject& first = expect->append();
      first["fooId"] = "1";
      first["fooString"] = "foobar";
      first["fooFlag"] = true;
      first["fooInt32"] = 3;
      DynamicObject& second = expect->append();
      second["fooId"] = "2";
      second["fooString"] = "bar";
      second["fooFlag"] = false;
      second["fooInt32"] = 3;
      assertNamedDynoCmp("expected", expect, "got", se->result);
   }
   tr.passIfNoException();

   tr.test("define seq table");
   {
      SchemaObject schema;
      schema["table"] = TABLE_TEST_3;

      // stored in object as string, in database as uint64
      DatabaseClient::addSchemaColumn(schema,
         "val", "INTEGER", "val", Int32);

      dbc->define(schema);
   }
   tr.passIfNoException();

   tr.test("create seq table");
   {
      dbc->create(TABLE_TEST_3, false);
   }
   tr.passIfNoException();

   tr.test("fill seq table");
   {
      for(int i = 0; i < 3; ++i)
      {
         DynamicObject row;
         row["val"] = i;
         SqlExecutableRef se = dbc->insert(TABLE_TEST_3, row);
         dbc->execute(se);
         assertNoExceptionSet();
      }
   }
   tr.passIfNoException();

   tr.test("select WHERE op");
   {
      {
         DynamicObject where;
         where["val"]["op"] = ">=";
         where["val"]["value"] = 1;
         SqlExecutableRef se = dbc->select(TABLE_TEST_3, &where);
         dbc->execute(se);
         assertNoExceptionSet();
         DynamicObject expect;
         for(int i = 1; i < 3; ++i)
         {
            DynamicObject& val = expect->append();
            val["val"] = i;
         }
         expect->setType(Array);
         assertNamedDynoCmp("expected", expect, "got", se->result);
      }

      {
         DynamicObject where;
         where["val"]["op"] = "<";
         where["val"]["value"] = 1;
         SqlExecutableRef se = dbc->select(TABLE_TEST_3, &where);
         dbc->execute(se);
         assertNoExceptionSet();
         DynamicObject expect;
         DynamicObject& val = expect->append();
         val["val"] = 0;
         expect->setType(Array);
         assertNamedDynoCmp("expected", expect, "got", se->result);
      }
   }
   tr.passIfNoException();

   tr.test("remove");
   {
      DynamicObject where;
      where["fooId"] = "1";
      SqlExecutableRef se = dbc->remove(TABLE_TEST_1, &where);
      dbc->execute(se);
      assert(se->rowsAffected == 1);
   }
   tr.passIfNoException();

   tr.test("select again");
   {
      SqlExecutableRef se = dbc->select(TABLE_TEST_1);
      dbc->execute(se);
      assertNoExceptionSet();

      DynamicObject expect;
      expect[0]["fooId"] = "2";
      expect[0]["fooString"] = "bar";
      expect[0]["fooFlag"] = false;
      expect[0]["fooInt32"] = 3;
      assertNamedDynoCmp("expected", expect, "got", se->result);
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runSqlite3RollbackTest(TestRunner& tr)
{
   tr.group("rollback");

   // create sqlite3 connection pool
   Sqlite3ConnectionPool* cp = new Sqlite3ConnectionPool("sqlite3::memory:", 1);
   ConnectionPoolRef pool(cp);
   assertNoExceptionSet();

   // create database client
   DatabaseClientRef dbc = new Sqlite3DatabaseClient();
   dbc->setDebugLogging(true);
   dbc->setReadConnectionPool(pool);
   dbc->setWriteConnectionPool(pool);

   tr.test("initialize");
   {
      dbc->initialize();
   }
   tr.passIfNoException();

   Connection* c = cp->getConnection();

   tr.test("define table");
   {
      SchemaObject schema;
      schema["table"] = TABLE_TEST_1;

      DatabaseClient::addSchemaColumn(schema,
         "foo_id", "INTEGER PRIMARY KEY", "fooId", UInt64);
      DatabaseClient::addSchemaColumn(schema,
         "foo_string", "TEXT", "fooString", String);
      DatabaseClient::addSchemaColumn(schema,
         "foo_flag", "INTEGER", "fooFlag", Boolean);
      DatabaseClient::addSchemaColumn(schema,
         "foo_int32", "INTEGER", "fooInt32", Int32);

      dbc->define(schema);
   }
   tr.passIfNoException();

   tr.test("create table");
   {
      dbc->create(TABLE_TEST_1, false, c);
   }
   tr.passIfNoException();

   tr.test("begin");
   {
      dbc->begin(c);
   }
   tr.passIfNoException();

   tr.test("insert");
   {
      DynamicObject row;
      row["fooString"] = "foobar";
      row["fooFlag"] = true;
      row["fooInt32"] = 3;
      SqlExecutableRef se = dbc->insert(TABLE_TEST_1, row);
      dbc->execute(se, c);
      assertNoExceptionSet();
      row["fooId"] = se->lastInsertRowId;

      DynamicObject expect;
      expect["fooId"] = 1;
      expect["fooString"] = "foobar";
      expect["fooFlag"] = true;
      expect["fooInt32"] = 3;
      assertNamedDynoCmp("expected", expect, "got", row);
   }
   tr.passIfNoException();

   tr.test("insert again");
   {
      DynamicObject row;
      row["fooString"] = "foobar";
      row["fooFlag"] = false;
      row["fooInt32"] = 3;
      SqlExecutableRef se = dbc->insert(TABLE_TEST_1, row);
      dbc->execute(se, c);
      assertNoExceptionSet();
      row["fooId"] = se->lastInsertRowId;

      DynamicObject expect;
      expect["fooId"] = 2;
      expect["fooString"] = "foobar";
      expect["fooFlag"] = false;
      expect["fooInt32"] = 3;
      assertNamedDynoCmp("expected", expect, "got", row);
   }
   tr.passIfNoException();

   tr.test("select bogus");
   {
      DynamicObject where;
      where["fooId"] = 1;
      SqlExecutableRef se = dbc->selectOne(TABLE_TEST_1, &where);
      se->sql.append("BADSQLBLAHBLAH");
      dbc->execute(se, c);
   }
   tr.passIfException();

   tr.test("rollback");
   {
      dbc->end(c, false);
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
      monarch::sql::Connection* c = pool->getConnection();
      executeSqlite3Statements(NULL, c);
      //executeSqlite3Statements(*tr, c);
      c->close();
   }
};

static void runSqlite3ConnectionPoolTest(TestRunner& tr)
{
   tr.group("Sqlite3 ConnectionPool");

   // create sqlite3 connection pool
   Sqlite3ConnectionPool cp("sqlite3:///tmp/sqlite3cptest.db", 1);
   assertNoExceptionSet();

   // create table
   monarch::sql::Connection* c = cp.getConnection();
   createSqlite3Table(NULL, c);
   c->close();

   // create connection test threads
   int testCount = 200;
   Sqlite3ConnectionPoolTest tests[testCount];
   Thread* threads[testCount];

   // create threads, set pool for tests
   for(int i = 0; i < testCount; ++i)
   {
      tests[i].pool = &cp;
      tests[i].tr = &tr;
      threads[i] = new Thread(&tests[i]);
   }

   uint64_t startTime = Timer::startTiming();

   // run connection threads
   for(int i = 0; i < testCount; ++i)
   {
      while(!threads[i]->start(131072))
      {
         threads[i - 1]->join();
      }
   }

   // join threads
   for(int i = 0; i < testCount; ++i)
   {
      threads[i]->join();
   }

   double seconds = Timer::getSeconds(startTime);

   // clean up threads
   for(int i = 0; i < testCount; ++i)
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

static void runSqlite3StatementBuilderTest(TestRunner& tr)
{
   tr.group("Sqlite3 StatementBuilder");

   /* ObjRelMap: {} of
    *    "objectType": object-type
    *    "members": {} of
    *       "member-name": {} of
    *          "group": "columns" or "fkeys" (the group the mapping is for)
    *          "table": database table name
    *          "column": database column name
    *          "memberType": object member type
    *          "columnType": database column type
    *          "ftable": if group="fkeys", foreign key database table
    *          "fkey": if group="fkeys", foreign key database key column
    *          "fcolumn": if group="fkeys", foreign key database value column
    */

   // create sqlite3 connection pool
   Sqlite3ConnectionPool* cp = new Sqlite3ConnectionPool("sqlite3::memory:", 1);
   ConnectionPoolRef pool(cp);
   assertNoExceptionSet();

   // create database client
   DatabaseClientRef dbc = new Sqlite3DatabaseClient();
   dbc->setDebugLogging(true);
   dbc->setReadConnectionPool(pool);
   dbc->setWriteConnectionPool(pool);
   dbc->initialize();
   assertNoExceptionSet();

   // define Test object type
   tr.test("set Test OR map");
   {
      ObjRelMap orMap;
      orMap["objectType"] = "Test";

      // define auto-increment members
      DynamicObject& autoIncrement = orMap["autoIncrement"];
      autoIncrement[TABLE_TEST_1] = "id";

      // define the object's members
      DynamicObject& members = orMap["members"];

      // id column
      {
         DynamicObject& entry = members["id"];
         entry["group"] = "columns";
         entry["table"] = TABLE_TEST_1;
         entry["column"] = "id";
         entry["columnType"]->setType(UInt64);
         entry["memberType"]->setType(String);
      }

      // t column
      {
         DynamicObject& entry = members["description"];
         entry["group"] = "columns";
         entry["table"] = TABLE_TEST_1;
         entry["column"] = "t";
         entry["columnType"]->setType(String);
         entry["memberType"]->setType(String);
      }

      // i column
      {
         DynamicObject& entry = members["number"];
         entry["group"] = "columns";
         entry["table"] = TABLE_TEST_1;
         entry["column"] = "i";
         entry["columnType"]->setType(UInt32);
         entry["memberType"]->setType(UInt32);
      }

      // type w/foreign key
      {
         DynamicObject& entry = members["type"];
         entry["group"] = "fkeys";
         entry["table"] = TABLE_TEST_1;
         entry["column"] = "type";
         entry["ftable"] = TABLE_TEST_2;
         entry["fkey"] = "type_id";
         entry["fcolumn"] = "type_value";
         entry["columnType"]->setType(String);
         entry["memberType"]->setType(String);
      }

      // lower case text column
      {
         DynamicObject& entry = members["lower"];
         entry["group"] = "columns";
         entry["table"] = TABLE_TEST_1;
         entry["column"] = "lowertext";
         entry["encode"]->append() = "LOWER";
         entry["columnType"]->setType(String);
         entry["memberType"]->setType(String);
      }

      dbc->setObjRelMap(orMap);
   }
   tr.passIfNoException();

   // FIXME: define a complex object type with hierarchy and foreign keys
   // FIXME: get the actual data out

   monarch::sql::Connection* c = dbc->getWriteConnection();

   // initialize database for testing
   tr.test("initialize database");
   {
      monarch::sql::Statement* s;

      s = c->prepare(
         "CREATE TABLE IF NOT EXISTS " TABLE_TEST_1
         " (id INTEGER PRIMARY KEY, t TEXT, i INTEGER UNSIGNED,"
         "type BIGINT UNSIGNED, lowertext TEXT)");
      assert(s != NULL);
      s->execute();
      assertNoExceptionSet();

      s = c->prepare(
         "CREATE TABLE IF NOT EXISTS " TABLE_TEST_2
         " (type_id INTEGER UNSIGNED PRIMARY KEY, type_value TEXT)");
      assert(s != NULL);
      s->execute();
      assertNoExceptionSet();

      s = c->prepare(
         "INSERT INTO " TABLE_TEST_2 " (type_id,type_value) "
         "VALUES (1,'type1')");
      assert(s != NULL);
      s->execute();
      assertNoExceptionSet();

      s = c->prepare(
         "INSERT INTO " TABLE_TEST_2 " (type_id,type_value) "
         "VALUES (2,'type2')");
      assert(s != NULL);
      s->execute();
      assertNoExceptionSet();
   }
   tr.passIfNoException();

   tr.test("add Test object");
   {
      DynamicObject testObj;
      testObj["description"] = "My test object description";
      testObj["number"] = 10;
      testObj["type"] = "type1";
      testObj["lower"] = "LOWERCASE";

      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->add("Test", testObj)->execute(c);
      assertNoExceptionSet();

      DynamicObject expect;
      expect["ids"]["id"] = "1";
      expect["changed"] = (uint64_t)1;
      expect["tables"][TABLE_TEST_1]["changed"] = (uint64_t)1;

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   tr.test("get");
   {
      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->get("Test")->limit(1)->execute(c);
      assertNoExceptionSet();

      DynamicObject result;
      result["id"] = "1";
      result["description"] = "My test object description";
      result["number"] = 10;
      result["type"] = "type1";
      result["lower"] = "lowercase";

      DynamicObject expect;
      expect->append(result);

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   tr.test("update Test object");
   {
      DynamicObject testObj;
      testObj["id"] = "1";
      testObj["description"] = "A different test object description";
      testObj["number"] = 12;
      testObj["type"] = "type2";

      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->update("Test", testObj)->execute(c);
      assertNoExceptionSet();

      DynamicObject expect;
      expect["changed"] = (uint64_t)1;
      expect["tables"][TABLE_TEST_1]["changed"] = (uint64_t)1;

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   tr.test("get");
   {
      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->get("Test")->limit(1)->execute(c);
      assertNoExceptionSet();

      DynamicObject result;
      result["id"] = "1";
      result["description"] = "A different test object description";
      result["number"] = 12;
      result["type"] = "type2";
      result["lower"] = "lowercase";

      DynamicObject expect;
      expect->append(result);

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   tr.test("update Test object w/id 123");
   {
      DynamicObject testObj;
      testObj["description"] = "Yet another test object description";
      testObj["type"] = "type2";

      DynamicObject where;
      where["id"] = "1";

      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->update("Test", testObj)->where("Test", where)->execute(c);
      assertNoExceptionSet();

      DynamicObject expect;
      expect["changed"] = (uint64_t)1;
      expect["tables"][TABLE_TEST_1]["changed"] = (uint64_t)1;

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   tr.test("get");
   {
      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->get("Test")->limit(1)->execute(c);
      assertNoExceptionSet();

      DynamicObject result;
      result["id"] = "1";
      result["description"] = "Yet another test object description";
      result["number"] = 12;
      result["type"] = "type2";
      result["lower"] = "lowercase";

      DynamicObject expect;
      expect->append(result);

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   tr.test("update Test object where number > 12");
   {
      DynamicObject testObj;
      testObj["id"] = "1";
      testObj["description"] = "This should not show up";
      testObj["number"] = 12;
      testObj["type"] = "type2";

      DynamicObject where;
      where["number"] = "12";

      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->update("Test", testObj)->where("Test", where, ">")->execute(c);
      assertNoExceptionSet();

      DynamicObject expect;
      expect["changed"] = (uint64_t)0;
      expect["tables"][TABLE_TEST_1]["changed"] = (uint64_t)0;

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   tr.test("get");
   {
      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->get("Test")->limit(1)->execute(c);
      assertNoExceptionSet();

      DynamicObject result;
      result["id"] = "1";
      result["description"] = "Yet another test object description";
      result["number"] = 12;
      result["type"] = "type2";
      result["lower"] = "lowercase";

      DynamicObject expect;
      expect->append(result);

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   tr.test("update Test object add number +1 where number >= 12");
   {
      DynamicObject update;
      update["number"] = 1;

      DynamicObject where;
      where["number"] = "12";

      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->update("Test", update, "+=")->where("Test", where, ">=")->execute(c);
      assertNoExceptionSet();

      DynamicObject expect;
      expect["changed"] = (uint64_t)1;
      expect["tables"][TABLE_TEST_1]["changed"] = (uint64_t)1;

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   tr.test("get");
   {
      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->get("Test")->limit(1)->execute(c);
      assertNoExceptionSet();

      DynamicObject result;
      result["id"] = "1";
      result["description"] = "Yet another test object description";
      result["number"] = 13;
      result["type"] = "type2";
      result["lower"] = "lowercase";

      DynamicObject expect;
      expect->append(result);

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   // FIXME: test array params
   // FIXME: test LIKE
   // FIXME: test range
   // FIXME: enum
   // FIXME: test chaining updates (and w/diff operators)

   tr.test("get Test object IDs");
   {
      DynamicObject testObj;
      testObj["id"];

      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->get("Test", &testObj)->execute(c);
      assertNoExceptionSet();

      DynamicObject result;
      result["id"] = "1";

      DynamicObject expect;
      expect->append(result);

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   tr.test("get Test objects of type2");
   {
      DynamicObject testObj;
      testObj["type"] = "type2";

      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->get("Test")->where("Test", testObj, "=")->execute(c);
      assertNoExceptionSet();

      DynamicObject result;
      result["id"] = "1";
      result["description"] = "Yet another test object description";
      result["number"] = 13;
      result["type"] = "type2";
      result["lower"] = "lowercase";

      DynamicObject expect;
      expect->append(result);

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   tr.test("get Test objects with 'lower'='LOWERCASE'");
   {
      DynamicObject testObj;
      testObj["lower"] = "LOWERCASE";

      StatementBuilderRef sb = dbc->createStatementBuilder();
      sb->get("Test")->where("Test", testObj, "=")->execute(c);
      assertNoExceptionSet();

      DynamicObject result;
      result["id"] = "1";
      result["description"] = "Yet another test object description";
      result["number"] = 13;
      result["type"] = "type2";
      result["lower"] = "lowercase";

      DynamicObject expect;
      expect->append(result);

      DynamicObject results = sb->fetch();
      assertDynoCmp(expect, results);
   }
   tr.passIfNoException();

   c->close();

   tr.ungroup();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runSqlite3ConnectionTest(tr);
      runSqlite3PrepareManyTest(tr);
      runSqlite3StatementTest(tr);
      runSqlite3TableTest(tr);
      runSqlite3TableMigrationTest(tr);
      runSqlite3ThreadTest(tr);
      runSqlite3ReuseTest(tr);
      runSqlite3DatabaseClientTest(tr);
      runSqlite3RollbackTest(tr);
   }
   if(tr.isTestEnabled("sqlite3-connection-pool"))
   {
      runSqlite3ConnectionPoolTest(tr);
   }
   if(tr.isTestEnabled("sqlite3-statement-builder"))
   {
      runSqlite3StatementBuilderTest(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.sqlite3.test", "1.0", mo_test_sqlite3::run)
