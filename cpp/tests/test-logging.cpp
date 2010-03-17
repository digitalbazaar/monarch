/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include <iostream>
#include <sstream>
#include <cstdio>

#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/io/OStreamOutputStream.h"
#include "monarch/logging/Logging.h"
#include "monarch/logging/FileLogger.h"
#include "monarch/logging/OutputStreamLogger.h"

using namespace std;
using namespace monarch::test;
using namespace monarch::io;
using namespace monarch::logging;
using namespace monarch::rt;

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
   MO_ERROR("[error message]");
   MO_WARNING("[warning message]");
   MO_INFO("[info message]");
   MO_DEBUG("[debug message]");

   tr.passIfNoException();

   /////////////////

   tr.test("in-memory first");

   // create 2-stage file logger, first in-memory, then file
   File file2(TMPDIR "/test-logging2.log");
   FileLogger flog2;
   assert(flog2.setInMemoryLog(16384));
   // log default category to the file
   Logger::addLogger(&flog2);

   // basic tests of levels
   MO_ERROR("[error message]");
   MO_WARNING("[warning message]");
   MO_INFO("[info message]");
   MO_DEBUG("[debug message]");

   // now set log file, dumping in-memory to file
   flog2.setFile(file2);

   tr.passIfNoException();

   /////////////////

   tr.test("TEST_CAT");

   // Create a test Logger and category
   OutputStreamLogger testLogger(&stdoutOS);
   Category TEST_CAT("MO_TEST", "Monarch Test Suite", NULL);

   // add logger for specific category
   Logger::addLogger(&testLogger, &TEST_CAT);

   // category test
   MO_CAT_ERROR(&TEST_CAT, "[(TEST_CAT,MO_ALL_CAT) error message]");

   // cat error with object address
   MO_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "[(TEST,ALL) error w/ object]");

   // check for cat logger removal
   Logger::removeLogger(&testLogger, &TEST_CAT);
   MO_CAT_ERROR(&TEST_CAT, "[(!TEST,ALL) error message]");

   tr.passIfNoException();

   /////////////////

   tr.test("MO_ALL_CAT");

   MO_DEBUG("ALL from MO_DEFAULT_CAT");
   MO_CAT_DEBUG(&TEST_CAT, "ALL from TEST_CAT");

   tr.passIfNoException();

   /////////////////

   tr.test("flags");

   Logger::LoggerFlags old = defaultLogger.getFlags();

   defaultLogger.setAllFlags(0);
   MO_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "none");

   defaultLogger.setAllFlags(Logger::LogDefaultFlags);
   MO_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "default");

   defaultLogger.setAllFlags(Logger::LogVerboseFlags);
   MO_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "verbose");

   defaultLogger.setAllFlags(Logger::LogDate);
   MO_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Date");

   defaultLogger.setAllFlags(Logger::LogThread);
   MO_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Thread");

   defaultLogger.setAllFlags(Logger::LogObject);
   MO_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Object");

   defaultLogger.setAllFlags(Logger::LogLevel);
   MO_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Level");

   defaultLogger.setAllFlags(Logger::LogCategory);
   MO_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Category");

   defaultLogger.setAllFlags(Logger::LogLocation);
   MO_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "Location");

   defaultLogger.setAllFlags(Logger::LogDate | Logger::LogThread |
      Logger::LogObject | Logger::LogLevel |
      Logger::LogCategory | Logger::LogLocation);
   MO_CAT_OBJECT_ERROR(&TEST_CAT, &obj, "all");

   defaultLogger.setAllFlags(old);

   tr.passIfNoException();

   /////////////////

   tr.test("object");

   MO_CAT_OBJECT_DEBUG(MO_DEFAULT_CAT, &obj, "object");
   MO_CAT_OBJECT_DEBUG(MO_DEFAULT_CAT, (void*)1, "object @ 0x1");
   MO_CAT_OBJECT_DEBUG(MO_DEFAULT_CAT, NULL, "NULL object");

   tr.passIfNoException();

   /////////////////

   tr.test("double log");

   // re-add default logger
   Logger::addLogger(&defaultLogger);
   // check if message is logged twice
   MO_DEBUG("double test");
   // remove it
   Logger::removeLogger(&defaultLogger);

   tr.passIfNoException();

   /////////////////

   tr.test("varargs");

   MO_ERROR("10=%d \"foo\"=\"%s\"", 10, "foo");

   tr.passIfNoException();

   /////////////////

   tr.test("dyno");

   DynamicObject dyno;
   dyno["logging"] = "is fun";
   //MO_DYNO_DEBUG(&dyno, "dyno smart pointer 1");

   DynamicObject dyno2 = dyno;
   //MO_DYNO_DEBUG(&dyno2, "dyno smart pointer 2");

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
   MO_DEBUG("Error if I am logged.");

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
   snprintf(fnr, 200, "/monarch-test-logging-rotation-%d-%d%s.log",
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
      MO_DEBUG("[%05d] 01234567890123456789012345678901234567890123456789", i);
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
   MO_ERROR("[error message]");
   MO_WARNING("[warning message]");
   MO_INFO("[info message]");
   MO_DEBUG("[debug message]");

   // test known Monarch categories
   MO_CAT_DEBUG(MO_APP_CAT, "[cat:MO_APP_CAT]");
   MO_CAT_DEBUG(MO_CONFIG_CAT, "[cat:MO_CONFIG_CAT]");
   MO_CAT_DEBUG(MO_CRYPTO_CAT, "[cat:MO_DATA_CAT]");
   MO_CAT_DEBUG(MO_DATA_CAT, "[cat:MO_DATA_CAT]");
   MO_CAT_DEBUG(MO_EVENT_CAT, "[cat:MO_EVENT_CAT]");
   MO_CAT_DEBUG(MO_GUI_CAT, "[cat:MO_GUI_CAT]");
   MO_CAT_DEBUG(MO_IO_CAT, "[cat:MO_IO_CAT]");
   MO_CAT_DEBUG(MO_LOGGING_CAT, "[cat:MO_LOGGING_CAT]");
   MO_CAT_DEBUG(MO_MAIL_CAT, "[cat:MO_MAIL_CAT]");
   MO_CAT_DEBUG(MO_MODEST_CAT, "[cat:MO_MODEST_CAT]");
   MO_CAT_DEBUG(MO_NET_CAT, "[cat:MO_NET_CAT]");
   MO_CAT_DEBUG(MO_RT_CAT, "[cat:MO_RT_CAT]");
   MO_CAT_DEBUG(MO_SPHINX_CAT, "[cat:MO_SPHINX_CAT]");
   MO_CAT_DEBUG(MO_SQL_CAT, "[cat:MO_SQL_CAT]");
   MO_CAT_DEBUG(MO_UTIL_CAT, "[cat:MO_UTIL_CAT]");
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

#undef TMPDIR

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runLevelTest(tr);
   }
   if(tr.isTestEnabled("logging"))
   {
      runLoggingTest(tr);
   }
   if(tr.isTestEnabled("log-rotation"))
   {
      runLogRotationTest(tr);
   }
   if(tr.isTestEnabled("color-logging"))
   {
      runColorLoggingTest(tr);
   }
   return true;
}

MO_TEST_MODULE_FN("monarch.tests.logging.test", "1.0", run)
