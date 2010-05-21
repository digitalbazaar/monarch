/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/AppConfig.h"

#include "monarch/app/AppRunner.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/logging/FileLogger.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::io;
using namespace monarch::logging;
using namespace monarch::rt;

#define MONARCH_CORE         "monarch.app.Core"
#define MONARCH_CONFIG       "monarch.app.Config"
#define MONARCH_LOGGING      "monarch.app.Logging"
#define MONARCH_KERNEL       "monarch.app.Kernel"
#define MONARCH_CORE_CL      MONARCH_CORE ".commandLine"
#define MONARCH_CONFIG_CL    MONARCH_CONFIG ".commandLine"
#define MONARCH_LOGGING_CL   MONARCH_LOGGING ".commandLine"
#define MONARCH_KERNEL_CL    MONARCH_KERNEL ".commandLine"
#define CMDLINE_ERROR        "monarch.app.CommandLineError"
#define CONFIG_ERROR         "monarch.app.ConfigError"

AppConfig::AppConfig() :
   // FIXME: remove this once ref-counting is implemented in logging
   mLogger(NULL)
{
}

AppConfig::~AppConfig()
{
}

static bool _initGeneralConfig(AppRunner* ar)
{
   bool rval = true;

   ConfigManager* cm = ar->getConfigManager();

   // create configuration tree structure
   rval =
      cm->addConfig(ar->makeConfig(
         MONARCH_CORE ".root.empty", "root",
         NULL)) &&
      cm->addConfig(ar->makeConfig(
         MONARCH_CORE ".boot.empty", "boot",
         "root")) &&
      cm->addConfig(ar->makeConfig(
         MONARCH_CORE ".beforeDefaults.empty", "before defaults",
         "boot")) &&
      cm->addConfig(ar->makeConfig(
         MONARCH_CORE ".defaults.empty", "defaults",
         "before defaults")) &&
      cm->addConfig(ar->makeConfig(
         MONARCH_CORE ".afterDefaults.empty", "after defaults",
         "defaults")) &&
      cm->addConfig(ar->makeConfig(
         MONARCH_CORE ".commandLine.empty", "command line",
         "after defaults")) &&
      cm->addConfig(ar->makeConfig(
         MONARCH_CORE ".main.empty", "main",
         "command line"));
   if(rval)
   {
      Config cfg = ar->makeConfig(MONARCH_CORE ".defaults", "defaults");
      Config& c = cfg[ConfigManager::MERGE][MONARCH_CORE];

      // general application options
      // use home environment var if available
      const char* home = getenv("MONARCH_APP_HOME");
      if(home == NULL)
      {
         home = "~";
      }
      c["home"] = home;
      c["printHelp"] = false;
      c["printVersion"] = false;
      rval = cm->addConfig(cfg);

      // command line options
      if(rval)
      {
         Config cfg = ar->makeConfig(MONARCH_CORE_CL, "command line");
         ar->getMetaConfig()["options"][MONARCH_CORE_CL] = cfg;
         Config& cm = cfg[ConfigManager::MERGE][MONARCH_CORE];
         cm->setType(Map);
      }
   }

   return rval;
}

static bool _initConfigConfig(AppRunner* ar)
{
   bool rval = true;

   ConfigManager* cm = ar->getConfigManager();
   Config cfg = ar->makeConfig(MONARCH_CONFIG ".defaults", "defaults");
   Config& c = cfg[ConfigManager::MERGE][MONARCH_CONFIG];

   // config options
   c["debug"] = false;
   c["dump"] = false;
   c["dumpAll"] = false;
   c["dumpMeta"] = false;
   rval = cm->addConfig(cfg);

   // command line options
   if(rval)
   {
      Config cfg = ar->makeConfig(MONARCH_CONFIG_CL, "command line");
      ar->getMetaConfig()["options"][MONARCH_CONFIG_CL] = cfg;
      Config& cm = cfg[ConfigManager::MERGE][MONARCH_CONFIG];
      Config& ca = cfg[ConfigManager::APPEND][MONARCH_CONFIG];
      cm->setType(Map);
      ca["appConfigs"]->setType(Array);
   }

   return rval;
}

