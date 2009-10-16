/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_app_App_H
#define db_app_App_H

#include <ostream>

#include "db/rt/Runnable.h"
#include "db/rt/Exception.h"
#include "db/config/ConfigManager.h"
#include "db/app/AppPlugin.h"
#include "db/app/MultiAppPlugin.h"

#include <vector>

namespace db
{
namespace app
{

/**
 * Top-level class for applications.
 *
 * This class provides basic functionality. Custom app functionality is
 * provided by AppPlugins. To run an App, create an App instance, optionally
 * add plugins to it, and call App::main():
 *
 * int main(int argc, const char* argv[])
 * {
 *    db::app::App app;
 *    std::vector<db::app::AppPluginRef> plugins;
 *    db::app::AppPluginRef plugin = new MyPlugin;
 *    plugins.push_back(plugin);
 *    ...
 *    return app.main(argc, argv, &plugins);
 * }
 *
 * Most functionality is delegated to plugins. See the AppPlugin documentation
 * for plugin details.
 *
 * @author David I. Lehn
 */
class App
{
protected:
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
    * Temporary command line options and specs storage.
    */
   db::config::Config mCLConfig;

   /**
    * Meta config. See getMetaConfig() for format.
    */
   db::config::Config mMetaConfig;

   /**
    * ConfigManager for this App.
    */
   db::config::ConfigManager* mConfigManager;

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
    * Add an AppPlugin.
    *
    * @param plugin an AppPlugin.
    *
    * @return true on success, false and exception set on error.
    */
   virtual bool addPlugin(AppPluginRef plugin);

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
    * Get the command line configuration.
    *
    * AppPlugins should add command line specs to the "specs" array of this
    * config.
    *
    * @return the main config for this app.
    */
   virtual db::config::Config getCommandLineConfig();

   /**
    * Set the ConfigManager.
    *
    * @param configManager the ConfigManager for this app.
    * @param cleanup true to cleanup this manager, false not to.
    */
   virtual void setConfigManager(
      db::config::ConfigManager* configManager, bool cleanup = true);

   /**
    * Gets this app's ConfigManager.
    *
    * @return the ConfigManager for this app.
    */
   virtual db::config::ConfigManager* getConfigManager();

   /**
    * Convenience for getConfigManager()->getConfig(getMainConfigGroup()).
    *
    * @return the main config for this app.
    */
   virtual db::config::Config getConfig();

   /**
    * Gets the meta configuration object.
    *
    * This mutable object is used to setup and load a hierarchy of configs. The
    * format and default object is as follows:
    *
    * {
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
   virtual db::config::Config getMetaConfig();

   /**
    * Sorts the configs in the meta config based on parent relationships and
    * loads them in the proper order.
    *
    * @return true on success, false on failure and exception set
    */
   virtual bool loadConfigs();

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
    * Add plugins, initialize the app and plugins, and run the app.
    *
    * @param argc number of command line arguments.
    * @param argv command line arguments.
    * @param plugins list of plugins to add to an app.
    * @param standard if true then also add standard plugins.
    *
    * @return exit status. 0 for success.
    */
   virtual int main(
      int argc, const char* argv[],
      std::vector<db::app::AppPluginRef>* plugins = NULL,
      bool standard = true);

   /**
    * Pretty print an exception.
    *
    * @param e the exception to print.
    */
   static void printException(db::rt::ExceptionRef& e);

   /**
    * Pretty print an exception to a given output stream.
    *
    * @param e the exception to print.
    * @param s the output stream.
    */
   static void printException(db::rt::ExceptionRef& e, std::ostream& s);

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
    *
    * @return true on success, false and exception on failure.
    */
   static db::config::Config makeMetaConfig(
      db::config::Config& meta,
      const char* id = NULL,
      const char* groupId = NULL);
};

/**
 * Macro to create a simple main() that creates an app, adds a plugin to it,
 * and calls App::main().
 *
 * @param appPluginClassName class name of an AppPlugin subclass.
 */
#define DB_APP_PLUGIN_MAIN(appPluginClassName)                \
int main(int argc, const char* argv[])                        \
{                                                             \
   db::app::App app;                                          \
   std::vector<db::app::AppPluginRef> plugins;                \
   db::app::AppPluginRef plugin = new appPluginClassName;     \
   plugins.push_back(plugin);                                 \
   return app.main(argc, argv, &plugins);                     \
}

} // end namespace app
} // end namespace db

#endif
