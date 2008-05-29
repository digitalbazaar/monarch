/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <sstream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/io/OStreamOutputStream.h"
#include "db/logging/Logging.h"
#include "db/logging/FileLogger.h"
#include "db/logging/OutputStreamLogger.h"

using namespace std;
using namespace db::test;
using namespace db::io;
using namespace db::logging;
using namespace db::rt;

void runLoggingTest(TestRunner& tr)
{
   int obj;
   tr.group("Logging");

   /////////////////

   tr.test("init");
   // Do a cleanup and re-init.  This could invalidate other unit test setup.
   Logging::cleanup();
   Logging::initialize();
   tr.passIfNoException();

   /////////////////

   tr.test("basic");
   
   // create the stdout output stream
   OStreamOutputStream stdoutOS(&cout);

   // Create the default logger
   OutputStreamLogger defaultLogger(&stdoutOS);

   // add a default logger for all categories
   Logger::addLogger(&defaultLogger);

   // create file logger
   File file("test.log");
   FileLogger flog(&file);
   // log default category to the file
   Logger::addLogger(&flog);

   // basic tests of levels
   DB_ERROR("[error message]");
   DB_WARNING("[warning message]");
   DB_INFO("[info message]");
   DB_DEBUG("[debug message]");
   
   tr.passIfNoException();

   /////////////////

   tr.test("TEST_CAT");

   // Create a test Logger and category
   OutputStreamLogger testLogger(&stdoutOS);
   Category TEST_CAT("DB_TEST", "DB Test Suite", NULL);
   
   // add logger for specific category
   Logger::addLogger(&testLogger, &TEST_CAT);

   // category test
   DB_CAT_ERROR(&TEST_CAT, "[(TEST_CAT,DB_ALL_CAT) error message]");
   
   // cat error with object address
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "[(TEST,ALL) error w/ object]");

   // check for cat logger removal
   Logger::removeLogger(&testLogger, &TEST_CAT);
   DB_CAT_ERROR(&TEST_CAT, "[(!TEST,ALL) error message]");

   tr.passIfNoException();
   
   /////////////////

   tr.test("DB_ALL_CAT");
   
   DB_DEBUG("ALL from DB_DEFAULT_CAT");
   DB_CAT_DEBUG(&TEST_CAT, "ALL from TEST_CAT");
   
   tr.passIfNoException();
   
   /////////////////

   tr.test("flags");
   
   Logger::LoggerFlags old = defaultLogger.getFlags();

   defaultLogger.setFlags(0);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "none");

   defaultLogger.setFlags(Logger::LogDefaultFlags);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "default");

   defaultLogger.setFlags(Logger::LogVerboseFlags);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "verbose");

   defaultLogger.setFlags(Logger::LogDate);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Date");

   defaultLogger.setFlags(Logger::LogThread);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Thread");

   defaultLogger.setFlags(Logger::LogObject);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Object");

   defaultLogger.setFlags(Logger::LogLevel);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Level");

   defaultLogger.setFlags(Logger::LogCategory);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Category");

   defaultLogger.setFlags(Logger::LogLocation);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Location");

   defaultLogger.setFlags(Logger::LogDate | Logger::LogThread |
      Logger::LogObject | Logger::LogLevel |
      Logger::LogCategory | Logger::LogLocation);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "all");

   defaultLogger.setFlags(old);

   tr.passIfNoException();

   /////////////////

   tr.test("object");

   DB_CAT_OBJECT_DEBUG(DB_DEFAULT_CAT, &obj, "object");
   DB_CAT_OBJECT_DEBUG(DB_DEFAULT_CAT, (void*)1, "object @ 0x1");
   DB_CAT_OBJECT_DEBUG(DB_DEFAULT_CAT, NULL, "NULL object");

   tr.passIfNoException();

   /////////////////

   tr.test("double log");

   // re-add default logger 
   Logger::addLogger(&defaultLogger);
   // check if message is logged twice
   DB_DEBUG("double test");
   // remove it
   Logger::removeLogger(&defaultLogger);

   tr.passIfNoException();

   /////////////////

   tr.test("varargs");

   DB_ERROR("10=%d \"foo\"=\"%s\"", 10, "foo");

   tr.passIfNoException();

   /////////////////

   tr.test("dyno");

   DynamicObject dyno;
   dyno["logging"] = "is fun";
   //DB_DYNO_DEBUG(&dyno, "dyno smart pointer 1");

   DynamicObject dyno2 = dyno;
   //DB_DYNO_DEBUG(&dyno2, "dyno smart pointer 2");

   tr.passIfNoException();

   /////////////////

   tr.test("clear");

   // create a string output stream
   ostringstream oss;
   OStreamOutputStream sos(&oss);

   // add logging for all log messages
   OutputStreamLogger sLogger(&sos);
      
   // add default logger
   Logger::addLogger(&sLogger);
   
   // clear ot
   Logger::clearLoggers();

   // Try to output
   DB_DEBUG("Error if I am logged.");
   
   assert(oss.str().length() == 0);

   Logger::clearLoggers();

   tr.passIfNoException();

   /////////////////

   tr.test("re-init");
   // Do a cleanup and re-init for other unit tests.
   Logging::cleanup();
   Logging::initialize();
   tr.passIfNoException();

   /////////////////

   tr.ungroup();
}

struct s2l_s {
   const char* key;
   Logger::Level level;
};
static const struct s2l_s s2l[] = {
   {"n", Logger::None},
   {"N", Logger::None},
   {"none", Logger::None},
   {"None", Logger::None},
   {"NONE", Logger::None},
   {"e", Logger::Error},
   {"error", Logger::Error},
   {"w", Logger::Warning},
   {"warning", Logger::Warning},
   {"i", Logger::Info},
   {"info", Logger::Info},
   {"d", Logger::Debug},
   {"debug", Logger::Debug},
   {"debug-data", Logger::DebugData},
   {"debug-detail", Logger::DebugDetail},
   {"m", Logger::Max},
   {"max", Logger::Max},
   {NULL, Logger::None}
};

struct l2s_s {
   const char* key;
   Logger::Level level;
};
static const struct l2s_s l2s[] = {
   {"NONE", Logger::None},
   {"ERROR", Logger::Error},
   {"WARNING", Logger::Warning},
   {"INFO", Logger::Info},
   {"DEBUG", Logger::Debug},
   {"DEBUG-DATA", Logger::DebugData},
   {"DEBUG-DETAIL", Logger::DebugDetail},
   {"MAX", Logger::Max},
   {NULL, Logger::None}
};

void runLevelTest(TestRunner& tr)
{
   tr.group("Levels");

   tr.test("string2level");
   {
      Logger::Level level;
      for(int i = 0; s2l[i].key != NULL; i++)
      {
         assert(Logger::stringToLevel(s2l[i].key, level));
         assert(level == s2l[i].level);
      }
      assert(!Logger::stringToLevel(NULL, level));
      assert(!Logger::stringToLevel("", level));
      assert(!Logger::stringToLevel("*bogus*", level));
   }
   tr.passIfNoException();

   tr.test("level2string");
   {
      for(int i = 0; l2s[i].key != NULL; i++)
      {
         assertStrCmp(Logger::levelToString(l2s[i].level), l2s[i].key);
      }
      assert(Logger::levelToString((Logger::Level)-1) == NULL);
   }
   tr.passIfNoException();

   tr.ungroup();
}

class DbLoggingTester : public db::test::Tester
{
public:
   DbLoggingTester()
   {
      setName("logging");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runLevelTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
//      runLoggingTest(tr);
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbLoggingTester)
#endif
