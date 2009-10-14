/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */

#include <cstdlib>

#include "db/app/App.h"
#include "db/io/File.h"
#include "db/io/FileOutputStream.h"
#include "db/logging/FileLogger.h"
#include "db/logging/OutputStreamLogger.h"

#include "db/app/CommonAppPlugin.h"

using namespace std;
using namespace db::app;
using namespace db::config;
using namespace db::io;
using namespace db::logging;
using namespace db::rt;

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
   bool rval = db::app::AppPlugin::willInitMetaConfig(meta);

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
   bool rval = db::app::AppPlugin::initMetaConfig(meta);

   // hard-coded empty root
   if(rval)
   {
      Config& config = meta["configs"]["db.app.root"];

      // no parent
      config[ConfigManager::GROUP] = meta["groups"]["root"]->getString();
      config[ConfigManager::ID] = "db.app.root";
      config[ConfigManager::VERSION] = DB_DEFAULT_CONFIG_VERSION;
   }

   // hard-coded application boot-up defaults
   if(rval)
   {
      Config& config = meta["configs"]["db.app.boot"];

      config[ConfigManager::GROUP] = meta["groups"]["boot"]->getString();
      config[ConfigManager::ID] = "db.app.boot";
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

   if(rval)
   {
      // defaults
      App::makeMetaConfig(
         meta, "db.app.beforeDefaults.empty", "before defaults");
      App::makeMetaConfig(meta, "db.app.defaults.empty", "defaults");
      App::makeMetaConfig(
         meta, "db.app.afterDefaults.empty", "after defaults");

      // command line (using this id later, no need to add empty config)
      App::makeMetaConfig(meta, "db.app.commandLine", "command line");

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
   const char* clid = "db.app.commandLine";

   Config clConfig = getApp()->getCommandLineConfig();
   opt = spec["options"]->append();
   opt["short"] = "-h";
   opt["long"] = "--help";
   opt["setTrue"]["target"] = clConfig["options"]["printHelp"];

   opt = spec["options"]->append();
   opt["short"] = "-V";
   opt["long"] = "--version";
   opt["setTrue"]["target"] = clConfig["options"]["printVersion"];

   opt = spec["options"]->append();
   opt["short"] = "-v";
   opt["long"] = "--verbose";
   opt["inc"]["config"] = clid;
   opt["inc"]["path"] = "app.verbose.level";

   opt = spec["options"]->append();
   opt["long"] = "--no-log";
   opt["setFalse"]["config"] = clid;
   opt["setFalse"]["path"] = "app.logging.enabled";

   opt = spec["options"]->append();
   opt["long"] = "--log-level";
   opt["arg"]["config"] = clid;
   opt["arg"]["path"] = "app.logging.level";
   opt["argError"] = "No log level specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log";
   opt["arg"]["config"] = clid;
   opt["arg"]["path"] = "app.logging.log";
   opt["argError"] = "No log file specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-overwrite";
   opt["setFalse"]["config"] = clid;
   opt["setFalse"]["path"] = "app.logging.append";

   opt = spec["options"]->append();
   opt["long"] = "--log-rotation-size";
   opt["arg"]["config"] = clid;
   opt["arg"]["path"] = "app.logging.rotationFileSize";
   opt["argError"] = "No rotation size specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-max-rotated";
   opt["arg"]["config"] = clid;
   opt["arg"]["path"] = "app.logging.maxRotatedFiles";
   opt["argError"] = "Max rotated files not specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-gzip";
   opt["setTrue"]["config"] = clid;
   opt["setTrue"]["path"] = "app.logging.gzip";

   opt = spec["options"]->append();
   opt["long"] = "--log-no-gzip";
   opt["setFalse"]["config"] = clid;
   opt["setFalse"]["path"] = "app.logging.gzip";

   opt = spec["options"]->append();
   opt["long"] = "--log-location";
   opt["setTrue"]["config"] = clid;
   opt["setTrue"]["path"] = "app.logging.location";

   opt = spec["options"]->append();
   opt["long"] = "--log-color";
   opt["setTrue"]["config"] = clid;
   opt["setTrue"]["path"] = "app.logging.color";

   opt = spec["options"]->append();
   opt["long"] = "--log-no-color";
   opt["setFalse"]["config"] = clid;
   opt["setFalse"]["path"] = "app.logging.color";

   opt = spec["options"]->append();
   opt["long"] = "--option";
   opt["set"]["config"] = clid;

   opt = spec["options"]->append();
   opt["long"] = "--json-option";
   opt["set"]["config"] = clid;
   opt["isJsonValue"] = true;

   opt = spec["options"]->append();
   opt["long"] = "--config-debug";
   opt["setTrue"]["config"] = clid;
   opt["setTrue"]["path"] = "app.config.debug";

   opt = spec["options"]->append();
   opt["long"] = "--config-dump";
   opt["setTrue"]["config"] = clid;
   opt["setTrue"]["path"] = "app.config.dump";

   DynamicObject specs = AppPlugin::getCommandLineSpecs();
   specs->append(spec);
   return specs;
}

bool CommonAppPlugin::willParseCommandLine(std::vector<const char*>* args)
{
   bool rval = AppPlugin::willParseCommandLine(args);

   Config clConfig = getApp()->getCommandLineConfig();

   // temporary flags for command line processing
   clConfig["options"]["printHelp"] = false;
   clConfig["options"]["printVersion"] = false;

   return rval;
}

bool CommonAppPlugin::didParseCommandLine()
{
   bool rval = AppPlugin::didParseCommandLine();

   Config clConfig = getApp()->getCommandLineConfig();

   // process help and version flags first
   if(clConfig["options"]["printHelp"]->getBoolean())
   {
      printf("Usage: %s [options]\n", getApp()->getProgramName());
      DynamicObjectIterator si = clConfig["specs"].getIterator();
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
   else if(clConfig["options"]["printVersion"]->getBoolean())
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
      const char* cfgLogLevel =
         getApp()->getConfig()["app"]["logging"]["level"]->getString();
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
   clConfig["options"]->removeMember("printHelp");
   clConfig["options"]->removeMember("printVersion");

   return rval;
}

bool CommonAppPlugin::initializeLogging()
{
   bool rval = AppPlugin::initializeLogging();

   // get logging config
   Config cfg = getApp()->getConfig()["app"]["logging"];

   if(cfg["enabled"]->getBoolean())
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
         File f(logFile);
         FileLogger* fileLogger = new FileLogger();
         fileLogger->setFile(f, append);
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
            "Invalid app.logging.level.", "bitmunk.app.ConfigError");
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

      // NOTE: logging is now initialized.  use logging system after this point
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
