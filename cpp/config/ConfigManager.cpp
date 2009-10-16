/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/config/ConfigManager.h"

#include <algorithm>
#include <vector>

#include "db/config/ConfigChangeListener.h"
#include "db/data/json/JsonReader.h"
#include "db/data/TemplateInputStream.h"
#include "db/io/BufferedOutputStream.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/io/ByteArrayOutputStream.h"
#include "db/io/File.h"
#include "db/io/FileList.h"
#include "db/io/FileInputStream.h"
#include "db/logging/Logging.h"
#include "db/util/StringTools.h"

using namespace std;
using namespace db::config;
using namespace db::data;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

#define CONFIG_EXCEPTION   "db.config.ConfigManager"

const char* ConfigManager::DEFAULT_VALUE = "_default_";
const char* ConfigManager::VERSION       = "_version_";
const char* ConfigManager::ID            = "_id_";
// FIXME: change to GROUPS and have support for multiple groups per config?
// seems like this feature would be simple enough to add without complicating
// too much
const char* ConfigManager::GROUP       = "_group_";
const char* ConfigManager::PARENT      = "_parent_";
const char* ConfigManager::MERGE       = "_merge_";
const char* ConfigManager::APPEND      = "_append_";
const char* ConfigManager::REMOVE      = "_remove_";
const char* ConfigManager::INCLUDE     = "_include_";
const char* ConfigManager::INCLUDE_EXT = ".config";
const char* ConfigManager::TMP         = "_tmp_";

ConfigManager::ConfigManager() :
   mConfigChangeListener(NULL)
{
   // initialize internal data structures
   mVersions->setType(Map);
   mKeywordMap->setType(Map);
   //addVersion(DB_DEFAULT_CONFIG_VERSION);
   mConfigs->setType(Map);
}

ConfigManager::~ConfigManager()
{
}

DynamicObject ConfigManager::getDebugInfo()
{
   DynamicObject debug;

   // read lock to clone configs and versions
   mLock.lockShared();
   {
      debug["configs"] = mConfigs.clone();
      debug["versions"] = mVersions.clone();
   }
   mLock.unlockShared();

   return debug;
}

void ConfigManager::clear()
{
   mLock.lockExclusive();
   mConfigs->clear();
   mLock.unlockExclusive();

   // notify listener of configuration change
   ConfigChangeListener* listener = getConfigChangeListener();
   if(listener != NULL)
   {
      listener->configCleared(this);
   }
}

bool ConfigManager::addConfig(Config& config, bool include, const char* dir)
{
   bool rval;

   // keep track of changed IDs
   DynamicObject changedIds;
   changedIds->setType(Map);
   rval = recursiveAddConfig(config, include, dir, &changedIds);

   if(rval)
   {
      ConfigChangeListener* listener = getConfigChangeListener();
      if(listener != NULL)
      {
         // notify listener of configuration addition
         listener->configAdded(this, config[ID]->getString());

         // FIXME: why aren't we locking while producing merged diffs?
         // if a config is removed (like a user config is removed in bitmunk)
         // we could end up with config corruption or a segfault, right?

         // produce merged diffs
         produceMergedDiffs(changedIds);

         // notify listener of all related configuration changes
         DynamicObjectIterator i = changedIds.getIterator();
         while(i->hasNext())
         {
            Config& d = i->next();
            listener->configChanged(this, i->getName(), d);
         }
      }
   }

   return rval;
}

