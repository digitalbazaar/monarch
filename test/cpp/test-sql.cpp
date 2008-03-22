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
#include "db/sql/mysql/MySqlConnection.h"
#include "db/sql/mysql/MySqlConnectionPool.h"

using namespace std;
using namespace db::test;
using namespace db::rt;
using namespace db::sql;
using namespace db::sql::sqlite3;
using namespace db::sql::mysql;

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
   
   db::sql::Statement* s;
   
   tr.test("drop table");
   s = c.prepare("DROP TABLE IF EXISTS test");
   assert(s != NULL);
   s->execute();
   tr.passIfNoException();
   
   tr.test("create table");
   s = c.prepare("CREATE TABLE IF NOT EXISTS test (t TEXT, i INT)");
   s->execute();
   tr.passIfNoException();
   
   tr.test("insert test 1");
   s = c.prepare("INSERT INTO test (t, i) VALUES ('test!', 1234)");
   s->execute();
   assert(s->getLastInsertRowId() == 1);
   tr.passIfNoException();
   
   tr.test("insert test 2");
   s = c.prepare("INSERT INTO test (t, i) VALUES ('!tset', 4321)");
   s->execute();
   assert(s->getLastInsertRowId() == 2);
   tr.passIfNoException();
   
   tr.test("insert positional parameters");
   s = c.prepare("INSERT INTO test (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setInt32(2, 2222);
   s->execute();
   assert(s->getLastInsertRowId() == 3);
   tr.passIfNoException();
   
   // insert named parameters test
   tr.test("insert named parameters");
   s = c.prepare("INSERT INTO test (t, i) VALUES (:first, :second)");
   s->setText(":first", "boundnamed");
   s->setInt32(":second", 2223);
   s->execute();
   assert(s->getLastInsertRowId() == 4);
   tr.passIfNoException();
   
   // select test
   s = c.prepare("SELECT * FROM test");
   s->execute();
   
   // fetch rows
   tr.test("fetch rows");
   db::sql::Row* row;
   string t;
   int i;

   row = s->fetch();
   assert(row != NULL);
   row->getText("t", t);
   assertNoException();
   row->getInt32("i", i);
   assertNoException();
   assert(t == "test!");
   assert(i == 1234);

   row = s->fetch();
   assert(row != NULL);
   row->getText("t", t);
   assertNoException();
   row->getInt32("i", i);
   assertNoException();
   assert(t == "!tset");
   assert(i == 4321);

   row = s->fetch();
   assert(row != NULL);
   row->getText("t", t);
   assertNoException();
   row->getInt32("i", i);
   assertNoException();
   assert(t == "boundpositional");
   assert(i == 2222);

   row = s->fetch();
   assert(row != NULL);
   row->getText("t", t);
   assertNoException();
   row->getInt32("i", i);
   assertNoException();
   assert(t == "boundnamed");
   assert(i == 2223);

   // done so next should be NULL
   row = s->fetch();
   assert(row == NULL);
   
   tr.pass();
   
   tr.test("connection close");
   c.close();
   tr.passIfNoException();
   
   tr.ungroup();
}

void runMySqlConnectionTest()
{
   cout << "Starting MySqlConnection test." << endl << endl;
   
   MySqlConnection c;
   c.connect("mysql://dbreadclient:k288m2s8f6gk39a@mojo.bitmunk.com/test");
   assertNoException();
   
   // clean up mysql
   mysql_library_end();
   
   cout << endl << "MySqlConnection test complete." << endl;
}

