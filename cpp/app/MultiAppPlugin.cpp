/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/app/MultiAppPlugin.h"

using namespace std;
using namespace db::app;
using namespace db::config;
using namespace db::rt;

typedef vector<AppPluginRef>::iterator PluginIterator;

MultiAppPlugin::MultiAppPlugin()
{
   mInfo["id"] = "db.app.plugin.Multi";
}

MultiAppPlugin::~MultiAppPlugin()
{
   mApp = NULL;
}

void MultiAppPlugin::addPlugin(AppPluginRef plugin)
{
   mPlugins.push_back(plugin);
   // FIXME: sort based on dependency info
}

bool MultiAppPlugin::willAddToApp(App* app)
{
   bool rval = AppPlugin::willAddToApp(app);

   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->willAddToApp(app);
   }
   return rval;
}

bool MultiAppPlugin::didAddToApp(App* app)
{
   bool rval = AppPlugin::didAddToApp(app);

   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->didAddToApp(app);
   }
   return rval;
}

bool MultiAppPlugin::initConfigManager()
{
   bool rval = AppPlugin::initConfigManager();

   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->initConfigManager();
   }
   return rval;
}

bool MultiAppPlugin::willInitMetaConfig(Config& meta)
{
   bool rval = AppPlugin::willInitMetaConfig(meta);

   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->willInitMetaConfig(meta);
   }
   return rval;
}

bool MultiAppPlugin::initMetaConfig(Config& meta)
{
   bool rval = AppPlugin::initMetaConfig(meta);

   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->initMetaConfig(meta);
   }
   return rval;
}

bool MultiAppPlugin::didInitMetaConfig(Config& meta)
{
   bool rval = AppPlugin::didInitMetaConfig(meta);

   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->didInitMetaConfig(meta);
   }
   return rval;
}

DynamicObject MultiAppPlugin::getCommandLineSpecs()
{
   DynamicObject rval = AppPlugin::getCommandLineSpecs();
   for(PluginIterator i = mPlugins.begin(); i != mPlugins.end(); i++)
   {
      DynamicObject pluginSpecs = (*i)->getCommandLineSpecs();
      DynamicObjectIterator psi = pluginSpecs.getIterator();
      while(psi->hasNext())
      {
         rval->append(psi->next());
      }
   }
   return rval;
}

bool MultiAppPlugin::willParseCommandLine(std::vector<const char*>* args)
{
   bool rval = AppPlugin::willParseCommandLine(args);
   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->willParseCommandLine(args);
   }
   return rval;
}

bool MultiAppPlugin::didParseCommandLine()
{
   bool rval = AppPlugin::didParseCommandLine();
   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->didParseCommandLine();
   }
   return rval;
}

bool MultiAppPlugin::initializeLogging()
{
   bool rval = AppPlugin::initializeLogging();
   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->initializeLogging();
   }
   return rval;
}

bool MultiAppPlugin::cleanupLogging()
{
   bool rval = AppPlugin::cleanupLogging();
   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->cleanupLogging();
   }
   return rval;
}

bool MultiAppPlugin::willRun()
{
   bool rval = AppPlugin::willRun();
   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->willRun();
   }
   return rval;
}

bool MultiAppPlugin::run()
{
   bool rval = AppPlugin::run();
   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->run();
   }
   return rval;
}

bool MultiAppPlugin::didRun()
{
   bool rval = AppPlugin::didRun();
   for(PluginIterator i = mPlugins.begin(); rval && i != mPlugins.end(); i++)
   {
      rval = (*i)->didRun();
   }
   return rval;
}
