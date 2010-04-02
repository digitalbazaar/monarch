/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include <cstdlib>

#include "monarch/app/App.h"
#include "monarch/app/AppPluginFactory.h"
#include "monarch/io/File.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/logging/FileLogger.h"
#include "monarch/logging/OutputStreamLogger.h"

#include "monarch/app/LoggingPlugin.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::io;
using namespace monarch::logging;
using namespace monarch::modest;
using namespace monarch::rt;

#define PLUGIN_NAME "monarch.app.Logging"
#define PLUGIN_CL_CFG_ID PLUGIN_NAME ".commandLine"

LoggingPlugin::LoggingPlugin() :
   mLogger(NULL)
{
}

LoggingPlugin::~LoggingPlugin()
{
   mLogger = NULL;
}

bool LoggingPlugin::initialize()
{
   bool rval = AppPlugin::initialize();
   if(getApp()->getMode() == App::BOOTSTRAP)
   {
      rval = rval && monarch::logging::Logging::initialize();
   }
   return rval;
}

void LoggingPlugin::cleanup()
{
   AppPlugin::cleanup();
   if(getApp()->getMode() == App::BOOTSTRAP)
   {
      monarch::logging::Logging::cleanup();
   }
}

bool LoggingPlugin::initMetaConfig(Config& meta)
{
   bool rval = monarch::app::AppPlugin::initMetaConfig(meta);

   // defaults
   if(rval)
   {
      const char* id = PLUGIN_NAME ".defaults";
      Config& c =
         App::makeMetaConfig(meta, id, "defaults")
         [ConfigManager::MERGE][PLUGIN_NAME];
      c["enabled"] = true;
      c["level"] = "warning";
      c["log"] = "-";
      c["append"] = true;
      c["rotationFileSize"] = UINT64_C(2000000);
      c["maxRotatedFiles"] = UINT32_C(10);
      c["gzip"] = true;
      c["location"] = false;
      c["color"] = false;
   }

   // command line options
   if(rval)
   {
      Config c =
         App::makeMetaConfig(
            meta, PLUGIN_CL_CFG_ID, "command line", "options");
      c[ConfigManager::MERGE][PLUGIN_NAME]->setType(Map);
   }

   return rval;
}

DynamicObject LoggingPlugin::getCommandLineSpecs()
{
   DynamicObject spec;
   spec["help"] =
"Logging options:\n"
"      --no-log        Disable default logging. (default: enabled)\n"
"      --log-level LEVEL\n"
"                      Set log level to one of the following (listed in\n"
"                      increasing level of detail): n[one], e[rror], w[arning],\n"
"                      i[nfo], d[ebug], debug-data, debug-detail, m[ax].\n"
"                      (default: \"warning\")\n"
"      --log LOG       Set log file.  Use \"-\" for stdout. (default: \"-\")\n"
"      --log-overwrite Overwrite log file instead of appending. (default: false)\n"
"      --log-rotation-size SIZE\n"
"                      Log size that triggers rotation in bytes. 0 to disable.\n"
"                      (default: 2000000)\n"
"      --log-max-rotated MAX\n"
"                      Maximum number of rotated log files. 0 for no limit.\n"
"                      (default: 10)\n"
"      --log-gzip      Do gzip rotated logs. (default: gzip logs)\n"
"      --log-no-gzip   Do not gzip rotated logs. (default: gzip logs)\n"
"      --log-color     Log with any available ANSI color codes. (default: false)\n"
"      --log-no-color  Log without ANSI color codes. (default: false)\n"
"      --log-location  Log source code locations.\n"
"                      (compile time option, default: false)\n"
"\n";

   DynamicObject opt;
   Config options = getApp()->getMetaConfig()
      ["options"][PLUGIN_CL_CFG_ID][ConfigManager::MERGE][PLUGIN_NAME];

   opt = spec["options"]->append();
   opt["long"] = "--no-log";
   opt["setFalse"]["root"] = options;
   opt["setFalse"]["path"] = "enabled";

   opt = spec["options"]->append();
   opt["long"] = "--log-level";
   opt["arg"]["root"] = options;
   opt["arg"]["path"] = "level";
   opt["argError"] = "No log level specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log";
   opt["arg"]["root"] = options;
   opt["arg"]["path"] = "log";
   opt["argError"] = "No log file specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-overwrite";
   opt["setFalse"]["root"] = options;
   opt["setFalse"]["path"] = "append";

   opt = spec["options"]->append();
   opt["long"] = "--log-rotation-size";
   opt["arg"]["root"] = options;
   opt["arg"]["path"] = "rotationFileSize";
   opt["argError"] = "No rotation size specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-max-rotated";
   opt["arg"]["root"] = options;
   opt["arg"]["path"] = "maxRotatedFiles";
   opt["argError"] = "Max rotated files not specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-gzip";
   opt["setTrue"]["root"] = options;
   opt["setTrue"]["path"] = "gzip";

   opt = spec["options"]->append();
   opt["long"] = "--log-no-gzip";
   opt["setFalse"]["root"] = options;
   opt["setFalse"]["path"] = "gzip";

   opt = spec["options"]->append();
   opt["long"] = "--log-location";
   opt["setTrue"]["root"] = options;
   opt["setTrue"]["path"] = "location";

   opt = spec["options"]->append();
   opt["long"] = "--log-color";
   opt["setTrue"]["root"] = options;
   opt["setTrue"]["path"] = "color";

   opt = spec["options"]->append();
   opt["long"] = "--log-no-color";
   opt["setFalse"]["root"] = options;
   opt["setFalse"]["path"] = "color";

   DynamicObject specs = AppPlugin::getCommandLineSpecs();
   specs->append(spec);
   return specs;
}