void runMySqlStatementTest(TestRunner& tr)
{
   //cout << "Starting MySql test." << endl << endl;
   
   // clear any exceptions
   Exception::clearLast();
   
   MySqlConnection c;
   c.connect("mysql://dbwriteclient:k288m2s8f6gk39a@mojo.bitmunk.com");
   assertNoException();
   
   db::sql::Statement* s;
   
   // drop table test
   s = c.prepare("DROP TABLE IF EXISTS test.dbmysqltest");
   assert(s != NULL);
   s->execute();
   assertNoException();
   //cout << "drop table test passed!" << endl;
   
   // create table test
   s = c.prepare(
      "CREATE TABLE IF NOT EXISTS test.dbmysqltest "
      "(id BIGINT AUTO_INCREMENT, t TEXT, i BIGINT, "
      "PRIMARY KEY (id))");
   s->execute();
   assertNoException();
   //cout << "create table test passed!" << endl;
   
   // insert test 1
   s = c.prepare("INSERT INTO test.dbmysqltest (t, i) VALUES ('test!', 1234)");
   s->execute();
   assert(s->getLastInsertRowId() == 1);
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   //cout << "insert test 1 passed!" << endl;
   
   // insert test 2
   s = c.prepare("INSERT INTO test.dbmysqltest (t, i) VALUES ('!tset', 4321)");
   s->execute();
   assert(s->getLastInsertRowId() == 2);
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   //cout << "insert test 2 passed!" << endl;
   
   // insert positional parameters test
   unsigned long long start = System::getCurrentMilliseconds();
   for(int i = 0; i < 20; i++)
   {
      s = c.prepare("INSERT INTO test.dbmysqltest (t, i) VALUES (?, ?)");
      s->setText(1, "boundpositional");
      s->setInt32(2, 2220 + i);
      s->execute();
      assert(s->getLastInsertRowId() == (unsigned int)(3 + i));
      //cout << "Row #: " << s->getLastInsertRowId() << endl;
      assertNoException();
   }
   unsigned long long end = System::getCurrentMilliseconds();
   //cout << "insert positional parameters test passed!" << endl;
   cout << "TIME=" << (end - start) << " ms" << std::endl;
   
//   // insert named parameters test
//   s = c.prepare("INSERT INTO test.dbmysqltest (t, i) VALUES (:first, :second)");
//   s->setText(":first", "boundnamed");
//   s->setInt32(":second", 2223);
//   s->execute();
//   cout << "Row #: " << s->getLastInsertRowId() << endl;
//   assertNoException();
//   cout << "insert named parameters test passed!" << endl;
   
   // select test
   s = c.prepare("SELECT t, i FROM test.dbmysqltest");
   assertNoException();
   s->execute();
   assertNoException();
   
   // fetch rows
   db::sql::Row* row;
   string t;
   int i;
   while((row = s->fetch()) != NULL)
   {
      //cout << endl << "Row result:" << endl;
      row->getText("t", t);
      assertNoException();
      row->getInt32("i", i);
      assertNoException();
      
      //cout << "t=" << t << endl;
      //cout << "i=" << i << endl;
   }
   
   // select test AGAIN, to test mysql command ordering
   s = c.prepare("SELECT t, i FROM test.dbmysqltest");
   assertNoException();
   s->execute();
   assertNoException();
   
   // fetch rows
   while((row = s->fetch()) != NULL)
   {
      //cout << endl << "Row result:" << endl;
      row->getText("t", t);
      assertNoException();
      row->getInt32("i", i);
      assertNoException();
      
      //cout << "t=" << t << endl;
      //cout << "i=" << i << endl;
   }
   
   //cout << endl << "Result Rows complete." << endl;
   //cout << "select test passed!" << endl;
   
   c.close();
   assertNoException();
   
   // clean up mysql
   mysql_library_end();
   
   //cout << endl << "MySql test complete." << endl;
}

