/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */

#include <cstdlib>

#include "monarch/app/App.h"
#include "monarch/io/File.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/logging/FileLogger.h"
#include "monarch/logging/OutputStreamLogger.h"

#include "monarch/app/CommonAppPlugin.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::io;
using namespace monarch::logging;
using namespace monarch::rt;

CommonAppPlugin::CommonAppPlugin() :
   mLogger(NULL)
{
   mInfo["id"] = "db.app.plugins.Common";
   // default is to depend on this plugin so clear the dependency loop.
   mInfo["dependencies"]->clear();
}

CommonAppPlugin::~CommonAppPlugin()
{
   mApp = NULL;
   mLogger = NULL;
}

bool CommonAppPlugin::willInitMetaConfig(Config& meta)
{
   bool rval = monarch::app::AppPlugin::willInitMetaConfig(meta);

   if(rval)
   {
      //  map of well-known ids that can be customized
      meta["groups"]["root"] = "root";
      meta["groups"]["boot"] = "boot";
      meta["groups"]["before defaults"] = "before defaults";
      meta["groups"]["defaults"] = "defaults";
      meta["groups"]["after defaults"] = "after defaults";
      meta["groups"]["command line"] = "command line";
      meta["groups"]["main"] = "main";

      // map of parents of well-known ids that can be customized
      meta["parents"]["root"].setNull();
      meta["parents"]["boot"] = "root";
      meta["parents"]["before defaults"] = "boot";
      meta["parents"]["defaults"] = "before defaults";
      meta["parents"]["after defaults"] = "defaults";
      meta["parents"]["command line"] = "after defaults";
      meta["parents"]["main"] = "command line";

      // map of configs indexed by id
      meta["configs"]->setType(Map);
   }

   return rval;
}

bool CommonAppPlugin::initMetaConfig(Config& meta)
{
   bool rval = monarch::app::AppPlugin::initMetaConfig(meta);

   // hard-coded empty root
   if(rval)
   {
      const char* id = "db.app.root";
      Config& config = meta["configs"][id];

      // no parent
      config[ConfigManager::GROUP] = meta["groups"]["root"]->getString();
      config[ConfigManager::ID] = id;
      config[ConfigManager::VERSION] = DB_DEFAULT_CONFIG_VERSION;
   }

   // hard-coded application boot-up defaults
   if(rval)
   {
      const char* id = "db.app.boot";
      Config& config = meta["configs"][id];

      config[ConfigManager::GROUP] = meta["groups"]["boot"]->getString();
      config[ConfigManager::ID] = id;
      config[ConfigManager::VERSION] = DB_DEFAULT_CONFIG_VERSION;

      Config& merge = config[ConfigManager::MERGE];
      merge["app"]["debug"]["init"] = false;
      merge["app"]["config"]["debug"] = false;
      merge["app"]["config"]["dump"] = false;
      merge["app"]["logging"]["enabled"] = true;
      merge["app"]["logging"]["level"] = "warning";
      merge["app"]["logging"]["log"] = "-";
      merge["app"]["logging"]["append"] = true;
      merge["app"]["logging"]["rotationFileSize"] = (uint64_t)(2000000ULL);
      merge["app"]["logging"]["maxRotatedFiles"] = (uint32_t)10;
      merge["app"]["logging"]["gzip"] = true;
      merge["app"]["logging"]["location"] = false;
      merge["app"]["logging"]["color"] = false;
      merge["app"]["verbose"]["level"] = (uint64_t)0;
   }

   // command line option config
   if(rval)
   {
      const char* id = "db.app.commandLine";
      Config& config = meta["options"][id];

      config[ConfigManager::GROUP] =
         meta["groups"]["command line"]->getString();
      config[ConfigManager::ID] = id;
      config[ConfigManager::VERSION] = DB_DEFAULT_CONFIG_VERSION;

      config[ConfigManager::TMP]->setType(Map);
      // must set since cmd line does read-modify-write directly on this config
      Config& merge = config[ConfigManager::MERGE];
      merge["app"]["verbose"]["level"] = (uint64_t)0;
   }

   if(rval)
   {
      // defaults
      App::makeMetaConfig(
         meta, "db.app.beforeDefaults.empty", "before defaults");
      App::makeMetaConfig(meta, "db.app.defaults.empty", "defaults");
      App::makeMetaConfig(
         meta, "db.app.afterDefaults.empty", "after defaults");

      // command line
      App::makeMetaConfig(meta, "db.app.commandLine.empty", "command line");

      // main
      App::makeMetaConfig(meta, "db.app.main.empty", "main");
   }

   return rval;
}

