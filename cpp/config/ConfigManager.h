/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_config_ConfigManager_H
#define db_config_ConfigManager_H

#include "db/rt/DynamicObject.h"
#include "db/rt/DynamicObjectIterator.h"
#include "db/rt/SharedLock.h"

namespace db
{
namespace config
{

#define DB_DEFAULT_CONFIG_VERSION "DB Config 3.0"

// typedef for a config and its iterator
typedef db::rt::DynamicObject Config;
typedef db::rt::DynamicObjectIterator ConfigIterator;

// forward declare config change listener
class ConfigChangeListener;

/**
 * A ConfigManager provides support for managing multiple sources of
 * configuration information. Each different configuration source (referred to
 * as a "config") has a unique ID. Each config may also have relationships
 * with other sources, namely, it may have one parent and many children.
 *
 * Each config's raw information is stored internally and can be retrieved. In
 * addition to the raw information, a merge of each config and its tree of
 * parent configs is stored and can be retrieved. This allows configs to be
 * overlayed with one another to produce a rich layered configuration system.
 *
 * Configs may also declare that they are members in a particular "group". A
 * group functions in a similar fashion to a normal config, having a
 * config ID (which is the same as its group ID), and having both a
 * "raw" and a merged configuration. Members in the same group cannot have
 * conflicting raw information, or else an exception will be raised. It is
 * assumed that the members in a group will contribute different and/or
 * compatible configuration information to a single group config. This allows
 * configurations to be split up amongst files and it allows other configs
 * to set their parents to a particular group ID, thereby sharing the aggregate
 * configuration information.
 *
 * Configs are typedefs of DynamicObjects.
 *
 * A comprehensive example of the use of this system is as follows:
 *
 * 1. config1 is assigned ID "system".
 * 2. config2 is assigned ID "engine", group ID "app", and parent ID "system".
 * 3. config3 is assigned ID "ui", group ID "app", and parent ID "system".
 * 4. config4 is assigned ID "user1" and parent ID "app".
 * 5. config5 is assigned ID "user2" and parent ID "app".
 * Note: A config for the "app" group will be implicitly created.
 *
 * Each config's (including the group "app") specific raw information can be
 * retrieved on its own. This data acts as a "diff" between the config and its
 * parent. Also, if config2 or config3's (for instance) "merged" configuration
 * information is desired, it can be retrieved. This data will show all of the
 * "system" configuration values with config2 or config3's (respectively) values
 * overlayed on top of it. Similarly, if config4 or config5's merged data is
 * retrieved it will show their specific options overlayed on the "app" config's
 * merged data (which includes all of system's config with config2 AND config3
 * merged on top of it).
 *
 * A special config key, ::INCLUDE ("_include_"), is available to control
 * including other files.  If present, the value of this key must be an
 * iterable (array/map) of values.  The type determines how the value is
 * handled:
 * String: The value is a required path to load.
 * Map: Options can be provided to control the include process:
 * "path": Path to include. (String, required)
 * "load": Suppress load of the path. (Boolean, optional, default: true)
 * "optional": Suppress failures if path not found.
 *    (Boolean, optional, default: false)
 * "defaults": Load a specific config as Defaults.
 *    (Boolean, optional, default: true)
 * "includeSubdirectories": Load each subdirectory as a directory of configs.
 *    (Boolean, optional, default: false)
 *
 * A path can be one of the following:
 * - A explicit file to load
 * - A directory with files with ::INCLUDE_EXT (".config") extensions.
 *
 * In the case of directories the paths are sorted first to allow for control
 * of file load order.
 *
 * The setKeyword() method can be used for custom keywords. There is a slight
 * performance penalty for using this option due to config tree walking and
 * string comparisons. The following are special reserved keywords that are
 * also available.
 *
 * "{CURRENT_DIR}": The directory of this config file.
 *
 * @author David I. Lehn
 * @author Dave Longley
 * @author Manu Sporny
 */
class ConfigManager
{
public:
   /**
    * Config IDs are strings.
    */
   typedef const char* ConfigId;

   /**
    * Keyword substitution value in config objects to inherit default value
    * when merging. Useful for arrays.
    */
   static const char* DEFAULT_VALUE;

   /**
    * Keyword key in config object to specify the config object format version.
    */
   static const char* VERSION;

   /**
    * Keyword in a config object to specify its ID.
    */
   static const char* ID;

   /**
    * Keyword in a config object to specify its group.
    */
   static const char* GROUP;

   /**
    * Keyword in a config object to specify its parent's ID.
    */
   static const char* PARENT;

   /**
    * Keyword in a config object to specify the values for config, which
    * will be merged with a parent config, if one is specified.
    */
   static const char* MERGE;

   /**
    * Keyword in a config object to specify configuration values to append to,
    * as opposed to merge with, a parent configuration.
    */
   static const char* APPEND;

   /**
    * Keyword in a config object to specify configuration values to remove
    * from a parent configuration.
    */
   static const char* REMOVE;

