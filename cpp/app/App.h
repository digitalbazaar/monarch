/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_App_H
#define monarch_app_App_H

#include <ostream>
#include <vector>

#include "monarch/rt/Runnable.h"
#include "monarch/rt/Exception.h"
#include "monarch/config/ConfigManager.h"
#include "monarch/app/AppPlugin.h"
#include "monarch/app/LoggingPlugin.h"
#include "monarch/app/MultiAppPlugin.h"

namespace monarch
{
namespace app
{

/**
 * Top-level class for applications.
 *
 * This class provides basic app functionality. Custom app functionality is
 * provided by AppPlugins. Normal use of this class is to have the program
 * main(...) call App::main(...).
 *
 * App will load up the Monarch, Config, Logging, and Kernel AppPlugins.
 * Other functionality can be provided by loading more plugins with command
 * line options or programatically from loaded modules. The App will create
 * and load AppPlugins created by all modules of type
 * "monarch.app.AppPluginFactory".
 *
 * @author David I. Lehn
 */
class App
{
public:
   /**
    * Run mode of the app.
    */
   enum Mode {
      /** Bootstrap mode. */
      BOOTSTRAP,
      /** Main mode. */
      NORMAL
   };

protected:
   /**
    * The run mode.
    */
   Mode mMode;

   /**
    * Parent of this app or NULL.
    */
   App* mParentApp;

   /**
    * The Apps LoggingPlugin.
    */
   LoggingPlugin* mLoggingPlugin;

   /**
    * Program name for this App.  Taken from the command line args.
    */
   char* mProgramName;

   /**
    * Name of this App
    */
   char* mName;

   /**
    * Version of this App
    */
   char* mVersion;

   /**
    * Exit status to use for all tests.
    */
   int mExitStatus;

   /**
    * Command line arguments converted to a mutable vector.
    */
   std::vector<const char*> mCommandLineArgs;

   /**
    * Meta config. See getMetaConfig() for format.
    */
   monarch::config::Config mMetaConfig;

   /**
    * ConfigManager for this App.
    */
   monarch::config::ConfigManager* mConfigManager;

   /**
    * Clean up the ConfigManager.
    */
   bool mCleanupConfigManager;

   /**
    * A table of pthread mutexes for openSSL.
    */
   static pthread_mutex_t* sOpenSSLMutexes;

   /**
    * A callback function required by OpenSSL for multi-threaded applications.
    *
    * This method will return the current thread's ID.
    *
    * @return the current thread's ID.
    */
   static unsigned long openSSLSetId();

   /**
    * A callback function required by OpenSSL for multi-threaded applications.
    *
    * It sets the n-th lock if mode & CRYPTO_LOCK, and releases it otherwise.
    *
    * @param mode the current mode.
    * @param n the lock number to alter.
    * @param file the current source file (unused).
    * @param line the line in the file (unused).
    */
   static void openSSLHandleLock(int mode, int n, const char* file, int line);

   /**
    * Create and initialize the app config manager.
    *
    * @return true on success, false and exception on failure.
    */
   virtual bool initConfigManager();

   /**
    * Perform ConfigManager specific cleanup if required.
    */
   virtual void cleanupConfigManager();

   /**
    * List of AppPlugins.
    */
   MultiAppPlugin* mPlugins;

public:
   /**
    * Create an App instance.
    */
   App();

   /**
    * Deconstruct this App instance.
    */
   virtual ~App();

   /**
    * Set the bootstrap mode.
    *
    * @param mode the bootstrap mode.
    *
    * @return true on success, false and exception set on error.
    */
   virtual bool setMode(Mode mode);

   /**
    * Get the app run mode.
    *
    * @return the run mode of the app.
    */
   virtual Mode getMode();

   /**
    * Set the parent app.
    *
    * @param parent the parent App.
    */
   virtual void setParentApp(App* parent);

   /**
    * Get the parent app or NULL if none. This can be used to get the bootstrap
    * app.
    *
    * @return the parent app.
    */
   virtual App* getParentApp();

   /**
    * Set the LoggingPlugin.
    *
    * @param logger the LoggingPlugin.
    */
   virtual void setLoggingPlugin(LoggingPlugin* logger);

   /**
    * Get the LoggingPlugin.
    *
    * @return the LoggingPlugin.
    */
   virtual LoggingPlugin* getLoggingPlugin();

   /**
    * Add an AppPlugin.
    *
    * @param plugin an AppPlugin.
    *
    * @return true on success, false and exception set on error.
    */
   virtual bool addPlugin(AppPluginRef plugin);

   /**
    * Get all the wait events from all AppPlugins. See
    * AppPlugin::getWaitEvents() for more details.
    *
    * @return the combined plugin wait events.
    */
   virtual monarch::rt::DynamicObject getWaitEvents();