bool ConfigManager::addConfigFile(
   const char* path, bool include, const char* dir,
   bool optional, bool includeSubdirectories)
{
   bool rval = true;

   string userPath;
   rval = File::expandUser(path, userPath);
   if(rval)
   {
      string fullPath;

      // if dir set and expanded user dir not absolute, build a full path
      if(dir && !File::isPathAbsolute(userPath.c_str()))
      {
         fullPath.assign(File::join(dir, userPath.c_str()));
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
               CONFIG_EXCEPTION ".FileNotFound");
            e->getDetails()["path"] = path;
            Exception::set(e);
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
            // include path to config (necessary for CURRENT_DIR replacement)
            string dirname = File::dirname(fullPath.c_str());
            rval = addConfig(cfg, include, dirname.c_str());
         }

         if(!rval)
         {
            ExceptionRef e = new Exception(
               "Configuration file load failure.",
               CONFIG_EXCEPTION ".ConfigFileError");
            e->getDetails()["path"] = path;
            Exception::push(e);
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
            string name = f->getAbsolutePath();
            if(f->isFile())
            {
               if(name.rfind(INCLUDE_EXT) ==
                  (name.length() - strlen(INCLUDE_EXT)))
               {
                  configFiles.push_back(File::basename(f->getAbsolutePath()));
               }
            }
            else if(
               includeSubdirectories &&
               strcmp(name.c_str(), ".") != 0 &&
               strcmp(name.c_str(), "..") != 0 &&
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
               (*i).c_str(), include, file->getAbsolutePath(), false);
         }

         // load each dir in order
         for(vector<string>::iterator i = configDirs.begin();
             rval && i != configDirs.end(); i++)
         {
            const char* dir = (*i).c_str();
            rval = addConfigFile(dir, include, dir, false);
         }
      }
      else
      {
         ExceptionRef e = new Exception(
            "Unknown configuration file type.",
            CONFIG_EXCEPTION ".FileNotFound");
         Exception::set(e);
         rval = false;
      }
   }

   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Invalid config file.",
         CONFIG_EXCEPTION ".InvalidConfigFile");
      e->getDetails()["path"] = path;
      if(dir != NULL)
      {
         e->getDetails()["dir"] = dir;
      }
      Exception::push(e);
   }

   return rval;
}

bool ConfigManager::removeConfig(ConfigId id)
{
   bool rval = false;

   // keep track of changed IDs
   DynamicObject changedIds;
   changedIds->setType(Map);

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

         // get raw config
         Config& raw = mConfigs[id]["raw"];

         // remove self from parent's children
         if(raw->hasMember(PARENT))
         {
            ConfigId parentId = raw[PARENT]->getString();
            Config& parent = mConfigs[parentId];
            ConfigIterator i = parent["children"].getIterator();
            while(i->hasNext())
            {
               Config& child = i->next();
               if(strcmp(child->getString(), id) == 0)
               {
                  i->remove();
                  break;
               }
            }
         }

         // build list of all related config IDs
         DynamicObject configIds;
         configIds->setType(Array);

         // add group if it has more members
         if(raw->hasMember(GROUP))
         {
            ConfigId groupId = raw[GROUP]->getString();
            Config& group = mConfigs[groupId];
            if(group["members"]->length() > 0)
            {
               // remove member from group
               ConfigIterator i = group["members"].getIterator();
               while(i->hasNext())
               {
                  Config& member = i->next();
                  if(strcmp(member->getString(), id) == 0)
                  {
                     i->remove();
                     break;
                  }
               }

               // group needs update
               configIds->append(raw[GROUP]);
            }
            else
            {
               // remove group, no more members
               mConfigs->removeMember(groupId);
            }
         }

         // add children
         configIds.merge(mConfigs[id]["children"], true);

         // remove config
         mConfigs->removeMember(id);

         // update all related configs
         DynamicObjectIterator i = configIds.getIterator();
         while(i->hasNext())
         {
            update(i->next()->getString(), &changedIds);
         }
      }
      else
      {
         ExceptionRef e = new Exception(
            "Could not remove config. Invalid config ID.",
            CONFIG_EXCEPTION ".InvalidId");
         e->getDetails()["id"] = id;
         Exception::set(e);
      }
   }
   mLock.unlockExclusive();

   if(rval)
   {
      ConfigChangeListener* listener = getConfigChangeListener();
      if(listener != NULL)
      {
         // notify listener of configuration removal
         listener->configRemoved(this, id);

         // produce merged diffs for each changed ID
         produceMergedDiffs(changedIds);

         // notify listener of all related configuration changes
         DynamicObjectIterator i = changedIds.getIterator();
         while(i->hasNext())
         {
            Config& d = i->next();
            listener->configChanged(this, i->getName(), d);
         }
      }
   }

   return rval;
}

