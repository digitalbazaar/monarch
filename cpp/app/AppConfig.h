/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_AppConfig_H
#define monarch_app_AppConfig_H

namespace monarch
{
namespace app
{

// forward declaration
class AppRunner;

/**
 * An AppConfig is used to configure an AppRunner and the App it runs.
 *
 * @author Dave Longley
 */
class AppConfig
{
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
    * Initializes the App configuration system. This will set up the core
    * configuration tree, default configs, and command line specifications.
    *
    * @param ar the AppRunner to initialize the configs for.
    *
    * @return true if successful, false on failure with exception set.
    */
   virtual bool initialize(AppRunner* ar);

   /**
    * Loads the command line configurations for the given AppRunner. This
    * should be called after parsing and processing the command line options.
    *
    * @param ar the AppRunner to load the command line configs for.
    * @param app true to load app configs, false to load main configs.
    *
    * @return true if successful, false on failure with exception set.
    */
   virtual bool loadCommandLineConfigs(AppRunner* ar, bool appConfigs);

   /**
    * Configures logging for an AppRunner.
    *
    * @param ar the AppRunner to configure logging for.
    *
    * @return true if successful, false on failure with exception set.
    */
   virtual bool configureLogging(AppRunner* ar);

   /**
    * Cleans up logging for an AppRunner.
    *
    * @param ar the AppRunner to cleanup logging from.
    */
   virtual void cleanupLogging(AppRunner* ar);
};

} // end namespace app
} // end namespace monarch

#endif