static bool _initLoggingConfig(AppRunner* ar)
{
   bool rval = true;

   ConfigManager* cm = ar->getConfigManager();
   Config cfg = ar->makeConfig(MONARCH_LOGGING ".defaults", "defaults");
   Config& c = cfg[ConfigManager::MERGE][MONARCH_LOGGING];

   // logging settings
   c["enabled"] = true;
   c["level"] = "warning";
   c["log"] = "-";
   c["logHome"] = "-";
   c["append"] = true;
   c["rotationFileSize"] = (uint64_t)2000000;
   c["maxRotatedFiles"] = (uint32_t)10;
   c["gzip"] = true;
   c["location"] = false;
   c["color"] = false;
   rval = cm->addConfig(cfg);

   // command line options
   if(rval)
   {
      Config cfg = ar->makeConfig(MONARCH_LOGGING_CL, "command line");
      ar->getMetaConfig()["options"][MONARCH_LOGGING_CL] = cfg;
      Config& cm = cfg[ConfigManager::MERGE][MONARCH_LOGGING];
      cm->setType(Map);
   }

   return rval;
}

static bool _initKernelConfig(AppRunner* ar)
{
   bool rval = true;

   ConfigManager* cm = ar->getConfigManager();
   Config cfg = ar->makeConfig(MONARCH_KERNEL ".defaults", "defaults");
   Config& c = cfg[ConfigManager::MERGE][MONARCH_KERNEL];

   // kernel settings
   // path to app
   c["appPath"] = "";
   // modulePath is an array of module paths
   c["modulePath"]->setType(Array);
   c["env"] = true;
   c["printModuleVersions"] = false;
   c["maxThreadCount"] = (uint32_t)100;
   c["maxConnectionCount"] = (uint32_t)100;
   rval = cm->addConfig(cfg);

   // command line options
   if(rval)
   {
      Config cfg = ar->makeConfig(MONARCH_KERNEL_CL, "command line");
      ar->getMetaConfig()["options"][MONARCH_KERNEL_CL] = cfg;
      Config& ca = cfg[ConfigManager::APPEND][MONARCH_KERNEL];
      Config& cm = cfg[ConfigManager::MERGE][MONARCH_KERNEL];
      ca["modulePath"]->setType(Array);
      cm->setType(Map);
   }

   return rval;
}

static bool _initConfigs(AppRunner* ar)
{
   return (
      _initGeneralConfig(ar) &&
      _initConfigConfig(ar) &&
      _initLoggingConfig(ar) &&
      _initKernelConfig(ar));
}

static DynamicObject _getGeneralCmdLineSpec(AppRunner* ar)
{
   // FIXME: build auto-formatting into command line spec
   DynamicObject spec;
   spec["help"] =
"Help options:\n"
"  -h, --help          Prints information on how to use the application.\n"
"\n"
"General options:\n"
"      --home          Sets the home directory for the application\n"
"  -V, --version       Prints the software version.\n"
"      --              Treat all remaining options as application arguments.\n"
"\n";

   DynamicObject opt;
   Config options = ar->getMetaConfig()["options"][MONARCH_CORE_CL];
   Config& om = options[ConfigManager::MERGE][MONARCH_CORE];

   opt = spec["options"]->append();
   opt["short"] = "-h";
   opt["long"] = "--help";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "printHelp";

   opt = spec["options"]->append();
   opt["long"] = "--home";
   opt["arg"]["root"] = om;
   opt["arg"]["path"] = "home";

   opt = spec["options"]->append();
   opt["short"] = "-V";
   opt["long"] = "--version";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "printVersion";

   return spec;
}