bool ConfigManager::setConfig(Config& config)
{
   bool rval = false;

   // get config ID, store all changed IDs
   ConfigId id = config[ID]->getString();
   DynamicObject changedIds;
   changedIds->setType(Map);

   // lock to modify internal storage
   mLock.lockExclusive();
   {
      // ensure the ID exists
      if(!mConfigs->hasMember(id))
      {
         ExceptionRef e = new Exception(
            "Could not set config. Invalid config ID.",
            CONFIG_EXCEPTION ".InvalidId");
         e->getDetails()["id"] = id;
         Exception::set(e);
      }
      // ensure the group ID hasn't changed
      else if(
         (!mConfigs[id]["raw"]->hasMember(GROUP) &&
          config->hasMember(GROUP)) ||
         (mConfigs[id]["raw"]->hasMember(GROUP) &&
          strcmp(config[GROUP]->getString(),
                 mConfigs[id]["raw"][GROUP]->getString()) != 0))
      {
         ExceptionRef e = new Exception(
            "Could not set config. Group changed.",
            CONFIG_EXCEPTION ".ConfigConflict");
         e->getDetails()["id"] = id;
         Exception::set(e);
      }
      // ensure the parent ID hasn't changed
      else if(
         (!mConfigs[id]["raw"]->hasMember(PARENT) &&
          config->hasMember(PARENT)) ||
         (mConfigs[id]["raw"]->hasMember(PARENT) &&
          strcmp(config[PARENT]->getString(),
                 mConfigs[id]["raw"][PARENT]->getString()) != 0))
      {
         ExceptionRef e = new Exception(
            "Could not set config. Parent changed.",
            CONFIG_EXCEPTION ".ConfigConflict");
         e->getDetails()["id"] = id;
         Exception::set(e);
      }
      else
      {
         // only include changed ID if this config's merged config has
         // been generated before (indicating that someone could be
         // watching for changes)
         if(mConfigs[id]->hasMember("merged"))
         {
            changedIds[id] = getMergedConfig(id, false);
         }
         mConfigs[id]["raw"] = config;
         update(id, &changedIds);
         rval = true;
      }
   }
   mLock.unlockExclusive();

   if(rval)
   {
      // notify listener of all configuration changes
      ConfigChangeListener* listener = getConfigChangeListener();
      if(listener != NULL)
      {
         // produce merged diffs for each changed ID
         produceMergedDiffs(changedIds);

         DynamicObjectIterator i = changedIds.getIterator();
         while(i->hasNext())
         {
            Config& d = i->next();
            listener->configChanged(this, i->getName(), d);
         }
      }
   }

   return rval;
}

Config ConfigManager::getConfig(ConfigId id, bool raw, bool cache)
{
   Config rval(NULL);

   mLock.lockShared();
   if(mConfigs->hasMember(id))
   {
      rval = (raw ? mConfigs[id]["raw"].clone() : getMergedConfig(id, cache));

      // implicit config groups do not have any raw configs, so do not
      // return any here
      if(raw && rval->hasMember(GROUP) &&
         strcmp(id, rval[GROUP]->getString()) == 0)
      {
         ExceptionRef e = new Exception(
            "Request for raw config with a group ID.",
            CONFIG_EXCEPTION ".InvalidId");
         e->getDetails()["id"] = id;
         Exception::set(e);
         rval.setNull();
      }

      mLock.unlockShared();
   }
   else
   {
      mLock.unlockShared();
      ExceptionRef e = new Exception(
         "Could not get config. Invalid config ID.",
         CONFIG_EXCEPTION ".InvalidId");
      e->getDetails()["id"] = id;
      Exception::set(e);
   }

   return rval;
}

bool ConfigManager::hasConfig(ConfigId id)
{
   return mConfigs->hasMember(id);
}

