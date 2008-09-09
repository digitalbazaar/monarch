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
const char* ConfigManager::VERSION = "__version__";
const char* ConfigManager::INCLUDE = "__include__";
const char* ConfigManager::INCLUDE_EXT = ".config";
const char* ConfigManager::TMP = "__tmp__";
const char* ConfigManager::DIR_MAGIC = "__dir__";

ConfigManager::ConfigManager()
{
   mConfig->setType(Map);
   // empty version array
   mVersions->setType(Array);
}

ConfigManager::~ConfigManager()
{
}

Config& ConfigManager::getConfig()
{
   return mConfig;
}

void ConfigManager::clear()
{
   lock();
   {
      mConfigs.clear();
      update();
   }
   unlock();
}

bool ConfigManager::addConfig(
   Config& config, ConfigType type, ConfigId* id, bool include, const char* dir)
{
   bool rval = true;
   
   lock();
   {
      // check version is present
      if(mVersions->length() > 0)
      {
         if(!config->hasMember(VERSION))
         {
            ExceptionRef e =
               new Exception("No version found.", "db.config.ConfigError");
            Exception::setLast(e, false);
            rval = false;
         }
         else
         {
            // check for known version
            const char* version = config[VERSION]->getString();
            
            // scan versions
            DynamicObjectIterator vi = mVersions.getIterator();
            bool found = false;
            while(!found && vi->hasNext())
            {
               DynamicObject& v = vi->next();
               found = (strcmp(
                  config[VERSION]->getString(), v->getString()) == 0);
            }
            
            if(!found)
            {
               ExceptionRef e =
                  new Exception("Unknown version.", "db.config.ConfigError");
               e->getDetails()["version"] = version;
               Exception::setLast(e, false);
               rval = false;
            }
         }
      }
      // process includes
      if(rval && include && config->hasMember(INCLUDE))
      {
         if(config[INCLUDE]->getType() != Array)
         {
            ExceptionRef e =
               new Exception("Include directive value must be an array.",
                  "db.config.ConfigError");
            e->getDetails()[INCLUDE] = config[INCLUDE];
            Exception::setLast(e, false);
            rval = false;
         }
         else
         {
            ConfigIterator i = config[INCLUDE].getIterator();
            while(rval && i->hasNext())
            {
               Config next = i->next();
               bool load = true;
               bool optional = false;
               bool deep = false;
               bool magic = false;
               ConfigType type = Default;
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
                     ExceptionRef e =
                        new Exception("Missing include path.",
                           "db.config.ConfigError");
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
                  // check for user override of type parameter
                  if(next->hasMember("user"))
                  {
                     type =  next["user"]->getBoolean() ? User : Default;
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
                  ExceptionRef e =
                     new Exception("Invalid include value type.",
                        "db.config.ConfigError");
                  e->getDetails()[INCLUDE] = config[INCLUDE];
                  Exception::setLast(e, false);
                  rval = false;
               }
               if(rval && load)
               {
                  DB_CAT_DEBUG(DB_CONFIG_CAT, "Loading include: %s", path);
                  rval = addConfig(
                     path, type, NULL, true, dir, optional, deep, magic);
               }
            }
         }
      }
      // add to configs
      if(rval)
      {
         mConfigs.push_back(ConfigPair(config, type));
         if(id != NULL)
         {
            *id = mConfigs.size() - 1;
         }
         update();
      }
   }
   unlock();
   
   return rval;
}

bool ConfigManager::addConfig(
   const char* path, ConfigType type, ConfigId* id, bool include,
   const char* dir, bool optional, bool deep, bool magic)
{
   bool rval = true;
   
   lock();
   {
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
         if(file->exists())
         {
            if(file->isFile())
            {
               DB_CAT_DEBUG(DB_CONFIG_CAT,
                  "Loading file: %s", fullPath.c_str());
               FileInputStream is(file);
               JsonReader r;
               Config cfg;
               r.start(cfg);
               rval = r.read(&is);
               if(rval)
               {
                  rval = r.finish();
               }
               if(rval)
               {
                  string dirname = File::dirname(fullPath.c_str());
                  if(magic)
                  {
                     DynamicObject magicMap;
                     magicMap[DIR_MAGIC] = dirname.c_str();
                     replaceMagic(cfg, magicMap);
                  }
                  rval = addConfig(cfg, type, id, include, dirname.c_str());
               }
               if(!rval)
               {
                  ExceptionRef e =
                     new Exception("Configuration file load failure.",
                        "db.config.ConfigFileError");
                  e->getDetails()["path"] = path;
                  Exception::setLast(e, true);
                  rval = false;
               }
            }
            else if(file->isDirectory())
            {
               DB_CAT_DEBUG(DB_CONFIG_CAT,
                  "Loading directory: %s", fullPath.c_str());
               FileList list;
               file->listFiles(list);
   
               // find all files with INCLUDE_EXT suffix
               vector<string> configFiles;
               vector<string> configDirs;
               db::rt::IteratorRef<File> i = list->getIterator();
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
                  else if(deep && name != "." && name != ".." &&
                     f->isDirectory())
                  {
                     configDirs.push_back(name);
                  }
               }
               
               // sort alphanumerically to allow NN-whatever[.config] ordering
               sort(configFiles.begin(), configFiles.end());
               sort(configDirs.begin(), configDirs.end());
               
               // load each file in order as
               for(vector<string>::iterator i = configFiles.begin();
                  rval && i != configFiles.end();
                  i++)
               {
                  rval = addConfig(
                     (*i).c_str(), type, NULL, include, file->getName());
               }
               
               // load each dir in order as
               for(vector<string>::iterator i = configDirs.begin();
                  rval && i != configDirs.end();
                  i++)
               {
                  const char* dir = (*i).c_str();
                  rval = addConfig(
                     dir, type, NULL, include, dir, false, false, magic);
               }
            }
            else
            {
               ExceptionRef e =
                  new Exception(
                     "Unknown configuration file type.",
                     "db.config.FileNotFound");
               Exception::setLast(e, false);
               rval = false;
            }
         }
         else
         {
            if(!optional)
            {
               ExceptionRef e =
                  new Exception("Configuration file not found.",
                     "db.config.FileNotFound");
               e->getDetails()["path"] = path;
               Exception::setLast(e, false);
               rval = false;
            }
         }
      }
   }
   unlock();
   
   return rval;
}

