/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/config/ConfigManager.h"

#include <algorithm>
#include <vector>

#include "db/io/BufferedOutputStream.h"
#include "db/data/json/JsonReader.h"
#include "db/io/File.h"
#include "db/io/FileList.h"
#include "db/io/FileInputStream.h"
#include "db/logging/Logging.h"

using namespace std;
using namespace db::config;
using namespace db::data;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;

const char* ConfigManager::DEFAULT_VALUE = "__default__";
const char* ConfigManager::VERSION       = "__version__";
const char* ConfigManager::ID            = "__id__";
const char* ConfigManager::GROUP         = "__group__";
const char* ConfigManager::PARENT        = "__parent__";
const char* ConfigManager::MERGE         = "__merge__";
const char* ConfigManager::APPEND        = "__append__";
const char* ConfigManager::REMOVE        = "__remove__";
const char* ConfigManager::INCLUDE       = "__include__";
const char* ConfigManager::INCLUDE_EXT   = ".config";
const char* ConfigManager::TMP           = "__tmp__";
const char* ConfigManager::DIR_MAGIC     = "__dir__";

ConfigManager::ConfigManager() :
   mInvalidConfig(NULL)
{
   // initialize internal data structures
   mVersions->setType(Map);
   mConfigs->setType(Map);
}

ConfigManager::~ConfigManager()
{
}

/**
 * Static helper method to insert a config. This method assumes there is no
 * existing config with the passed ID and that any parent in the config is
 * valid.
 * 
 * @param id the config ID of the config to insert.
 * @param storage the config to use for storage.
 * @param raw the raw config to insert.
 */
static void insertConfig(
   ConfigManager::ConfigId id, Config& storage, Config& raw)
{
   Config& c = storage[id];
   c["children"]->setType(Array);
   c["raw"] = raw;
   
   // if has parent
   if(raw->hasMember(ConfigManager::PARENT))
   {
      // set parent and update parent's children
      ConfigManager::ConfigId parent = raw[ConfigManager::PARENT]->getString();
      c["parent"] = parent;
      storage[parent]["children"]->append() = id;
   }
}

/**
 * Static helper method that removes the config values from one config
 * from another config.
 * 
 * @param target the target config to update.
 * @param remove the config with entries to remove.
 */
static void removeLeafNodes(Config& target, Config& remove)
{
   // for each config entry, remove leaf nodes from parent config
   ConfigIterator i = remove.getIterator();
   while(i->hasNext())
   {
      Config& next = i->next();
      
      // proceed if value is in parent configuration
      if(target->hasMember(i->getName()))
      {
         if(next->getType() == Map || next->getType() == Array)
         {
            // empty map/array leaf node to be removed
            if(next->length() == 0)
            {
               target->removeMember(i->getName());
            }
            // recurse to find leaf node
            else
            {
               removeLeafNodes(target[i->getName()], next);
            }
         }
         else
         {
            // primitive type leaf node to be removed
            target->removeMember(i->getName());
         }
      }
   }
}

void ConfigManager::merge(Config& target, Config& source, bool append)
{
   if(source.isNull())
   {
      target = Config(NULL);
   }
   // if the source value is DEFAULT_VALUE then nothing needs to be done to
   // the target to modify it and it can be skipped here
   else if(
      source->getType() != String ||
      strcmp(source->getString(), DEFAULT_VALUE) != 0)
   {
      switch(source->getType())
      {
         case String:
         case Boolean:
         case Int32:
         case UInt32:
         case Int64:
         case UInt64:
         case Double:
            target = source.clone();
            break;
         case Map:
         {
            target->setType(Map);
            ConfigIterator i = source.getIterator();
            while(i->hasNext())
            {
               Config& next = i->next();
               merge(target[i->getName()], next, append);
            }
            break;
         }
         case Array:
         {
            target->setType(Array);
            ConfigIterator i = source.getIterator();
            for(int ii = 0; i->hasNext(); ii++)
            {
               merge(target[ii], i->next(), append);
            }
            break;
         }
      }
   }
}