void ConfigManager::update(ConfigId id, DynamicObject* changedIds)
{
   // lock to modify internal storage
   mLock.lockExclusive();
   {
      // get config
      Config& config = mConfigs[id];

      if(changedIds != NULL)
      {
         /* Note: Here we only want to report changes for configs that have
          * merged data. If a config does not have any merged data, then its
          * values have not been used yet and so any changes are irrelevant
          * and we can optimize them out. This is particularly useful during
          * initialization of a config system.
          *
          * We also assume here that if the current config we are updating
          * does not have a merged config, then it is necessary that none of
          * its children can have a merged config either (as to generate a
          * child's merged config, the parent merged config must be generated).
          *
          * Therefore, we only save current merged configs for children that
          * have them. We will use these configs later to produce a diff for
          * config change listeners.
          */
         if(config->hasMember("merged"))
         {
            DynamicObjectIterator i = config["children"].getIterator();
            while(i->hasNext())
            {
               ConfigId nextId = i->next()->getString();
               if(!(*changedIds)->hasMember(nextId) &&
                  mConfigs->hasMember(nextId))
               {
                  Config& cfg = mConfigs[nextId];
                  if(cfg->hasMember("merged"))
                  {
                     // save a reference to the old merged config
                     (*changedIds)[nextId] = mConfigs[nextId]["merged"];
                  }
               }
            }
         }
      }

      // if updating a group, recombine members to rebuild RAW config
      if(config->hasMember("members"))
      {
         // clear old raw config
         Config& raw = config["raw"];
         if(raw->hasMember(MERGE))
         {
            raw[MERGE]->clear();
         }
         if(raw->hasMember(APPEND))
         {
            raw[APPEND]->clear();
         }
         if(raw->hasMember(REMOVE))
         {
            raw[REMOVE]->clear();
         }

         // merge raw configs together
         ConfigIterator i = config["members"].getIterator();
         while(i->hasNext())
         {
            Config& memberId = i->next();
            Config& member = mConfigs[memberId->getString()]["raw"];

            // merge the merge property (do not append)
            if(member->hasMember(MERGE))
            {
               merge(raw[MERGE], member[MERGE], false);
            }

            // aggregate append properties
            if(member->hasMember(APPEND))
            {
               merge(raw[APPEND], member[APPEND], true);
            }

            // aggregate remove properties
            if(member->hasMember(REMOVE))
            {
               merge(raw[REMOVE], member[REMOVE], true);
            }
         }
      }

      // wipe old merged config for config ID and replace it with a new one
      // we only do this if there was an existing merged config, otherwise
      // we don't bother as its correct merged config will be lazily created
      // only once a user requests the config
      if(config->hasMember("merged"))
      {
         config->removeMember("merged");
         makeMergedConfig(id, NULL);
      }

      // update group config (if not already the group)
      if(config["raw"]->hasMember(GROUP) &&
         strcmp(id, config["raw"][GROUP]->getString()) != 0)
      {
         update(config["raw"][GROUP]->getString(), changedIds);
      }

      // update each child of config ID
      DynamicObjectIterator i = config["children"].getIterator();
      while(i->hasNext())
      {
         update(i->next()->getString(), changedIds);
      }
   }
   mLock.unlockExclusive();
}

void ConfigManager::setKeyword(const char* keyword, const char* value)
{
   // lock to modify internal storage
   mLock.lockExclusive();
   {
      mKeywordMap[keyword] = value;
   }
   mLock.unlockExclusive();
}

void ConfigManager::addVersion(const char* version)
{
   // lock to modify internal storage
   mLock.lockExclusive();
   {
      mVersions[version] = true;
      DB_CAT_DEBUG(DB_CONFIG_CAT,
         "Added version: \"%s\"", (version != NULL) ? version : "(none)");
   }
   mLock.unlockExclusive();
}

DynamicObject& ConfigManager::getVersions()
{
   return mVersions;
}

void ConfigManager::setConfigChangeListener(ConfigChangeListener* listener)
{
   mLock.lockExclusive();
   mConfigChangeListener = listener;
   mLock.unlockExclusive();
}