void executeStatements(db::sql::Connection* c)
{
   db::sql::Statement* s;
   
   // drop table test
   s = c->prepare("DROP TABLE IF EXISTS test");
   assert(s != NULL);
   s->execute();
   assertNoException();
   //cout << "drop table test passed!" << endl;
   
   // create table test
   s = c->prepare("CREATE TABLE IF NOT EXISTS test (t TEXT, i INT)");
   s->execute();
   assertNoException();
   //cout << "create table test passed!" << endl;
   
   // insert test 1
   s = c->prepare("INSERT INTO test (t, i) VALUES ('test!', 1234)");
   s->execute();
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   //cout << "insert test 1 passed!" << endl;
   
   // insert test 2
   s = c->prepare("INSERT INTO test (t, i) VALUES ('!tset', 4321)");
   s->execute();
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   //cout << "insert test 2 passed!" << endl;
   
   // insert positional parameters test
   s = c->prepare("INSERT INTO test (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setInt32(2, 2222);
   s->execute();
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   //cout << "insert positional parameters test passed!" << endl;
   
   // insert named parameters test
   s = c->prepare("INSERT INTO test (t, i) VALUES (:first, :second)");
   s->setText(":first", "boundnamed");
   s->setInt32(":second", 2223);
   s->execute();
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   //cout << "insert named parameters test passed!" << endl;
   
   // select test
   s = c->prepare("SELECT * FROM test");
   s->execute();
   
   // fetch rows
   db::sql::Row* row;
   string t;
   int i;
   while((row = s->fetch()) != NULL)
   {
      //cout << endl << "Row result:" << endl;
      row->getText("t", t);
      assertNoException();
      row->getInt32("i", i);
      assertNoException();
      
      //cout << "t=" << t << endl;
      //cout << "i=" << i << endl;
   }
   
   //cout << endl << "Result Rows complete." << endl;
   //cout << "select test passed!" << endl;
   
   Thread::sleep(100);
   
   c->close();
   
   //cout << "Statements finished executing." << endl;
}

class SqlConnectionTest : public Runnable
{
public:
   Sqlite3ConnectionPool* pool;
   
   virtual void run()
   {
      db::sql::Connection* c = pool->getConnection();
      executeStatements(c);
   }
};

void runConnectionPoolTest()
{
   cout << "Starting ConnectionPool test." << endl << endl;
   
   int size = 300;
   
   // create sqlite3 connection pool
   Sqlite3ConnectionPool cp("sqlite3::memory:", 100);
   assertNoException();
   
   // create connection test threads
   SqlConnectionTest tests[size];
   Thread* threads[size];
   
   // create threads, set pool for tests
   for(int i = 0; i < size; i++)
   {
      tests[i].pool = &cp;
      threads[i] = new Thread(&tests[i]);
   }
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   // run connection threads
   int count = 1;
   for(int i = 0; i < size; i++, count++)
   {
      //cout << "RUNNING CONNECTION #" << count << endl;
      while(!threads[i]->start(131072))
      {
         threads[i - 1]->join();
      }
   }
   
   // join threads
   for(int i = 0; i < size; i++)
   {
      threads[i]->join();
   }
   
   unsigned long long end = System::getCurrentMilliseconds();
   
   // clean up threads
   for(int i = 0; i < size; i++)
   {
      delete threads[i];
   }
   
   cout << endl;
   cout << "Number of independent connection uses: " << size << endl;
   cout << "Number of pooled connections created: " << cp.getConnectionCount()
      << endl;
   
   cout << "Total time: " << (end - start) << "ms" << endl;
   
   cout << endl << "ConnectionPool test complete." << endl;
}

void runConnectionPoolTest2()
{
   cout << "Starting ConnectionPool test 2." << endl << endl;
   
   // clear any exceptions
   Exception::clearLast();
   
   // get a sqlite3 connection pool
   ConnectionPool* cp = new Sqlite3ConnectionPool("sqlite3::memory:");
   assertNoException();
   
   // get a connection
   db::sql::Connection* c = cp->getConnection();
   assertNoException();
   
   // drop table test
   db::sql::Statement* s = c->prepare("DROP TABLE IF EXISTS test");
   assert(s != NULL);
   s->execute();
   assertNoException();
   cout << "sqlite3 drop table test passed!" << endl;
   
   // create table test
   s = c->prepare("CREATE TABLE IF NOT EXISTS test (t TEXT, i INT)");
   s->execute();
   assertNoException();
   cout << "sqlite3 create table test passed!" << endl;
   
   // insert test 1
   s = c->prepare("INSERT INTO test (t, i) VALUES ('test!', 1234)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   cout << "sqlite3 insert test 1 passed!" << endl;
   
   // insert test 2
   s = c->prepare("INSERT INTO test (t, i) VALUES ('!tset', 4321)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   cout << "sqlite3 insert test 2 passed!" << endl;
   
   // insert positional parameters test
   s = c->prepare("INSERT INTO test (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setUInt32(2, 2222);
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   cout << "sqlite3 insert positional parameters test passed!" << endl;
   
   // insert named parameters test
   s = c->prepare("INSERT INTO test (t, i) VALUES (:first, :second)");
   s->setText(":first", "boundnamed");
   s->setInt32(":second", 2223);
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   cout << "sqlite3 insert named parameters test passed!" << endl;
   
   // select test
   s = c->prepare("SELECT * FROM test");
   s->execute();
   
   // fetch rows
   db::sql::Row* row;
   string t;
   int i;
   while((row = s->fetch()) != NULL)
   {
      cout << endl << "Row result:" << endl;
      row->getText((unsigned int)0, t);
      assertNoException();
      row->getInt32(1, i);
      assertNoException();
      
      cout << "t=" << t << endl;
      cout << "i=" << i << endl;
   }
   
   cout << endl << "Result Rows complete." << endl;
   cout << "sqlite3 select test passed!" << endl;
   
   c->close();
   assertNoException();
   
   // clean up connection pool
   delete cp;
   
   // get a mysql connection pool
   cp = new MySqlConnectionPool(
      "mysql://dbwriteclient:k288m2s8f6gk39a@mojo.bitmunk.com/test");
   assertNoException();
   
   // get a connection
   c = cp->getConnection();
   assertNoException();
   
   // drop table test
   s = c->prepare("DROP TABLE IF EXISTS dbmysqltest");
   assert(s != NULL);
   s->execute();
   assertNoException();
   cout << "mysql drop table test passed!" << endl;
   
   // create table test
   string sql;
   sql.append("CREATE TABLE IF NOT EXISTS dbmysqltest ");
   sql.append("(id BIGINT AUTO_INCREMENT, t TEXT, i BIGINT, ");
   sql.append("PRIMARY KEY (id))");
   s = c->prepare(sql.c_str());
   s->execute();
   assertNoException();
   cout << "mysql create table test passed!" << endl;
   
   // insert test 1
   s = c->prepare("INSERT INTO dbmysqltest (t, i) VALUES ('test!', 1234)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   cout << "mysql insert test 1 passed!" << endl;
   
   // insert test 2
   s = c->prepare("INSERT INTO dbmysqltest (t, i) VALUES ('!tset', 4321)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   cout << "mysql insert test 2 passed!" << endl;
   
   // insert positional parameters test
   s = c->prepare("INSERT INTO dbmysqltest (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setUInt32(2, 2222);
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   assertNoException();
   cout << "mysql insert positional parameters test passed!" << endl;
   
//   // insert named parameters test
//   s = c->prepare("INSERT INTO dbmysqltest (t, i) VALUES (:first, :second)");
//   s->setText(":first", "boundnamed");
//   s->setInt32(":second", 2223);
//   s->execute();
//   cout << "Row #: " << s->getLastInsertRowId() << endl;
//   assertNoException();
//   cout << "mysql insert named parameters test passed!" << endl;
   
   // select test
   s = c->prepare("SELECT * FROM dbmysqltest");
   s->execute();
   
   // fetch rows
   while((row = s->fetch()) != NULL)
   {
      cout << endl << "Row result:" << endl;
      row->getText("t", t);
      assertNoException();
      row->getInt32("i", i);
      assertNoException();
      
      cout << "t=" << t << endl;
      cout << "i=" << i << endl;
   }
   
   cout << endl << "Result Rows complete." << endl;
   cout << "mysql select test passed!" << endl;
   
   c->close();
   assertNoException();
   
   // clean up connection pool
   delete cp;
   
   // clean up mysql
   mysql_library_end();
   
   cout << endl << "DatabaseClient test complete." << endl;
}

class DbSqlTester : public db::test::Tester
{
public:
   DbSqlTester()
   {
      setName("sql");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runSqlite3ConnectionTest(tr);
      runSqlite3StatementTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
//      runMySqlConnectionTest();
//      runMySqlStatementTest(tr);
//      runConnectionPoolTest();
//      runDatabaseClientTest();
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbSqlTester)
#endif
