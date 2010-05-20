/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/AppConfig.h"

#include "monarch/app/App.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/logging/FileLogger.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::io;
using namespace monarch::logging;
using namespace monarch::rt;

#define MONARCH_APP          "monarch.app.App"
#define MONARCH_CONFIG       "monarch.app.Config"
#define MONARCH_LOGGING      "monarch.app.Logging"
#define MONARCH_KERNEL       "monarch.app.Kernel"
#define MONARCH_APP_CL       MONARCH_APP ".commandLine"
#define MONARCH_CONFIG_CL    MONARCH_CONFIG ".commandLine"
#define MONARCH_LOGGING_CL   MONARCH_LOGGING ".commandLine"
#define MONARCH_KERNEL_CL    MONARCH_KERNEL ".commandLine"
#define CMDLINE_ERROR       "monarch.app.CommandLineError"

AppConfig::AppConfig() :
   // FIXME: remove this once ref-counting is implemented in logging
   mLogger(NULL)
{
}

AppConfig::~AppConfig()
{
}

/**
 * Recursively traverse up the hierarchy to order parent configs first.
 */
// FIXME: probably deprecated
/*
static bool _orderIds(
   Config& meta, DynamicObject& ids, DynamicObject& seen,
   DynamicObject& visited, const char* id)
{
   bool rval = true;

   visited[id] = true;
   Config& c = meta["configs"][id];
   const char* parent =
      c->hasMember(ConfigManager::PARENT) ?
      c[ConfigManager::PARENT]->getString() :
      NULL;
   if(parent != NULL && !seen->hasMember(parent))
   {
      // parent not yet seen
      // find and load a config with id or group id of parent
      ConfigIterator i = meta["configs"].getIterator();
      bool found = false;
      while(!found && i->hasNext())
      {
         Config& next = i->next();
         const char* nextId = i->getName();
         if(!visited->hasMember(nextId) &&
            strcmp(nextId, id) != 0 &&
            (strcmp(nextId, parent) == 0 ||
            (next->hasMember(ConfigManager::GROUP) &&
               strcmp(next[ConfigManager::GROUP]->getString(), parent) == 0)))
         {
            found = true;
            rval = _orderIds(meta, ids, seen, visited, nextId);
            break;
         }
      }
      if(!found)
      {
         ExceptionRef e = new Exception(
            "Could not find parent config.", "monarch.app.ConfigError");
         e->getDetails()["id"] = id;
         e->getDetails()["parent"] = parent;
         Exception::push(e);
         rval = false;
      }
   }
   if(rval)
   {
      seen[id] = true;
      ids->append() = id;
   }

   return rval;
}
*/

/**
 * Sorts the configs in the meta config based on parent relationships and
 * loads them in the proper order.
 *
 * @return true on success, false on failure and exception set
 */
// FIXME: probably deprecated
/*
static bool _loadMetaConfigs(App* app)
{
   bool rval = true;

   Config meta = app->getMetaConfig();

   // ids to load in order
   DynamicObject ids;
   ids->setType(Array);
   DynamicObject seen;
   seen->setType(Map);

   // order all ids
   {
      ConfigIterator i = meta["configs"].getIterator();
      while(rval && i->hasNext())
      {
         i->next();
         const char* name = i->getName();
         if(!seen->hasMember(name))
         {
            // start with empty visited list
            DynamicObject visited;
            rval = _orderIds(meta, ids, seen, visited, name);
         }
      }
   }

   // load ids in order
   if(rval)
   {
      ConfigIterator i = ids.getIterator();
      while(rval && i->hasNext())
      {
         DynamicObject& configId = i->next();
         rval = app->getConfigManager()->addConfig(
            meta["configs"][configId->getString()]);
      }
   }

   return rval;
}
*/