ConfigChangeListener* ConfigManager::getConfigChangeListener()
{
   ConfigChangeListener* rval = NULL;

   mLock.lockShared();
   rval = mConfigChangeListener;
   mLock.unlockShared();

   return rval;
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
            // FIXME: only want to "append" if node is a leaf?
            target->setType(Array);
            int ii = (append ? target->length() : 0);
            ConfigIterator i = source.getIterator();
            for(; i->hasNext(); ii++)
            {
               merge(target[ii], i->next(), append);
            }
            break;
         }
      }
   }
}

/**
 * A helper method that removes the config values from one config
 * from another config.
 *
 * @param target the target config to update.
 * @param remove the config with entries to remove.
 */
static void _removeLeafNodes(Config& target, Config& remove)
{
   // for each config entry, remove leaf nodes from parent config
   ConfigIterator i = remove.getIterator();
   while(i->hasNext())
   {
      Config& next = i->next();

      // proceed if value is in parent configuration
      if(target->hasMember(i->getName()))
      {
         // FIXME: need a method to remove a single element from an array
         // also -- this currently will not be able to differentiate
         // between removing "index" X and removing value "Y" from an array
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
               _removeLeafNodes(target[i->getName()], next);
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

void ConfigManager::makeMergedConfig(ConfigId id, Config* out)
{
   // only need to do work if merged config doesn't already exist
   Config& config = mConfigs[id];
   if(!config->hasMember("merged"))
   {
      // produce a merged configuration that contains only config values, not
      // any "_special_" config format values
      Config merged(NULL);

      // get raw configuration
      Config& raw = config["raw"];

      // get merged config from parent
      if(raw->hasMember(PARENT))
      {
         ConfigId parent = raw[PARENT]->getString();
         if(mConfigs[parent]->hasMember("merged"))
         {
            // parent already cached, so just clone it
            merged = mConfigs[parent]["merged"].clone();
         }
         else if(out == NULL)
         {
            // caching is on, so generate and cache parent config
            makeMergedConfig(parent, NULL);
            merged = mConfigs[parent]["merged"].clone();
         }
         else
         {
            // caching is off and parent config not yet cached, so
            // generate parent config and store it in "merged"
            makeMergedConfig(parent, &merged);
         }

         // remove appropriate entries from parent config
         if(raw->hasMember(REMOVE))
         {
            _removeLeafNodes(merged, raw[REMOVE]);
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
         else
         {
            // empty merged config
            merged = Config();
            merged->setType(Map);
         }
      }

      if(out == NULL)
      {
         // caching is on, set merged config
         config["merged"] = merged;
      }
      else
      {
         // caching is off, return generated config
         *out = merged;
      }
   }
   else if(out != NULL)
   {
      // return cached merged config
      *out = config["merged"];
   }
}

Config ConfigManager::getMergedConfig(ConfigId id, bool cache)
{
   Config rval(NULL);

   if(mConfigs->hasMember(id))
   {
      if(!mConfigs[id]->hasMember("merged"))
      {
         // lazily produce the merged config
         if(cache)
         {
            // generate and cache merged config
            makeMergedConfig(id, NULL);
            rval = mConfigs[id]["merged"];
         }
         else
         {
            // generate merged config but do not cache it
            makeMergedConfig(id, &rval);
         }
      }
      else
      {
         // use cached merged config
         rval = mConfigs[id]["merged"];
      }
   }

   return rval;
}

struct _replaceKeywordsState_s
{
   ByteArrayInputStream* bais;
   TemplateInputStream* tis;
   ByteBuffer* output;
   ByteArrayOutputStream* baos;
};

static void _replaceKeywords(
   Config& config, DynamicObject& keywordMap,
   struct _replaceKeywordsState_s* state)
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
            // setup template processing chain
            state->bais->setByteArray(config->getString(), config->length());
            state->tis->setVariables(keywordMap, true);
            state->output->clear();
            // reset input stream and parsing state
            state->tis->setInputStream(state->bais);
            state->tis->parse(state->baos);
            state->output->putByte(0, 1, true);
            // set new string
            config = state->output->data();
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
               _replaceKeywords(i->next(), keywordMap, state);
            }
            break;
         }
      }
   }
}