bool ConfigManager::removeConfig(ConfigId id)
{
   bool rval = false;
   
   lock();
   {
      if(id >= 0 && id < mConfigs.size())
      {
         mConfigs[id] = ConfigPair(Config(NULL), None);
         update();
         rval = true;
      }
      else
      {
         ExceptionRef e = new Exception("Invalid ConfigId");
         Exception::setLast(e, false);
      }
   }
   unlock();
   
   return rval;
}

bool ConfigManager::getConfig(ConfigId id, Config& config)
{
   bool rval = false;
   
   if(id >= 0 && id < mConfigs.size())
   {
      rval = true;
      config = mConfigs[id].first;
   }
   else
   {
      ExceptionRef e = new Exception("Invalid ConfigId");
      Exception::setLast(e, false);
   }
   
   return rval;
}

bool ConfigManager::setConfig(ConfigId id, Config& config)
{
   bool rval = false;
   
   if(id >= 0 && id < mConfigs.size())
   {
      rval = true;
      mConfigs[id].first = config;
      update();
   }
   else
   {
      ExceptionRef e = new Exception("Invalid ConfigId");
      Exception::setLast(e, false);
   }
  
   return rval;
}

void ConfigManager::merge(Config& target, Config& source)
{
   if(source.isNull())
   {
      target = Config(NULL);
   }
   else if(!(source->getType() == String &&
      strcmp(source->getString(), DEFAULT_VALUE) == 0))
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
               merge(target[i->getName()], i->next());
            }
            break;
         }
         case Array:
         {
            target->setType(Array);
            ConfigIterator i = source.getIterator();
            for(int ii = 0; i->hasNext(); ii++)
            {
               merge(target[ii], i->next());
            }
            break;
         }
      }
   }
}

void ConfigManager::makeMergedConfig(Config& target, ConfigType types)
{
   lock();
   {
      for(vector<ConfigPair>::iterator i = mConfigs.begin();
          i != mConfigs.end(); i++)
      {
         if(!(*i).first.isNull())
         {
            if((types == All) || (types == (*i).second))
            {
               merge(target, (*i).first);
            }
         }
      }
   }
   unlock();
}

void ConfigManager::update()
{
   lock();
   {
      mConfig->clear();
      makeMergedConfig(mConfig, All);
   }
   unlock();
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

void ConfigManager::getChanges(
   Config& target, ConfigType baseType, bool addVersion)
{
   Config original;
   makeMergedConfig(original, baseType);
   diff(target, original, mConfig);
   // add first version added with addVersion if present
   if(mVersions->length() > 0)
   {
      target[VERSION] = mVersions[0];
   }
}

bool ConfigManager::isValidConfig(Config& config, Config& schema)
{
   bool rval = false;
   
   if(schema.isNull())
   {
      // schema not specified, any config value is ok
      rval = true;
   }
   else if(!config.isNull() && schema->getType() == config->getType())
   {
      // schema not null and types match, do deep compare
      switch(schema->getType())
      {
         case String:
         case Boolean:
         case Int32:
         case UInt32:
         case Int64:
         case UInt64:
         case Double:
            // simple types match
            rval = true;
            break;
         case Map:
         {
            // Compare all schema keys
            ConfigIterator i = schema.getIterator();
            // assume true and verify
            rval = true;
            while(rval && i->hasNext())
            {
               Config next = i->next();
               const char* name = i->getName();
               if(!config->hasMember(name))
               {
                  // key not in config, fail
                  ExceptionRef e = new Exception(
                     "Missing key",
                     "db.config.InvalidConfig");
                  e->getDetails()["key"] = name;
                  Exception::setLast(e, false);
                  rval = false;
               }
               else
               {
                  // check values
                  rval = isValidConfig(config[name], next);
               }
            }
            break;
         }
         case Array:
         {
            if(schema->length() == 0)
            {
               // allow any array values
               rval = true;
            }
            else if(schema->length() == 1)
            {
               // all config elements must match template 
               ConfigIterator i = config.getIterator();
               rval = true;
               while(rval && i->hasNext())
               {
                  rval = isValidConfig(i->next(), schema[0]);
               }
            }
            else
            {
               // multiple schema elements not allowed
               ExceptionRef e =
                  new Exception(
                     "Multiple Array schema values not allowed",
                     "db.config.InvalidConfig");
               Exception::setLast(e, false);
            }
            break;
         }
      }
   }
   else if(schema->getType() != config->getType())
   {
      // key not in config, fail
      ExceptionRef e = new Exception(
         "Type mismatch.",
         "db.config.InvalidConfig");
      e->getDetails()["schema"] =
         DynamicObject::descriptionForType(schema->getType());
      e->getDetails()["config"] =
         DynamicObject::descriptionForType(config->getType());
      Exception::setLast(e, false);
   }
   
   return rval;
}

void ConfigManager::addVersion(const char* version)
{
   mVersions->append() = version;
   DB_CAT_DEBUG(DB_CONFIG_CAT,
      "Add version: \"%s\"", (version != NULL) ? version : "(none)");
}

DynamicObject& ConfigManager::getVersions()
{
   return mVersions;
}
