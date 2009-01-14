/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_app_App_H
#define db_app_App_H

#include <ostream>

#include "db/logging/Logging.h"
#include "db/rt/Runnable.h"
#include "db/rt/Exception.h"
#include "db/config/ConfigManager.h"

#include <vector>

namespace db
{
namespace app
{

/**
 * Top-level class for applications.
 * 
 * To use: create a App subclass, override the appropriate methods, and use a
 * main() function which works similar to the following:
 * 
 * int main(int argc, const char* argv[])
 * {
 *    App app;
 *    Delegate delegate;
 *    app.setDelegate(delegate);
 *    app.initialize();
 *    return app.main(argc, argv);
 * }
 * 
 * Apps support App delegates.  This allows the use of standard apps as well
 * as a sibling Apps.  For instance, if MyApp inherits from App as well as
 * Tester, you can create a custom MyTester and gain functionality of using
 * MyApp as the main app with MyTester as the delegate.
 *
 * App ---- MyApp
 *      \-- Tester -- MyTester
 *
 * MyApp->delegate = MyTester
 * 
 * The common order that overridden App methods are called is:
 * - parseCommandLine(argc, argv)
 * - initializeLogging()
 * - initialize()
 * - run()
 * - cleanup()
 * - cleanupLogging()
 * 
 * @author David I. Lehn
 */
class App : public db::rt::Runnable
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
    * The default logger.
    */
   db::logging::Logger* mLogger;
   
   /**
    * Start the defauilt logger if enabled.
    * 
    * @return true on success, false and exception on failure.
    */
   virtual bool startLogging();
   
   /**
    * Stop the default logger if enabled.
    * 
    * @return true on success, false and exception on failure.
    */
   virtual bool stopLogging();
   
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
    * Set the ConfigManager.
    * 
    * @param configManager the ConfigManager for this app.
    * @param cleanup true to cleanup this manager, false not to.
    */
   virtual void setConfigManager(
      db::config::ConfigManager* configManager, bool cleanup = true);
   
   /**
    * Called before initConfigGroups().  Used to configure groups that are
    * required by other groups.
    * 
    * Subclasses should call the superclass method.
    * 
    * @return true on success, false and exception on failure.
    */
   virtual bool willInitConfigGroups();

   /**
    * Initialize config groups as needed.
    * 
    * Subclasses should call the superclass method.
    * 
    * @return true on success, false and exception on failure.
    */
   virtual bool initConfigGroups();

   /**
    * Called after initConfigGroups().  Used to configure groups that require
    * others to be configured.
    * 
    * Subclasses should call the superclass method.
    * 
    * @return true on success, false and exception on failure.
    */
   virtual bool didInitConfigGroups();

   /**
    * App's delegate
    */
   App* mDelegate;

   /**
    * App's owner if a delegate
    */
   App* mOwner;
   
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
    * Initialize this App.  This method MUST be called for non-delegates.
    * If using a delegate then call setDelegate() before initiailize().  This
    * method will call initConfigManager(), initConfigGroups(), and
    * initConfigGroups() on the delegate.
    * 
    * @return true on success, false and exception set on error.
    */
   virtual bool initialize();

   /**
    * Set the App's delegate.
    * 
    * @param delegate the App's delegate.
    */
   virtual void setDelegate(App* delegate);

   /**
    * Get the App's delegate.
    * 
    * @return the App's delegate.
    */
   virtual App* getDelegate();
   
   /**
    * Set the App's owner when a delegate.
    * 
    * @param owner the App's owner.
    */
   virtual void setOwner(App* owner);

   /**
    * Get the App's owner.
    * 
    * @return the App's owner.
    */
   virtual App* getOwner();
   
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
    * Gets this app's ConfigManager.
    * 
    * @return the ConfigManager for this app.
    */
   virtual db::config::ConfigManager* getConfigManager();
   
   /**
    * Convienience for getConfigManager()->getConfig(getMainConfigGroup()).
    * 
    * @return the main config for this app.
    */
   virtual db::config::Config getConfig();
   
   /**
    * Gets the name of the main config group.
    * 
    * @return the name of the main config group.
    */
   virtual const char* getMainConfigGroup();
   