void ConfigManager::makeMergedConfig(ConfigId id)
{
   // only need to do work if merged config doesn't already exist
   Config& config = mConfigs[id];
   if(!config->hasMember("merged"))
   {
      // produce a merged configuration that contains only config values, not
      // any "__special__" config format values
      Config merged(NULL);
      
      // get raw configuration
      Config& raw = config["raw"];
      
      // get merged config from parent
      if(config->hasMember("parent"))
      {
         ConfigId parent = mConfigs[id]["parent"]->getString();
         makeMergedConfig(parent);
         mConfigs[parent]["merged"].clone();
         
         // remove appropriate entries from parent config
         if(raw->hasMember(REMOVE))
         {
            removeLeafNodes(merged, raw[REMOVE]);
         }
         
         // merge appropriate entries
         if(raw->hasMember(MERGE))
         {
            merge(merged, raw[MERGE], false);
         }
         
         // add append field
         if(raw->hasMember(APPEND))
         {
            merge(merged, raw[APPEND], true); 
         }
      }
      else
      {
         // clone MERGE field, if it exists
         if(raw->hasMember(MERGE))
         {
            merged = raw[MERGE].clone();
            
            // add append field, if it exists
            if(raw->hasMember(APPEND))
            {
               merge(merged, raw[APPEND], true); 
            }
         }
         // clone APPEND field, if it exists
         else if(raw->hasMember(APPEND))
         {
            merged = raw[APPEND].clone();
         }
      }
   }
}

void ConfigManager::update(ConfigId id)
{
   // lock to modify internal storage
   mLock.lockExclusive();
   {
      // reproduce merged config for config ID
      mConfigs[id]->removeMember("merged");
      makeMergedConfig(id);
      
      // update each child of config ID
      DynamicObjectIterator i = mConfigs[id]["children"].getIterator();
      while(i->hasNext())
      {
         update(i->next()->getString());
      }
   }
   mLock.unlockExclusive();
}

void ConfigManager::replaceMagic(Config& config, DynamicObject& magicMap)
{
   if(config.isNull())
   {
      // pass
   }
   else
   {
      switch(config->getType())
      {
         case String:
         {
            const char* s = config->getString();
            if(magicMap->hasMember(s))
            {
               config = magicMap[s];
            }
            break;
         }
         case Boolean:
         case Int32:
         case UInt32:
         case Int64:
         case UInt64:
         case Double:
            break;
         case Map:
         case Array:
         {
            ConfigIterator i = config.getIterator();
            while(i->hasNext())
            {
               replaceMagic(i->next(), magicMap);
            }
            break;
         }
      }
   }
}

bool ConfigManager::diff(Config& target, Config& config1, Config& config2)
{
   bool rval = false;
   
   if(config1.isNull() && config2.isNull())
   {
      // same: no diff
   }
   else if(!config1.isNull() && config2.isNull())
   {
      // <stuff> -> NULL: diff=NULL
      rval = true;
      target = Config(NULL);
   }
   else if((config1.isNull() && !config2.isNull()) ||
      (config1->getType() != config2->getType()))
   {
      // NULL -> <stuff> -or- types differ: diff=config2
      rval = true;
      target = config2.clone();
   }
   else
   {
      // not null && same type: diff=deep compare
      switch(config1->getType())
      {
         case String:
         case Boolean:
         case Int32:
         case UInt32:
         case Int64:
         case UInt64:
         case Double:
            // compare simple types directly
            if(config1 != config2)
            {
               // changed: diff=config2
               rval = true;
               target = config2.clone();
            }
            break;
         case Map:
         {
            // compare config2 keys since we are only concerned with
            // additions and updates, not removals
            ConfigIterator i = config2.getIterator();
            while(i->hasNext())
            {
               Config next = i->next();
               const char* name = i->getName();
               if(strcmp(name, TMP) != 0)
               {
                  if(!config1->hasMember(name))
                  {
                     // key not in config1, so add to diff
                     rval = true;
                     target[name] = next.clone();
                  }
                  else
                  {
                     // recusively get sub-diff
                     Config d;
                     if(diff(d, config1[name], next))
                     {
                        // diff found, add it
                        rval = true;
                        target[name] = d;
                     }
                  }
               }
            }
            break;
         }
         case Array:
         {
            // compare config2 indexes since we are only concerned with
            // additions and updates, not removals
            ConfigIterator i = config2.getIterator();
            for(int ii = 0; i->hasNext(); ii++)
            {
               DynamicObject next = i->next();
               Config d;
               if(diff(d, config1[ii], next))
               {
                  // diff found
                  rval = true;
                  target[ii] = d;
               }
               else
               {
                  // set magic value
                  target[ii] = DEFAULT_VALUE;
               }
            }
            break;
         }
      }
   }
   
   return rval;
}