void ConfigManager::replaceKeywords(Config& config, DynamicObject& keywordMap)
{
   if(!config.isNull())
   {
      // only process includes and non-meta config info
      const char* keys[] = {INCLUDE, MERGE, APPEND, REMOVE, NULL};
      // only create state if needed
      struct _replaceKeywordsState_s* state = NULL;
      for(int i = 0; keys[i] != NULL; i++)
      {
         if(config->hasMember(keys[i]))
         {
            state = new struct _replaceKeywordsState_s;
            state->bais = new ByteArrayInputStream(NULL, 0);
            state->tis = new TemplateInputStream(state->bais, false);
            state->output = new ByteBuffer(2048);
            state->baos = new ByteArrayOutputStream(state->output, true);
            // only create once
            break;
         }
      }
      // replace keywords
      for(int i = 0; keys[i] != NULL; i++)
      {
         const char* key = keys[i];
         if(config->hasMember(key))
         {
            _replaceKeywords(config[key], keywordMap, state);
         }
      }
      if(state != NULL)
      {
         delete state->baos;
         delete state->output;
         delete state->tis;
         delete state->bais;
         delete state;
      }
   }
}

bool ConfigManager::diff(
   Config& target, Config& config1, Config& config2, int level)
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
                  // ignore ID, APPEND, and REMOVE properties
                  if(level != 0 ||
                     (strcmp(name, ID) != 0 &&
                      strcmp(name, APPEND) != 0 &&
                      strcmp(name, REMOVE) != 0))
                  {
                     if(!config1->hasMember(name))
                     {
                        // ensure VERSION, PARENT, and GROUP exist in both
                        if(level == 0 &&
                           (strcmp(name, VERSION) == 0 ||
                            strcmp(name, PARENT) == 0 ||
                            strcmp(name, GROUP) == 0))
                        {
                           // special property not in config1, so add to diff
                           rval = true;
                           target[name] = next.clone();
                        }
                     }
                     else
                     {
                        // recusively get sub-diff
                        Config d;
                        if(diff(d, config1[name], next, level + 1))
                        {
                           // diff found, add it
                           rval = true;
                           target[name] = d;
                        }
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
            Config temp;
            temp->setType(Array);
            ConfigIterator i = config2.getIterator();
            for(int ii = 0; i->hasNext(); ii++)
            {
               DynamicObject next = i->next();
               Config d;
               if(diff(d, config1[ii], next, level + 1))
               {
                  // diff found
                  rval = true;
                  temp[ii] = d;
               }
               else
               {
                  // set keyword value
                  temp[ii] = DEFAULT_VALUE;
               }
            }

            // only set array to target if a diff was found
            if(rval)
            {
               target = temp;
            }

            break;
         }
      }
   }

   return rval;
}

bool ConfigManager::checkConflicts(
   ConfigId id, Config& existing, Config& config, bool isGroup)
{
   bool rval = true;

   // calculate the conflict-diff between existing and config
   Config d;
   diff(d, existing, config, 0);

   // check for parent, group, or merge conflicts
   // version check done elsewhere
   if(d->hasMember(PARENT) ||
      d->hasMember(GROUP) ||
      d->hasMember(MERGE))
   {
      ExceptionRef e = new Exception(
         "Config conflict. Parent, group, or merge field differs for "
         "a particular config ID.",
         CONFIG_EXCEPTION ".ConfigConflict");
      e->getDetails()["configId"] = id;
      e->getDetails()["diff"] = d;
      e->getDetails()["isGroup"] = isGroup;
      Exception::set(e);
      rval = false;
   }

   return rval;
}

