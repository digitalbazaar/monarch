/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_config_ConfigManager_H
#define db_config_ConfigManager_H

#include "db/rt/Object.h"
#include "db/util/DynamicObject.h"

#include <vector>

namespace db
{
namespace config
{

/**
 * A ConfigManager provides support for managing multiple sources of
 * configuration information, merging those configs into one top-level
 * view, and providing a diff between modifications to the view and the
 * merged config.  Configs are exposed as DynamicObjects.
 *
 * As you add configs they overlay previously added configs:
 * [sys0, sys1, sys2, user0, sys3, ...]
 * <-- low priority -- high priority -->
 *
 * getChanges() will retrieve the difference between the system configs and
 * the current config.  This allows for a preferences system to make it easy
 * to save user changes.
 * 
 * @author David I. Lehn
 */
class ConfigManager : public virtual db::rt::Object
{
protected:
   /**
    * Pair to hold config and system flag.
    */
   typedef std::pair<db::util::DynamicObject, bool> ConfigPair;

   /**
    * Source configs and system/user flag.
    */
   std::vector<ConfigPair> mConfigs;
   
   /**
    * Merged configuration after update().  Read/Write access.
    */
   db::util::DynamicObject mConfig;
   
   /**
    * Merge source over data in target.  Simple values are cloned.  Arrays
    * and Maps are iterated through recursively.
    * 
    * @param target destination to merge into.
    * @param source source to merge from. 
    */
   void merge(db::util::DynamicObject& target, db::util::DynamicObject& source);

   /**
    * Merge all configs into target.
    * 
    * @param target destination to merge into.
    * @param systemOnly only use system configs, not user configs
    */
   void makeMergedConfig(db::util::DynamicObject& target,
      bool systemOnly = true);

   /**
    * Compute the difference from dyno1 to dyno2 and store in diff.  Only
    * calculates new or updated elements.  Removed elements are not in the
    * diff.
    * 
    * @param diff diff destination.
    * @param dyno1 original DynamicObject
    * @param dyno2 new DynamicObject
    * 
    * @return true if diff found, else false
    */
   bool diff(db::util::DynamicObject& diff,
      db::util::DynamicObject& dyno1, db::util::DynamicObject& dyno2);

public:
   /**
    * Storage type for config ids.
    */
   typedef std::string::size_type ConfigId;
   
   /**
    * Magic value in config files to inherit default value when merging.
    * Useful for arrays.
    */
   static const char* DEFAULT_VALUE;

   /**
    * Check if a configuration has all values and types from a template
    * schema.
    * 
    * @param config configuration to check against the schema
    * @param schema template values and types to verify
    * 
    * @return true on success, false on failure with exception set
    */
   static bool isValidConfig(
      db::util::DynamicObject& config, db::util::DynamicObject& schema); 
   
   /**
    * Creates a new ConfigManager.
    */
   ConfigManager();
   
   /**
    * Destructs this JsonWriter.
    */
   virtual ~ConfigManager();
   
   /**
    * Sets option to minimize whitespace.
    * 
    * @return DynamicObject representation of all overlayed configurations
    */
   virtual db::util::DynamicObject getConfig();
   
   /**
    * Clear all configurations.  Invalidates previous addConfig() ids.
    */
   virtual void clear();
   
   /**
    * Adds a DynamicObject configuration.
    * 
    * @param dyno a config
    * @param system true if system config, false for user config
    * @param id location to store id of new config or NULL
    * 
    * @return true on success, false on failure and exception will be set.
    */
   virtual bool addConfig(db::util::DynamicObject& dyno,
      bool system = true, ConfigId* id = NULL);

   /**
    * Remove a configuration.
    * 
    * @param id config id returned from addConfig()
    * 
    * @return true on success, false on failure and exception will be set.
    */
   virtual bool removeConfig(ConfigId id);

   /**
    * Get a specific config.
    * 
    * @param id id returned by addConfig
    * @param dyno DynamicObject to set to return value
    * 
    * @return true on success, false on failure and exception will be set.
    */
   virtual bool getConfig(ConfigId id, db::util::DynamicObject& dyno);

   /**
    * Sets a specific config.
    * 
    * @param id id returned by addConfig
    * @param dyno the new config
    * 
    * @return true on success, false on failure and exception will be set.
    */
   virtual bool setConfig(ConfigId id, db::util::DynamicObject& dyno);

   /**
    * Update config from all current configs.  Update is called after
    * adding and removing configs.  It should also be called if an
    * individual config has data changed.
    */
   virtual void update();

   /**
    * Calculate the differences between the source configs and the
    * read-write config.  Only records new or updated elements.
    *
    * @param target object to store changes to
    * @param systemOnly only get changes between current and system configs
    */
   virtual void getChanges(db::util::DynamicObject& target,
      bool systemOnly = true);
};

} // end namespace data
} // end namespace db
#endif