bool ConfigManager::checkConflicts(
   ConfigId id, Config& existing, Config& config)
{
   bool rval = false;
   
   if(existing.isSubset(config))
   {
      rval = true;
   }
   else
   {
      // calculate the diff to include it in the exception
      Config d;
      diff(d, existing, config);
      
      ExceptionRef e = new Exception(
         "Config conflict.", "db.config.ConfigManager.ConfigConflict");
      e->getDetails()["configId"] = id;
      e->getDetails()["diff"] = d;
      Exception::setLast(e, false);
   }
   
   return rval;
}

void ConfigManager::clear()
{
   mLock.lockExclusive();
   {
      mConfigs->clear();
   }
   mLock.unlockExclusive();
}

bool ConfigManager::addConfig(Config& config, bool include, const char* dir)
{
   bool rval = true;
   
   // get config ID
   ConfigId id;
   if(config->hasMember(ID))
   {
      id = config[ID]->getString();
   }
   else
   {
      ExceptionRef e = new Exception(
         "No valid config ID found.",
         "db.config.ConfigManager.MissingId");
      Exception::setLast(e, false);
      rval = false;
   }
   
   if(rval)
   {
      // read lock to check version & parent
      mLock.lockShared();
      {
         // check version is present
         if(mVersions->length() > 0)
         {
            if(!config->hasMember(VERSION))
            {
               ExceptionRef e = new Exception(
                  "No version found.",
                  "db.config.ConfigManager.UnspecifiedVersion");
               Exception::setLast(e, false);
               rval = false;
            }
            else
            {
               // check for known version
               const char* version = config[VERSION]->getString();
               if(!mVersions->hasMember(version))
               {
                  ExceptionRef e = new Exception(
                     "Unsupported version.",
                     "db.config.ConfigManager.UnsupportedVersion");
                  e->getDetails()["version"] = version;
                  Exception::setLast(e, false);
                  rval = false;
               }
               // if has parent
               else if(config->hasMember(PARENT))
               {
                  // ensure parent exists
                  ConfigId parent = config[PARENT]->getString();
                  if(!mConfigs->hasMember(parent))
                  {
                     ExceptionRef e = new Exception(
                        "Invalid parent config ID.",
                        "db.config.ConfigManager.InvalidParent");
                     e->getDetails()["configId"] = id;
                     e->getDetails()["parentId"] = parent;
                     Exception::setLast(e, false);
                     rval = false;
                  }
               }
            }
         }
      }
      mLock.unlockShared();
   }
   
   // process includes
   if(rval && include && config->hasMember(INCLUDE))
   {
      if(config[INCLUDE]->getType() != Array)
      {
         ExceptionRef e = new Exception(
            "Include directive value must be an array.",
            "db.config.ConfigManager.InvalidIncludeType");
         e->getDetails()["configId"] = id;
         e->getDetails()[INCLUDE] = config[INCLUDE];
         Exception::setLast(e, false);
         rval = false;
      }
      else
      {
         ConfigIterator i = config[INCLUDE].getIterator();
         while(rval && i->hasNext())
         {
            Config& next = i->next();
            bool load = true;
            bool optional = false;
            bool deep = false;
            bool magic = false;
            const char* path = NULL;
            
            if(next->getType() == String)
            {
               path = next->getString();
            }
            else if(next->getType() == Map)
            {
               if(next->hasMember("path"))
               {
                  path = next["path"]->getString();
               }
               else
               {
                  ExceptionRef e = new Exception(
                     "Missing include path.",
                     "db.config.ConfigManager.MissingIncludePath");
                  e->getDetails()["configId"] = id;
                  e->getDetails()[INCLUDE] = config[INCLUDE];
                  Exception::setLast(e, false);
                  rval = false;
               }
               // should include be loaded?
               if(next->hasMember("load"))
               {
                  load = next["load"]->getBoolean();;
               }
               // is include optional?
               if(next->hasMember("optional"))
               {
                  optional = next["optional"]->getBoolean();
               }
               // should subdirs be scanned too?
               if(next->hasMember("deep"))
               {
                  deep =  next["deep"]->getBoolean();
               }
               // replace magic strings?
               if(next->hasMember("magic"))
               {
                  magic =  next["magic"]->getBoolean();
               }
            }
            else
            {
               ExceptionRef e = new Exception(
                  "Invalid include value type.",
                  "db.config.ConfigManager.InvalidIncludeType");
               e->getDetails()["configId"] = id;
               e->getDetails()[INCLUDE] = config[INCLUDE];
               Exception::setLast(e, false);
               rval = false;
            }
            
            // if load, then load the included config file
            if(rval && load)
            {
               DB_CAT_DEBUG(DB_CONFIG_CAT, "Loading include: %s", path);
               rval = addConfigFile(
                  path, true, dir, optional, deep, magic);
            }
         }
      }
   }
   
   // add configuration
   if(rval)
   {
      // lock to add config to internal storage
      mLock.lockExclusive();
      {
         // get the group ID, use the config ID if there is no group
         ConfigId groupId = (config->hasMember(GROUP) ?
            config[GROUP]->getString() : id);
         
         // determine if the group differs from the config ID
         bool group = (strcmp(groupId, id) != 0);
         bool mergeConfig = false;
         bool mergeGroup = false;
         
         // if the config ID already exists, then we must ensure there are no
         // conflicts between them (also this optimizes the case where the
         // group ID is the same as the config ID or there is no group)
         if(mConfigs->hasMember(id))
         {
            mergeConfig = true;
            rval = checkConflicts(id, mConfigs[id]["raw"], config);
         }
         // if there is a group then we also must ensure that there are no
         // conflicts with the group config ID
         else if(group && mConfigs->hasMember(groupId))
         {
            mergeGroup = true;
            rval = checkConflicts(groupId, mConfigs[groupId]["raw"], config);
         }
         
         if(rval)
         {
            if(mergeConfig)
            {
               // merge the passed config into the existing config
               merge(mConfigs[id]["raw"], config, false);
            }
            else
            {
               // insert config
               insertConfig(id, mConfigs, config);
            }
            
            if(group)
            {
               if(mergeGroup)
               {
                  // merge the passed config into the existing group config
                  merge(mConfigs[groupId]["raw"], config, false);
               }
               else
               {
                  // insert group config
                  insertConfig(groupId, mConfigs, config);
               }
            }
         }
         
         if(rval)
         {
            // only update related merged configs
            update(id);
         }
      }
      mLock.unlockExclusive();
   }
   
   return rval;
}

