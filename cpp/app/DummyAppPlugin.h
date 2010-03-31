/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_DummyAppPlugin_H
#define monarch_app_DummyAppPlugin_H

#include "monarch/app/AppPlugin.h"

namespace monarch
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
   virtual bool initialize();

   /**
    * {@inheritDoc}
    */
   virtual void cleanup();

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
   virtual monarch::rt::DynamicObject getWaitEvents();

   /**
    * {@inheritDoc}
    */
   virtual bool initConfigManager();

   /**
    * {@inheritDoc}
    */
   virtual bool willInitMetaConfig(monarch::config::Config& meta);

   /**
    * {@inheritDoc}
    */
   virtual bool initMetaConfig(monarch::config::Config& meta);

   /**
    * {@inheritDoc}
    */
   virtual bool didInitMetaConfig(monarch::config::Config& meta);

   /**
    * {@inheritDoc}
    */
   virtual monarch::rt::DynamicObject getCommandLineSpecs();

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
} // end namespace monarch

#endif
