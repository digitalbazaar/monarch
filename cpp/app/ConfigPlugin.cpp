/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include <cstdlib>

#include "monarch/app/App.h"
#include "monarch/app/AppPluginFactory.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/File.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/logging/FileLogger.h"
#include "monarch/logging/OutputStreamLogger.h"

#include "monarch/app/ConfigPlugin.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::logging;
using namespace monarch::modest;
using namespace monarch::rt;

#define PLUGIN_NAME "monarch.app.Config"
#define PLUGIN_CL_CFG_ID PLUGIN_NAME ".commandLine"

ConfigPlugin::ConfigPlugin() :
   mDebugLogger(NULL)
{
}

ConfigPlugin::~ConfigPlugin()
{
}

bool ConfigPlugin::initMetaConfig(Config& meta)
{
   bool rval = monarch::app::AppPlugin::initMetaConfig(meta);

   // defaults
   if(rval)
   {
      Config& c =
         App::makeMetaConfig(meta, PLUGIN_NAME ".defaults", "defaults")
            [ConfigManager::MERGE][PLUGIN_NAME];
      // configs is a map of arrays
      // Keys are unique per module path source to allow for configured other
      // lists of paths. Values are arrays of files or dirs to load.
      c["configs"]->setType(Map);
      c["debug"] = false;
      c["dump"] = false;
      c["dumpAll"] = false;
      c["dumpMeta"] = false;
      c["keywords"]->setType(Map);
   }

   // command line options
   if(rval)
   {
      Config& c = App::makeMetaConfig(
         meta, PLUGIN_CL_CFG_ID, "command line", "options")
            [ConfigManager::MERGE][PLUGIN_NAME];
      c["configs"][PLUGIN_CL_CFG_ID]->setType(Array);
      c["keywords"]->setType(Map);
   }

   return rval;
}

DynamicObject ConfigPlugin::getCommandLineSpecs()
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
   Config& options = getApp()->getMetaConfig()
      ["options"][PLUGIN_CL_CFG_ID][ConfigManager::MERGE];
   Config& configOptions = options[PLUGIN_NAME];

   opt = spec["options"]->append();
   opt["short"] = "-c";
   opt["long"] = "--config";
   opt["append"] = configOptions["configs"];
   opt["argError"] = "No config file specified.";

   opt = spec["options"]->append();
   opt["long"] = "--option";
   opt["set"]["root"] = options;

   opt = spec["options"]->append();
   opt["long"] = "--json-option";
   opt["set"]["root"] = options;
   opt["isJsonValue"] = true;

   opt = spec["options"]->append();
   opt["long"] = "--config-debug";
   opt["setTrue"]["root"] = configOptions;
   opt["setTrue"]["path"] = "debug";

   opt = spec["options"]->append();
   opt["long"] = "--config-dump";
   opt["setTrue"]["root"] = configOptions;
   opt["setTrue"]["path"] = "dump";

   opt = spec["options"]->append();
   opt["long"] = "--config-dump-all";
   opt["setTrue"]["root"] = configOptions;
   opt["setTrue"]["path"] = "dumpAll";

   opt = spec["options"]->append();
   opt["long"] = "--config-dump-meta";
   opt["setTrue"]["root"] = configOptions;
   opt["setTrue"]["path"] = "dumpMeta";

   opt = spec["options"]->append();
   opt["short"] = "-r";
   opt["long"] = "--resource-path";
   opt["arg"]["root"] = configOptions["keywords"];
   opt["arg"]["path"] = "RESOURCE_PATH";
   opt["argError"] = "No resource path specified.";

   DynamicObject specs = AppPlugin::getCommandLineSpecs();
   specs->append(spec);
   return specs;
}

bool ConfigPlugin::didParseCommandLine()
{
   bool rval = AppPlugin::didParseCommandLine();

   // setup to load configs from the command line via includes
   if(rval)
   {
      // add includes to options config
      Config& cfg = getApp()->getMetaConfig()["options"][PLUGIN_CL_CFG_ID];

      DynamicObjectIterator i =
         cfg[ConfigManager::MERGE][PLUGIN_NAME]["configs"].getIterator();
      while(i->hasNext())
      {
         DynamicObjectIterator ci = i->next().getIterator();
         while(ci->hasNext())
         {
            DynamicObject& next = ci->next();
            Config& inc = cfg[ConfigManager::INCLUDE]->append();
            inc["path"] = next->getString();
            inc["load"] = true;
            inc["optional"] = false;
            inc["includeSubdirectories"] = true;
         }
      }
   }

   return rval;
}

bool ConfigPlugin::willLoadConfigs()
{
   bool rval = AppPlugin::willLoadConfigs();

   // configs to be loaded are put into the command line options config
   Config& options = getApp()->getMetaConfig()
      ["options"][PLUGIN_CL_CFG_ID][ConfigManager::MERGE][PLUGIN_NAME];

   if(rval)
   {
      // setup keywords from options config
      Config& keywords = options["keywords"];
      DynamicObjectIterator i = keywords.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         getApp()->getConfigManager()->setKeyword(
            i->getName(), next->getString());
      }
   }

   if(rval)
   {
      // special config logger
      // used only during configuration
      OutputStream* logStream =
         new FileOutputStream(FileOutputStream::StdOut);
      mDebugLogger = new OutputStreamLogger(logStream, true);

      // default to warnings, can use command line option to be more verbose
      Logger::Level logLevel = Logger::Warning;
      if(options->hasMember("debug") && options["debug"]->getBoolean())
      {
         logLevel = Logger::Debug;
      }
      mDebugLogger->setLevel(logLevel);
      Logger::addLogger(mDebugLogger);
   }

   return rval;
}

bool ConfigPlugin::didLoadConfigs()
{
   bool rval = AppPlugin::didLoadConfigs();

   if(rval)
   {
      if(mDebugLogger != NULL)
      {
         Logger::removeLogger(mDebugLogger);
         delete mDebugLogger;
      }
   }

   return rval;
}

bool ConfigPlugin::run()
{
   bool rval = AppPlugin::run();

   if(rval && getApp()->getMode() != App::BOOTSTRAP)
   {
      if(getApp()->getConfig()[PLUGIN_NAME]["dump"]->getBoolean())
      {
         JsonWriter::writeToStdOut(getApp()->getConfig());
      }
      if(getApp()->getConfig()[PLUGIN_NAME]["dumpAll"]->getBoolean())
      {
         JsonWriter::writeToStdOut(
            getApp()->getConfigManager()->getDebugInfo());
      }
      if(getApp()->getConfig()[PLUGIN_NAME]["dumpMeta"]->getBoolean())
      {
         JsonWriter::writeToStdOut(getApp()->getMetaConfig());
      }
   }

   return rval;
}

class ConfigPluginFactory :
   public AppPluginFactory
{
public:
   ConfigPluginFactory() :
      AppPluginFactory(PLUGIN_NAME, "1.0")
   {
      addDependency("monarch.app.Monarch", "1.0");
   }

   virtual ~ConfigPluginFactory() {}

   virtual AppPluginRef createAppPlugin()
   {
      return new ConfigPlugin();
   }
};

Module* monarch::app::createConfigPluginFactory()
{
   return new ConfigPluginFactory();
}
