/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/DummyAppPlugin.h"

using namespace std;
using namespace db::app;
using namespace db::config;
using namespace db::rt;

DummyAppPlugin::DummyAppPlugin()
{
   mInfo["id"] = "db.app.plugin.Dummy";
   //mInfo["dependencies"]->append() = "db.app.plugins.Common";
}

DummyAppPlugin::~DummyAppPlugin()
{
   mApp = NULL;
}

bool DummyAppPlugin::willAddToApp(App* app)
{
   bool rval = AppPlugin::willAddToApp(app);
   return rval;
}

bool DummyAppPlugin::didAddToApp(App* app)
{
   bool rval = AppPlugin::didAddToApp(app);
   return rval;
}

bool DummyAppPlugin::initConfigManager()
{
   bool rval = AppPlugin::initConfigManager();
   return rval;
}

bool DummyAppPlugin::willInitMetaConfig(Config& meta)
{
   bool rval = AppPlugin::willInitMetaConfig(meta);
   return rval;
}

bool DummyAppPlugin::initMetaConfig(Config& meta)
{
   bool rval = AppPlugin::initMetaConfig(meta);
   return rval;
}

bool DummyAppPlugin::didInitMetaConfig(Config& meta)
{
   bool rval = AppPlugin::didInitMetaConfig(meta);
   return rval;
}

DynamicObject DummyAppPlugin::getCommandLineSpecs()
{
   DynamicObject rval = AppPlugin::getCommandLineSpecs();
   return rval;
}

bool DummyAppPlugin::willLoadConfigs()
{
   bool rval = AppPlugin::willLoadConfigs();
   return rval;
}

bool DummyAppPlugin::didLoadConfigs()
{
   bool rval = AppPlugin::didLoadConfigs();
   return rval;
}

bool DummyAppPlugin::willParseCommandLine(std::vector<const char*>* args)
{
   bool rval = AppPlugin::willParseCommandLine(args);
   return rval;
}

bool DummyAppPlugin::didParseCommandLine()
{
   bool rval = AppPlugin::didParseCommandLine();
   return rval;
}

bool DummyAppPlugin::initializeLogging()
{
   bool rval = AppPlugin::initializeLogging();
   return rval;
}

bool DummyAppPlugin::cleanupLogging()
{
   bool rval = AppPlugin::cleanupLogging();
   return rval;
}

bool DummyAppPlugin::willRun()
{
   bool rval = AppPlugin::willRun();
   return rval;
}

bool DummyAppPlugin::run()
{
   bool rval = AppPlugin::run();
   return rval;
}

bool DummyAppPlugin::didRun()
{
   bool rval = AppPlugin::didRun();
   return rval;
}
