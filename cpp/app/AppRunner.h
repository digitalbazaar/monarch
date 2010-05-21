/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_AppRunner_H
#define monarch_app_AppRunner_H

#include "monarch/kernel/MicroKernel.h"
#include "monarch/app/App.h"

namespace monarch
{
namespace app
{

/**
 * An AppRunner is a top-level class for running an application.
 *
 * This class provides basic functionality: ie: parameter parsing, logging,
 * configuration, and a MicroKernel. Specific application functionality is
 * provided by a custom-written App that is loaded by the MicroKernel. Normal
 * use of this class is to have the program main(...) call App::main(...).
 *
 * An App is created by an AppFactory which is a specific type of
 * MicroKernelModule. Additional modules may be loaded after the App is
 * loaded using command line options or programmatically from other loaded
 * modules.
 *
 * @author David I. Lehn
 * @author Dave Longley
 */
class AppRunner
{
protected:
   /**
    * Program name for this AppRunner. Taken from the command line args.
    */
   char* mProgramName;

   /**
    * Exit status to use for all tests.
    */
   int mExitStatus;

   /**
    * The meta config for the App. Contains unloaded config file paths and
    * command line option configs.
    */
   monarch::rt::DynamicObject mMetaConfig;

   /**
    * The MicroKernel for this App.
    */
   monarch::kernel::MicroKernel* mKernel;

   /**
    * The AppRunner state types.
    */
   enum State
   {
      // AppRunner is stopped.
      Stopped,
      // In the process of starting.
      Starting,
      // AppRunner has been started and is running.
      Running,
      // In the process of restarting the AppRunner.
      Restarting,
      // In the process of stopping the AppRunner.
      Stopping
   };

   /**
    * The current AppRunner state.
    */
   State mState;

public:
   /**
    * Creates an AppRunner instance.
    */
   AppRunner();

   /**
    * Deconstructs this AppRunner instance.
    */
   virtual ~AppRunner();

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
    * Gets this AppRunner's MicroKernel.
    *
    * @return the MicroKernel for this AppRunner.
    */
   virtual monarch::kernel::MicroKernel* getKernel();

   /**
    * Gets this AppRunner's ConfigManager.
    *
    * @return the ConfigManager for this AppRunner.
    */
   virtual monarch::config::ConfigManager* getConfigManager();

   /**
    * Convenience for getConfigManager()->getConfig("main").
    *
    * @return the main config for this AppRunner.
    */
   virtual monarch::config::Config getConfig();

   /**
    * Makes a config by setting its ID, group, parent, and version. It must
    * be added to the ConfigManager once it has been filled out. If no parent
    * ID is specified, it will be taken from the group or set to none.
    *
    * @param id the ID for the config.
    * @param group the group ID for the config.
    * @param parent the parent ID for the config (NULL to detect or for none).
    *
    * @return the config.
    */
   virtual monarch::config::Config makeConfig(
      monarch::config::ConfigManager::ConfigId id,
      monarch::config::ConfigManager::ConfigId groupId,
      monarch::config::ConfigManager::ConfigId parentId = NULL);

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
    *    # see App::getCommandLineSpec()
    *    "specs": [
    *       <spec>,
    *       ...
    *    ],
    *    # builtin AppRunner command line option configs indexed by id
    *    "options": {
    *       "<id>": <config>,
    *       ...,
    *    }
    *    # customized app command line option configs
    *    "appOptions": <config>
    * }
    *
    * @return the meta config.
    */
   virtual monarch::config::Config getMetaConfig();

   /**
    * Starts this AppRunner. If specified in the configuration, an App will be
    * run that provides the custom application behavior.
    *
    * @param argc number of command line arguments.
    * @param argv command line arguments.
    *
    * @return exit status. 0 for success.
    */
   virtual int start(int argc, const char* argv[]);

   /**
    * Called from ::main() to run a top-level AppRunner.
    *
    * @param argc number of command line arguments.
    * @param argv command line arguments.
    *
    * @return exit status. 0 for success.
    */
   static int main(int argc, const char* argv[]);

protected:
   /**
    * Runs this AppRunner.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool run();

   /**
    * Configures the App.
    *
    * @param app the App to configure.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool configureApp(App* app);

   /**
    * Runs the App.
    *
    * @param app the App to run.
    * @param waitEvents the events to wait for while the app runs.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool runApp(App* app, monarch::rt::DynamicObject& waitEvents);
};

} // end namespace app
} // end namespace monarch

#endif
