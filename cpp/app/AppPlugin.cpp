/*
 * Copyright (c) 2008-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/AppPlugin.h"

#include "monarch/app/App.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::rt;

AppPlugin::AppPlugin() :
   mApp(NULL)
{
}

AppPlugin::~AppPlugin()
{
   mApp = NULL;
}

void AppPlugin::setApp(App* app)
{
   mApp = app;
}

App* AppPlugin::getApp()
{
   return mApp;
}

bool AppPlugin::initialize()
{
   return true;
}

void AppPlugin::cleanup()
{
}

bool AppPlugin::initConfigs(Config& defaults)
{
   return true;
}

bool AppPlugin::initCommandLineConfig(Config& cfg)
{
   return true;
}

DynamicObject AppPlugin::getCommandLineSpec()
{
   DynamicObject spec;
   spec->setType(Array);
   return spec;
}

bool AppPlugin::willLoadConfigs()
{
   return true;
}

bool AppPlugin::didLoadConfigs()
{
   return true;
}

DynamicObject AppPlugin::getWaitEvents()
{
   DynamicObject rval;
   rval->setType(Array);
   return rval;
}

bool AppPlugin::run()
{
   return true;
}