   /**
    * Set the program name.
    *
    * @param name the program name.
    */
   virtual void setProgramName(const char* name);

   /**
    * Get the program name.
    *
    * @return the program name.
    */
   virtual const char* getProgramName();

   /**
    * Set the name.
    *
    * @param name the name.
    */
   virtual void setName(const char* name);

   /**
    * Get the name.
    *
    * @return the name.
    */
   virtual const char* getName();

   /**
    * Set the version.
    *
    * @param name the version.
    */
   virtual void setVersion(const char* version);

   /**
    * Get the version.
    *
    * @return the version.
    */
   virtual const char* getVersion();

   /**
    * Set the application exit status.
    *
    * @param status the application exit status.
    */
   virtual void setExitStatus(int exitStatus);

   /**
    * Get the application exit status.
    *
    * @return the application exit status.
    */
   virtual int getExitStatus();

   /**
    * Set the ConfigManager.
    *
    * @param configManager the ConfigManager for this app.
    * @param cleanup true to cleanup this manager, false not to.
    */
   virtual void setConfigManager(
      monarch::config::ConfigManager* configManager, bool cleanup = true);

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
    * Gets the meta configuration object.
    *
    * This mutable object is used for command line options and to setup and
    * load a hierarchy of configs. The format and default object is as follows:
    *
    * {
    *    # array of command line option specs
    *    # see AppPlugin::getCommandLineSpecs()
    *    "specs": [
    *       <spec>,
    *       ...
    *    ],
    *    # command line option configs indexed by id
    *    "options": {
    *       "<id>": <config>,
    *       ...,
    *    }
    *
    *    # map of well-known ids that can be customized
    *    "groups": {
    *       "root": "root",
    *       "boot": "boot",
    *       "defaults": "defaults",
    *       "command line": "command line",
    *       "main": "main"
    *    },
    *    # map of parents of well-known ids that can be customized
    *    "parents": {
    *       "root": null,
    *       "boot": "root",
    *       "defaults": "boot",
    *       "command line": "defaults",
    *       "main": "command line"
    *    },
    *    # map of configs indexed by id
    *    "configs": {
    *       "<id>": <config>,
    *       ...
    *    }
    * }
    *
    * @return the meta config.
    */
   virtual monarch::config::Config getMetaConfig();

   /**
    * Get the command line args used for this app.
    *
    * @return the command line args.
    */
   virtual std::vector<const char*>* getCommandLine();

   /**
    * Parses the command line options that were passed to the application.
    * Implementations may call exit() depending on the arguments.  For normal
    * errors it is preferable to return false and set an exception.
    *
    * @param args read-write vector of command line paramters.
    *
    * @return true on success, false on failure and exception set
    */
   virtual bool parseCommandLine(std::vector<const char*>* args);

   /**
    * Initialize OpenSSL.
    *
    * @return true on success, false on failure and exception set
    */
   virtual bool initializeOpenSSL();

   /**
    * Cleanup OpenSSL.
    */
   virtual void cleanupOpenSSL();

   /**
    * Run the app. Plugins can be added before this is called.
    *
    * @param args the command line arguments.
    *
    * @return true on success, false on failure and exception set
    */
   virtual int start(std::vector<const char*>* args);

   /**
    * Complete top-level Monarch App process.
    *
    * @param argc number of command line arguments.
    * @param argv command line arguments.
    *
    * @return exit status. 0 for success.
    */
   static int main(int argc, const char* argv[]);

   /**
    * Pretty print an exception.
    *
    * @param e the exception to print.
    */
   static void printException(monarch::rt::ExceptionRef& e);

   /**
    * Pretty print an exception to a given output stream.
    *
    * @param e the exception to print.
    * @param s the output stream.
    */
   static void printException(monarch::rt::ExceptionRef& e, std::ostream& s);

   /**
    * Pretty print last exception.
    */
   static void printException();

   /**
    * Make a default empty config. If an id is given it will be added to the
    * config and used as a key to add the config to the meta config. If a group
    * id is given it is considered to be a common name id and the real group
    * id is looked up in the meta config before adding it to the new config.
    * The parent of the new config will be looked up with the resolved group id.
    *
    * @param meta the meta config.
    * @param id the config id.
    * @param groupId the group id.
    * @param section section for this config ("configs" or "options").
    *
    * @return the new config.
    */
   static monarch::config::Config makeMetaConfig(
      monarch::config::Config& meta,
      const char* id = NULL,
      const char* groupId = NULL,
      const char* section = "configs");
};

} // end namespace app
} // end namespace monarch

#endif