void ConfigManager::produceMergedDiffs(DynamicObject& changedIds)
{
   DynamicObject tmp = changedIds;
   changedIds = DynamicObject();
   changedIds->setType(Map);
   DynamicObjectIterator i = tmp.getIterator();
   while(i->hasNext())
   {
      Config& oldMerged = i->next();
      ConfigManager::ConfigId nextId = i->getName();
      if(mConfigs->hasMember(nextId))
      {
         // diff new merged config with the old one we saved a copy of
         Config d;
         Config newMerged = getMergedConfig(nextId, true);
         if(diff(d, oldMerged, newMerged))
         {
            changedIds[nextId] = d;
         }
      }
   }
}

/**
 * A helper method to insert a config. This method assumes there is no
 * existing config with the passed ID and that any parent in the config is
 * valid.
 *
 * @param id the config ID of the config to insert.
 * @param storage the config to use for storage.
 * @param raw the raw config to insert.
 */
static void _insertConfig(
   ConfigManager::ConfigId id, Config& storage, Config& raw)
{
   Config& c = storage[id];
   c["children"]->setType(Array);
   c["raw"] = raw;

   // if has parent
   if(raw->hasMember(ConfigManager::PARENT))
   {
      // update parent's children
      ConfigManager::ConfigId parent = raw[ConfigManager::PARENT]->getString();
      storage[parent]["children"]->append() = id;
   }
}

