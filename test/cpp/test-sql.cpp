/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/data/DataMappingFunctor.h"
#include "db/rt/Thread.h"
#include "db/sql/Row.h"
#include "db/sql/RowObject.h"
#include "db/sql/sqlite3/Sqlite3Connection.h"
#include "db/sql/sqlite3/Sqlite3ConnectionPool.h"
#include "db/sql/mysql/MySqlConnection.h"
#include "db/sql/mysql/MySqlConnectionPool.h"
#include "db/sql/util/DatabaseClient.h"

using namespace std;
using namespace db::test;
using namespace db::data;
using namespace db::io;
using namespace db::net;
using namespace db::rt;
using namespace db::util;
using namespace db::sql::sqlite3;
using namespace db::sql::mysql;
using namespace db::sql::util;

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
   delete s;
   tr.passIfNoException();
   
   tr.test("create table");
   s = c.prepare("CREATE TABLE IF NOT EXISTS test (t TEXT, i INT)");
   s->execute();
   delete s;
   tr.passIfNoException();
   
   tr.test("insert test 1");
   s = c.prepare("INSERT INTO test (t, i) VALUES ('test!', 1234)");
   s->execute();
   assert(s->getLastInsertRowId() == 1);
   delete s;
   tr.passIfNoException();
   
   tr.test("insert test 2");
   s = c.prepare("INSERT INTO test (t, i) VALUES ('!tset', 4321)");
   s->execute();
   assert(s->getLastInsertRowId() == 2);
   delete s;
   tr.passIfNoException();
   
   tr.test("insert positional parameters");
   s = c.prepare("INSERT INTO test (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setInt32(2, 2222);
   s->execute();
   assert(s->getLastInsertRowId() == 3);
   delete s;
   tr.passIfNoException();
   
   // insert named parameters test
   tr.test("insert named parameters");
   s = c.prepare("INSERT INTO test (t, i) VALUES (:first, :second)");
   s->setText(":first", "boundnamed");
   s->setInt32(":second", 2223);
   s->execute();
   assert(s->getLastInsertRowId() == 4);
   delete s;
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
   
   delete s;

   tr.pass();
   
   tr.test("connection close");
   c.close();
   tr.passIfNoException();
   
   tr.ungroup();
}

class TestRowObject
{
protected:
   char* mText;
   bool mBoolean;
   int mInt32;
   unsigned int mUInt32;
   
public:
   TestRowObject()
   {
      mText = strdup("");
      mBoolean = false;
      mInt32 = 1;
      mUInt32 = 2;
   }
   
   virtual ~TestRowObject()
   {
      free(mText);
   }
   
   virtual void setText(const char* t)
   {
      free(mText);
      mText = strdup(t);
   }
   
   virtual const char* getText()
   {
      return mText;
   }
   
   virtual void setBoolean(bool b)
   {
      mBoolean = b;
   }
   
   virtual bool getBoolean()
   {
      return mBoolean;
   }
   
   virtual void setInt32(int i)
   {
      mInt32 = i;
   }
   
   virtual int getInt32()
   {
      return mInt32;
   }
   
   virtual void setUInt32(unsigned int i)
   {
      mUInt32 = i;
   }
   
   virtual unsigned int getUInt32() const
   {
      return mUInt32;
   }
};

class TestRowObjectBinding : public DataBinding
{
protected:
   DataMappingFunctor<TestRowObject> mTextMapping;
   DataMappingFunctor<TestRowObject> mBooleanMapping;
   DataMappingFunctor<TestRowObject> mInt32Mapping;
   DataMappingFunctor<TestRowObject> mUInt32Mapping;
   
public:
   TestRowObjectBinding(TestRowObject* ro) :
      mTextMapping(&TestRowObject::setText, &TestRowObject::getText),
      mBooleanMapping(&TestRowObject::setBoolean, &TestRowObject::getBoolean),
      mInt32Mapping(&TestRowObject::setInt32, &TestRowObject::getInt32),
      mUInt32Mapping(&TestRowObject::setUInt32, &TestRowObject::getUInt32)
   {
      setObject(ro);
      
      addDataMapping(NULL, "t", true, true, &mTextMapping);
      addDataMapping(NULL, "b", true, true, &mBooleanMapping);
      addDataMapping(NULL, "i32", true, true, &mInt32Mapping);
      addDataMapping(NULL, "ui32", true, true, &mUInt32Mapping);
   }
   
   virtual ~TestRowObjectBinding()
   {
   }
};