bool LoggingPlugin::initializeLogging()
{
   bool rval = AppPlugin::initializeLogging();

   // get logging config
   Config cfg = getApp()->getConfig()[PLUGIN_NAME];

   if(rval && getApp()->getMode() == App::BOOTSTRAP &&
      cfg["enabled"]->getBoolean())
   {
      // setup logging
      const char* logFile = cfg["log"]->getString();
      if(strcmp(logFile, "-") == 0)
      {
         OutputStream* logStream = new FileOutputStream(
            FileOutputStream::StdOut);
         mLogger = new OutputStreamLogger(logStream, true);
      }
      else
      {
         bool append = cfg["append"]->getBoolean();

         // attempt to expand "~" (in case not handled natively)
         string expandedLogFile;
         if(!File::isPathAbsolute(logFile))
         {
            rval = File::expandUser(logFile, expandedLogFile);
         }
         else
         {
            expandedLogFile.assign(logFile);
         }

         if(rval)
         {
            File f(expandedLogFile.c_str());
            FileLogger* fileLogger = new FileLogger();
            rval = fileLogger->setFile(f, append);
            if(rval)
            {
               if(cfg["gzip"]->getBoolean())
               {
                  fileLogger->setFlags(FileLogger::GzipCompressRotatedLogs);
               }
               fileLogger->setRotationFileSize(
                  cfg["rotationFileSize"]->getUInt64());
               fileLogger->setMaxRotatedFiles(
                  cfg["maxRotatedFiles"]->getUInt32());
               mLogger = fileLogger;
            }
            else
            {
               delete fileLogger;
            }
         }
      }

      if(rval)
      {
         // FIXME: add cfg option to pick categories to log
         //Logger::addLogger(&mLogger, BM_..._CAT);
         // FIXME: add cfg options for logging options
         //logger.setDateFormat("%H:%M:%S");
         //logger.setFlags(Logger::LogThread);
         Logger::Level logLevel;
         const char* levelStr = cfg["level"]->getString();
         bool found = Logger::stringToLevel(levelStr, logLevel);
         if(found)
         {
            mLogger->setLevel((Logger::Level)logLevel);
         }
         else
         {
            ExceptionRef e = new Exception(
               "Invalid monarch.logging.level.", "monarch.app.ConfigError");
            e->getDetails()["level"] = (levelStr ? levelStr : "\"\"");
            Exception::set(e);
            rval = false;
         }
         if(cfg["color"]->getBoolean())
         {
            mLogger->setFlags(Logger::LogColor);
         }
         if(cfg["location"]->getBoolean())
         {
            mLogger->setFlags(Logger::LogLocation);
         }
         Logger::addLogger(mLogger);

         // NOTE: Logging is now initialized. Use standard logging system after
         // NOTE: this point.
      }
   }

   return rval;
}

bool LoggingPlugin::cleanupLogging()
{
   bool rval = AppPlugin::cleanupLogging();

   if(mLogger != NULL)
   {
      Logger::removeLogger(mLogger);
      delete mLogger;
      mLogger = NULL;
   }

   return rval;
}

class LoggingPluginFactory :
   public AppPluginFactory
{
public:
      LoggingPluginFactory() :
      AppPluginFactory(PLUGIN_NAME, "1.0")
   {
      addDependency("monarch.app.Config", "1.0");
   }

   virtual ~LoggingPluginFactory() {}

   virtual AppPluginRef createAppPlugin()
   {
      return new LoggingPlugin();
   }
};

Module*  monarch::app::createLoggingPluginFactory()
{
   return new LoggingPluginFactory();
}