   /**
    * Gets the name of the parent of the main config group.
    * 
    * @return the name of the parent of the main config group.
    */
   virtual const char* getParentOfMainConfigGroup();
   
   /**
    * See run().
    * 
    * @return true on success, false on failure and exception set
    */
   virtual bool initializeRun();
   
   /**
    * Run the app.
    * 
    * @return true on success, false on failure and exception set
    */
   virtual bool runApp();
   
   /**
    * See run().
    */
   virtual void cleanupRun();

   /**
    * Run the app and set mExitStatus.
    * 
    * The sequence of events is (error handling not shown):
    * 
    * initializeRun()
    *   delegate->initializeRun()
    * startLogging()
    *   delegate->startLogging()
    * runApp()
    *   delegate->runApp()
    * stopLogging();
    *   delegate->stopLogging()
    * cleanupRun();
    *   delegate->cleanupRun()
    * 
    * If logging options need to be set on the apps config, do so in
    * initializeRun().
    */
   virtual void run();
   
   /**
    * Called before the default App processes the command line arguments.
    * Subclasses may use this hook to process arguments in a read-only mode.
    * 
    * This hook should be used if a delegate needs to processes arguments
    * before normal default App processing.
    * 
    * Subclasses MUST call the superclass implementation first.
    * 
    * @param args read-only vector of command line arguments.
    * 
    * @return true on success, false on failure and exception set
    */
   virtual bool willParseCommandLine(std::vector<const char*>* args);

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
    * Called after the App processes the command line arguments.  Subclasses
    * may use this hook to check and process the command line args.
    * 
    * Subclasses MUST call the superclass implementation first.
    * 
    * @return true on success, false on failure and exception set
    */
   virtual bool didParseCommandLine();
   
   /**
    * Get command line specifications for default paramters.  Subclasses MUST
    * call the superclass implementation and append their spec to the return
    * value from that call.  The spec is in the following format:
    * 
    * Spec = {
    *    "options" = [ OptionSpec[, ...] ],
    *    "help" = "Help string for options.",
    *    ...
    * }
    * 
    * "help" should be in a format such as:
    * "[Name] options:\n"
    * "  -x, --set-x         Simple option.\n"
    * "      --set-y         Simple option, only long version.\n"
    * "  -f, --file FILE     Option with parameter.\n"
    * "  -l, --long-option OPT\n"
    * "                      Longer option. (default: \"default\")\n"
    * "  -L, --long-help     Option that has a long option help string which\n"
    * "                      needs to wrap to the next line after 80 chars.\n"
    * 
    * An optional key is "args" which should be a DynamicObject array which
    * will be filled with remaining args when a non-option is found. 
    * 
    * OptionSpec = {
    *    "short": "-o",
    *    "long": "--long-option",
    *    ...
    * }
    * 
    * Action keys which consume arguments cannot appear in parallel.  Actions
    * which do not, such as setTrue/setFalse/inc/dec, can appear in parallel.
    * 
    * Options that specify a "target" specify target options that can be one
    * of the following formats:
    * Specify a target DynamicObject directly:
    * ...["arg"]["target"] = <dyno>
    * A relative path from a root DynamicObject:
    * ...["arg"]["root"] = <dyno>
    * ...["arg"]["path"] = <string path>
    * A relative path in a named raw config.  Will be set after changing.
    * ...["arg"]["config"] = <raw config name>
    * ...["arg"]["path"] = <string path>
    * 
    * Paths are split on '.'.  If a segment matches r"[^\]*\$" it is joined
    * with the next segment.  Ie, if last char is a '\' but the last two chars
    * are not "\\" then a join occurs but last '\' is dropped.
    * 
    * For example, following paths are applied to a target:
    * "" => target[""]
    * "a.b.c" => target["a"]["b"]["c"]
    * "a\.b.c" => target["a.b"]["c"]
    * "a\\.b.c" => target["a\"]["b"]["c"]
    * "a\\b.c" => target["a\\b"]["c"]
    * 
    * If "isJsonValue" exists and is true then the value argument will be
    * decoded as a JSON value.  It can be any text that could appear as a JSON
    * value.  (In other words, it does not have JSON top-level {} or []
    * requirement)
    * 
    * The type of the new value will be either the type of a special "type"
    * object, the type of an existing object, or will default to a string.
    * ...["arg"]["type"] = <dyno>: will use type of dyno
    * ...["arg"]["target"] = <dyno>: will use type of dyno
    * otherwise: string
    * 
    * If option found then set DynamicObject as appropriate:
    * "setTrue": target | [ target[, ...] ]
    * "setFalse": target | [ target[, ...] ]
    * 
    * If option found then increment or decrement DynamicObject value by 1:
    * "inc": target | [ target[, ...] ]
    * "dec": target | [ target[, ...] ]
    * Note: This will read/write to a specific DynamicObject.  Interaction with
    *       a multi-level ConfigManager setup may not be straightforward.
    * 
    * Read next argument or arguments, convert to the DynamicObject type, and
    * store them.  On error use argError message.  The command line must have
    * enough arguments to satisfy the args array length.
    * "arg": DynamicObject
    * "args": [ target[, ...] ]
    * "argError": string
    * 
    * Append arg or args to an Array DynamicObject:
    * "append": target
    * 
    * Set a named config value.  Reads the first argument as a path.  The "set"
    * target is used to find the final target.  Then this target is assigned
    * the next argument via the above "arg" process.
    * "set": target
    * 
    * The default implementation will parse the following parameters:
    * -h, --help: print out default help and delegates help
    * -V --version: print out app name and version if present
    * -v, --verbose: set verbose mode for use by apps
    * --log-level: parse and set a log level variable
    * 
    * @return an array of command line spec
    */
   virtual db::rt::DynamicObject getCommandLineSpecs();

