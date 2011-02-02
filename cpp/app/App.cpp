/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/app/App.h"

#include "monarch/app/AppRunner.h"
#include "monarch/logging/Logging.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::kernel;
using namespace monarch::rt;

App::App() :
   mAppRunner(NULL),
   mName(NULL),
   mVersion(NULL)
{
   setName("(unknown)");
}

App::~App()
{
   mAppRunner = NULL;
   setName(NULL);
   setVersion(NULL);
}

void App::setAppRunner(AppRunner* ar)
{
   mAppRunner = ar;
}

AppRunner* App::getAppRunner()
{
   return mAppRunner;
}

void App::setName(const char* name)
{
   free(mName);
   mName = (name != NULL) ? strdup(name) : NULL;
}

const char* App::getName()
{
   return mName;
}

void App::setVersion(const char* version)
{
   free(mVersion);
   mVersion = version ? strdup(version) : NULL;
}

const char* App::getVersion()
{
   return mVersion;
}

Config App::getConfig()
{
   return mAppRunner->getConfig();
}

ConfigManager* App::getConfigManager()
{
   return mAppRunner->getConfigManager();
}

MicroKernel* App::getKernel()
{
   return mAppRunner->getKernel();
}

Config App::makeConfig(
   ConfigManager::ConfigId id,
   ConfigManager::ConfigId groupId,
   ConfigManager::ConfigId parentId)
{
   return mAppRunner->makeConfig(id, groupId, parentId);
}

Config App::getMetaConfig()
{
   return mAppRunner->getMetaConfig();
}

bool App::initialize()
{
   return true;
}

void App::cleanup()
{
}

bool App::initConfigs(Config& defaults)
{
   return true;
}

DynamicObject App::getCommandLineSpec(Config& cfg)
{
   DynamicObject spec;
   spec["options"]->setType(Array);
   return spec;
}

bool App::willLoadConfigs()
{
   return true;
}

bool App::didLoadConfigs()
{
   return true;
}

DynamicObject App::getWaitEvents()
{
   DynamicObject rval;
   rval->setType(Array);
   return rval;
}

bool App::run()
{
   MO_CAT_INFO(MO_APP_CAT, "Started in %" PRIu64 " ms",
      getAppRunner()->getTimer()->getElapsedMilliseconds());
   return true;
}
