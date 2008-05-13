/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/config/ConfigManager.h"

#include "db/io/BufferedOutputStream.h"
#include "db/data/json/JsonWriter.h"

using namespace std;
using namespace db::config;
using namespace db::data;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;

const char* ConfigManager::DEFAULT_VALUE = "__default__";

ConfigManager::ConfigManager()
{
   mConfig->setType(Map);
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

bool ConfigManager::addConfig(Config& config, ConfigType type, ConfigId* id)
{
   bool rval = true;
   
   lock();
   {
      mConfigs.push_back(ConfigPair(config, type));
      if(id != NULL)
      {
         *id = mConfigs.size() - 1;
      }
      update();
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
               ConfigIterator i = source.getIterator();
               while(i->hasNext())
               {
                  Config next = i->next();
                  merge(target[i->getName()], next);
               }
            }
            break;
         case Array:
            {
               ConfigIterator i = source.getIterator();
               for(int ii = 0; i->hasNext(); ii++)
               {
                  merge(target[ii], i->next());
               }
            }
            break;
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
            }
            break;
      }
   }
   
   return rval;
}

void ConfigManager::getChanges(Config& target, ConfigType baseType)
{
   Config original;
   makeMergedConfig(original, baseType);
   diff(target, original, mConfig);
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
                     rval = false;
                  }
                  else
                  {
                     // check values
                     rval = isValidConfig(config[name], next);
                  }
               }
            }
            break;
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
                     new Exception("Multiple Array schema values not allowed");
                  Exception::setLast(e, false);
               }
            }
            break;
      }
   }
   
   return rval;
}
