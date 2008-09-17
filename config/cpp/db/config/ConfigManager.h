/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_config_ConfigManager_H
#define db_config_ConfigManager_H

#include "db/rt/DynamicObject.h"
#include "db/rt/DynamicObjectIterator.h"
#include "db/rt/Collectable.h"
#include "db/rt/WindowsSupport.h"

#include <vector>

namespace db
{
namespace config
{

#ifdef WIN32
#   ifdef BUILD_DB_CONFIG_DLL
#      define DLL_CLASS __WIN32_DLL_EXPORT
#   else
#      define DLL_CLASS __WIN32_DLL_IMPORT
#   endif
#else
#   define DLL_CLASS
#endif

#define DB_CONFIG_VERSION "DB Config"

// typedef for a config and its iterator
typedef db::rt::DynamicObject Config;
typedef db::rt::DynamicObjectIterator ConfigIterator;

/**
 * A ConfigManager provides support for managing multiple sources of
 * configuration information, merging those configs into one top-level
 * view, and providing a diff between modifications to the view and the
 * merged config. Configs are typedefs of DynamicObjects.
 *
 * As you add configs they overlay previously added configs:
 * [sys0, sys1, sys2, user0, sys3, ...]
 * <-- low priority -- high priority -->
 *
 * getChanges() will retrieve the difference between the system configs and
 * the current config. This allows for a preferences system to make it easy
 * to save user changes.
 * 
 * A special config key, ::INCLUDE ("__include__"), is available to control
 * including other files.  If present, the value of this key must be an
 * iterable (array/map) of values.  The type determines how the value is
 * handled:
 * String: The value is a required path to load.
 * Map: Options can be provided to control the include process:
 * "path": Path to include. (String, required)
 * "load": Supress load of the path. (Boolean, optional, default: true)
 * "optional": Supress failures if path not found.
 *    (Boolean, optional, default: false)
 * "user": Load a specific config as a User type rather tha Default.
 *    (Boolean, optional, default: false)
 * "deep": Load each subdir as a dir of configs.
 *    (Boolean, optional, default: false)
 * "magic": Recursively scan for magic keys and replace them with appropriate
 *    values. (Boolean, optional, default: false) 
 * 
 * A path can be one of the following:
 * - A explicit file to load
 * - A directory with files with ::INCLUDE_EXT (".config") extensions.
 * 
 * In the case of directories the paths are sorted first to allow for control
 * of file load order.
 * 
 * The "magic" include parameter allows for the following special value strings
 * to be replaced.  There is a slight performance penalty for using this
 * option due to config tree walking and string comparisons.
 * "__dir__": The directory of this config. (::DIR_MAGIC)
 * 
 * @author David I. Lehn
 */
class DLL_CLASS ConfigManager : public virtual db::rt::ExclusiveLock
{
public:
   /**
    * The type of a configuration.
    */
   enum ConfigType {
      /**
       * No type.
       */
      None,
      
      /**
       * Default configs.  Used for system defaults and those provided by
       * components.
       */
      Default,
      
      /**
       * User configs.  Provided by users of the system.
       */
      User,
      
      /**
       * Both Default and User configs
       */
      All
   };
   
protected:
   /**
    * A list of acceptable versions or empty list to accept all versions.
    * Uses the VERSION key of a config.
    */
   db::rt::DynamicObject mVersions;
   
   /**
    * Pair to hold config and system flag.
    */
   typedef std::pair<Config, ConfigType> ConfigPair;

   /**
    * Source configs and system/user flag.
    */
   std::vector<ConfigPair> mConfigs;
   
   /**
    * Merged configuration after update().  Read/Write access.
    */
   Config mConfig;
   
   /**
    * Merge source over data in target.  Simple values are cloned.  Arrays
    * and Maps are iterated through recursively.
    * 
    * @param target destination to merge into.
    * @param source source to merge from. 
    */
   void merge(Config& target, Config& source);

   /**
    * Merge all configs into target.
    * 
    * @param target destination to merge into.
    * @param types type of configs to merge.
    */
   void makeMergedConfig(Config& target, ConfigType types = Default);

   /**
    * Compute the difference from dyno1 to dyno2 and store in diff.  Only
    * calculates new or updated elements.  Removed elements are not in the
    * diff.
    * 
    * @param diff the Config to write the diff to.
    * @param config1 original Config.
    * @param config2 the new Config.
    * 
    * @return true if diff found, else false
    */
   bool diff(Config& diff, Config& config1, Config& config2);

   /**
    * Replaces magic values with appropriate values.  See the class docs.
    * 
    * @param config the Config to process.
    * @param magicMap a map of strings to replacement values.
    */
   void replaceMagic(Config& config, db::rt::DynamicObject& magicMap);

public:
   /**
    * Storage type for config ids.
    */
   typedef std::string::size_type ConfigId;
   
   /**
    * Magic value in config objects to inherit default value when merging.
    * Useful for arrays.
    */
   static const char* DEFAULT_VALUE;
   