bool ConfigManager::addConfigFile(
   const char* path, bool include, const char* dir,
   bool optional, bool deep, bool magic)
{
   bool rval = true;
   
   // FIXME: why are we doing this here? are we making assumptions about
   // how the config manager is being used?
   string userPath;
   rval = File::expandUser(path, userPath);
   if(rval)
   {
      string fullPath;
      
      // if dir set and expanded user dir not absolute, build a full path
      if(dir && !File::isPathAbsolute(userPath.c_str()))
      {
         fullPath.assign(File::join(dir, userPath.c_str(), NULL));
      }
      else
      {
         fullPath.assign(userPath);
      }
      
      File file(fullPath.c_str());
      if(!file->exists())
      {
         if(!optional)
         {
            ExceptionRef e = new Exception(
               "Configuration file not found.",
               "db.config.ConfigManager.FileNotFound");
            e->getDetails()["path"] = path;
            Exception::setLast(e, false);
            rval = false;
         }
      }
      else if(file->isFile())
      {
         // read in configuration
         DB_CAT_DEBUG(DB_CONFIG_CAT,
            "Loading config file: %s", fullPath.c_str());
         FileInputStream is(file);
         JsonReader r;
         Config cfg;
         r.start(cfg);
         rval = r.read(&is) && r.finish();
         is.close();
         
         if(rval)
         {
            // handle magic replacement
            string dirname = File::dirname(fullPath.c_str());
            if(magic)
            {
               DynamicObject magicMap;
               magicMap[DIR_MAGIC] = dirname.c_str();
               replaceMagic(cfg, magicMap);
            }
            rval = addConfig(cfg, include, dirname.c_str());
         }
         
         if(!rval)
         {
            ExceptionRef e = new Exception(
               "Configuration file load failure.",
               "db.config.ConfigManager.ConfigFileError");
            e->getDetails()["path"] = path;
            Exception::setLast(e, true);
            rval = false;
         }
      }
      else if(file->isDirectory())
      {
         DB_CAT_DEBUG(DB_CONFIG_CAT,
            "Loading config directory: %s", fullPath.c_str());
         FileList list;
         file->listFiles(list);
         
         // find all files with INCLUDE_EXT suffix
         vector<string> configFiles;
         vector<string> configDirs;
         IteratorRef<File> i = list->getIterator();
         while(i->hasNext())
         {
            File& f = i->next();
            string name = f->getName();
            if(f->isFile())
            {
               if(name.rfind(INCLUDE_EXT) ==
                  (name.length() - strlen(INCLUDE_EXT)))
               {
                  configFiles.push_back(File::basename(f->getName()));
               }
            }
            else if(
               deep && name != "." && name != ".." &&
               f->isDirectory())
            {
               configDirs.push_back(name);
            }
         }
         
         // sort alphanumerically to allow NN-whatever[.config] ordering
         sort(configFiles.begin(), configFiles.end());
         sort(configDirs.begin(), configDirs.end());
         
         // load each file in order
         for(vector<string>::iterator i = configFiles.begin();
             rval && i != configFiles.end(); i++)
         {
            rval = addConfigFile(
               (*i).c_str(), include, file->getName(),
               false, false, magic);
         }
         
         // load each dir in order
         for(vector<string>::iterator i = configDirs.begin();
             rval && i != configDirs.end(); i++)
         {
            const char* dir = (*i).c_str();
            rval = addConfigFile(dir, include, dir, false, false, magic);
         }
      }
      else
      {
         ExceptionRef e = new Exception(
            "Unknown configuration file type.",
            "db.config.ConfigManager.FileNotFound");
         Exception::setLast(e, false);
         rval = false;
      }
   }
   
   return rval;
}

