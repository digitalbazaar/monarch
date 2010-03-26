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
#include "monarch/rt/Platform.h"

#include "monarch/app/MonarchPlugin.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::io;
using namespace monarch::logging;
using namespace monarch::modest;
using namespace monarch::rt;

#define PLUGIN_NAME "monarch.app.Monarch"
#define PLUGIN_CL_CFG_ID PLUGIN_NAME ".commandLine"

MonarchPlugin::MonarchPlugin()
{
}

MonarchPlugin::~MonarchPlugin()
{
}

bool MonarchPlugin::initialize()
{
   bool rval = AppPlugin::initialize();
   if(getApp()->getMode() == App::BOOTSTRAP)
   {
      rval = rval && monarch::rt::Platform::initialize();
   }
   return rval;
}

void MonarchPlugin::cleanup()
{
   AppPlugin::cleanup();
   if(getApp()->getMode() == App::BOOTSTRAP)
   {
      monarch::rt::Platform::cleanup();
   }
}

bool MonarchPlugin::initConfigManager()
{
   // Set the known valid config manager versions.
   // Using explicit versions here. Code generating new configs should use
   // MO_DEFAULT_CONFIG_VERSION. Leave DEFAULT out here so that when the
   // default is changed it will quickly error out as a reminder to add the new
   // version.
   getApp()->getConfigManager()->addVersion(DB_CONFIG_VERSION_3_0);
   getApp()->getConfigManager()->addVersion(MO_CONFIG_VERSION_3_0);

   return true;
}

bool MonarchPlugin::willInitMetaConfig(Config& meta)
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

bool MonarchPlugin::initMetaConfig(Config& meta)
{
   bool rval = monarch::app::AppPlugin::initMetaConfig(meta);

   // empty defaults to ensure configs exist
   if(rval)
   {
      App::makeMetaConfig(
         meta, PLUGIN_NAME ".root.empty", "root");
      App::makeMetaConfig(
         meta, PLUGIN_NAME ".boot.empty", "boot");
      App::makeMetaConfig(
         meta, PLUGIN_NAME ".beforeDefaults.empty", "before defaults");
      App::makeMetaConfig(
         meta, PLUGIN_NAME ".defaults.empty", "defaults");
      App::makeMetaConfig(
         meta, PLUGIN_NAME ".afterDefaults.empty", "after defaults");
      App::makeMetaConfig(
         meta, PLUGIN_NAME ".commandLine.empty", "command line");
      App::makeMetaConfig(
         meta, PLUGIN_NAME ".main.empty", "main");
   }

   // defaults
   if(rval)
   {
      Config& c = App::makeMetaConfig(
         meta, PLUGIN_NAME ".defaults", "defaults")
            [ConfigManager::MERGE][PLUGIN_NAME];
      c["printHelp"] = false;
      c["printVersion"] = false;
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

DynamicObject MonarchPlugin::getCommandLineSpecs()
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
   Config cfg = getApp()->getMetaConfig()
      ["options"][PLUGIN_CL_CFG_ID][ConfigManager::MERGE][PLUGIN_NAME];

   opt = spec["options"]->append();
   opt["short"] = "-h";
   opt["long"] = "--help";
   opt["setTrue"]["root"] = cfg;
   opt["setTrue"]["path"] = "printHelp";

   opt = spec["options"]->append();
   opt["short"] = "-V";
   opt["long"] = "--version";
   opt["setTrue"]["root"] = cfg;
   opt["setTrue"]["path"] = "printVersion";

   DynamicObject specs = AppPlugin::getCommandLineSpecs();
   specs->append(spec);
   return specs;
}

bool MonarchPlugin::didParseCommandLine()
{
   bool rval = AppPlugin::didParseCommandLine();

   // process help and version flags
   // only done after bootstrap mode so that all modules help is available
   if(rval && getApp()->getMode() != App::BOOTSTRAP)
   {
      Config& cfg = getApp()->getMetaConfig()
         ["options"][PLUGIN_CL_CFG_ID][ConfigManager::MERGE][PLUGIN_NAME];

      if(rval && cfg["printHelp"]->getBoolean())
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
         // Raise known exit exception.
         ExceptionRef e = new Exception(
            "Help printed.",
            "monarch.app.Exit", EXIT_SUCCESS);
         Exception::set(e);
         rval = false;
      }
      if(rval && cfg["printVersion"]->getBoolean())
      {
         // TODO: allow other version info (modules, etc) via delegate?
         const char* version = getApp()->getVersion();
         printf("%s%s%s\n",
            getApp()->getName(),
            (version != NULL) ? " v" : "",
            (version != NULL) ? version : "");
         // Raise known exit exception.
         ExceptionRef e = new Exception(
            "Version printed.",
            "monarch.app.Exit", EXIT_SUCCESS);
         Exception::set(e);
         rval = false;
      }
   }

   return rval;
}

class MonarchPluginFactory :
   public AppPluginFactory
{
public:
   MonarchPluginFactory() :
      AppPluginFactory(PLUGIN_NAME, "1.0")
   {
   }

   virtual ~MonarchPluginFactory() {}

   virtual AppPluginRef createAppPlugin()
   {
      return new MonarchPlugin();
   }
};

Module*  monarch::app::createMonarchPluginFactory()
{
   return new MonarchPluginFactory();
}