   /**
    * Magic key in config object to specify the config object format version.
    */
   static const char* VERSION;
   
   /**
    * Magic key in a config object to specify a list of config files or
    * directories to include before this object.
    */
   static const char* INCLUDE;
   
   /**
    * Extension of files to load if an include directive is a directory.
    */
   static const char* INCLUDE_EXT;
   
   /**
    * Magic key for a property which is only temporary for this session.
    * getChanges and similar will skip this value.  Useful for run-time
    * caches and other data which should not be saved as user config.
    */
   static const char* TMP;
   
   /**
    * Magic config value which will be replaced with the directory a config
    * file was loaded from if the include "magic" option is used.
    */
   static const char* DIR_MAGIC;
   
   /**
    * Check if a configuration has all values and types from a template
    * schema.
    * 
    * @param config configuration to check against the schema.
    * @param schema template values and types to verify.
    * 
    * @return true on success, false on failure with exception set
    */
   static bool isValidConfig(Config& config, Config& schema); 
   
   /**
    * Creates a new ConfigManager.
    */
   ConfigManager();
   
   /**
    * Destructs this ConfigManager.
    */
   virtual ~ConfigManager();
   
   /**
    * Gets the main configuration for this ConfigManager.
    * 
    * @return the configuration of all overlayed configurations.
    */
   virtual Config& getConfig();
   
   /**
    * Clear all configurations. Invalidates previous addConfig() ids.
    */
   virtual void clear();
   
   /**
    * Adds a configuration.  The special key "__include__" can be used
    * to provide an array of files or directories of files to load if the
    * include parameter is true.  Note that there is currently no way to
    * get the ConfigId for included files.  See the class documentation for a
    * full description of the include functionality.
    * 
    * @param config the Config to add.
    * @param type the type of Config.
    * @param id the location to store the id of the new Config or NULL.
    * @param include process include directives.
    * @param dir directory of this config used for processing relative includes
    *        or NULL.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool addConfig(
      Config& config, ConfigType type = Default, ConfigId* id = NULL,
      bool include = true, const char* dir = NULL);
   
   /**
    * Adds a configuration file or directory of files with addConfig().
    * 
    * @param path the file or directory of files to parse and add.
    * @param type the type of Config.
    * @param id the location to store the id of the new Config or NULL.
    * @param include process include directives.
    * @param dir directory of this config used for processing relative includes
    *        or NULL.
    * @param optional true to supress failure if path is not found,
    *        false to require path to be present.
    * @param deep process subdirs as dirs of configs
    * @param magic replace magic strings with appropriate values.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool addConfig(
      const char* path, ConfigType type = Default, ConfigId* id = NULL,
      bool include = true, const char* dir = NULL, bool optional = false,
      bool deep = false, bool magic = false);
   
   /**
    * Removes a configuration.
    * 
    * @param id the Config's id (as returned from addConfig()).
    * 
    * @return true on success, false on failure and exception will be set.
    */
   virtual bool removeConfig(ConfigId id);

   /**
    * Get a specific config.
    * 
    * @param id the Config's id (as returned by addConfig()).
    * @param config the Config to populate with the retrieved Config.
    * 
    * @return true on success, false on failure and exception will be set.
    */
   virtual bool getConfig(ConfigId id, Config& config);

   /**
    * Sets a specific config.
    * 
    * @param id the Config's id (as returned by addConfig()).
    * @param config the new Config to use.
    * 
    * @return true on success, false on failure and exception will be set.
    */
   virtual bool setConfig(ConfigId id, Config& config);

   /**
    * Update config from all current configs.  Update is called after
    * adding and removing configs.  It should also be called if an
    * individual config has data changed.
    */
   virtual void update();

   /**
    * Calculate the differences between the source configs and the
    * read-write config. Only records new or updated config fields.
    *
    * @param target the Config to store the changes in.
    * @param baseType the type to compare against (useful values are Default,
    *        User, and All).
    * @param addVersion add the first version string added with addVersion
    *        to target.  Useful if target will be saved and needs to be
    *        reloaded.
    */
   virtual void getChanges(
      Config& target, ConfigType baseType = Default, bool addVersion = false);

   /**
    * Set the version of configurations this manager uses.  When adding a
    * config the VERSION string will be checked against this value if not NULL.
    * If a mismatch occurs and exception will be thrown.
    *
    * @param version a valid version to use.
    */
   virtual void addVersion(const char* version);

   /**
    * Return an Array of version this manager is configured to accept.
    *
    * This array starts off empty in order to accept any type of config.  A
    * define is available called DB_CONFIG_VERSION ("DB Config") for default
    * use but is not added by default.
    *  
    * @return an Array of versions or an empty to accept all versions.
    */
   virtual db::rt::DynamicObject& getVersions();
};

// define a reference counted type
typedef db::rt::Collectable<ConfigManager> ConfigManagerRef;

#undef DLL_CLASS

} // end namespace config
} // end namespace db
#endif
