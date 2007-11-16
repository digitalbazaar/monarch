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
 * [config0, config1, config2, ...]
 * <-- low priority -- high priority -->
 * 
 * @author David I. Lehn
 */
class ConfigManager : public virtual db::rt::Object
{
protected:
   /**
    * Source configs.
    */
   std::vector<db::util::DynamicObject> mConfigs;
   
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
    */
   void makeMergedConfig(db::util::DynamicObject& target);

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
    * Storage type for config ids
    */
   typedef std::string::size_type ConfigId;
   
   /**
    * Magic value in config files to inherit default value when merging.
    * Useful for arrays.
    */
   static const char* DEFAULT_VALUE;
   
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
    * @param id location to store id of new config or NULL
    * 
    * @return true on success, false on failure and exception will be set.
    */
   virtual bool addConfig(db::util::DynamicObject dyno, ConfigId* id = NULL);

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
   virtual bool setConfig(ConfigId id, db::util::DynamicObject dyno);

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
    */
   virtual void getChanges(db::util::DynamicObject& target);
};

} // end namespace data
} // end namespace db
#endif