static DynamicObject _getConfigCmdLineSpec(AppRunner* ar)
{
   DynamicObject spec;
   spec["help"] =
"Config options:\n"
"      --config FILE   Load a configuration file or directory of files that\n"
"                      will be loaded immediately on startup. May be\n"
"                      specified multiple times.\n"
"      --config-include FILE\n"
"                      Load a configuration file or directory of files that\n"
"                      will be included after the App loads. May be specified\n"
"                      multiple times.\n"
"      --option NAME=VALUE\n"
"                      Set dotted config path NAME to the string VALUE.\n"
"      --json-option NAME=JSONVALUE\n"
"                      Set dotted config path NAME to the decoded JSONVALUE.\n"
"      --config-debug  Debug the configuration loading process to stdout.\n"
"      --config-dump   Dump main configuration to stdout.\n"
"      --config-dump-all\n"
"                      Dump the raw configuration storage to stdout.\n"
"      --config-dump-meta\n"
"                      Dump the raw meta configuration storage to stdout.\n"
"      --config-keyword KEYWORD=VALUE\n"
"                      Sets the value of a configuration keyword.\n"
"\n";

   DynamicObject opt;
   Config options = ar->getMetaConfig()["options"][MONARCH_CONFIG_CL];
   Config& om = options[ConfigManager::MERGE][MONARCH_CONFIG];
   Config& oa = options[ConfigManager::APPEND][MONARCH_CONFIG];

   opt = spec["options"]->append();
   opt["long"] = "--config";
   opt["include"]["config"] =
      ar->getMetaConfig()["options"][MONARCH_CONFIG_CL];
   opt["argError"] = "No config file specified.";

   opt = spec["options"]->append();
   opt["long"] = "--config-include";
   opt["append"] = oa["appConfigs"];
   opt["argError"] = "No app config file specified.";

   opt = spec["options"]->append();
   opt["long"] = "--option";
   opt["set"]["root"] = options[ConfigManager::MERGE];
   opt["argError"] = "No config key=value specified.";

   opt = spec["options"]->append();
   opt["long"] = "--json-option";
   opt["set"]["root"] = options[ConfigManager::MERGE];
   opt["isJsonValue"] = true;
   opt["argError"] = "No config key=json specified.";

   opt = spec["options"]->append();
   opt["long"] = "--config-debug";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "debug";

   opt = spec["options"]->append();
   opt["long"] = "--config-dump";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "dump";

   opt = spec["options"]->append();
   opt["long"] = "--config-dump-all";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "dumpAll";

   opt = spec["options"]->append();
   opt["long"] = "--config-dump-meta";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "dumpMeta";

   opt = spec["options"]->append();
   opt["long"] = "--config-keyword";
   opt["set"]["keyword"] = true;
   opt["argError"] = "No keyword value specified.";

   return spec;
}

static DynamicObject _getLoggingCmdLineSpec(AppRunner* ar)
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
"      --log LOG       Set log file. Use \"-\" for stdout. (default: \"-\")\n"
"      --log-home LOG  Write log file to the application's home directory.\n"
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
   Config options = ar->getMetaConfig()["options"][MONARCH_LOGGING_CL];
   Config& om = options[ConfigManager::MERGE][MONARCH_LOGGING];

   opt = spec["options"]->append();
   opt["long"] = "--no-log";
   opt["setFalse"]["root"] = om;
   opt["setFalse"]["path"] = "enabled";

   opt = spec["options"]->append();
   opt["long"] = "--log-level";
   opt["arg"]["root"] = om;
   opt["arg"]["path"] = "level";
   opt["argError"] = "No log level specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log";
   opt["arg"]["root"] = om;
   opt["arg"]["path"] = "log";
   opt["argError"] = "No log file specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-home";
   opt["arg"]["root"] = om;
   opt["arg"]["path"] = "logHome";
   opt["argError"] = "No log file specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-overwrite";
   opt["setFalse"]["root"] = om;
   opt["setFalse"]["path"] = "append";

   opt = spec["options"]->append();
   opt["long"] = "--log-rotation-size";
   opt["arg"]["root"] = om;
   opt["arg"]["path"] = "rotationFileSize";
   opt["argError"] = "No rotation size specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-max-rotated";
   opt["arg"]["root"] = om;
   opt["arg"]["path"] = "maxRotatedFiles";
   opt["argError"] = "Max rotated files not specified.";

   opt = spec["options"]->append();
   opt["long"] = "--log-gzip";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "gzip";

   opt = spec["options"]->append();
   opt["long"] = "--log-no-gzip";
   opt["setFalse"]["root"] = om;
   opt["setFalse"]["path"] = "gzip";

   opt = spec["options"]->append();
   opt["long"] = "--log-location";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "location";

   opt = spec["options"]->append();
   opt["long"] = "--log-color";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "color";

   opt = spec["options"]->append();
   opt["long"] = "--log-no-color";
   opt["setFalse"]["root"] = om;
   opt["setFalse"]["path"] = "color";

   return spec;
}

