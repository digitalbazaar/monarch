/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_ConfigPlugin_H
#define monarch_app_ConfigPlugin_H

#include "monarch/app/AppPlugin.h"
#include "monarch/logging/Logging.h"
#include "monarch/modest/Module.h"

namespace monarch
{
namespace app
{

/**
 * AppPlugin that provides support for loading and managing configs.
 *
 * @author David I. Lehn
 */
class ConfigPlugin : public AppPlugin
{
protected:
   /**
    * Logger used for config debugging.
    */
   monarch::logging::Logger* mDebugLogger;

public:
   /**
    * {@inheritDoc}
    */
   ConfigPlugin();

   /**
    * {@inheritDoc}
    */
   virtual ~ConfigPlugin();

   /**
    * {@inheritDoc}
    */
   virtual bool initMetaConfig(monarch::config::Config& meta);

   /**
    * {@inheritDoc}
    */
   virtual monarch::rt::DynamicObject getCommandLineSpecs();

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
   virtual bool run();
};

/**
 * Create a ConfigPlugin factory.
 *
 * @return a ConfigPlugin factory module.
 */
monarch::modest::Module* createConfigPluginFactory();

} // end namespace app
} // end namespace monarch

#endif
