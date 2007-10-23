/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/ModuleLibrary.h"

using namespace std;
using namespace db::modest;
using namespace db::rt;

ModuleLibrary::ModuleLibrary(Kernel* k)
{
   mKernel = k;
}

ModuleLibrary::~ModuleLibrary()
{
   // unload all modules
   ModuleLibrary::unloadAllModules();
}

Module* ModuleLibrary::findModule(const char* name)
{
   Module* rval = NULL;
   
   // find module
   ModuleMap::iterator i = mModules.find(name);
   if(i != mModules.end())
   {
      rval = i->second->module;
   }
   
   return rval;
}

Module* ModuleLibrary::loadModule(const char* filename)
{
   Module* rval = NULL;
   
   lock();
   {
      // try to load module
      ModuleInfo* mi = mLoader.loadModule(filename);
      if(mi != NULL)
      {
         // ensure the module isn't already loaded
         if(findModule(mi->module->getId().name) == NULL)
         {
            // initialize the module
            Exception* e = mi->module->initialize(mKernel);
            if(e == NULL)
            {
               // add Module to the map and list
               mModules[mi->module->getId().name] = mi;
               mLoadOrder.push_back(mi->module->getId().name);
               rval = mi->module;
            }
            else
            {
               // could not initialize module, so unload it
               string msg;
               msg.append("Could not initialize module '");
               msg.append(filename);
               msg.append("', module named '");
               msg.append(mi->module->getId().name);
               msg.append("', version '");
               msg.append(mi->module->getId().version);
               msg.append("',exception=");
               msg.append(e->getMessage());
               msg.append(1, ':');
               msg.append(e->getType());
               msg.append(1, ':');
               char temp[20];
               sprintf(temp, "%i", e->getCode());
               msg.append(temp);
               Exception::setLast(new Exception(msg.c_str()));
               mLoader.unloadModule(mi);
            }
         }
         else
         {
            // module is already loaded, set exception and unload it
            string msg;
            msg.append("Could not load module '");
            msg.append(filename);
            msg.append("', another module named '");
            msg.append(mi->module->getId().name);
            msg.append("' with version '");
            msg.append(mi->module->getId().version);
            msg.append("' is already loaded.");
            Exception::setLast(new Exception(msg.c_str()));
            mLoader.unloadModule(mi);
         }
      }
   }
   unlock();
   
   return rval;
}

void ModuleLibrary::unloadModule(const char* name)
{
   lock();
   {
      // find module
      ModuleMap::iterator i = mModules.find(name);
      if(i != mModules.end())
      {
         // get module
         ModuleInfo* mi = i->second;
         
         // erase module from map and list
         mModules.erase(i);
         for(list<const char*>::iterator li = mLoadOrder.begin();
             li != mLoadOrder.end(); li++)
         {
            if(strcmp(*li, name) == 0)
            {
               mLoadOrder.erase(li);
               break;
            }
         }
         
         // clean up and unload module
         mi->module->cleanup(mKernel);
         mLoader.unloadModule(mi);
      }
   }
   unlock();
}

void ModuleLibrary::unloadAllModules()
{
   lock();
   {
      // clean up and free every module
      while(!mLoadOrder.empty())
      {
         // find ModuleInfo
         ModuleMap::iterator i = mModules.find(mLoadOrder.back());
         ModuleInfo* mi = i->second;
         
         // remove module from map and list
         mModules.erase(i);
         mLoadOrder.pop_back();
         
         // clean up and unload module
         mi->module->cleanup(mKernel);
         mLoader.unloadModule(mi);
      }
   }
   unlock();
}

Module* ModuleLibrary::getModule(const char* name)
{
   Module* rval = NULL;
   
   lock();
   {
      // find Module
      rval = findModule(name);
   }
   unlock();
   
   return rval;
}

const ModuleId* ModuleLibrary::getModuleId(const char* name)
{
   const ModuleId* rval = NULL;
   
   lock();
   {
      // find Module
      Module* m = findModule(name);
      if(m != NULL)
      {
         rval = &m->getId();
      }
   }
   unlock();
   
   return rval;
}

ModuleInterface* ModuleLibrary::getModuleInterface(const char* name)
{
   ModuleInterface* rval = NULL;
   
   lock();
   {
      // find Module
      Module* m = findModule(name);
      if(m != NULL)
      {
         rval = m->getInterface();
      }
   }
   unlock();
   
   return rval;
}
