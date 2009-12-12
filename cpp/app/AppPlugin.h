/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_AppPlugin_H
#define monarch_app_AppPlugin_H

#include "monarch/logging/Logging.h"
#include "monarch/rt/Runnable.h"
#include "monarch/rt/Exception.h"
#include "monarch/config/ConfigManager.h"

namespace monarch
{
namespace app
{

/**
 * AppPlugins provide the main functionality of an App. Plugins allow an app
 * to be extended with new command line options, configuration settings, and
 * runtime behavior.
 *
 * This class provides a base plugin that can be used for subclasses.
 *
 * @author David I. Lehn
 */

/**
 * AppPluginInfo contains information about a particular plugin. The dependency
 * information is used to sort plugins so they run methods in a known order.
 *
 * AppPluginInfo
 * {
 *    "id" : AppPluginId,
 *    "dependencies": [] (AppPluginId)
 * }
 *
 * @member id the ID of the plugin.
 * @member dependencies an array of AppPluginIds that this plugin depends on.
 */
typedef monarch::rt::DynamicObject AppPluginInfo;
typedef monarch::rt::DynamicObjectIterator AppPluginInfoIterator;
typedef const char* AppPluginId;

// forward declaration
class App;

class AppPlugin
{
protected:
   /**
    * App owner of this plugin.
    */
   App* mApp;

   /**
    * AppPluginInfo for this plugin.
    */
   AppPluginInfo mInfo;

public:
   /**
    * Create an App instance.
    */
   AppPlugin();

   /**
    * Deconstruct this App instance.
    */
   virtual ~AppPlugin();

   /**
    * Called before a plugin is added to an app.  A plugin can refuse to be
    * added by returning false.
    *
    * @param app the app plugin is being added to.
    *
    * @return true on success, false and exception on failure.
    */
   virtual bool willAddToApp(App* app);

   /**
    * Called after a plugin has been added to an app.
    *
    * @param app the app plugin was added to.
    *
    * @return true on success, false and exception on failure.
    */
   virtual bool didAddToApp(App* app);

   /**
    * Initialize the app config manager. Can be used by plugins to replace or
    * configure the app's config manager.
    *
    * @return true on success, false and exception on failure.
    */
   virtual bool initConfigManager();

   /**
    * Called before initMetaConfig(). Used to initialize group and parent ids.
    *
    * Subclasses should call the superclass method.
    *
    * @param meta the meta config.
    *
    * @return true on success, false and exception on failure.
    */
   virtual bool willInitMetaConfig(monarch::config::Config& meta);

   /**
    * Initialize the meta config as needed. Note that subclasses may override
    * common group and parent ids in willInitMetaConfig() so the current values
    * should be used.
    *
    * Subclasses should call the superclass method.
    *
    * @param meta the meta config.
    *
    * @return true on success, false and exception on failure.
    */
   virtual bool initMetaConfig(monarch::config::Config& meta);

   /**
    * Called after initMetaConfig().
    *
    * Subclasses should call the superclass method.
    *
    * @param meta the meta config.
    *
    * @return true on success, false and exception on failure.
    */
   virtual bool didInitMetaConfig(monarch::config::Config& meta);

   /**
    * Set the owner App.
    *
    * @param app the App.
    */
   virtual void setApp(App* app);

   /**
    * Get the owner App.
    *
    * @return the owner App.
    */
   virtual App* getApp();

   /**
    * Get the AppPluginInfo for this plugin.
    *
    * @return the AppPluginInfo.
    */
   virtual AppPluginInfo getInfo();

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
   virtual monarch::rt::DynamicObject getCommandLineSpecs();

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
    * Called after the App processes the command line arguments.  Subclasses
    * may use this hook to check and process the command line args.
    *
    * Subclasses MUST call the superclass implementation first.
    *
    * @return true on success, false on failure and exception set
    */
   virtual bool didParseCommandLine();

   /**
    * Called before the App loads the configs setup in the meta config.
    * Subclasses may use this hook to setup the config manager or do other
    * preparation before configs are loaded.
    *
    * Subclasses MUST call the superclass implementation first.
    *
    * @return true on success, false on failure and exception set
    */
   virtual bool willLoadConfigs();

   /**
    * Called after the App loads configs from the meta config. Subclasses can
    * use this hook to load other configs or do other processing.
    *
    * Subclasses MUST call the superclass implementation first.
    *
    * @return true on success, false on failure and exception set
    */
   virtual bool didLoadConfigs();

   /**
    * Initialize logging.
    *
    * @return true on success, false on failure and exception set
    */
   virtual bool initializeLogging();

   /**
    * Cleanup logging.
    *
    * @return true on success, false on failure and exception set
    */
   virtual bool cleanupLogging();

   /**
    * Called before run().
    *
    * @return true on success, false on failure and exception set
    */
   virtual bool willRun();

   /**
    * Run the app plugin.
    *
    * If logging options need to be set on the apps config, do so in
    * willRun().
    *
    * @return true on success, false on failure and exception set
    */
   virtual bool run();

   /**
    * Called after run().
    *
    * @return true on success, false on failure and exception set
    */
   virtual bool didRun();
};

// type definition for a reference-counted AppPlugin
typedef monarch::rt::Collectable<AppPlugin> AppPluginRef;

} // end namespace app
} // end namespace monarch

#endif
