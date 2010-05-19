/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_AppConfig_H
#define monarch_app_AppConfig_H

#include "monarch/logging/Logging.h"

namespace monarch
{
namespace app
{

// forward declaration
class App;

/**
 * An AppConfig is used to configure an App.
 *
 * @author Dave Longley
 */
class AppConfig
{
protected:
   /**
    * Main App logger.
    * FIXME: This is only here until we add reference counting to logging.
    */
   monarch::logging::Logger* mLogger;

public:
   /**
    * Creates an AppConfig instance.
    */
   AppConfig();

   /**
    * Deconstructs this AppConfig instance.
    */
   virtual ~AppConfig();

   /**
    * Initializes the App configuration system. This will set up the
    * configuration tree, default configs, and command line specifications
    * for the App.
    *
    * @param app the App to initialize the configs for.
    *
    * @return true if successful, false on failure with exception set.
    */
   virtual bool initialize(App* app);

   /**
    * Loads the command line configurations for the given App. This should
    * be called after parsing and processing the command line options.
    *
    * @param app the App to load the command line configs for.
    * @param plugin true to load plugin configs, false to load main configs.
    *
    * @return true if successful, false on failure with exception set.
    */
   virtual bool loadCommandLineConfigs(App* app, bool plugin);

   /**
    * Configures logging for an App.
    *
    * @param app the App.
    *
    * @return true if successful, false on failure with exception set.
    */
   virtual bool configureLogging(App* app);

   /**
    * Cleans up logging for an App.
    */
   virtual void cleanupLogging(App* app);
};

} // end namespace app
} // end namespace monarch

#endif