static bool _initGeneralConfig(App* app)
{
   bool rval = true;

   ConfigManager* cm = app->getConfigManager();

   // create configuration tree structure
   rval =
      cm->addConfig(app->makeConfig(
         MONARCH_APP ".root.empty", "root",
         NULL)) &&
      cm->addConfig(app->makeConfig(
         MONARCH_APP ".boot.empty", "boot",
         "root")) &&
      cm->addConfig(app->makeConfig(
         MONARCH_APP ".beforeDefaults.empty", "before defaults",
         "boot")) &&
      cm->addConfig(app->makeConfig(
         MONARCH_APP ".defaults.empty", "defaults",
         "before defaults")) &&
      cm->addConfig(app->makeConfig(
         MONARCH_APP ".afterDefaults.empty", "after defaults",
         "defaults")) &&
      cm->addConfig(app->makeConfig(
         MONARCH_APP ".commandLine.empty", "command line",
         "after defaults")) &&
      cm->addConfig(app->makeConfig(
         MONARCH_APP ".main.empty", "main",
         "command line"));
   if(rval)
   {
      Config cfg = app->makeConfig(MONARCH_APP ".defaults", "defaults");
      Config& c = cfg[ConfigManager::MERGE][MONARCH_APP];

      // determines whether or not help/version should be printed
      c["printHelp"] = false;
      c["printVersion"] = false;
      c["debug"] = false;
      rval = cm->addConfig(cfg);

      // command line options
      if(rval)
      {
         Config cfg = app->makeConfig(MONARCH_APP_CL, "command line");
         app->getMetaConfig()["options"][MONARCH_APP_CL] = cfg;
         Config& cm = cfg[ConfigManager::MERGE][MONARCH_APP];
         cm->setType(Map);
      }
   }

   return rval;
}

static bool _initConfigConfig(App* app)
{
   bool rval = true;

   ConfigManager* cm = app->getConfigManager();
   Config cfg = app->makeConfig(MONARCH_CONFIG ".defaults", "defaults");
   Config& c = cfg[ConfigManager::MERGE][MONARCH_CONFIG];

   // configs is an array of files or dirs to load
   // other configs should append to this array
   c["configs"]->setType(Array);
   c["dump"] = false;
   c["dumpAll"] = false;
   c["dumpMeta"] = false;
   rval = cm->addConfig(cfg);

   // command line options
   if(rval)
   {
      Config cfg = app->makeConfig(MONARCH_CONFIG_CL, "command line");
      app->getMetaConfig()["options"][MONARCH_CONFIG_CL] = cfg;
      Config& ca = cfg[ConfigManager::APPEND][MONARCH_CONFIG];
      Config& cm = cfg[ConfigManager::MERGE][MONARCH_CONFIG];
      ca["config"]["configs"]->setType(Array);
      cm->setType(Map);
   }

   return rval;
}

static bool _initLoggingConfig(App* app)
{
   bool rval = true;

   ConfigManager* cm = app->getConfigManager();
   Config cfg = app->makeConfig(MONARCH_LOGGING ".defaults", "defaults");
   Config& c = cfg[ConfigManager::MERGE][MONARCH_LOGGING];

   // logging settings
   c["enabled"] = true;
   c["level"] = "warning";
   c["log"] = "-";
   c["append"] = true;
   c["rotationFileSize"] = (uint64_t)2000000;
   c["maxRotatedFiles"] = (uint32_t)10;
   c["gzip"] = true;
   c["location"] = false;
   c["color"] = false;
   c["delayOpen"] = false;
   rval = cm->addConfig(cfg);

   // command line options
   if(rval)
   {
      Config cfg = app->makeConfig(MONARCH_LOGGING_CL, "command line");
      app->getMetaConfig()["options"][MONARCH_LOGGING_CL] = cfg;
      Config& cm = cfg[ConfigManager::MERGE][MONARCH_LOGGING];
      cm->setType(Map);
   }

   return rval;
}

