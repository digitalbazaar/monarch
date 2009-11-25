/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_app_DummyAppPlugin_H
#define db_app_DummyAppPlugin_H

#include "db/app/AppPlugin.h"

namespace db
{
namespace app
{

/**
 * Dummy AppPlugin that does nothing. Useful as a basis for a real plugin.
 *
 * @author David I. Lehn
 */
class DummyAppPlugin : public AppPlugin
{
public:
   /**
    * {@inheritDoc}
    */
   DummyAppPlugin();

   /**
    * {@inheritDoc}
    */
   virtual ~DummyAppPlugin();

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
   virtual bool willLoadConfigs();

   /**
    * {@inheritDoc}
    */
   virtual bool didLoadConfigs();

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
