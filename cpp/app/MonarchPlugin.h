/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_MonarchPlugin_h
#define monarch_app_MonarchPlugin_h

#include "monarch/app/AppPlugin.h"
#include "monarch/logging/Logging.h"
#include "monarch/modest/Module.h"

namespace monarch
{
namespace app
{

/**
 * AppPlugin providing basic functionality such as setting up some of the
 * config defaults and handling the help option.
 *
 * @author David I. Lehn
 */
class MonarchPlugin : public AppPlugin
{
protected:
   /**
    * An app ConfigManager.
    */
   monarch::config::ConfigManager mConfigManager;

public:
   /**
    * {@inheritDoc}
    */
   MonarchPlugin();

   /**
    * {@inheritDoc}
    */
   virtual ~MonarchPlugin();

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
   virtual monarch::rt::DynamicObject getCommandLineSpecs();

   /**
    * {@inheritDoc}
    */
   virtual bool didParseCommandLine();
};

/**
 * Create a MonarchPlugin factory.
 *
 * @return a MonarchPlugin factory module.
 */
monarch::modest::Module* createMonarchPluginFactory();

} // end namespace app
} // end namespace monarch

#endif