bool ConfigManager::removeConfig(ConfigId id)
{
   bool rval = false;
   
   // lock to modify internal storage
   mLock.lockExclusive();
   {
      // FIXME: what happens if a parent or group is removed before
      // its child members are removed? eek! fail or just let the user
      // potentially burn themselves?, easy check is for
      // (mConfigs[id]["children"]->length() != 0)
      if(mConfigs->hasMember(id))
      {
         rval = true;
         
         // get all related configs
         DynamicObject configIds;
         configIds->setType(Array);
         if(mConfigs[id]->hasMember("parent"))
         {
            configIds->append(mConfigs[id]["parent"]);
         }
         configIds.merge(mConfigs[id]["children"], true);
         
         // remove config
         mConfigs->removeMember(id);
         
         // update all related configs
         DynamicObjectIterator i = configIds.getIterator();
         while(i->hasNext())
         {
            update(i->next()->getString());
         }
      }
      else
      {
         ExceptionRef e = new Exception(
            "Could not remove config. Invalid config ID.",
            "db.config.ConfigManager.InvalidId");
         Exception::setLast(e, false);
      }
   }
   mLock.unlockExclusive();
   
   return rval;
}

bool ConfigManager::getConfig(ConfigId id, Config& config, bool raw)
{
   bool rval = false;
   
   if(mConfigs->hasMember(id))
   {
      rval = true;
      config = (raw ? mConfigs[id]["raw"] : mConfigs[id]["merged"]);
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not get config. Invalid config ID.",
         "db.config.ConfigManager.InvalidId");
      Exception::setLast(e, false);
   }
   
   return rval;
}

Config& ConfigManager::getConfig(ConfigId id)
{
   Config* rval;
   
   if(mConfigs->hasMember(id))
   {
      rval = &mConfigs[id]["merged"];
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not get config. Invalid config ID.",
         "db.config.ConfigManager.InvalidId");
      Exception::setLast(e, false);
      rval = &mInvalidConfig;
   }
   
   return *rval;
}

void ConfigManager::addVersion(const char* version)
{
   mVersions[version] = true;
   DB_CAT_DEBUG(DB_CONFIG_CAT,
      "Add version: \"%s\"", (version != NULL) ? version : "(none)");
}

DynamicObject& ConfigManager::getVersions()
{
   return mVersions;
}