DynamicObject CommonAppPlugin::getCommandLineSpecs()
{
   DynamicObject spec;
   spec["help"] =
"Help options:\n"
"  -h, --help          Prints information on how to use the application.\n"
"\n"
"General options:\n"
"  -V, --version       Prints the software version.\n"
"  -v, --verbose       Increase verbosity level by 1. (default: 0)\n"
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
"      --              Treat all remaining options as application arguments.\n"
"\n"
"Config options:\n"
"      --config-debug  Debug the configuration loading process to stdout.\n"
"      --config-dump   Load and dump all configuration data to stdout.\n"
"      --option NAME VALUE\n"
"                      Set dotted config path NAME to the string VALUE.\n"
"      --json-option NAME JSONVALUE\n"
"                      Set dotted config path NAME to the decoded JSONVALUE.\n"
"\n";

   DynamicObject opt;
   Config tempOptions = getApp()->getMetaConfig()
      ["options"]["db.app.commandLine"][ConfigManager::TMP];
   Config options = getApp()->getMetaConfig()
      ["options"]["db.app.commandLine"][ConfigManager::MERGE];

   opt = spec["options"]->append();
   opt["short"] = "-h";
   opt["long"] = "--help";
   opt["setTrue"]["root"] = tempOptions;
   opt["setTrue"]["path"] = "printHelp";

   opt = spec["options"]->append();
   opt["short"] = "-V";
   opt["long"] = "--version";
   opt["setTrue"]["root"] = tempOptions;
   opt["setTrue"]["path"] = "printVersion";

   opt = spec["options"]->append();
   opt["short"] = "-v";
   opt["long"] = "--verbose";
   opt["inc"]["root"] = options;
   opt["inc"]["path"] = "app.verbose.level";

   opt = spec["options"]->append();
   opt["long"] = "--no-log";
   opt["setFalse"]["root"] = options;
   opt["setFalse"]["path"] = "app.logging.enabled";

   opt = spec["options"]->append();
   opt["long"] = "--log-level";
   opt["arg"]["root"] = options;
   opt["arg"]["path"] = "app.logging.level";
   opt["argError"] = "No log level specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log";
   opt["arg"]["root"] = options;
   opt["arg"]["path"] = "app.logging.log";
   opt["argError"] = "No log file specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-overwrite";
   opt["setFalse"]["root"] = options;
   opt["setFalse"]["path"] = "app.logging.append";

   opt = spec["options"]->append();
   opt["long"] = "--log-rotation-size";
   opt["arg"]["root"] = options;
   opt["arg"]["path"] = "app.logging.rotationFileSize";
   opt["argError"] = "No rotation size specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-max-rotated";
   opt["arg"]["root"] = options;
   opt["arg"]["path"] = "app.logging.maxRotatedFiles";
   opt["argError"] = "Max rotated files not specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-gzip";
   opt["setTrue"]["root"] = options;
   opt["setTrue"]["path"] = "app.logging.gzip";

   opt = spec["options"]->append();
   opt["long"] = "--log-no-gzip";
   opt["setFalse"]["root"] = options;
   opt["setFalse"]["path"] = "app.logging.gzip";

   opt = spec["options"]->append();
   opt["long"] = "--log-location";
   opt["setTrue"]["root"] = options;
   opt["setTrue"]["path"] = "app.logging.location";

   opt = spec["options"]->append();
   opt["long"] = "--log-color";
   opt["setTrue"]["root"] = options;
   opt["setTrue"]["path"] = "app.logging.color";

   opt = spec["options"]->append();
   opt["long"] = "--log-no-color";
   opt["setFalse"]["root"] = options;
   opt["setFalse"]["path"] = "app.logging.color";

   opt = spec["options"]->append();
   opt["long"] = "--option";
   opt["set"]["root"] = options;

   opt = spec["options"]->append();
   opt["long"] = "--json-option";
   opt["set"]["root"] = options;
   opt["isJsonValue"] = true;

   opt = spec["options"]->append();
   opt["long"] = "--config-debug";
   opt["setTrue"]["root"] = options;
   opt["setTrue"]["path"] = "app.config.debug";

   opt = spec["options"]->append();
   opt["long"] = "--config-dump";
   opt["setTrue"]["root"] = options;
   opt["setTrue"]["path"] = "app.config.dump";

   DynamicObject specs = AppPlugin::getCommandLineSpecs();
   specs->append(spec);
   return specs;
}

