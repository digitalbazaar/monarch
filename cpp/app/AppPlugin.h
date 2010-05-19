/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_AppPlugin_h
#define monarch_app_AppPlugin_h

#include "monarch/rt/DynamicObject.h"

namespace monarch
{
namespace app
{

// forward declaration
class App;

/**
 * An AppPlugin provides the custom behavior for an App. It allows an App to
 * be extended with new command line options, configuration settings, and
 * runs the App's custom behavior.
 *
 * @author Dave Longley
 * @author David I. Lehn
 */
class AppPlugin
{
protected:
   /**
    * The App owner of this AppPlugin.
    */
   App* mApp;

public:
   /**
    * Creates a AppPlugin instance.
    */
   AppPlugin();

   /**
    * Deconstructs this AppPlugin instance.
    */
   virtual ~AppPlugin();

   /**
    * Sets the owner App.
    *
    * @param app the App.
    */
   virtual void setApp(App* app);

   /**
    * Gets the owner App.
    *
    * @return the owner App.
    */
   virtual App* getApp();

   /**
    * Performs custom initialization.
    *
    * @return true if successful, false with exception set on failure.
    */
   virtual bool initialize();

   /**
    * Performs custom clean up.
    */
   virtual void cleanup();

   /**
    * Called to create and initialize configs.
    *
    * @return true on success, false with exception set on failure.
    */
   virtual bool initConfigs();

   /**
    * Gets the command line specification for this plugin. The spec is in the
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
    * Action keys which consume arguments cannot appear in parallel. Actions
    * which do not, such as setTrue/setFalse/inc/dec, can appear in parallel.
    *
    * Options that specify a "target" specify target options that can be one
    * of the following formats:
    * Specify a target DynamicObject directly:
    * ...["arg"]["target"] = <dyno>
    * A relative path from a root DynamicObject:
    * ...["arg"]["root"] = <dyno>
    * ...["arg"]["path"] = <string path>
    * A relative path in a named raw config. Will be set after changing.
    * ...["arg"]["config"] = <raw config name>
    * ...["arg"]["path"] = <string path>
    *
    * Paths are split on '.'. If a segment matches r"[^\]*\$" it is joined
    * with the next segment. Ie, if last char is a '\' but the last two chars
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
    * decoded as a JSON value. It can be any text that could appear as a JSON
    * value. (In other words, it does not have JSON top-level {} or []
    * requirement).
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
    * Note: This will read/write to a specific DynamicObject. Interaction with
    *       a multi-level ConfigManager setup may not be straightforward.
    *
    * Read next argument or arguments, convert to the DynamicObject type, and
    * store them. On error use argError message. The command line must have
    * enough arguments to satisfy the args array length.
    * "arg": DynamicObject
    * "args": [ target[, ...] ]
    * "argError": string
    *
    * Append arg or args to an Array DynamicObject:
    * "append": target
    *
    * Set a named config value. Reads its argument as a key=value pair. The
    * key will be read as a path. The "set" target is used to find the final
    * target via the path. Then this target is assigned the next argument via
    * the above "arg" process.
    * "set": target
    *
    * The default implementation will parse the following parameters:
    * -h, --help: print out default help and delegates help
    * -V --version: print out app name and version if present
    * -v, --verbose: set verbose mode for use by apps
    * --log-level: parse and set a log level variable
    *
    * @return the command line specification for this AppPlugin.
    */
   virtual monarch::rt::DynamicObject getCommandLineSpec();

   /**
    * Called before an App loads any of its unloaded config files. Subclasses
    * may use this hook to do preparation before configs are loaded.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool willLoadConfigs();

   /**
    * Called after an App loads config files. Subclasses can use this hook to
    * load other configs or do other processing.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool didLoadConfigs();

   /**
    * Gets the wait events for this plugin. These events will be waited for
    * by a parent App's kernel before it stops.
    *
    * The return value from plugins should be an array of objects of the form:
    * {
    *    "id": "{waiterId(string)}",
    *    "type": "{waitEventType(string)}",
    * }
    * May return an empty map.
    *
    * @return the plugin wait events.
    */
   virtual monarch::rt::DynamicObject getWaitEvents();

   /**
    * Runs this AppPlugin.
    *
    * @return true on success, false with exception set on failure.
    */
   virtual bool run();
};

} // end namespace app
} // end namespace monarch

#endif