bool ConfigManager::recursiveAddConfig(
   Config& config, bool include, const char* dir, DynamicObject* changedIds)
{
   bool rval = true;

   // get config ID
   ConfigId id = "";
   if(!config.isNull() && config->hasMember(ID))
   {
      id = config[ID]->getString();
   }
   else
   {
      ExceptionRef e = new Exception(
         "No valid config ID found.",
         CONFIG_EXCEPTION ".MissingId");
      e->getDetails()["config"] = config;
      Exception::set(e);
      rval = false;
   }

   // ensure group ID doesn't match config ID
   if(rval && config->hasMember(GROUP) &&
      strcmp(id, config[GROUP]->getString()) == 0)
   {
      ExceptionRef e = new Exception(
         "Group ID cannot be the same as config ID.",
         CONFIG_EXCEPTION ".ConfigConflict");
      e->getDetails()["id"] = id;
      Exception::set(e);
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
                  "No config file version found.",
                  CONFIG_EXCEPTION ".UnspecifiedVersion");
               Exception::set(e);
               rval = false;
            }
            else
            {
               // check for known version
               const char* version = config[VERSION]->getString();
               if(!mVersions->hasMember(version))
               {
                  ExceptionRef e = new Exception(
                     "Unsupported config file version.",
                     CONFIG_EXCEPTION ".UnsupportedVersion");
                  e->getDetails()["version"] = version;
                  Exception::set(e);
                  rval = false;
               }
            }
         }

         // if has parent
         if(rval && config->hasMember(PARENT))
         {
            // ensure parent exists
            ConfigId parent = config[PARENT]->getString();
            if(!mConfigs->hasMember(parent))
            {
               ExceptionRef e = new Exception(
                  "Invalid parent configuration file ID.",
                  CONFIG_EXCEPTION ".InvalidParent");
               e->getDetails()["configId"] = id;
               e->getDetails()["parentId"] = parent;
               Exception::set(e);
               rval = false;
            }
         }
      }
      mLock.unlockShared();
   }

   // handle global keyword replacement
   if(rval)
   {
      // add special current directory keyword
      if(dir != NULL)
      {
         mLock.lockExclusive();
         mKeywordMap["CURRENT_DIR"] = dir;
         mLock.unlockExclusive();
      }

      // do keyword replacement (custom and special)
      mLock.lockShared();
      replaceKeywords(config, mKeywordMap);
      mLock.unlockShared();

      // remove special keywords
      if(dir != NULL)
      {
         mLock.lockExclusive();
         mKeywordMap->removeMember("CURRENT_DIR");
         mLock.unlockExclusive();
      }
   }

   // process includes
   if(rval && include && config->hasMember(INCLUDE))
   {
      if(config[INCLUDE]->getType() != Array)
      {
         ExceptionRef e = new Exception(
            "The include directive value must be an array.",
            CONFIG_EXCEPTION ".InvalidIncludeType");
         e->getDetails()["configId"] = id;
         e->getDetails()[INCLUDE] = config[INCLUDE];
         Exception::set(e);
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
            bool includeSubdirectories = false;
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
                     "The include path is missing.",
                     CONFIG_EXCEPTION ".MissingIncludePath");
                  e->getDetails()["configId"] = id;
                  e->getDetails()[INCLUDE] = config[INCLUDE];
                  Exception::set(e);
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
               if(next->hasMember("includeSubdirectories"))
               {
                  includeSubdirectories =
                     next["includeSubdirectories"]->getBoolean();
               }
            }
            else
            {
               ExceptionRef e = new Exception(
                  "The type of the include value is invalid.",
                  CONFIG_EXCEPTION ".InvalidIncludeType");
               e->getDetails()["configId"] = id;
               e->getDetails()[INCLUDE] = config[INCLUDE];
               Exception::set(e);
               rval = false;
            }

            // if load, then load the included config file
            if(rval && load)
            {
               DB_CAT_DEBUG(DB_CONFIG_CAT, "Loading include: %s", path);
               rval = addConfigFile(
                  path, true, dir, optional, includeSubdirectories);
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
         // get the group ID
         ConfigId groupId = "";
         bool group = false;
         if(config->hasMember(GROUP))
         {
            group = true;
            groupId = config[GROUP]->getString();
         }

         // if the config ID already exists, ensure there are no conflicts
         bool mergeConfig = false;
         if(mConfigs->hasMember(id))
         {
            mergeConfig = true;
            rval = checkConflicts(id, mConfigs[id]["raw"], config, false);
         }

         // if the group ID already exists, ensure there are no conflicts
         if(group && mConfigs->hasMember(groupId))
         {
            rval = checkConflicts(
               groupId, mConfigs[groupId]["raw"], config, true);
         }

         if(rval)
         {
            if(mergeConfig)
            {
               Config& raw = mConfigs[id]["raw"];

               // merge the merge property (do not append)
               if(raw->hasMember(MERGE) || config->hasMember(MERGE))
               {
                  merge(raw[MERGE], config[MERGE], false);
               }

               // aggregate append properties
               if(raw->hasMember(APPEND) || config->hasMember(APPEND))
               {
                  merge(raw[APPEND], config[APPEND], true);
               }

               // aggregate remove properties
               if(raw->hasMember(REMOVE) || config->hasMember(REMOVE))
               {
                  merge(raw[REMOVE], config[REMOVE], true);
               }
            }
            else
            {
               // insert config
               _insertConfig(id, mConfigs, config);
            }

            if(group)
            {
               // add group if it does not exist
               if(!mConfigs->hasMember(groupId))
               {
                  // insert blank group config, will be updated via update()
                  // Note: Only implicit groups can have the same group ID
                  // and config ID ... as their "raw" config is fake ... and
                  // cannot be retrieved outside of the config manager.
                  Config& groupConfig = mConfigs[groupId];
                  groupConfig["raw"][ID] = groupId;
                  groupConfig["raw"][GROUP] = groupId;
                  groupConfig["children"]->setType(Array);
                  if(config->hasMember(PARENT))
                  {
                     groupConfig["raw"][PARENT] = config[PARENT]->getString();
                  }
                  groupConfig["members"]->append() = id;
               }
               // add member to group if not already in group
               else
               {
                  bool add = true;
                  Config& groupConfig = mConfigs[groupId];
                  ConfigIterator i = groupConfig["members"].getIterator();
                  while(add && i->hasNext())
                  {
                     Config& member = i->next();
                     if(strcmp(member->getString(), id) == 0)
                     {
                        add = false;
                     }
                  }
                  if(add)
                  {
                     groupConfig["members"]->append() = id;
                  }
               }
            }
         }

         if(rval)
         {
            // only update related merged configs
            update(id, changedIds);
         }
      }
      mLock.unlockExclusive();
   }

   return rval;
}
