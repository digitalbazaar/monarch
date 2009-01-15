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

#ifdef WIN32
#define TMPDIR "c:/WINDOWS/Temp"
#else
#define TMPDIR "/tmp"
#endif

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
   File file(TMPDIR "/test-logging.log");
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

   defaultLogger.setAllFlags(0);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "none");

   defaultLogger.setAllFlags(Logger::LogDefaultFlags);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "default");

   defaultLogger.setAllFlags(Logger::LogVerboseFlags);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "verbose");

   defaultLogger.setAllFlags(Logger::LogDate);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Date");

   defaultLogger.setAllFlags(Logger::LogThread);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Thread");

   defaultLogger.setAllFlags(Logger::LogObject);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Object");

   defaultLogger.setAllFlags(Logger::LogLevel);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Level");

   defaultLogger.setAllFlags(Logger::LogCategory);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Category");

   defaultLogger.setAllFlags(Logger::LogLocation);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Location");

   defaultLogger.setAllFlags(Logger::LogDate | Logger::LogThread |
      Logger::LogObject | Logger::LogLevel |
      Logger::LogCategory | Logger::LogLocation);
   DB_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "all");

   defaultLogger.setAllFlags(old);

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

static void rotatetest(unsigned int maxFiles, off_t maxSize, bool compress)
{
   char fnr[200];
   snprintf(fnr, 200, "/db-test-logging-rotation-%d-%d%s.log",
      (int)maxFiles, (int)maxSize, compress ? "-gz" : ""); 
   string fn;
   fn.append(TMPDIR);
   fn.append(fnr); 
   // create file logger
   File file(fn.c_str());
   FileLogger flog(&file);
   flog.setMaxRotatedFiles(maxFiles);
   flog.setRotationFileSize(maxSize);
   if(compress)
   {
      flog.setFlags(FileLogger::GzipCompressRotatedLogs);
   }
   // log default category to the file
   Logger::addLogger(&flog);
   
   for(int i = 0; i < 500; i++)
   {
      DB_DEBUG("[%05d] 01234567890123456789012345678901234567890123456789", i);
   }
   
   Logger::removeLogger(&flog);
}

void runLogRotationTest(TestRunner& tr)
{
   tr.group("Log Rotation");

   tr.test("init");
   {
      // Do a cleanup and re-init.  This could invalidate other unit test setup.
      Logging::cleanup();
      Logging::initialize();
   }
   tr.passIfNoException();

   tr.test("no rotate");
   {
      rotatetest(0, 0, false);
   }
   tr.passIfNoException();

   tr.test("rotate size:1000");
   {
      rotatetest(0, 1000, false);
   }
   tr.passIfNoException();

   tr.test("rotate size:1000 max:3");
   {
      rotatetest(3, 1000, false);
   }
   tr.passIfNoException();

   tr.test("rotate size:1000 max:3 gz");
   {
      rotatetest(3, 1000, true);
   }
   tr.passIfNoException();

   tr.test("re-init");
   {
      // Do a cleanup and re-init for other unit tests.
      Logging::cleanup();
      Logging::initialize();
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runColorLoggingTestAll(TestRunner& tr)
{
   // test of levels
   DB_ERROR("[error message]");
   DB_WARNING("[warning message]");
   DB_INFO("[info message]");
   DB_DEBUG("[debug message]");
   
   // test known dbcore categories
   DB_CAT_DEBUG(DB_APP_CAT, "[cat:DB_APP_CAT]");
   DB_CAT_DEBUG(DB_CONFIG_CAT, "[cat:DB_CONFIG_CAT]");
   DB_CAT_DEBUG(DB_CRYPTO_CAT, "[cat:DB_DATA_CAT]");
   DB_CAT_DEBUG(DB_DATA_CAT, "[cat:DB_DATA_CAT]");
   DB_CAT_DEBUG(DB_EVENT_CAT, "[cat:DB_EVENT_CAT]");
   DB_CAT_DEBUG(DB_GUI_CAT, "[cat:DB_GUI_CAT]");
   DB_CAT_DEBUG(DB_IO_CAT, "[cat:DB_IO_CAT]");
   DB_CAT_DEBUG(DB_LOGGING_CAT, "[cat:DB_LOGGING_CAT]");
   DB_CAT_DEBUG(DB_MAIL_CAT, "[cat:DB_MAIL_CAT]");
   DB_CAT_DEBUG(DB_MODEST_CAT, "[cat:DB_MODEST_CAT]");
   DB_CAT_DEBUG(DB_NET_CAT, "[cat:DB_NET_CAT]");
   DB_CAT_DEBUG(DB_RT_CAT, "[cat:DB_RT_CAT]");
   DB_CAT_DEBUG(DB_SPHINX_CAT, "[cat:DB_SPHINX_CAT]");
   DB_CAT_DEBUG(DB_SQL_CAT, "[cat:DB_SQL_CAT]");
   DB_CAT_DEBUG(DB_UTIL_CAT, "[cat:DB_UTIL_CAT]");
}

void runColorLoggingTest(TestRunner& tr)
{
   tr.group("color");

   // create the stdout output stream
   OStreamOutputStream stdoutOS(&cout);

   // Create the default logger
   OutputStreamLogger logger(&stdoutOS);
   // Set color mode
   logger.setFlags(Logger::LogColor);

   // clear previous loggers
   Logger::clearLoggers();
   // add a logger for all categories
   Logger::addLogger(&logger);
   
   tr.test("no color");
   {
      logger.clearFlags(Logger::LogColor);
      runColorLoggingTestAll(tr);
   }
   tr.passIfNoException();
   
   tr.test("color");
   {
      logger.setFlags(Logger::LogColor);
      runColorLoggingTestAll(tr);
   }
   tr.passIfNoException();

   Logger::removeLogger(&logger);

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
      runLoggingTest(tr);
      runLogRotationTest(tr);
      runColorLoggingTest(tr);
      return 0;
   }
};

#undef TMPDIR

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbLoggingTester)
#endif
