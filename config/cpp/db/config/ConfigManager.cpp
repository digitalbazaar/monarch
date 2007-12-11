/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/config/ConfigManager.h"
#include "db/io/BufferedOutputStream.h"
#include "db/data/json/JsonWriter.h"
#include "db/util/DynamicObjectIterator.h"

using namespace std;
using namespace db::config;
using namespace db::data;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

const char* ConfigManager::DEFAULT_VALUE = "__default__";

ConfigManager::ConfigManager()
{
   mConfig->setType(Map);
}

ConfigManager::~ConfigManager()
{
}

DynamicObject& ConfigManager::getConfig()
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

bool ConfigManager::addConfig(DynamicObject& dyno, ConfigType type,
   ConfigId *id)
{
   bool rval = true;

   lock();
   {
      mConfigs.push_back(ConfigPair(dyno, type));
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
         mConfigs[id] = ConfigPair(DynamicObject(NULL), None);
         update();
         rval = true;
      }
      else
      {
         Exception* e = new Exception("Invalid ConfigId");
         Exception::setLast(e);
      }
   }
   unlock();
   
   return rval;
}

bool ConfigManager::getConfig(ConfigId id, DynamicObject& dyno)
{
   bool rval = false;
   
   if(id >= 0 && id < mConfigs.size())
   {
      rval = true;
      dyno = mConfigs[id].first;
   }
   else
   {
      Exception* e = new Exception("Invalid ConfigId");
      Exception::setLast(e);
   }
  
   return rval;
}

bool ConfigManager::setConfig(ConfigId id, DynamicObject& dyno)
{
   bool rval = false;
   
   if(id >= 0 && id < mConfigs.size())
   {
      rval = true;
      mConfigs[id].first = dyno;
      update();
   }
   else
   {
      Exception* e = new Exception("Invalid ConfigId");
      Exception::setLast(e);
   }
  
   return rval;
}

void ConfigManager::merge(DynamicObject& target, DynamicObject& source)
{
   if(source == NULL)
   {
      target = DynamicObject(NULL);
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
               DynamicObjectIterator i = source.getIterator();
               while(i->hasNext())
               {
                  DynamicObject next = i->next();
                  merge(target[i->getName()], next);
               }
            }
            break;
         case Array:
            {
               DynamicObjectIterator i = source.getIterator();
               for(int ii = 0; i->hasNext(); ii++)
               {
                  merge(target[ii], i->next());
               }
            }
            break;
      }
   }
}

void ConfigManager::makeMergedConfig(DynamicObject& target, ConfigType types)
{
   lock();
   {
      for(vector<ConfigPair>::iterator i = mConfigs.begin();
         i != mConfigs.end();
         i++)
      {
         if((*i).first != NULL)
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

bool ConfigManager::diff(DynamicObject& target,
   DynamicObject& dyno1, DynamicObject& dyno2)
{
   bool rval = false;
   
   if(dyno1 == NULL && dyno2 == NULL)
   {
      // same: no diff
   }
   else if(dyno1 != NULL && dyno2 == NULL)
   {
      // <stuff> -> NULL: diff=NULL
      rval = true;
      target = DynamicObject(NULL);
   }
   else if((dyno1 == NULL && dyno2 != NULL) ||
      (dyno1->getType() != dyno2->getType()))
   {
      // NULL -> <stuff> -or- types differ: diff=dyno2
      rval = true;
      target = dyno2.clone();
   }
   else
   {
      // not null && same type: diff=deep compare
      switch(dyno1->getType())
      {
         case String:
         case Boolean:
         case Int32:
         case UInt32:
         case Int64:
         case UInt64:
         case Double:
            // compare simple types directly
            if(dyno1 != dyno2)
            {
               // changed: diff=dyno2
               rval = true;
               target = dyno2.clone();
            }
            break;
         case Map:
            {
               // Compare dyno2 keys since we are only concerned with
               // additions and updates, not removals.
               DynamicObjectIterator i = dyno2.getIterator();
               while(i->hasNext())
               {
                  DynamicObject next = i->next();
                  const char* name = i->getName();
                  if(!dyno1->hasMember(name))
                  {
                     // key not in dyno1, add to diff.
                     rval = true;
                     target[name] = next.clone();
                  }
                  else
                  {
                     // recusively get sub-diff
                     DynamicObject d;
                     if(diff(d, dyno1[name], next))
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
               // Compare dyno2 indexes since we are only concerned with
               // additions and updates, not removals.
               DynamicObjectIterator i = dyno2.getIterator();
               for(int ii = 0; i->hasNext(); ii++)
               {
                  DynamicObject next = i->next();
                  DynamicObject d;
                  if(diff(d, dyno1[ii], next))
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

void ConfigManager::getChanges(DynamicObject& target, ConfigType baseType)
{
   DynamicObject original;
   makeMergedConfig(original, baseType);
   diff(target, original, mConfig);
}

bool ConfigManager::isValidConfig(
   db::util::DynamicObject& config, db::util::DynamicObject& schema)
{
   bool rval = false;
   
   if(schema == NULL)
   {
      // schema not specified, any config value is ok
      rval = true;
   }
   else if(config != NULL && schema->getType() == config->getType())
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
               DynamicObjectIterator i = schema.getIterator();
               // assume true and verify
               rval = true;
               while(rval && i->hasNext())
               {
                  DynamicObject next = i->next();
                  const char* name = i->getName();
                  if(!config->hasMember(name))
                  {
                     // key not in config, fail
                     rval = false;
                  }
                  else
                  {
                     // check values
                     DynamicObject d;
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
                  DynamicObjectIterator i = config.getIterator();
                  rval = true;
                  while(rval && i->hasNext())
                  {
                     rval = isValidConfig(i->next(), schema[0]); 
                  }
               }
               else
               {
                  // multiple schema elements not allowed
                  Exception* e =
                     new Exception("Multiple Array schema values not allowed");
                  Exception::setLast(e);
               }
            }
            break;
      }
   }
   
   return rval;
}
