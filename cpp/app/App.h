/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_App_H
#define monarch_app_App_H

#include "monarch/kernel/MicroKernel.h"
#include "monarch/app/AppPlugin.h"

namespace monarch
{
namespace app
{

/**
 * An App is a top-level class for applications.
 *
 * This class provides basic app functionality: ie: parameter parsing, logging,
 * configuration and a MicroKernel. Specific application functionality is
 * provided by a custom-written AppPlugin. Normal use of this class is to have
 * the program main(...) call App::main(...).
 *
 * An AppPlugin is created by an AppPluginFactory which is a specific type of
 * MicroKernelModule. Additional modules may be loaded after the AppPlugin is
 * loaded using command line options or programmatically from other loaded
 * modules.
 *
 * @author David I. Lehn
 * @author Dave Longley
 */
class App
{
protected:
   /**
    * Program name for this App. Taken from the command line args.
    */
   char* mProgramName;

   /**
    * Name of this App.
    */
   char* mName;

   /**
    * Version of this App.
    */
   char* mVersion;

   /**
    * Exit status to use for all tests.
    */
   int mExitStatus;

   /**
    * The meta config for this App. Contains unloaded config file paths and
    * command line option configs.
    */
   monarch::rt::DynamicObject mMetaConfig;

   /**
    * The MicroKernel for this App.
    */
   monarch::kernel::MicroKernel* mKernel;

   /**
    * The app state types.
    */
   enum State
   {
      // App is stopped.
      Stopped,
      // In the process of starting the app.
      Starting,
      // App has been started and is running.
      Running,
      // In the process of restarting the app.
      Restarting,
      // In the process of stopping the app.
      Stopping
   };

   /**
    * The current app state.
    */
   State mState;

public:
   /**
    * Creates an App instance.
    */
   App();

   /**
    * Deconstructs this App instance.
    */
   virtual ~App();

   /**
    * Sets the program name.
    *
    * @param name the program name.
    */
   virtual void setProgramName(const char* name);

   /**
    * Gets the program name.
    *
    * @return the program name.
    */
   virtual const char* getProgramName();

   /**
    * Sets the application name.
    *
    * @param name the name.
    */
   virtual void setName(const char* name);

   /**
    * Gets the application name.
    *
    * @return the name.
    */
   virtual const char* getName();

   /**
    * Sets the version.
    *
    * @param name the version.
    */
   virtual void setVersion(const char* version);

   /**
    * Gets the version.
    *
    * @return the version.
    */
   virtual const char* getVersion();

   /**
    * Sets the application exit status.
    *
    * @param status the application exit status.
    */
   virtual void setExitStatus(int exitStatus);

   /**
    * Gets the application exit status.
    *
    * @return the application exit status.
    */
   virtual int getExitStatus();

   /**
    * Gets this app's MicroKernel.
    *
    * @return the MicroKernel for this app.
    */
   virtual monarch::kernel::MicroKernel* getKernel();

   /**
    * Gets this app's ConfigManager.
    *
    * @return the ConfigManager for this app.
    */
   virtual monarch::config::ConfigManager* getConfigManager();

   /**
    * Convenience for getConfigManager()->getConfig(getMainConfigGroup()).
    *
    * @return the main config for this app.
    */
   virtual monarch::config::Config getConfig();

   /**
    * Makes a builtin config by setting its ID, group, parent, and version. It
    * must be added to the ConfigManager once it has been filled out.
    *
    * @param id the ID for the config.
    * @param group the group for the config.
    *
    * @return the config.
    */
   virtual monarch::config::Config makeConfig(
      monarch::config::ConfigManager::ConfigId id,
      monarch::config::ConfigManager::ConfigId groupId);

   /**
    * Gets the meta configuration object.
    *
    * This mutable object is used for command line options and to store config
    * file paths for loading. The format and default object is as follows:
    *
    * {
    *    # the parsed command line
    *    "commandLine": [
    *       "options": [
    *          "consumed": <true/false>,
    *          "short"/"long": <option>,
    *          "value": <value>
    *       ],
    *       "extra": [
    *          <arg>,
    *          ...
    *       ]
    *    ],
    *    # array of command line option specs
    *    # see AppPlugin::getCommandLineSpecs()
    *    "specs": [
    *       <spec>,
    *       ...
    *    ],
    *    # builtin command line option configs indexed by id
    *    "options": {
    *       "<id>": <config>,
    *       ...,
    *    }
    *    # plugin command line option configs
    *    "pluginOptions": <config>
    * }
    *
    * @return the meta config.
    */
   virtual monarch::config::Config getMetaConfig();

   /**
    * Runs the App. If specified in the configuration, an AppPlugin will be
    * run that customizes the behavior of the application.
    *
    * @param argc number of command line arguments.
    * @param argv command line arguments.
    *
    * @return exit status. 0 for success.
    */
   virtual int start(int argc, const char* argv[]);

   /**
    * Called from ::main() to run a top-level App.
    *
    * @param argc number of command line arguments.
    * @param argv command line arguments.
    *
    * @return exit status. 0 for success.
    */
   static int main(int argc, const char* argv[]);

protected:
   /**
    * Runs this App.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool run();

   /**
    * Configures the AppPlugin.
    *
    * @param plugin the AppPlugin to configure.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool configurePlugin(AppPlugin* plugin);

   /**
    * Runs the AppPlugin.
    *
    * @param plugin the AppPlugin to run.
    * @param waitEvents the events to wait for while the plugin runs.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool runPlugin(
      AppPlugin* plugin, monarch::rt::DynamicObject& waitEvents);
};

} // end namespace app
} // end namespace monarch

#endif