static DynamicObject _getKernelCmdLineSpec(AppRunner* ar)
{
   DynamicObject spec;
   spec["help"] =
"Module options:\n"
"  -a, --app FILE      The filename for a module containing an AppFactory\n"
"                      that can create the custom App.\n"
"  -m, --module-path PATH\n"
"                      A colon separated list of modules or directories where\n"
"                      modules are stored. May be specified multiple times.\n"
"                      Loaded after modules in MONARCH_MODULE_PATH.\n"
"      --no-module-path-env\n"
"                      Disable MONARCH_MODULE_PATH.\n"
"      --module-versions\n"
"                      Prints the module versions.\n"
"\n";

   DynamicObject opt;
   Config options = ar->getMetaConfig()["options"][MONARCH_KERNEL_CL];
   Config& oa = options[ConfigManager::APPEND][MONARCH_KERNEL];
   Config& om = options[ConfigManager::MERGE][MONARCH_KERNEL];

   opt = spec["options"]->append();
   opt["short"] = "-a";
   opt["long"] = "--app";
   opt["arg"]["root"] = om;
   opt["arg"]["path"] = "appPath";
   opt["argError"] = "No AppFactory module filename specified.";

   opt = spec["options"]->append();
   opt["short"] = "-m";
   opt["long"] = "--module-path";
   opt["append"] = oa["modulePath"];
   opt["argError"] = "No module path specified.";

   opt = spec["options"]->append();
   opt["long"] = "--no-module-path-env";
   opt["setFalse"]["root"] = om;
   opt["setFalse"]["path"] = "env";

   opt = spec["options"]->append();
   opt["long"] = "--module-versions";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "printModuleVersions";

   return spec;
}

static void _getCommandLineSpecs(AppRunner* ar)
{
   DynamicObject meta = ar->getMetaConfig();
   meta["specs"]->append() = _getGeneralCmdLineSpec(ar);
   meta["specs"]->append() = _getConfigCmdLineSpec(ar);
   meta["specs"]->append() = _getLoggingCmdLineSpec(ar);
   meta["specs"]->append() = _getKernelCmdLineSpec(ar);
}

bool AppConfig::initialize(AppRunner* ar)
{
   bool rval = _initConfigs(ar);
   if(rval)
   {
      _getCommandLineSpecs(ar);
   }
   return rval;
}

bool AppConfig::loadCommandLineConfigs(AppRunner* ar, bool appConfigs)
{
   bool rval = true;

   // get meta config
   DynamicObject meta = ar->getMetaConfig();

   bool debug = false;
   string levelStr;
   if(appConfigs)
   {
      // configs loaded, read from main config
      Config cfg = ar->getConfig();
      debug = cfg[MONARCH_CONFIG]["debug"]->getBoolean();
      levelStr = cfg[MONARCH_LOGGING]["level"]->getString();
   }
   else
   {
      // configs not yet loaded, read from meta config
      DynamicObject& options = meta["options"];
      Config cfg = options[MONARCH_CONFIG_CL][ConfigManager::MERGE];
      if(cfg[MONARCH_CONFIG]->hasMember("debug"))
      {
         debug = cfg[MONARCH_CONFIG]["debug"]->getBoolean();
      }
      cfg = options[MONARCH_LOGGING_CL][ConfigManager::MERGE];
      if(cfg[MONARCH_LOGGING]->hasMember("level"))
      {
         levelStr = cfg[MONARCH_LOGGING]->getString();
      }
   }

   // if debug mode is on, add debug logger
   Logger* debugLogger = NULL;
   if(debug)
   {
      OutputStream* os = new FileOutputStream(FileOutputStream::StdOut);
      debugLogger = new OutputStreamLogger(os, true);

      // default to warnings, can use command line option to be more verbose
      Logger::Level logLevel;
      bool found = Logger::stringToLevel(levelStr.c_str(), logLevel);
      if(!found || logLevel < Logger::Warning)
      {
         logLevel = Logger::Warning;
      }
      debugLogger->setLevel(logLevel);
      Logger::addLogger(debugLogger);
   }

   // add the command line config for the app
   ConfigManager* cm = ar->getConfigManager();
   if(appConfigs)
   {
      DynamicObject& options = meta["appOptions"];

      // add app configs as includes
      Config cfg = ar->getConfig()[MONARCH_CONFIG]["appConfigs"];
      ConfigIterator ci = cfg.getIterator();
      while(ci->hasNext())
      {
         Config& inc = options[ConfigManager::INCLUDE]->append();
         inc["path"] = ci->next();
         inc["load"] = true;
         inc["optional"] = false;
         inc["includeSubdirectories"] = true;
      }
      rval = cm->addConfig(options);
   }
   // add the builtin command line configs
   else
   {
      ConfigIterator i = meta["options"].getIterator();
      while(rval && i->hasNext())
      {
         Config& next = i->next();
         rval = cm->addConfig(next);
      }
   }

   // clean up debug logger
   if(debugLogger != NULL)
   {
      Logger::removeLogger(debugLogger);
      delete debugLogger;
   }

   return rval;
}

