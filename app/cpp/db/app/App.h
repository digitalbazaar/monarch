/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_app_App_H
#define db_app_App_H

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
 * Forward declaration.
 */
class App;

/**
 * Delegate interface for an application.
 * 
 * Author: David I. Lehn
 */
class AppDelegate
{
public:
   /**
    * Create an AppDelegate instance.
    */
   AppDelegate();
   
   /**
    * Deconstruct this AppDelegate instance.
    */
   virtual ~AppDelegate();
   
   /**
    * Called when App sets this object as its delegate.
    * 
    * @param app the App.
    */
   virtual void registeredForApp(App* app);
   
   /**
    * Get a specification of the command line paramters.  The spec is in the
    * following format:
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
    * If option found then set DynamicObject as appropriate:
    * "setTrue": DynamicObject | [ DynamicObject[, ...] ]
    * "setFalse": DynamicObject | [ DynamicObject[, ...] ]
    * 
    * If option found then increment or decrement DynamicObject value by 1:
    * "inc": DynamicObject | [ DynamicObject[, ...] ]
    * "dec": DynamicObject | [ DynamicObject[, ...] ]
    * 
    * Read next argument or arguments, convert to the DynamicObject type, and
    * store them.  On error use argError message.  The command line must have
    * enough arguments to satisfy the args array length.
    * "arg": DynamicObject
    * "args": [ DynamicObject[, ...] ]
    * "argError": string
    * 
    * Append arg or args to an Array DynamicObject:
    * "append": DynamicObject
    * 
    * Set a named config value.  Reads the first argument as a path.  The "set"
    * target is used to find the final target.  Then this target is assigned
    * the next argument via the above "arg" process.
    * "set": DynamicObject
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
    * If spec key "isJsonValue" exists and is true then the value argument will
    * be decoded as a JSON value.  It can be any text that could appear as a
    * JSON value.  (In other words, it does not have JSON top-level {} or []
    * requirement)
    * 
    * @return the command line spec
    */
   virtual db::rt::DynamicObject getCommandLineSpec();
   
   /**
    * Called before the default App processes the command line arguments.
    * AppDelegates may use this hook to process arguments in a read-only mode.
    * 
    * This hook should be used if a delegate needs to processes arguments
    * before normal default App processing.
    * 
    * @param args read-only vector of command line arguments.
    * 
    * @return true on success, false on failure and exception set
    */
   virtual bool willParseCommandLine(std::vector<const char*>* args);

   /**
    * Called after the default App processes the command line arguments.
    * AppDelegates may use this hook to check and process the command line
    * args.
    * 
    * @return true on success, false on failure and exception set
    */
   virtual bool didParseCommandLine();

   /**
    * Called after App::initializeLogging()
    */
   virtual void didInitializeLogging();
   
   /**
    * Called before App::cleanupLogging()
    */
   virtual void willCleanupLogging();
   
   /**
    * See run().
    * 
    * @return true on success, false on failure and exception set
    */
   virtual bool initializeRun();
   
   /**
    * Run the app.  The sequence of events is (error handling not shown):
    * 
    * delegate->initializeRun()
    * app->startLogging()
    * delegate->runApp()
    * app->stopLogging();
    * delegate->cleanupRun();
    * 
    * If logging options need to be set on the apps config, do so in
    * initializeRun(). 
    * 
    * @return true on success, false on failure and exception set
    */
   virtual bool runApp();
   
   /**
    * See run().
    */
   virtual void cleanupRun();
};

/**
 * Top-level class to make running applications easier.
 * 
 * To use: create a App subclass, override the appropriate methods, and use a
 * main() function which works similar to the following or use the
 * DB_APP_MAIN(AppClassName) macro.
 * 
 * int main(int argc, const char* argv[])
 * {
 *    int rval;
 *    AppClassName app();
 *    return app.main(argc, argv);
 * }
 * 
 * The common order that overridden App methods are called is:
 * - parseCommandLine(argc, argv)
 * - initializeLogging()
 * - initialize()
 * - run()
 * - cleanup()
 * - cleanupLogging()
 * 
 * Author: David I. Lehn
 */
class App :
   public db::rt::Runnable,
   public AppDelegate
{
protected:
   /**
    * Delegate for this application.
    */
   AppDelegate* mDelegate;
   
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
    * Configuration for this App.
    */
   db::config::Config mAppConfig;
   
   /**
    * Temporary command line options and specs storage.
    */
   db::config::Config mCLConfig;
   
   /**
    * ConfigManager for this App.
    */
   db::config::ConfigManagerRef mConfigManager;
   
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
    * @return true on succes, false and exception on failure.
    */
   virtual bool startLogging();
   
   /**
    * Stop the default logger if enabled;
    * 
    * @return true on succes, false and exception on failure.
    */
   virtual bool stopLogging();
   
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
    * Set the app delegate.
    * 
    * @param delegate the app delegate.
    */
   virtual void setDelegate(AppDelegate* delegate);

   /**
    * Get the app delegate.
    * 
    * @return the app delegate.
    */
   virtual AppDelegate* getDelegate();

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
    * Gets this app's Config.  This config is automatically added to the
    * default ConfigManager.  Remember to call update on the ConfigManager
    * after updates.
    * 
    * @return the Config for this app.
    */
   virtual db::config::Config& getConfig();
   
   /**
    * Gets this app's ConfigManager.
    * 
    * @return the ConfigManager for this app.
    */
   virtual db::config::ConfigManagerRef& getConfigManager();
   
   /**
    * Run all tests and set mExitStatus.
    */
   virtual void run();
   
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
    * Get command line spec for default paramters which will be stored in the
    * object returned from getConfig().
    * 
    * The default implementation will parse the following parameters:
    * -h, --help: print out default help and delegates help
    * -V --version: print out app name and version if present
    * -v, --verbose: set verbose mode for use by apps
    * --log-level: parse and set a log level variable
    * 
    * @return the command line spec
    */
   virtual db::rt::DynamicObject getCommandLineSpec();

   /**
    * Setup default for default command line options.
    * 
    * @param args read-only vector of command line arguments.
    * 
    * @return true on success, false on failure and exception set
    */
   virtual bool willParseCommandLine(std::vector<const char*>* args);
   
   /**
    * Process default command line options.
    * 
    * @return true on success, false on failure and exception set
    */
   virtual bool didParseCommandLine();
   
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
    */
   void printException(db::rt::ExceptionRef& e);

   /**
    * Pretty print last exception.
    */
   void printException();
};

/**
 * Macro to call main on a custom App and a custom AppDelegate.
 * 
 * @param appClassName class name of an App subclass.
 * @param delegateClassName class name of an AppDelegate subclass.
 */
#define DB_APP_DELEGATE_MAIN(appClassName, delegateClassName) \
int main(int argc, const char* argv[])                        \
{                                                             \
   int rval;                                                  \
   appClassName app;                                          \
   delegateClassName delegate;                                \
   app.setDelegate(&delegate);                                \
   rval = app.main(argc, argv);                               \
   return rval;                                               \
}

/**
 * Macro to call main on a custom App with no delegate.
 * 
 * @param appClassName class name of an App subclass.
 */
#define DB_APP_MAIN(appClassName)      \
int main(int argc, const char* argv[]) \
{                                      \
   int rval;                           \
   appClassName app;                   \
   rval = app.main(argc, argv);        \
   return rval;                        \
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