   /**
    * Initialize OpenSSL.
    */
   virtual void initializeOpenSSL();
    
   /**
    * Cleanup OpenSSL.
    */
   virtual void cleanupOpenSSL();
   
   /**
    * Initialize logging.
    */
   virtual void initializeLogging();
   
   /**
    * Called after initializeLogging()
    */
   virtual void didInitializeLogging();
   
   /**
    * Called before cleanupLogging()
    */
   virtual void willCleanupLogging();
   
   /**
    * Cleanup logging.
    */
   virtual void cleanupLogging();
   
   /**
    * Start an app andPerform the main Run all tests.
    * 
    * @param argc number of command line arguments.
    * @param argv command line arguments.
    * 
    * @return exit status. 0 for success.
    */
   virtual int main(int argc, const char* argv[]);

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
};

/**
 * Macro to call main on a custom App and a custom delegate.
 * 
 * @param appClassName class name of an App subclass.
 * @param delegateClassName class name of an App subclass.
 */
#define DB_APP_DELEGATE_MAIN(appClassName, delegateClassName) \
int main(int argc, const char* argv[])                        \
{                                                             \
   int rval = 1;                                              \
   appClassName* app = new appClassName;                      \
   delegateClassName* delegate = new delegateClassName;       \
   app->setDelegate(delegate);                                \
   if(app->initialize())                                      \
   {                                                          \
      rval = app->main(argc, argv);                           \
   }                                                          \
   else                                                       \
   {                                                          \
      db::app::App::printException();                         \
   }                                                          \
   delete delegate;                                           \
   delete app;                                                \
   return rval;                                               \
}

/**
 * Macro to call main on a custom App with no delegate.
 * 
 * @param appClassName class name of an App subclass.
 */
#define DB_APP_MAIN(appClassName)                             \
int main(int argc, const char* argv[])                        \
{                                                             \
   int rval = 1;                                              \
   appClassName* app = new appClassName;                      \
   if(app->initialize())                                      \
   {                                                          \
      rval = app->main(argc, argv);                           \
   }                                                          \
   else                                                       \
   {                                                          \
      db::app::App::printException();                         \
   }                                                          \
   delete app;                                                \
   return rval;                                               \
}

/**
 * Macro to call main on a db::app::App and a custom AppDelegate.
 * 
 * @param delegateClassName class name of an AppDelegate subclass.
 */
#define DB_DELEGATE_MAIN(delegateClassName) \
   DB_APP_DELEGATE_MAIN(db::app::App, delegateClassName)

} // end namespace app
} // end namespace db

#endif