   /**
    * Keyword in a config object to specify a list of config files or
    * directories to include before this object.
    */
   static const char* INCLUDE;

   /**
    * Extension of files to load if an include directive is a directory.
    */
   static const char* INCLUDE_EXT;

   /**
    * Keyword for a property which is only temporary for this session.
    * getChanges and similar will skip this value.  Useful for run-time
    * caches and other data which should not be saved as non-default config.
    */
   static const char* TMP;

   /**
    * Replaces keyword values with appropriate values.  See the class docs.
    *
    * @param config the Config to process.
    * @param keywordMap a map of strings to replacement values.
    */
   static void replaceKeywords(
      Config& config, db::rt::DynamicObject& keywordMap);

protected:
   /**
    * A map of acceptable versions or empty list to accept all versions.
    * Uses the VERSION key of a config.
    */
   db::rt::DynamicObject mVersions;

   /**
    * A map of replacement keywords that can be used on strings contained in
    * configuration files.
    */
   db::rt::DynamicObject mKeywordMap;

   /**
    * The stored configurations. This object has the following format:
    *
    * Map (ConfigId -> configuration data)
    * {
    *    members: [] of ConfigIds (members of a group)
    *    children: [] of ConfigIds (child config IDs)
    *    raw : Config (raw configuration for the given ID)
    *    merged : Config (merged w/parent configuration for the given ID)
    * }
    */
   Config mConfigs;

   /**
    * A lock for modifying the internal configuration data.
    */
   db::rt::SharedLock mLock;

   /**
    * An interface to report configuration changes to.
    */
   ConfigChangeListener* mConfigChangeListener;

public:
   /**
    * Creates a new ConfigManager.
    */
   ConfigManager();

   /**
    * Destructs this ConfigManager.
    */
   virtual ~ConfigManager();

   /**
    * Get internal debug info.
    *
    * @return debug info
    */
   virtual db::rt::DynamicObject getDebugInfo();

   /**
    * Clear all configurations. Invalidates previous addConfig() ids.
    */
   virtual void clear();

   /**
    * Adds a configuration.
    *
    * The special key "_id_" *must* be present to specify the configuration's
    * ID.
    *
    * The special key "_parent_" can be provided if the configuration has a
    * parent configuration that it should receive default values from and be
    * merged with.
    *
    * The special key "_include_" can be used to provide an array of files
    * or directories of files to load if the passed include parameter is true.
    *
    * The special key "_version_" must be provided if this configuration
    * manager requires specific configuration versions.
    *
    * @param config the Config to add.
    * @param include true to process include directives, false to ignore them.
    * @param dir the directory of this config used for processing relative
    *            includes or NULL.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool addConfig(
      Config& config, bool include = true, const char* dir = NULL);

   /**
    * Adds a configuration file or directory of files with addConfig().
    *
    * @param path the file or directory of files to parse and add.
    * @param include true to process include directives, false to ignore them.
    * @param dir the directory of this config used for processing relative
    *            includes or NULL.
    * @param optional true to suppress failure if path is not found,
    *                 false to require path to be present.
    * @param processSubdirectories true to process subdirs as dirs of configs.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool addConfigFile(
      const char* path, bool include = true, const char* dir = NULL,
      bool optional = false, bool processSubdirectories = false);

   /**
    * Removes a configuration.
    *
    * @param id the Config's ID.
    *
    * @return true on success, false on failure and exception will be set.
    */
   virtual bool removeConfig(ConfigId id);

   /**
    * Sets the a particular config's raw data and updates any related
    * configs.
    *
    * @param config the config to update.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool setConfig(Config& config);

   /**
    * Gets a specific config by its ID. This method will return a clone
    * of the raw configuration if "raw" is set to true. To set the
    * configuration, setConfig() must be called with the returned "raw"
    * clone. If "raw" is not true, then a READ-ONLY copy of the merged
    * configuration will be returned. The read-only state of the config is
    * enforced only by policy and any modification to the config may have
    * undefined side effects.
    *
    * Config groups can be retrieved via their group ID, however, they
    * do not have any associated raw configuration, only a merged config
    * can be retrieved. Requests for the raw config with a group ID will
    * return a NULL config.
    *
    * @param id the Config's ID.
    * @param raw true to get the raw config, false to get the config as merged
    *            with all up-tree parents.
    * @param cache true to cache any generated merged config, false not to.
    *
    * @return the Config or NULL if the ID was invalid.
    */
   virtual Config getConfig(ConfigId id, bool raw = false, bool cache = true);

   /**
    * Check if a config with a specific ID exists.
    *
    * @param id the Config's ID.
    *
    * @return true if ID is valid, false if not.
    */
   virtual bool hasConfig(ConfigId id);

