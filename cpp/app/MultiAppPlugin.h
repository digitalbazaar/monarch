/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_app_MultiAppPlugin_H
#define db_app_MultiAppPlugin_H

#include "db/app/AppPlugin.h"

namespace db
{
namespace app
{

/**
 * AppPlugin which contains many sub-AppPlugins. All calls are broadcast to a
 * dependency sorted list of children AppPlugins.
 *
 * @author David I. Lehn
 */
class MultiAppPlugin : public AppPlugin
{
   /**
    * List of AppPlugins.
    */
   std::vector<AppPluginRef> mPlugins;

public:
   /**
    * {@inheritDoc}
    */
   MultiAppPlugin();

   /**
    * {@inheritDoc}
    */
   virtual ~MultiAppPlugin();

   /**
    * Add an AppPlugin.
    *
    * @param plugin an AppPlugin.
    */
   virtual void addPlugin(AppPluginRef plugin);

   /**
    * {@inheritDoc}
    */
   virtual bool willAddToApp(App* app);

   /**
    * {@inheritDoc}
    */
   virtual bool didAddToApp(App* app);

   /**
    * {@inheritDoc}
    */
   virtual bool initConfigManager();

   /**
    * {@inheritDoc}
    */
   virtual bool willInitMetaConfig(db::config::Config& meta);

   /**
    * {@inheritDoc}
    */
   virtual bool initMetaConfig(db::config::Config& meta);

   /**
    * {@inheritDoc}
    */
   virtual bool didInitMetaConfig(db::config::Config& meta);

   /**
    * {@inheritDoc}
    */
   virtual db::rt::DynamicObject getCommandLineSpecs();

   /**
    * {@inheritDoc}
    */
   virtual bool willParseCommandLine(std::vector<const char*>* args);

   /**
    * {@inheritDoc}
    */
   virtual bool didParseCommandLine();

   /**
    * {@inheritDoc}
    */
   virtual bool initializeLogging();

   /**
    * {@inheritDoc}
    */
   virtual bool cleanupLogging();

   /**
    * {@inheritDoc}
    */
   virtual bool willRun();

   /**
    * {@inheritDoc}
    */
   virtual bool run();

   /**
    * {@inheritDoc}
    */
   virtual bool didRun();
};

} // end namespace app
} // end namespace db

#endif