static bool _initKernelConfig(App* app)
{
   bool rval = true;

   ConfigManager* cm = app->getConfigManager();
   Config cfg = app->makeConfig(MONARCH_KERNEL ".defaults", "defaults");
   Config& c = cfg[ConfigManager::MERGE][MONARCH_KERNEL];

   // kernel settings
   // modulePath is an array of module paths
   c["modulePath"]->setType(Array);
   c["env"] = true;
   c["printModuleVersions"] = false;
   c["maxThreadCount"] = (uint32_t)100;
   c["maxConnectionCount"] = (uint32_t)100;
   // waitEvents is a map of arrays of event ids. The map keys should be
   // unique such as plugin ids. The kernel will wait for all these events
   // to occur before exiting. (Some special kernel events also can cause
   // a quicker exit.)
   c["waitEvents"]->setType(Map);
   rval = cm->addConfig(cfg);

   // command line options
   if(rval)
   {
      Config cfg = app->makeConfig(MONARCH_KERNEL_CL, "command line");
      app->getMetaConfig()["options"][MONARCH_KERNEL_CL] = cfg;
      Config& ca = cfg[ConfigManager::APPEND][MONARCH_KERNEL];
      Config& cm = cfg[ConfigManager::MERGE][MONARCH_KERNEL];
      ca["kernel"]["modulePath"]->setType(Array);
      cm->setType(Map);
   }

   return rval;
}

static bool _initConfigs(App* app)
{
   // FIXME: update ConfigManager to allow non-existent parents, etc.
   return (
      _initGeneralConfig(app) &&
      _initConfigConfig(app) &&
      _initLoggingConfig(app) &&
      _initKernelConfig(app));
}

static DynamicObject _getGeneralCmdLineSpec(App* app)
{
   DynamicObject spec;
   spec["help"] =
"Help options:\n"
"  -h, --help          Prints information on how to use the application.\n"
"\n"
"General options:\n"
"  -V, --version       Prints the software version.\n"
"      --              Treat all remaining options as application arguments.\n"
"\n";

   DynamicObject opt;
   Config options = app->getMetaConfig()["options"][MONARCH_APP_CL];
   Config& om = options[ConfigManager::MERGE][MONARCH_APP];

   opt = spec["options"]->append();
   opt["short"] = "-h";
   opt["long"] = "--help";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "printHelp";

   opt = spec["options"]->append();
   opt["short"] = "-V";
   opt["long"] = "--version";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "printVersion";

   return spec;
}

static DynamicObject _getConfigCmdLineSpec(App* app)
{
   DynamicObject spec;
   spec["help"] =
"Config options:\n"
"  -c, --config FILE   Load a configuration file or directory of files. May\n"
"                      be specified multiple times.\n"
"      --option NAME VALUE\n"
"                      Set dotted config path NAME to the string VALUE.\n"
"      --json-option NAME JSONVALUE\n"
"                      Set dotted config path NAME to the decoded JSONVALUE.\n"
"      --config-debug  Debug the configuration loading process to stdout.\n"
"      --config-dump   Dump main configuration to stdout.\n"
"      --config-dump-all\n"
"                      Dump the raw configuration storage to stdout.\n"
"      --config-dump-meta\n"
"                      Dump the raw meta configuration storage to stdout.\n"
"  -r, --resource-path PATH\n"
"                      The directory where application resource files were\n"
"                      installed.\n"
"                      Available in paths and configs as {RESOURCE_PATH}.\n"
"\n";

   DynamicObject opt;
   Config options = app->getMetaConfig()["options"][MONARCH_CONFIG_CL];
   Config& oa = options[ConfigManager::APPEND][MONARCH_CONFIG];
   Config& om = options[ConfigManager::MERGE][MONARCH_CONFIG];

   opt = spec["options"]->append();
   opt["short"] = "-h";
   opt["long"] = "--help";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "printHelp";

   opt = spec["options"]->append();
   opt["short"] = "-V";
   opt["long"] = "--version";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "printVersion";

   // FIXME: add option to add config includes to an unloaded command line
   // config (["include"]["config"] = the unloaded config to update)
   // FIXME: need to add an option to load AppPlugin configs that won't be
   // loaded until after the AppPlugin loads
   opt = spec["options"]->append();
   opt["short"] = "-c";
   opt["long"] = "--config";
   opt["append"] = oa["configs"];
   opt["argError"] = "No config file specified.";

   opt = spec["options"]->append();
   opt["long"] = "--option";
   opt["set"]["root"] = om;

   opt = spec["options"]->append();
   opt["long"] = "--json-option";
   opt["set"]["root"] = om;
   opt["isJsonValue"] = true;

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
   opt["short"] = "-r";
   opt["long"] = "--resource-path";
   opt["keyword"] = "RESOURCE_PATH";
   opt["argError"] = "No resource path specified.";

   return spec;
}