   /**
    * Get ConfigIds in a group.
    *
    * @param id the Config's ID.
    *
    * @return DynamicObject Array with ids of configs in a group, if any.
    */
   virtual db::rt::DynamicObject getIdsInGroup(ConfigId groupId);

   /**
    * Reproduces the merged config for the given config ID. This method is
    * automatically called when adding, removing, or setting a config, it
    * should not ordinarily need to be called manually.
    *
    * @param id the ID of the config to update (all related configs will be
    *           updated).
    * @param changedIds an optional map to populate with changed config IDs.
    */
   virtual void update(ConfigId id, db::rt::DynamicObject* changedIds = NULL);

   /**
    * Associates a value with a keyword such that variable replacement can
    * happen in JSON strings.
    *
    * @param keyword The keyword to use in the config manager.
    * @param value The value to associate with the keyword.
    */
   virtual void setKeyword(const char* keyword, const char* value);

   /**
    * Set the version of configurations this manager uses.  When adding a
    * config the VERSION string will be checked against this value if not NULL.
    * If a mismatch occurs and exception will be thrown.
    *
    * @param version a valid version to use.
    */
   virtual void addVersion(const char* version);

   /**
    * Return a Map of versions this manager is configured to accept.  Acceptable
    * versions are set to true.
    *
    * This map is initialized with DB_DEFAULT_CONFIG_VERSION.
    *
    * @return an Array of versions or an empty to accept all versions.
    */
   virtual db::rt::DynamicObject& getVersions();

   /**
    * Sets the configuration change listener for this config manager.
    *
    * @param listener the listener to notify when a config changes.
    */
   virtual void setConfigChangeListener(ConfigChangeListener* listener);

   /**
    * Gets the configuration change listener for this config manager.
    *
    * @return the configuration change listener for this config manager.
    */
   ConfigChangeListener* getConfigChangeListener();

protected:
   /**
    * Merges source over data in target. Simple values are cloned. Arrays
    * and Maps are iterated through recursively.
    *
    * @param target destination to merge into.
    * @param source source to merge from.
    * @param append true to append array values, false to merge them.
    */
   virtual void merge(Config& target, Config& source, bool append);

   /**
    * Merges raw and parent merged configs and stores them in the "merged"
    * property for the given config ID. The merged configuration has no
    * no "__special__" tags. This method assumes the lock for modifying
    * internal storage is engaged in exclusive mode.
    *
    * @param id the config ID to create the merged config for.
    * @param out the config to store the merged config in *if and only if*
    *            the generated merged config should not be cached, NULL
    *            to enable caching.
    */
   virtual void makeMergedConfig(ConfigId id, Config* out);

   /**
    * Gets a merged config, lazily creating that merged config if necessary.
    * A merged config can be retrieved without caching it, which is
    * particularly useful when first initializing a config system. If a merged
    * config is not cached, it will not need to be updated when new configs
    * are added to the system.
    *
    * @param id the Config's ID.
    * @param cache true to cache any generated merged config, false not to.
    *
    * @return the Config or NULL if the ID was invalid.
    */
   virtual Config getMergedConfig(ConfigId id, bool cache);

   /**
    * Computes the differences from config1 to config2 and stores them in
    * target. Only includes diff additions for main properties (i.e. VERSION,
    * PARENT, GROUP), ignores all diffs in APPEND and REMOVE, and only includes
    * diff updates in MERGE.
    *
    * @param diff the Config to write the diff to.
    * @param config1 original Config.
    * @param config2 the new Config.
    * @param level set by recursive algorithm, must be initialized to 0.
    *
    * @return true if diff found, else false
    */
   virtual bool diff(
      Config& target, Config& config1, Config& config2, int level = 0);

   /**
    * Helper method to check two configs for conflicts. There is a conflict
    * between the two configs if "existing" has a main property (i.e. parent,
    * group, version, etc.) or a merge value that differs from "config".
    *
    * @param id the config ID of the two configs.
    * @param existing the existing config.
    * @param config the new config.
    * @param isGroup true if the given config ID is for a group, false if not.
    *
    * @return true if no conflict, false if conflict with exception set.
    */
   virtual bool checkConflicts(
      ConfigId id, Config& existing, Config& config, bool isGroup);

   /**
    * A helper function that converts the old merged config value stored
    * at each changed ID to a diff between the old merged config and the
    * new merged config, removing any ID entries that have no difference.
    *
    * @param changedIds the map of config ID to old merged configs to update.
    */
   virtual void produceMergedDiffs(db::rt::DynamicObject& changedIds);

   /**
    * The recursive version of addConfig() called by addConfig().
    *
    * @param config the Config to add.
    * @param include true to process include directives, false to ignore them.
    * @param dir the directory of this config used for processing relative
    *            includes or NULL.
    * @param changedIds an optional map to populate with changed config IDs.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool recursiveAddConfig(
      Config& config, bool include, const char* dir,
      db::rt::DynamicObject* changedIds = NULL);
};

} // end namespace config
} // end namespace db
#endif