bool CommonAppPlugin::willParseCommandLine(std::vector<const char*>* args)
{
   bool rval = AppPlugin::willParseCommandLine(args);

   Config tempOptions = getApp()->getMetaConfig()
      ["options"]["db.app.commandLine"][ConfigManager::TMP];

   // temporary flags for command line processing
   tempOptions["printHelp"] = false;
   tempOptions["printVersion"] = false;

   return rval;
}

bool CommonAppPlugin::didParseCommandLine()
{
   bool rval = AppPlugin::didParseCommandLine();

   Config tempOptions = getApp()->getMetaConfig()
      ["options"]["db.app.commandLine"][ConfigManager::TMP];

   // process help and version flags first
   if(tempOptions["printHelp"]->getBoolean())
   {
      printf("Usage: %s [options]\n", getApp()->getProgramName());
      DynamicObjectIterator si =
         getApp()->getMetaConfig()["specs"].getIterator();
      while(si->hasNext())
      {
         DynamicObject& spec = si->next();
         if(spec->hasMember("help"))
         {
            printf("%s", spec["help"]->getString());
         }
      }
      // FIXME: change to known exit exception?
      exit(EXIT_SUCCESS);
   }
   else if(tempOptions["printVersion"]->getBoolean())
   {
      // TODO: allow other version info (modules, etc) via delegate?
      const char* version = getApp()->getVersion();
      printf("%s%s%s\n",
         getApp()->getName(),
         (version != NULL) ? " v" : "",
         (version != NULL) ? version : "");
      // FIXME: change to known exit exception?
      exit(EXIT_SUCCESS);
   }

   // check logging level
   if(rval)
   {
      Config cfg = getApp()->getConfig();
      const char* cfgLogLevel = cfg["app"]["logging"]["level"]->getString();
      Logger::Level level;
      bool found = Logger::stringToLevel(cfgLogLevel, level);
      if(!found)
      {
         ExceptionRef e =
            new Exception("Invalid log level.", "db.app.CommandLineError");
         e->getDetails()["level"] = cfgLogLevel;
         Exception::set(e);
         rval = false;
      }
   }

   // done with temporary command line config options
   tempOptions->removeMember("printHelp");
   tempOptions->removeMember("printVersion");

   return rval;
}

bool CommonAppPlugin::initializeLogging()
{
   bool rval = AppPlugin::initializeLogging();

   // get logging config
   Config cfg = getApp()->getConfig()["app"]["logging"];

   if(rval && cfg["enabled"]->getBoolean())
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
               "Invalid app.logging.level.", "db.app.ConfigError");
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

bool CommonAppPlugin::cleanupLogging()
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

bool CommonAppPlugin::willRun()
{
   return true;
}

bool CommonAppPlugin::run()
{
   return true;
}

bool CommonAppPlugin::didRun()
{
   return true;
}