/**
 * Check if log file is special stdout "-" value.
 *
 * @param cfg the current logging config
 *
 * @return true if logging to stdout.
 */
static bool _loggingToStdOut(Config& cfg)
{
   return
      strcmp(cfg["log"]->getString(), "-") == 0 &&
      strcmp(cfg["logHome"]->getString(), "-") == 0;
}

bool AppConfig::configureLogging(AppRunner* ar)
{
   bool rval = true;

   // setup logger if logging is enabled
   Config cfg = ar->getConfig()[MONARCH_LOGGING];
   if(cfg["enabled"]->getBoolean())
   {
      // determine if logging to file or stdout
      FileLogger* fileLogger = NULL;
      bool stdoutlog = _loggingToStdOut(cfg);
      if(stdoutlog)
      {
         OutputStream* os = new FileOutputStream(FileOutputStream::StdOut);
         mLogger = new OutputStreamLogger(os, true);
      }
      else
      {
         // determine if writing to app home dir or not
         string logFile;
         bool logHome = (cfg["logHome"]->length() > 0);
         if(logHome)
         {
            // prepend home dir
            logFile = File::join(
               ar->getConfig()[MONARCH_CORE]["home"]->getString(),
               cfg["logHome"]->getString());
         }
         else
         {
            // get log file
            logFile = cfg["log"]->getString();
         }

         // expand non-absolute paths (handle "~", relative path)
         if(!File::isPathAbsolute(logFile.c_str()))
         {
            rval = File::expandUser(logFile.c_str(), logFile);
         }

         if(rval)
         {
            // create file logger and set file
            File file(logFile.c_str());
            bool append = cfg["append"]->getBoolean();
            mLogger = fileLogger = new FileLogger();
            rval = fileLogger->setFile(file, append);
            if(!rval)
            {
               // setting file failed, clean up logger
               delete mLogger;
               mLogger = NULL;
            }
            else
            {
               // handle log rotation
               if(cfg["gzip"]->getBoolean())
               {
                  fileLogger->setFlags(FileLogger::GzipCompressRotatedLogs);
               }
               fileLogger->setRotationFileSize(
                  cfg["rotationFileSize"]->getUInt64());
               fileLogger->setMaxRotatedFiles(
                  cfg["maxRotatedFiles"]->getUInt32());
            }
         }
      }

      // set logging level, flags
      if(rval)
      {
         // FIXME: add cfg options for logging options
         //logger.setDateFormat("%H:%M:%S");
         //logger.setFlags(Logger::LogThread);
         Logger::Level logLevel;
         const char* levelStr = cfg["level"]->getString();
         if(Logger::stringToLevel(levelStr, logLevel))
         {
            mLogger->setLevel(logLevel);
         }
         else
         {
            ExceptionRef e = new Exception(
               "Invalid monarch.logging.level.",
               CONFIG_ERROR);
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
      }

      // add logger after setup complete
      if(rval)
      {
         // FIXME: add cfg option to pick categories to log
         //Logger::addLogger(&mLogger, BM_..._CAT);
         Logger::addLogger(mLogger);
         MO_CAT_DEBUG(MO_LOGGING_CAT, "Logging initialized.");
      }
   }

   return rval;
}

void AppConfig::cleanupLogging(AppRunner* ar)
{
   if(mLogger != NULL)
   {
      Logger::removeLogger(mLogger);
      delete mLogger;
      mLogger = NULL;
   }
}