static DynamicObject _getLoggingCmdLineSpec(App* app)
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
"      --log-delay-open\n"
"                      Delay opening of the --log file until after configs are\n"
"                      loaded. Log messages will be queued. Can be used if the\n"
"                      log filename requires custom config variables.\n"
"                      (default: no delay)\n"
"\n";

   DynamicObject opt;
   Config options = app->getMetaConfig()["options"][MONARCH_LOGGING_CL];
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

   opt = spec["options"]->append();
   opt["long"] = "--log-delay-open";
   opt["setTrue"]["root"] = om;
   opt["setTrue"]["path"] = "delayOpen";

   return spec;
}

static DynamicObject _getKernelCmdLineSpec(App* app)
{
   DynamicObject spec;
   spec["help"] =
"Module options:\n"
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
   Config options = app->getMetaConfig()["options"][MONARCH_KERNEL_CL];
   Config& oa = options[ConfigManager::APPEND][MONARCH_KERNEL];
   Config& om = options[ConfigManager::MERGE][MONARCH_KERNEL];

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

static void _getCommandLineSpecs(App* app)
{
   DynamicObject meta = app->getMetaConfig();
   meta["specs"]->append() = _getGeneralCmdLineSpec(app);
   meta["specs"]->append() = _getConfigCmdLineSpec(app);
   meta["specs"]->append() = _getLoggingCmdLineSpec(app);
   meta["specs"]->append() = _getKernelCmdLineSpec(app);
}

bool AppConfig::initialize(App* app)
{
   bool rval = _initConfigs(app);
   if(rval)
   {
      _getCommandLineSpecs(app);
   }
   return rval;
}

bool AppConfig::loadCommandLineConfigs(App* app, bool plugin)
{
   bool rval = true;

   // if debug mode is on, add debug logger
   Logger* debugLogger = NULL;
   Config cfg = app->getConfig();
   if(cfg[MONARCH_APP]["debug"]->getBoolean())
   {
      OutputStream* os = new FileOutputStream(FileOutputStream::StdOut);
      debugLogger = new OutputStreamLogger(os, true);

      // default to warnings, can use command line option to be more verbose
      Logger::Level logLevel;
      const char* levelStr = cfg[MONARCH_LOGGING]["level"]->getString();
      bool found = Logger::stringToLevel(levelStr, logLevel);
      if(!found || logLevel < Logger::Warning)
      {
         logLevel = Logger::Warning;
      }
      debugLogger->setLevel(logLevel);
      Logger::addLogger(debugLogger);
   }

   // FIXME: add configs from command line as includes
   // FIXME: add option to command line spec for unloaded config?
   // FIXME: add option for plugin configs (delayed
   // loading until app plugin is ready)
   /*
      DynamicObjectIterator i =
         cfg[ConfigManager::APPEND][PLUGIN_NAME]["configs"].getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         Config& inc = cfg[ConfigManager::INCLUDE]->append();
         inc["path"] = next->getString();
         inc["load"] = true;
         inc["optional"] = false;
         inc["includeSubdirectories"] = true;
      }
   */

   ConfigManager* cm = app->getConfigManager();

   // add the command line config for the plugin
   if(plugin)
   {
      Config cfg = app->getMetaConfig()["pluginOptions"];
      rval = cm->addConfig(cfg);
   }
   // add the builtin command line configs
   else
   {
      ConfigIterator i = app->getMetaConfig()["options"].getIterator();
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
// FIXME: from old LoggingPlugin
static bool _loggingToStdOut(Config& cfg)
{
   return cfg->hasMember("log") && strcmp(cfg["log"]->getString(), "-") == 0;
}

/**
 * Check if logging is in delayed mode.
 *
 * @param cfg the current logging config
 *
 * @return true if logging is delayed.
 */
// FIXME: from old LoggingPlugin
static bool _delayed(Config& cfg)
{
   return cfg->hasMember("delayOpen") && cfg["delayOpen"]->getBoolean();
}

bool AppConfig::configureLogging(App* app)
{
   // get logging config
   Config cfg = app->getConfig()[MONARCH_LOGGING];
/*
   // NOTE: might need to get parent config in non-bootstrap mode to ensure
   // consistent config for the setup below.  non-bootstrap configs could
   // overwrite these options and cause havoc.

   FileLogger* fileLogger = NULL;
   bool bootstrap = getApp()->getMode() == App::BOOTSTRAP;
   bool delayed = _delayed(cfg);
   bool enabled = cfg["enabled"]->getBoolean();
   bool stdoutlog = !delayed && _loggingToStdOut(cfg);

   // setup logger when enabled and in bootstrap mode
   if(rval && enabled && bootstrap)
   {
      if(stdoutlog)
      {
         OutputStream* logStream = new FileOutputStream(
            FileOutputStream::StdOut);
         mLogger = new OutputStreamLogger(logStream, true);
      }
      else
      {
         fileLogger = new FileLogger();
         if(delayed)
         {
            // set arbitrary 32k log size in delayed mode
            // can add cmd line option to set this if needed
            // FIXME: would be better if this were dynamic with a max to avoid
            // FIXME: over or under allocation.
            rval = fileLogger->setInMemoryLog(32*1024);
         }
         if(rval)
         {
            mLogger = fileLogger;
         }
         else
         {
            delete fileLogger;
            fileLogger = NULL;
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
      }
   }

   // setup new logging file if and when needed
   if(rval && enabled &&
      ((bootstrap && !delayed && !stdoutlog) || (!bootstrap && delayed)))
   {
      const char* logFile = cfg["log"]->getString();
      string expandedLogFile;
      // attempt to expand "~", else just use plain value
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
         // if delayed, get the real file logger
         // otherwise we're in bootstrap mode and it's already set
         if(delayed)
         {
            fileLogger =
               getApp()->getParentApp()->getLoggingBuiltin()->getFileLogger();
         }

         // sanity check in case fileLogger not set yet
         if(fileLogger)
         {
            bool append = cfg["append"]->getBoolean();
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
            }
         }
      }
   }

   // add logger after setup complete when in bootstrap mode
   if(rval && enabled)
   {
      if(bootstrap)
      {
         Logger::addLogger(mLogger);

         // NOTE: Logging is now initialized. Use standard logging system after
         // NOTE: this point.
         MO_CAT_DEBUG(MO_LOGGING_CAT, "%s initialized.",
            delayed ? " Delayed logging" : "Logging");
      }
      else if(delayed)
      {
         // logging now fully setup
         MO_CAT_DEBUG(MO_LOGGING_CAT, "Logging fully initialized.");
      }
   }
*/
   return true;
}

void AppConfig::cleanupLogging(App* app)
{
   if(mLogger != NULL)
   {
      Logger::removeLogger(mLogger);
      delete mLogger;
      mLogger = NULL;
   }
}
