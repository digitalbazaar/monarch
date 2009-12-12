/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/AppPlugin.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::rt;

AppPlugin::AppPlugin() :
   mApp(NULL)
{
   mInfo["id"] = "db.app.plugin.Unknown";
   mInfo["dependencies"]->setType(Array);
   mInfo["dependencies"]->append() = "db.app.plugins.Common";
}

AppPlugin::~AppPlugin()
{
   mApp = NULL;
}

bool AppPlugin::willAddToApp(App* app)
{
   return true;
}

bool AppPlugin::didAddToApp(App* app)
{
   setApp(app);
   return true;
}

bool AppPlugin::initConfigManager()
{
   return true;
}

bool AppPlugin::willInitMetaConfig(Config& meta)
{
   return true;
}

bool AppPlugin::initMetaConfig(Config& meta)
{
   return true;
}

bool AppPlugin::didInitMetaConfig(Config& meta)
{
   return true;
}

void AppPlugin::setApp(App* app)
{
   mApp = app;
}

App* AppPlugin::getApp()
{
   return mApp;
}

AppPluginInfo AppPlugin::getInfo()
{
   return mInfo;
}

DynamicObject AppPlugin::getCommandLineSpecs()
{
   DynamicObject specs;
   specs->setType(Array);
   return specs;
}

bool AppPlugin::willParseCommandLine(std::vector<const char*>* args)
{
   return true;
}

bool AppPlugin::didParseCommandLine()
{
   return true;
}

bool AppPlugin::willLoadConfigs()
{
   return true;
}

bool AppPlugin::didLoadConfigs()
{
   return true;
}

bool AppPlugin::initializeLogging()
{
   return true;
}

bool AppPlugin::cleanupLogging()
{
   return true;
}

bool AppPlugin::willRun()
{
   return true;
}

bool AppPlugin::run()
{
   return true;
}

bool AppPlugin::didRun()
{
   return true;
}