void runSqlite3RowObjectTest(TestRunner& tr)
{
   tr.group("Sqlite3 RowObject");
   
   // clear any exceptions
   Exception::clearLast();
   
   Sqlite3Connection c;
   c.connect("sqlite3::memory:");
   
   db::sql::Statement* s;
   
   // drop existing table
   tr.test("drop table");
   s = c.prepare("DROP TABLE IF EXISTS test");
   assert(s != NULL);
   s->execute();
   delete s;
   tr.passIfNoException();
   
   // create table
   tr.test("create table");
   s = c.prepare(
      "CREATE TABLE IF NOT EXISTS test "
      "(t TEXT, b INT, i32 INT, ui32 INT)");
   s->execute();
   delete s;
   tr.passIfNoException();
   
   // insert a row object
   TestRowObject tro1;
   tro1.setText("This is some text.");
   tro1.setBoolean(false);
   tro1.setInt32(5);
   tro1.setUInt32(14);
   TestRowObjectBinding binding1(&tro1);
   db::sql::RowObject ro1(&binding1);
   
   tr.test("insert row object 1");
   ro1.insert(&c, "test");
   tr.passIfNoException();
   
   // insert another row object
   TestRowObject tro2;
   tro2.setText("The second row object.");
   tro2.setBoolean(false);
   tro2.setInt32(-1);
   tro2.setUInt32(17);
   TestRowObjectBinding binding2(&tro2);
   db::sql::RowObject ro2(&binding2);
   
   tr.test("insert row object 2");
   ro2.insert(&c, "test");
   tr.passIfNoException();
   
   // update row object 1
   TestRowObject tro3;
   tro3.setText("The first row object.");
   tro3.setBoolean(true);
   tro3.setInt32(5);
   tro3.setUInt32(14);
   TestRowObjectBinding binding3(&tro3);
   db::sql::RowObject ro3(&binding3);
   
   tr.test("update row object 1");
   ro3.update(&c, "test", "i32");
   tr.passIfNoException();
   
   // select row object 1 using binding 2
   db::sql::RowObject ro4(&binding2);
   tr.test("select row object 1");
   tro2.setBoolean(true);
   ro4.fetch(&c, "test", "b");
   
   // assert contents
   assertStrCmp(tro2.getText(), "The first row object.");
   assert(tro2.getBoolean());
   assert(tro2.getInt32() == 5);
   assert(tro2.getUInt32() == 14);
   tr.passIfNoException();
   
   // select row object 2 using binding 3
   db::sql::RowObject ro5(&binding3);
   tro3.setBoolean(false);
   tr.test("select row object 2");
   ro5.fetch(&c, "test", "b");
   
   // assert contents
   assertStrCmp(tro3.getText(), "The second row object.");
   assert(!tro3.getBoolean());
   assert(tro3.getInt32() == -1);
   assert(tro3.getUInt32() == 17);
   tr.passIfNoException();
   
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
   cout << "Starting MySql test." << endl << endl;
   
   // clear any exceptions
   Exception::clearLast();
   
   MySqlConnection c;
   c.connect("mysql://dbwriteclient:k288m2s8f6gk39a@mojo.bitmunk.com/test");
   assertNoException();
   
   db::sql::Statement* s;
   
   // drop table test
   s = c.prepare("DROP TABLE IF EXISTS dbmysqltest");
   assert(s != NULL);
   s->execute();
   delete s;
   assertNoException();
   cout << "drop table test passed!" << endl;
   
   // create table test
   s = c.prepare(
      "CREATE TABLE IF NOT EXISTS dbmysqltest "
      "(id BIGINT AUTO_INCREMENT, t TEXT, i BIGINT, "
      "PRIMARY KEY (id))");
   s->execute();
   delete s;
   assertNoException();
   cout << "create table test passed!" << endl;
   
   // insert test 1
   s = c.prepare("INSERT INTO dbmysqltest (t, i) VALUES ('test!', 1234)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "insert test 1 passed!" << endl;
   
   // insert test 2
   s = c.prepare("INSERT INTO dbmysqltest (t, i) VALUES ('!tset', 4321)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "insert test 2 passed!" << endl;
   
   // insert positional parameters test
   s = c.prepare("INSERT INTO dbmysqltest (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setInt32(2, 2222);
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "insert positional parameters test passed!" << endl;
   
//   // insert named parameters test
//   s = c.prepare("INSERT INTO dbmysqltest (t, i) VALUES (:first, :second)");
//   s->setText(":first", "boundnamed");
//   s->setInt32(":second", 2223);
//   s->execute();
//   cout << "Row #: " << s->getLastInsertRowId() << endl;
//   delete s;
//   assertNoException();
//   cout << "insert named parameters test passed!" << endl;
   
   // select test
   s = c.prepare("SELECT t, i FROM dbmysqltest");
   s->execute();
   
   // fetch rows
   db::sql::Row* row;
   string t;
   int i;
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
   delete s;
   cout << "select test passed!" << endl;
   
   c.close();
   assertNoException();
   
   // clean up mysql
   mysql_library_end();
   
   cout << endl << "MySql test complete." << endl;
}

void runMySqlRowObjectTest(TestRunner& tr)
{
   tr.group("MySql RowObject");
   
   // clear any exceptions
   Exception::clearLast();
   
   MySqlConnection c;
   c.connect("mysql://dbwriteclient:k288m2s8f6gk39a@mojo.bitmunk.com/test");
   assertNoException();
   
   db::sql::Statement* s;
   
   // drop existing table
   tr.test("drop table");
   s = c.prepare("DROP TABLE IF EXISTS test");
   assert(s != NULL);
   s->execute();
   delete s;
   tr.passIfNoException();
   
   // create table
   tr.test("create table");
   s = c.prepare(
      "CREATE TABLE IF NOT EXISTS test "
      "(t TEXT, b INT, i32 INT, ui32 INT)");
   s->execute();
   delete s;
   tr.passIfNoException();
   
   // insert a row object
   TestRowObject tro1;
   tro1.setText("This is some text.");
   tro1.setBoolean(false);
   tro1.setInt32(5);
   tro1.setUInt32(14);
   TestRowObjectBinding binding1(&tro1);
   db::sql::RowObject ro1(&binding1);
   
   tr.test("insert row object 1");
   ro1.insert(&c, "test");
   tr.passIfNoException();
   
   // insert another row object
   TestRowObject tro2;
   tro2.setText("The second row object.");
   tro2.setBoolean(false);
   tro2.setInt32(-1);
   tro2.setUInt32(17);
   TestRowObjectBinding binding2(&tro2);
   db::sql::RowObject ro2(&binding2);
   
   tr.test("insert row object 2");
   ro2.insert(&c, "test");
   tr.passIfNoException();
   
   // update row object 1
   TestRowObject tro3;
   tro3.setText("The first row object.");
   tro3.setBoolean(true);
   tro3.setInt32(5);
   tro3.setUInt32(14);
   TestRowObjectBinding binding3(&tro3);
   db::sql::RowObject ro3(&binding3);
   
   tr.test("update row object 1");
   ro3.update(&c, "test", "i32");
   tr.passIfNoException();
   
   // select row object 1 using binding 2
   db::sql::RowObject ro4(&binding2);
   tr.test("select row object 1");
   tro2.setBoolean(true);
   ro4.fetch(&c, "test", "b");
   
   // assert contents
   assertStrCmp(tro2.getText(), "The first row object.");
   assert(tro2.getBoolean());
   assert(tro2.getInt32() == 5);
   assert(tro2.getUInt32() == 14);
   tr.passIfNoException();
   
   // select row object 2 using binding 3
   db::sql::RowObject ro5(&binding3);
   tro3.setBoolean(false);
   tr.test("select row object 2");
   ro5.fetch(&c, "test", "b");
   
   // assert contents
   assertStrCmp(tro3.getText(), "The second row object.");
   assert(!tro3.getBoolean());
   assert(tro3.getInt32() == -1);
   assert(tro3.getUInt32() == 17);
   tr.passIfNoException();
   
   tr.test("connection close");
   c.close();
   tr.passIfNoException();
   
   // clean up mysql
   mysql_library_end();
   
   tr.ungroup();
}

void executeStatements(db::sql::Connection* c)
{
   db::sql::Statement* s;
   
   // drop table test
   s = c->prepare("DROP TABLE IF EXISTS test");
   assert(s != NULL);
   s->execute();
   delete s;
   assertNoException();
   //cout << "drop table test passed!" << endl;
   
   // create table test
   s = c->prepare("CREATE TABLE IF NOT EXISTS test (t TEXT, i INT)");
   s->execute();
   delete s;
   assertNoException();
   //cout << "create table test passed!" << endl;
   
   // insert test 1
   s = c->prepare("INSERT INTO test (t, i) VALUES ('test!', 1234)");
   s->execute();
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   //cout << "insert test 1 passed!" << endl;
   
   // insert test 2
   s = c->prepare("INSERT INTO test (t, i) VALUES ('!tset', 4321)");
   s->execute();
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   //cout << "insert test 2 passed!" << endl;
   
   // insert positional parameters test
   s = c->prepare("INSERT INTO test (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setInt32(2, 2222);
   s->execute();
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   //cout << "insert positional parameters test passed!" << endl;
   
   // insert named parameters test
   s = c->prepare("INSERT INTO test (t, i) VALUES (:first, :second)");
   s->setText(":first", "boundnamed");
   s->setInt32(":second", 2223);
   s->execute();
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
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
   delete s;
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

void runDatabaseClientTest()
{
   cout << "Starting DatabaseClient test." << endl << endl;
   
   // clear any exceptions
   Exception::clearLast();
   
   // get a sqlite3 database client
   DatabaseClient* dc = DatabaseClient::create("sqlite3::memory:");
   assertNoException();
   
   // get a connection
   db::sql::Connection* c = dc->getConnection();
   assertNoException();
   
   // drop table test
   db::sql::Statement* s = c->prepare("DROP TABLE IF EXISTS test");
   assert(s != NULL);
   s->execute();
   delete s;
   assertNoException();
   cout << "sqlite3 drop table test passed!" << endl;
   
   // create table test
   s = c->prepare("CREATE TABLE IF NOT EXISTS test (t TEXT, i INT)");
   s->execute();
   delete s;
   assertNoException();
   cout << "sqlite3 create table test passed!" << endl;
   
   // insert test 1
   s = c->prepare("INSERT INTO test (t, i) VALUES ('test!', 1234)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "sqlite3 insert test 1 passed!" << endl;
   
   // insert test 2
   s = c->prepare("INSERT INTO test (t, i) VALUES ('!tset', 4321)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "sqlite3 insert test 2 passed!" << endl;
   
   // insert positional parameters test
   s = c->prepare("INSERT INTO test (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setUInt32(2, 2222);
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "sqlite3 insert positional parameters test passed!" << endl;
   
   // insert named parameters test
   s = c->prepare("INSERT INTO test (t, i) VALUES (:first, :second)");
   s->setText(":first", "boundnamed");
   s->setInt32(":second", 2223);
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
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
   delete s;
   cout << "sqlite3 select test passed!" << endl;
   
   c->close();
   assertNoException();
   
   // clean up database client
   delete dc;
   
   // get a mysql database client
   dc = DatabaseClient::create(
      "mysql://dbwriteclient:k288m2s8f6gk39a@mojo.bitmunk.com/test");
   assertNoException();
   
   // get a connection
   c = dc->getConnection();
   assertNoException();
   
   // drop table test
   s = c->prepare("DROP TABLE IF EXISTS dbmysqltest");
   assert(s != NULL);
   s->execute();
   delete s;
   assertNoException();
   cout << "mysql drop table test passed!" << endl;
   
   // create table test
   string sql;
   sql.append("CREATE TABLE IF NOT EXISTS dbmysqltest ");
   sql.append("(id BIGINT AUTO_INCREMENT, t TEXT, i BIGINT, ");
   sql.append("PRIMARY KEY (id))");
   s = c->prepare(sql.c_str());
   s->execute();
   delete s;
   assertNoException();
   cout << "mysql create table test passed!" << endl;
   
   // insert test 1
   s = c->prepare("INSERT INTO dbmysqltest (t, i) VALUES ('test!', 1234)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "mysql insert test 1 passed!" << endl;
   
   // insert test 2
   s = c->prepare("INSERT INTO dbmysqltest (t, i) VALUES ('!tset', 4321)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "mysql insert test 2 passed!" << endl;
   
   // insert positional parameters test
   s = c->prepare("INSERT INTO dbmysqltest (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setUInt32(2, 2222);
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "mysql insert positional parameters test passed!" << endl;
   
//   // insert named parameters test
//   s = c->prepare("INSERT INTO dbmysqltest (t, i) VALUES (:first, :second)");
//   s->setText(":first", "boundnamed");
//   s->setInt32(":second", 2223);
//   s->execute();
//   cout << "Row #: " << s->getLastInsertRowId() << endl;
//   delete s;
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
   delete s;
   cout << "mysql select test passed!" << endl;
   
   c->close();
   assertNoException();
   
   // clean up database client
   delete dc;
   
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
      runSqlite3RowObjectTest(tr);
      runMySqlRowObjectTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
//      runMySqlConnectionTest();
//      runMySqlStatementTest();
//      runMySqlRowObjectTest(tr);
//      runConnectionPoolTest();
//      runDatabaseClientTest();
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbSqlTester)
#endif
